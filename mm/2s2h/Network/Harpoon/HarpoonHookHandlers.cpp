// =============================================================================
// HarpoonHookHandlers — game-side hooks for Harpoon multiplayer.
//
// Lives between the Harpoon singleton (network-only, no MM headers) and the
// MM engine (PlayState, Actor, Player). Registers GameInteractor hooks at
// startup and forwards state to/from the singleton each frame.
//
// Scope:
//   - Drain the Harpoon inbound queue once per frame.                  (Phase B)
//   - Send PLAYER.UPDATE_TRANSFORM/SKELETON/VISUAL_STATE for local Link.(Phase C)
//   - Spawn / destroy dummy actors on scene transitions.                (Phase D)
//   - Apply incoming COMBAT.DEAL_DAMAGE / APPLY_STATUS to local player. (Phase E)
//
// Phases D/E/F that need deep MM rendering / collision integration are stubbed
// with clear TODOs.
// =============================================================================

#include "Harpoon.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include <spdlog/spdlog.h>

extern "C" {
#include "z64.h"
#include "z64actor.h"
#include "z64player.h"
#include "z64save.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

// Forward decls implemented in HarpoonDummyPlayer.cpp.
void HarpoonDummyPlayer_DrawAll(PlayState* play);    // render remote players (draw hook)
void HarpoonDummyPlayer_SyncNametags();              // no-op for now

// Forward decls implemented in Combat/CombatSync.cpp.
void HarpoonCombat_OnLocalPlayerUpdate(PlayState* play, Player* player);

// -----------------------------------------------------------------------------
// Local player sync: send our pose / skeleton / form to peers each frame.
// -----------------------------------------------------------------------------

namespace {

int g_frameCounter = 0;

// IMPORTANT: payload field names/structure MUST match the Harpoon server's
// schema (built for SoH) or the server drops the packet and never relays it.
// That was why remotes stayed at scene=-1 pos=(0,0,0).

void SendLocalPlayerTransform(Player* player) {
    nlohmann::json p = {
        { "type", HarpoonPT::PLAYER_UPDATE_TRANSFORM },
        { "payload", {
            { "clientId", Harpoon::Instance()->OwnClientId() },
            { "posRot", {
                { "pos", { { "x", player->actor.world.pos.x }, { "y", player->actor.world.pos.y },
                           { "z", player->actor.world.pos.z } } },
                { "rot", { { "x", (int)player->actor.shape.rot.x }, { "y", (int)player->actor.shape.rot.y },
                           { "z", (int)player->actor.shape.rot.z } } },
            }},
            { "prevTransl", { { "x", (int)player->skelAnime.prevTransl.x },
                              { "y", (int)player->skelAnime.prevTransl.y },
                              { "z", (int)player->skelAnime.prevTransl.z } } },
            { "movementFlags", (int)player->skelAnime.movementFlags },
            { "quiet", true },
        }}
    };
    Harpoon::Instance()->SendJson(p);
}

void SendLocalPlayerSkeleton(Player* player) {
    if (player->skelAnime.jointTable == nullptr) return;
    int limbs = player->skelAnime.limbCount;
    std::vector<int> jointArray;
    jointArray.reserve(24 * 3);
    for (int i = 0; i < 24; ++i) {
        if (i < limbs) {
            jointArray.push_back(player->skelAnime.jointTable[i].x);
            jointArray.push_back(player->skelAnime.jointTable[i].y);
            jointArray.push_back(player->skelAnime.jointTable[i].z);
        } else {
            jointArray.push_back(0); jointArray.push_back(0); jointArray.push_back(0);
        }
    }
    nlohmann::json p = {
        { "type", HarpoonPT::PLAYER_UPDATE_SKELETON },
        { "payload", {
            { "clientId", Harpoon::Instance()->OwnClientId() },
            { "jointTable", jointArray },
            { "movementFlags", (int)player->skelAnime.movementFlags },
            { "quiet", true },
        }}
    };
    Harpoon::Instance()->SendJson(p);
}

// Sent continuously at ~5Hz so peers that join after us still learn our scene
// (that's what makes their client draw our dummy). Server schema key is
// "sceneNum" (we put MM's sceneId value there).
void SendLocalPlayerVisualState(PlayState* play, Player* player) {
    nlohmann::json p = {
        { "type", HarpoonPT::PLAYER_UPDATE_VISUAL_STATE },
        { "payload", {
            { "clientId", Harpoon::Instance()->OwnClientId() },
            { "isSaveLoaded", true },
            { "sceneNum", (int)play->sceneId },
            { "entranceIndex", 0 },
            { "linkAge", 0 },
        }}
    };
    Harpoon::Instance()->SendJson(p);
}

// Equipment visible state (held weapon / shield / mask / boots), ~10Hz. Schema
// matches SoH's PLAYER.UPDATE_EQUIP_VISIBLE so the server relays it.
void SendLocalPlayerEquipVisible(Player* player) {
    nlohmann::json p = {
        { "type", HarpoonPT::PLAYER_UPDATE_EQUIP_VISIBLE },
        { "payload", {
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
        }}
    };
    Harpoon::Instance()->SendJson(p);
}

void SendLocalTransformation(Player* player) {
    nlohmann::json p = {
        { "type", HarpoonPT::PLAYER_SET_TRANSFORMATION },
        { "payload", {
            { "clientId", Harpoon::Instance()->OwnClientId() },
            { "transformation", (int)player->transformation },
            { "cylRadius", 0 }, { "cylHeight", 0 }, { "cylYShift", 0 },
            { "mmStateFlags3", 0 }, { "mmSpeedXZ", 0.0f },
        }}
    };
    Harpoon::Instance()->SendJson(p);
}

}  // namespace

// -----------------------------------------------------------------------------
// Hook registration entry points (called from Harpoon::InitHooks).
// -----------------------------------------------------------------------------

extern "C" {

int Harpoon_RegisterFrameHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
        Harpoon::Instance()->DrainIncomingQueue();
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
        if (h->State() != HarpoonConnState::InRoom) return;

        if (actor->id != ACTOR_PLAYER) return;
        Player* player = (Player*)actor;
        PlayState* play = gPlayState;
        if (!play) return;

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
        // ~10 Hz equipment (held weapon / shield / mask / boots).
        if ((g_frameCounter % 6) == 0) {
            SendLocalPlayerEquipVisible(player);
        }

        // PvP detection lives here (Phase E).
        HarpoonCombat_OnLocalPlayerUpdate(play, player);
    });
}

// Renders remote players as Link-in-form at the end of the world draw.
int Harpoon_RegisterDrawHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayDrawWorldEnd>([]() {
        HarpoonDummyPlayer_DrawAll(gPlayState);
    });
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
        [](FlagType flagType, u32 flag) {
            Harpoon::Instance()->SendFlag(true, (int)flagType, -1, flag);
        });
}

int Harpoon_RegisterFlagUnsetHook() {
    return GameInteractor::Instance->RegisterGameHook<GameInteractor::OnFlagUnset>(
        [](FlagType flagType, u32 flag) {
            Harpoon::Instance()->SendFlag(false, (int)flagType, -1, flag);
        });
}

// Apply a remote flag change. Writes straight into gSaveContext bit arrays so
// it works cross-scene and never re-triggers the OnFlagSet hooks (no rebroadcast
// loop). Called from Harpoon::HandleSaveFlag on the game thread.
void Harpoon_ApplyRemoteFlag(bool set, int flagType, int sceneId, uint32_t flag) {
    switch (flagType) {
        case FLAG_WEEK_EVENT_REG:
        case FLAG_WEEK_EVENT_REG_HORSE_RACE: {
            u8& reg = gSaveContext.save.saveInfo.weekEventReg[(flag) >> 8];
            if (set) reg |= (flag & 0xFF);
            else     reg &= ~(flag & 0xFF);
            break;
        }
        case FLAG_EVENT_INF: {
            u8& reg = gSaveContext.eventInf[(flag) >> 4];
            if (set) reg |= (1 << ((flag) & 0xF));
            else     reg &= ~(1 << ((flag) & 0xF));
            break;
        }
        case FLAG_PERM_SCENE_CHEST:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.save.saveInfo.permanentSceneFlags[sceneId].chest;
                if (set) f |= (1u << flag); else f &= ~(1u << flag);
            }
            break;
        case FLAG_PERM_SCENE_SWITCH:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.save.saveInfo.permanentSceneFlags[sceneId].switch0;
                if (set) f |= (1u << flag); else f &= ~(1u << flag);
            }
            break;
        case FLAG_PERM_SCENE_CLEARED_ROOM:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.save.saveInfo.permanentSceneFlags[sceneId].clearedRoom;
                if (set) f |= (1u << flag); else f &= ~(1u << flag);
            }
            break;
        case FLAG_PERM_SCENE_COLLECTIBLE:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.save.saveInfo.permanentSceneFlags[sceneId].collectible;
                if (set) f |= (1u << flag); else f &= ~(1u << flag);
            }
            break;
        case FLAG_CYCL_SCENE_CHEST:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.cycleSceneFlags[sceneId].chest;
                if (set) f |= (1u << flag); else f &= ~(1u << flag);
            }
            break;
        case FLAG_CYCL_SCENE_SWITCH:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.cycleSceneFlags[sceneId].switch0;
                if (set) f |= (1u << flag); else f &= ~(1u << flag);
            }
            break;
        case FLAG_CYCL_SCENE_CLEARED_ROOM:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.cycleSceneFlags[sceneId].clearedRoom;
                if (set) f |= (1u << flag); else f &= ~(1u << flag);
            }
            break;
        case FLAG_CYCL_SCENE_COLLECTIBLE:
            if (sceneId >= 0 && sceneId < 120) {
                u32& f = gSaveContext.cycleSceneFlags[sceneId].collectible;
                if (set) f |= (1u << flag); else f &= ~(1u << flag);
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

void Harpoon_ApplyIncomingDamage(uint8_t damage, uint16_t /*weaponId*/) {
    if (!gPlayState || damage == 0) return;
    // play->damagePlayer is a callback on PlayState. Vanilla MM Game Over
    // triggers naturally when HP reaches 0.
    if (gPlayState->damagePlayer != nullptr) {
        gPlayState->damagePlayer(gPlayState, (s32)-damage);
    }
}

void Harpoon_ApplyIncomingStatus(uint8_t /*effect*/, uint16_t /*durationFrames*/) {
    // TODO Phase E status effects (burn DOT, freeze, blindness, stun).
    // Burn could be implemented by repeated damagePlayer calls over time;
    // freeze by zeroing input via OnPassPlayerInputs; blindness by drawing
    // a black overlay in InterfaceDrawStart; stun by clearing input.
}

}  // extern "C"
