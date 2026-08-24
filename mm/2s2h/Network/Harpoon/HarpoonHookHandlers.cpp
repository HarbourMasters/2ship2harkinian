// =============================================================================
// HarpoonHookHandlers — game-side hooks for Harpoon multiplayer.
//
// Lives between the Harpoon singleton (network-only, no MM headers) and the
// MM engine (PlayState, Actor, Player). Registers GameInteractor hooks at
// startup and forwards state to/from the singleton each frame.
//
// Scope:
//   - Drain the Harpoon inbound queue once per frame.
//   - Send PLAYER.UPDATE_TRANSFORM/SKELETON/VISUAL_STATE for local Link.
//   - Send PLAYER.UPDATE_CUSTOM_ITEMS (NEI item visual state) when it changes.
//   - Keep one ACTOR_HARPOON_PEER alive per remote player in our scene.
//   - Apply incoming COMBAT.DEAL_DAMAGE / APPLY_STATUS to the local player.
// =============================================================================

#include "Harpoon.h"
#include "Combat/CombatSync.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <cstring>

extern "C" {
#include "z64.h"
#include "z64actor.h"
#include "z64player.h"
#include "z64save.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "mods/items/custom_items.h" // CustomItemVisualSync + Build/Apply
extern PlayState* gPlayState;
extern SaveContext gSaveContext;

// Player damage entry points. Both are non-static globals in z_player.c but have
// no public declaration, so declare them here.
//
// func_80833B18 is MM's equivalent of OoT's func_80837C0C: one call gives damage
// (from colChkInfo.damage), i-frames, the right reaction animation, rumble and
// Link's voice clip. arg2: 0 stagger, 1 knockdown, 2 damage-run, 3 ICE
// (Player_Action_82 + link_normal_ice_down), 4 ELECTRIC (Player_Action_83 +
// link_normal_electric_shock).
// (parameter named `player` here, not `this` as in z_player.c — this is a C++ TU)
void func_80833B18(PlayState* play, Player* player, s32 arg2, f32 speed, f32 velocityY, s16 yaw,
                   s32 invincibilityTimer);
// Sets Link on fire: fills bodyFlameTimers[] + bodyIsBurning. The damage-over-
// time (-1 HP every 4 frames) and the burn-out are then driven by
// Player_UpdateBodyBurn on its own — we do not tick anything.
s32 func_808344C0(PlayState* play, Player* player);
}

// Forward decls implemented in HarpoonDummyPlayer.cpp.
void HarpoonDummyPlayer_DrawAll(PlayState* play); // render remote players (draw hook)
void HarpoonDummyPlayer_SyncNametags();           // no-op for now
void HarpoonPeer_RefreshActors(PlayState* play);  // spawn/kill one collider actor per peer
void HarpoonPeer_OnActorDestroyed(Actor* actor);
void HarpoonPeer_OnPlayDestroy();
Actor* HarpoonPeer_FindByClientId(uint32_t clientId);

// How long the local player stays blinded / input-locked, ticked once per frame
// by Harpoon_TickLocalStatus. Written by Harpoon_ApplyIncomingStatus below.
static int sBlindnessFrames = 0;
static int sInputLockFrames = 0;
extern "C" void Harpoon_TickLocalStatus();

// -----------------------------------------------------------------------------
// Local player sync: send our pose / skeleton / form to peers each frame.
// -----------------------------------------------------------------------------

namespace {

int g_frameCounter = 0;

// IMPORTANT: payload field names/structure MUST match the Harpoon server's
// schema (built for SoH) or the server drops the packet and never relays it.
// That was why remotes stayed at scene=-1 pos=(0,0,0).

void SendLocalPlayerTransform(Player* player) {
    nlohmann::json p = { { "type", HarpoonPT::PLAYER_UPDATE_TRANSFORM },
                         { "payload",
                           {
                               { "clientId", Harpoon::Instance()->OwnClientId() },
                               { "posRot",
                                 {
                                     { "pos",
                                       { { "x", player->actor.world.pos.x },
                                         { "y", player->actor.world.pos.y },
                                         { "z", player->actor.world.pos.z } } },
                                     { "rot",
                                       { { "x", (int)player->actor.shape.rot.x },
                                         { "y", (int)player->actor.shape.rot.y },
                                         { "z", (int)player->actor.shape.rot.z } } },
                                 } },
                               { "prevTransl",
                                 { { "x", (int)player->skelAnime.prevTransl.x },
                                   { "y", (int)player->skelAnime.prevTransl.y },
                                   { "z", (int)player->skelAnime.prevTransl.z } } },
                               { "movementFlags", (int)player->skelAnime.movementFlags },
                               { "quiet", true },
                           } } };
    Harpoon::Instance()->SendJson(p);
}

void SendLocalPlayerSkeleton(Player* player) {
    if (player->skelAnime.jointTable == nullptr)
        return;
    int limbs = player->skelAnime.limbCount;
    std::vector<int> jointArray;
    jointArray.reserve(24 * 3);
    for (int i = 0; i < 24; ++i) {
        if (i < limbs) {
            jointArray.push_back(player->skelAnime.jointTable[i].x);
            jointArray.push_back(player->skelAnime.jointTable[i].y);
            jointArray.push_back(player->skelAnime.jointTable[i].z);
        } else {
            jointArray.push_back(0);
            jointArray.push_back(0);
            jointArray.push_back(0);
        }
    }
    nlohmann::json p = { { "type", HarpoonPT::PLAYER_UPDATE_SKELETON },
                         { "payload",
                           {
                               { "clientId", Harpoon::Instance()->OwnClientId() },
                               { "jointTable", jointArray },
                               { "movementFlags", (int)player->skelAnime.movementFlags },
                               { "quiet", true },
                           } } };
    Harpoon::Instance()->SendJson(p);
}

// Sent continuously at ~5Hz so peers that join after us still learn our scene
// (that's what makes their client draw our dummy). Server schema key is
// "sceneNum" (we put MM's sceneId value there).
void SendLocalPlayerVisualState(PlayState* play, Player* player) {
    nlohmann::json p = { { "type", HarpoonPT::PLAYER_UPDATE_VISUAL_STATE },
                         { "payload",
                           {
                               { "clientId", Harpoon::Instance()->OwnClientId() },
                               { "isSaveLoaded", true },
                               { "sceneNum", (int)play->sceneId },
                               { "entranceIndex", 0 },
                               { "linkAge", 0 },
                           } } };
    Harpoon::Instance()->SendJson(p);
}

// Equipment visible state (held weapon / shield / mask / boots), ~10Hz. Schema
// matches SoH's PLAYER.UPDATE_EQUIP_VISIBLE so the server relays it.
void SendLocalPlayerEquipVisible(Player* player) {
    nlohmann::json p = { { "type", HarpoonPT::PLAYER_UPDATE_EQUIP_VISIBLE },
                         { "payload",
                           {
                               { "clientId", Harpoon::Instance()->OwnClientId() },
                               { "currentBoots", (int)player->currentBoots },
                               { "currentShield", (int)player->currentShield },
                               { "currentTunic", 0 },
                               { "buttonItem0", 0 },
                               { "itemAction", (int)player->itemAction },
                               { "heldItemAction", (int)player->heldItemAction },
                               { "currentMask", (int)player->currentMask },
                               { "wornMask", 0 },
                               { "quiet", true },
                           } } };
    Harpoon::Instance()->SendJson(p);
}

// NEI custom items. CustomItems_BuildVisualSync already collects exactly what
// the per-item draw functions read (the CI_FLAG_* bits plus each item's pose /
// projectile / matrix state), so the peer can call CustomItems_OverrideDraw and
// get the real thing.
//
// The struct carries MtxF and Vec3f arrays, so it goes over as one base64 blob
// rather than field-by-field JSON — that also means it can't drift out of sync
// with the X-macro lists in custom_items_common.c. It is only sent when it
// actually CHANGES (plus a keepalive), because at ~700 bytes a fixed 10Hz feed
// would be pure noise: with no custom item in use the state never moves.
std::string Base64Encode(const uint8_t* data, size_t len) {
    static constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t v = (uint32_t)data[i] << 16;
        if (i + 1 < len)
            v |= (uint32_t)data[i + 1] << 8;
        if (i + 2 < len)
            v |= (uint32_t)data[i + 2];
        out.push_back(kAlphabet[(v >> 18) & 0x3F]);
        out.push_back(kAlphabet[(v >> 12) & 0x3F]);
        out.push_back(i + 1 < len ? kAlphabet[(v >> 6) & 0x3F] : '=');
        out.push_back(i + 2 < len ? kAlphabet[v & 0x3F] : '=');
    }
    return out;
}

void SendLocalCustomItems() {
    CustomItemVisualSync sync;
    memset(&sync, 0, sizeof(sync));
    CustomItems_BuildVisualSync(&sync);

    static CustomItemVisualSync sLastSent;
    static bool sHasLast = false;
    static int sKeepAlive = 0;
    bool changed = !sHasLast || memcmp(&sync, &sLastSent, sizeof(sync)) != 0;
    // Resend unchanged state every ~2s anyway (this runs at 10Hz) so a peer that
    // joined mid-session still learns what we're holding.
    if (!changed && ++sKeepAlive < 20)
        return;
    sKeepAlive = 0;
    sLastSent = sync;
    sHasLast = true;

    nlohmann::json p = { { "type", HarpoonPT::PLAYER_UPDATE_CUSTOM_ITEMS },
                         { "payload",
                           {
                               { "clientId", Harpoon::Instance()->OwnClientId() },
                               { "ciVer", (int)sizeof(CustomItemVisualSync) },
                               { "ciBlob", Base64Encode((const uint8_t*)&sync, sizeof(sync)) },
                               { "quiet", true },
                           } } };
    Harpoon::Instance()->SendJson(p);
}

void SendLocalTransformation(Player* player) {
    nlohmann::json p = { { "type", HarpoonPT::PLAYER_SET_TRANSFORMATION },
                         { "payload",
                           {
                               { "clientId", Harpoon::Instance()->OwnClientId() },
                               { "transformation", (int)player->transformation },
                               { "cylRadius", 0 },
                               { "cylHeight", 0 },
                               { "cylYShift", 0 },
                               { "mmStateFlags3", 0 },
                               { "mmSpeedXZ", 0.0f },
                           } } };
    Harpoon::Instance()->SendJson(p);
}

} // namespace

// -----------------------------------------------------------------------------
// Hook registration entry points (called from Harpoon::InitHooks).
// -----------------------------------------------------------------------------

extern "C" {

int Harpoon_RegisterFrameHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
        Harpoon::Instance()->DrainIncomingQueue();
        Harpoon_TickLocalStatus();
    });
}

// Stun / freeze that should take the controls away. freezeTimer already stops
// the actor updating; this stops the inputs from queueing up behind it.
int Harpoon_RegisterInputHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPassPlayerInputs>([](Input* input) {
        if (sInputLockFrames <= 0 || input == nullptr)
            return;
        input->cur.button = 0;
        input->press.button = 0;
        input->rel.button = 0;
        input->cur.stick_x = 0;
        input->cur.stick_y = 0;
        input->rel.stick_x = 0;
        input->rel.stick_y = 0;
    });
}

// Blindness overlay. No engine API for this, so it's a foreground rect — same
// approach as SoH's BlindnessEffect.
int Harpoon_RegisterBlindnessHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnInterfaceDrawStart>([]() {
        if (sBlindnessFrames <= 0)
            return;
        // Ramp in and out over the last/first 45 frames so it never snaps.
        float alpha = sBlindnessFrames >= 45 ? 0.985f : (sBlindnessFrames / 45.0f) * 0.985f;
        ImGuiIO& io = ImGui::GetIO();
        ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(io.DisplaySize.x, io.DisplaySize.y),
                                                      ImGui::GetColorU32(ImVec4(0.05f, 0.0f, 0.08f, alpha)));
    });
}

// Called from scene loading (z_scene_2SH.cpp) to decide whether to preload all
// transformation objects so cross-form remote players can be drawn.
int Harpoon_WantsAllFormObjects(void) {
    return Harpoon::Instance()->State() == HarpoonConnState::InRoom ? 1 : 0;
}

int Harpoon_RegisterActorUpdateHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorUpdate>([](Actor* actor) {
        Harpoon* h = Harpoon::Instance();
        if (h->State() != HarpoonConnState::InRoom)
            return;

        if (actor->id != ACTOR_PLAYER)
            return;
        Player* player = (Player*)actor;
        PlayState* play = gPlayState;
        if (!play)
            return;

        ++g_frameCounter;
        // 30 Hz pose + skeleton.
        if ((g_frameCounter & 1) == 0) {
            SendLocalPlayerTransform(player);
            SendLocalPlayerSkeleton(player);
        }
        // 5 Hz visual state + form, sent CONTINUOUSLY so late joiners learn our
        // scene/form and spawn our dummy.
        if ((g_frameCounter % 12) == 0) {
            SendLocalPlayerVisualState(play, player);
            SendLocalTransformation(player);
        }
        // ~10 Hz equipment (held weapon / shield / mask / boots) + NEI custom
        // item visual state (the latter self-throttles: change-driven).
        if ((g_frameCounter % 6) == 0) {
            SendLocalPlayerEquipVisible(player);
            SendLocalCustomItems();
        }

        // Keep one ACTOR_HARPOON_PEER alive per remote in our scene. PvP hit
        // detection is the engine's job from there: each peer carries an AC
        // cylinder, so our sword / form attacks / bombs / custom items land on
        // it and HarpoonPeer_Update forwards the result.
        HarpoonPeer_RefreshActors(play);
    });
}

// Peer actors and EffectBlure slots both die with the scene: Play_Destroy calls
// Effect_DestroyAll, so any index we kept would now name someone else's effect.
int Harpoon_RegisterPlayDestroyHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayDestroy>(
        []() { HarpoonPeer_OnPlayDestroy(); });
}

int Harpoon_RegisterActorDestroyHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorDestroy>(
        [](Actor* actor) { HarpoonPeer_OnActorDestroyed(actor); });
}

// Renders remote players as Link-in-form at the end of the world draw.
int Harpoon_RegisterDrawHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayDrawWorldEnd>(
        []() { HarpoonDummyPlayer_DrawAll(gPlayState); });
}

void Harpoon_UnregisterHook(int /*kind*/, int hookId) {
    (void)hookId;
    // GameInteractor in 2ship does not currently expose typed unregister; the
    // hooks live for the process lifetime. Shutdown is a no-op for now.
}

// -----------------------------------------------------------------------------
// Flag sync (Phase I). Send our local flag changes; apply remote ones directly
// to gSaveContext so they take effect even for scenes we're not currently in.
// -----------------------------------------------------------------------------

int Harpoon_RegisterSceneFlagSetHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneFlagSet>(
        [](s16 sceneId, FlagType flagType, u32 flag) {
            Harpoon::Instance()->SendFlag(true, (int)flagType, (int)sceneId, flag);
        });
}

int Harpoon_RegisterSceneFlagUnsetHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneFlagUnset>(
        [](s16 sceneId, FlagType flagType, u32 flag) {
            Harpoon::Instance()->SendFlag(false, (int)flagType, (int)sceneId, flag);
        });
}

int Harpoon_RegisterFlagSetHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnFlagSet>(
        [](FlagType flagType, u32 flag) { Harpoon::Instance()->SendFlag(true, (int)flagType, -1, flag); });
}

int Harpoon_RegisterFlagUnsetHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnFlagUnset>(
        [](FlagType flagType, u32 flag) { Harpoon::Instance()->SendFlag(false, (int)flagType, -1, flag); });
}

// Apply a remote flag change. Writes straight into gSaveContext bit arrays so
// it works cross-scene and never re-triggers the OnFlagSet hooks (no rebroadcast
// loop). Called from Harpoon::HandleSaveFlag on the game thread.
void Harpoon_ApplyRemoteFlag(bool set, int flagType, int sceneId, uint32_t flag) {
    switch (flagType) {
        case FLAG_WEEK_EVENT_REG:
        case FLAG_WEEK_EVENT_REG_HORSE_RACE: {
            u8& reg = gSaveContext.save.saveInfo.weekEventReg[(flag) >> 8];
            if (set)
                reg |= (flag & 0xFF);
            else
                reg &= ~(flag & 0xFF);
            break;
        }
        case FLAG_EVENT_INF: {
            u8& reg = gSaveContext.eventInf[(flag) >> 4];
            if (set)
                reg |= (1 << ((flag)&0xF));
            else
                reg &= ~(1 << ((flag)&0xF));
            break;
        }
        case FLAG_PERM_SCENE_CHEST:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.save.saveInfo.permanentSceneFlags[sceneId].chest;
                if (set)
                    f |= (1u << flag);
                else
                    f &= ~(1u << flag);
            }
            break;
        case FLAG_PERM_SCENE_SWITCH:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.save.saveInfo.permanentSceneFlags[sceneId].switch0;
                if (set)
                    f |= (1u << flag);
                else
                    f &= ~(1u << flag);
            }
            break;
        case FLAG_PERM_SCENE_CLEARED_ROOM:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.save.saveInfo.permanentSceneFlags[sceneId].clearedRoom;
                if (set)
                    f |= (1u << flag);
                else
                    f &= ~(1u << flag);
            }
            break;
        case FLAG_PERM_SCENE_COLLECTIBLE:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.save.saveInfo.permanentSceneFlags[sceneId].collectible;
                if (set)
                    f |= (1u << flag);
                else
                    f &= ~(1u << flag);
            }
            break;
        case FLAG_CYCL_SCENE_CHEST:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.cycleSceneFlags[sceneId].chest;
                if (set)
                    f |= (1u << flag);
                else
                    f &= ~(1u << flag);
            }
            break;
        case FLAG_CYCL_SCENE_SWITCH:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.cycleSceneFlags[sceneId].switch0;
                if (set)
                    f |= (1u << flag);
                else
                    f &= ~(1u << flag);
            }
            break;
        case FLAG_CYCL_SCENE_CLEARED_ROOM:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.cycleSceneFlags[sceneId].clearedRoom;
                if (set)
                    f |= (1u << flag);
                else
                    f &= ~(1u << flag);
            }
            break;
        case FLAG_CYCL_SCENE_COLLECTIBLE:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.cycleSceneFlags[sceneId].collectible;
                if (set)
                    f |= (1u << flag);
                else
                    f &= ~(1u << flag);
            }
            break;
        default:
            // FLAG_EVENT_INF subtypes, FLAG_SCENES_VISIBLE, FLAG_OWL_ACTIVATION,
            // FLAG_RANDO_INF etc. not synced in this phase.
            break;
    }
}

// -----------------------------------------------------------------------------
// Apply incoming damage / status to the local player (called from Harpoon.cpp).
// -----------------------------------------------------------------------------

int Harpoon_GetBlindnessFrames() {
    return sBlindnessFrames;
}

void Harpoon_TickLocalStatus() {
    if (sBlindnessFrames > 0)
        sBlindnessFrames--;
    if (sInputLockFrames > 0)
        sInputLockFrames--;
}

// Apply a hit from a peer. `damage` arrives in DamageTable units (quarter
// hearts, what CollisionCheck wrote into the attacker-side colChkInfo);
// Health_ChangeBy works in sixteenths, so scale by 4.
void Harpoon_ApplyIncomingDamage(uint8_t damageEffect, uint8_t damage, uint32_t attackerClientId) {
    if (!gPlayState)
        return;
    Player* self = GET_PLAYER(gPlayState);
    if (self == nullptr)
        return;
    if (Player_InBlockingCsMode(gPlayState, self))
        return;
    if (self->stateFlags1 & PLAYER_STATE1_DEAD)
        return;

    // Gamemode decides, not a manual toggle. Outside PvP a hit can still stun
    // (harmless, and it keeps coop gimmicks working) but never wounds.
    if (!Harpoon::Instance()->IsPvpActive()) {
        if (damageEffect == HARPOON_HIT_STUN) {
            self->actor.freezeTimer = 20;
            Actor_SetColorFilter(&self->actor, 0, 0xFF, 0, 24);
        }
        return;
    }

    s32 arg2 = 0; // func_80833B18 reaction
    f32 knockSpeed = 4.0f;
    f32 knockYVel = 5.0f;
    s32 invTimer = 20;
    s32 finalDamage = (s32)damage * 4;
    bool setOnFire = false;

    switch (damageEffect) {
        case HARPOON_HIT_KNOCKBACK_LARGE:
            arg2 = 1;
            knockSpeed = 14.0f;
            knockYVel = 10.0f;
            invTimer = 25;
            break;
        case HARPOON_HIT_KNOCKBACK_SMALL:
            arg2 = 0;
            knockSpeed = 4.0f;
            knockYVel = 5.0f;
            invTimer = 20;
            break;
        case HARPOON_HIT_FROZEN:
            // Player_Action_82 + link_normal_ice_down. Sages' ice resistance
            // downgrades this to a plain stagger inside func_80833B18.
            arg2 = 3;
            knockSpeed = 0.0f;
            knockYVel = 0.0f;
            invTimer = 60;
            break;
        case HARPOON_HIT_ELECTRIFIED:
            arg2 = 4;
            knockSpeed = 2.0f;
            knockYVel = 3.0f;
            invTimer = 20;
            self->bodyShockTimer = 40;
            break;
        case HARPOON_HIT_STUN:
            arg2 = 0;
            knockSpeed = 0.0f;
            knockYVel = 0.0f;
            invTimer = 20;
            self->actor.freezeTimer = 20;
            Actor_SetColorFilter(&self->actor, 0, 0xFF, 0, 24);
            break;
        case HARPOON_HIT_FIRE:
            arg2 = 0;
            knockSpeed = 5.0f;
            knockYVel = 6.0f;
            invTimer = 30;
            setOnFire = true;
            break;
        case HARPOON_HIT_LIGHT:
            arg2 = 0;
            knockSpeed = 6.0f;
            knockYVel = 7.0f;
            invTimer = 35;
            Actor_SetColorFilter(&self->actor, 0x8000, 0xFF, 0, 40);
            break;
        case HARPOON_HIT_DARK:
            arg2 = 0;
            knockSpeed = 3.0f;
            knockYVel = 4.0f;
            invTimer = 20;
            Actor_SetColorFilter(&self->actor, 0x4000, 0x80, 0, 30);
            break;
        case HARPOON_HIT_SOUL_DRAIN:
            arg2 = 0;
            knockSpeed = 1.0f;
            knockYVel = 2.0f;
            invTimer = 20;
            break;
        case HARPOON_HIT_WIND_BLOW:
            arg2 = 2;
            knockSpeed = 18.0f;
            knockYVel = 4.0f;
            invTimer = 15;
            finalDamage = 0; // wind shoves, it doesn't wound
            break;
        case HARPOON_HIT_WIND_PUSH:
            arg2 = 0;
            knockSpeed = 10.0f;
            knockYVel = 3.0f;
            invTimer = 10;
            finalDamage = 0;
            break;
        case HARPOON_HIT_NONE:
            if (damage == 0)
                return;
            break;
        case HARPOON_HIT_NORMAL:
        default:
            break;
    }

    // Aim the knockback away from whoever hit us. The peer actor makes this
    // exact; without one (they left, or they're mid-respawn) fall back to 0.
    s16 yaw = 0;
    Actor* attacker = HarpoonPeer_FindByClientId(attackerClientId);
    if (attacker != nullptr) {
        yaw = Actor_WorldYawTowardActor(attacker, &self->actor);
    }

    self->actor.colChkInfo.damage = finalDamage;
    func_80833B18(gPlayState, self, arg2, knockSpeed, knockYVel, yaw, invTimer);

    if (setOnFire) {
        // Zora and Deku DIE on fire and Goron shrugs it off — that's vanilla
        // behaviour and we let it stand.
        func_808344C0(gPlayState, self);
    }
}

void Harpoon_ApplyIncomingStatus(uint8_t effect, uint16_t durationFrames) {
    if (!gPlayState)
        return;
    Player* self = GET_PLAYER(gPlayState);
    if (self == nullptr)
        return;
    if (Player_InBlockingCsMode(gPlayState, self))
        return;
    if (self->stateFlags1 & PLAYER_STATE1_DEAD)
        return;
    if (!Harpoon::Instance()->IsPvpActive())
        return;

    u16 duration = durationFrames != 0 ? durationFrames : 60;

    switch (effect) {
        case HARPOON_STATUS_BURN_DOT:
            // Player_UpdateBodyBurn owns the tick and the burn-out from here.
            func_808344C0(gPlayState, self);
            break;
        case HARPOON_STATUS_FREEZE:
            func_80833B18(gPlayState, self, 3, 0.0f, 0.0f, 0, duration);
            break;
        case HARPOON_STATUS_STUN:
            // freezeTimer is u16 in MM (s16 in OoT) — never test it for < 0.
            self->actor.freezeTimer = duration;
            sInputLockFrames = duration;
            Actor_SetColorFilter(&self->actor, 0, 0xFF, 0, duration);
            break;
        case HARPOON_STATUS_BLINDNESS:
            sBlindnessFrames = duration;
            break;
        case HARPOON_STATUS_NONE:
        default:
            break;
    }
}

} // extern "C"
