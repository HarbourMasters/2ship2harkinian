// =============================================================================
// Quartz of Motion (MM side) — sensor brain. See DesireCompass.h.
//
// Every tick we walk the live actor lists, resolve each actor to its check, and
// keep the nearest one still uncollected that matches the tracked category.
// From that single distance we drive both signals: the "something is here"
// indicator (any match loaded at all) and the proximity blip rate.
//
// MM has no central actor->check index, so a single recognizer covers every
// carrier:
//   - Obj* families / EnCow / ObjTsubo  -> Rando::ActorBehavior::GetObjectRandoCheckId
//   - the floating rando item           -> ACTOR_EN_ITEM00 w/ params==ITEM00_NOTHING, RC in home.rot.z
//   - chests                            -> ACTOR_EN_BOX, RC in home.rot.x
//   - EnGamelupy / EnElforg / EnTakaraya-> RC in home.rot.x
// Checks with no in-scene carrier (songs, minigame rewards, hardcoded-RC NPCs)
// simply never match.
// =============================================================================

#include "2s2h/Rando/Rando.h"
#include "2s2h/Rando/StaticData/StaticData.h"
#include "2s2h/Rando/ActorBehavior/ActorBehavior.h"
#include "2s2h/Rando/DesireCompass.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#include <chrono>

// Quartz ownership + persistent category selection live in the NEI save blob.
#include "mods/nei_save.h"

extern "C" {
#include "z64.h"
#include "variables.h"
#include "functions.h"
#include "macros.h"
#include "overlays/actors/ovl_En_Si/z_en_si.h"     // ENSI_GET_CHEST_FLAG
#include "overlays/actors/ovl_Obj_Grass/z_obj_grass.h" // ObjGrass groups + elements
extern PlayState* gPlayState;
// The live grass manager (one per scene). Grass patches are NOT one actor each:
// they are elements inside this manager, and ObjGrass.cpp attaches their checks
// to the ELEMENT pointer, not to any Actor* — so the actor sweep can never see
// them. See the dedicated pass in FindNearestLoaded.
extern ObjGrass* sGrassManager;
// Rumble (z_rumble.c).
void func_800AA000(f32 arg0, u8 arg1, u8 arg2, u8 arg3);
}

// Mirror of the private macros in the owning behaviors (kept local so we don't
// pull their headers). CustomItem stashes its RandoCheckId in home.rot.z
// (CUSTOM_ITEM_PARAM, CustomItem.h:6); chests/gamelupy/elforg/takaraya use
// home.rot.x (EnBox.cpp:29, EnGamelupy.cpp:90, EnElforg.cpp:13, EnTakaraya.cpp:10).
#define DCOMPASS_RC_FROM_ROT_Z(a) ((RandoCheckId)(a)->home.rot.z)
#define DCOMPASS_RC_FROM_ROT_X(a) ((RandoCheckId)(a)->home.rot.x)

namespace {

// -----------------------------------------------------------------------------
// Category membership
// -----------------------------------------------------------------------------

bool InRange(RandoItemId id, RandoItemId lo, RandoItemId hi) {
    return id >= lo && id <= hi;
}

bool IsBossSoul(RandoItemId id) {
    return InRange(id, RI_SOUL_BOSS_GOHT, RI_SOUL_BOSS_TWINMOLD);
}
bool IsOtherSoul(RandoItemId id) {
    return InRange(id, RI_SOUL_ENEMY_ALIEN, RI_SOUL_ENEMY_WOLFOS);
}
bool IsTriforce(RandoItemId id) {
    return id == RI_TRIFORCE_PIECE || id == RI_TRIFORCE_PIECE_PREVIOUS;
}
bool IsSkill(RandoItemId id) {
    // Only RI_ABILITY_SWIM exists today; keeps future RI_ABILITY_* covered.
    return id == RI_ABILITY_SWIM;
}

bool ItemMatchesCategory(RandoItemId id, DesireCompassCategory cat, s32 subcat) {
    if (id == RI_UNKNOWN) {
        return false;
    }
    RandoItemType type = Rando::StaticData::Items[id].randoItemType;

    switch (cat) {
        case DCOMPASS_CAT_BOSS_SOULS:
            return IsBossSoul(id);
        case DCOMPASS_CAT_OTHER_SOULS:
            return IsOtherSoul(id);
        case DCOMPASS_CAT_TRIFORCE:
            return IsTriforce(id);
        case DCOMPASS_CAT_SKILLS:
            return IsSkill(id);
        case DCOMPASS_CAT_KEYS:
            if (subcat == 1) return type == RITYPE_SMALL_KEY;
            if (subcat == 2) return type == RITYPE_BOSS_KEY;
            return type == RITYPE_SMALL_KEY || type == RITYPE_BOSS_KEY;
        case DCOMPASS_CAT_JUNK:
            return type == RITYPE_JUNK;
        case DCOMPASS_CAT_MAJOR:
            // The RITYPE_MAJOR/MASK bucket minus the slices with their own category.
            return (type == RITYPE_MAJOR || type == RITYPE_MASK) && !IsBossSoul(id) && !IsOtherSoul(id) &&
                   !IsTriforce(id) && !IsSkill(id);
        case DCOMPASS_CAT_OTHER:
            if (IsBossSoul(id) || IsOtherSoul(id) || IsTriforce(id) || IsSkill(id)) {
                return false;
            }
            if (type == RITYPE_SMALL_KEY || type == RITYPE_BOSS_KEY || type == RITYPE_JUNK ||
                type == RITYPE_MAJOR || type == RITYPE_MASK) {
                return false;
            }
            return true;
        default:
            return false;
    }
}

// Shuffled into the seed and not yet obtained (permanently or this cycle).
bool CheckIsOutstanding(RandoCheckId rc) {
    if (rc <= RC_UNKNOWN || rc >= RC_MAX) {
        return false;
    }
    const RandoSaveCheck& sc = RANDO_SAVE_CHECKS[rc];
    return sc.shuffled && !sc.obtained && !sc.cycleObtained;
}

RandoCheckId ResolveActorCheck(Actor* actor) {
    // Object-extension: the general Actor*->RC attachment used by every Obj*
    // family, EnCow, and the ObjTsubo fallback.
    RandoCheckId rc = Rando::ActorBehavior::GetObjectRandoCheckId(actor);
    if (rc > RC_UNKNOWN && rc < RC_MAX) {
        return rc;
    }

    switch (actor->id) {
        case ACTOR_EN_ITEM00:
            // The floating rando replacement item — the dominant carrier.
            if (actor->params == ITEM00_NOTHING) {
                return DCOMPASS_RC_FROM_ROT_Z(actor);
            }
            return RC_UNKNOWN;
        case ACTOR_EN_BOX:       // chests
        case ACTOR_EN_GAMELUPY:  // Deku playground rupees
        case ACTOR_EN_ELFORG:    // stray fairies
        case ACTOR_EN_TAKARAYA:  // treasure chest game
            return DCOMPASS_RC_FROM_ROT_X(actor);
        case ACTOR_EN_SI: // gold skulltula token — resolved by flag, never stored
            return Rando::StaticData::GetCheckFromFlag(FLAG_CYCL_SCENE_CHEST, ENSI_GET_CHEST_FLAG(actor),
                                                       gPlayState->sceneId)
                .randoCheckId;
        case ACTOR_ITEM_B_HEART: // heart container — always collectible flag 0x1F
            return Rando::StaticData::GetCheckFromFlag(FLAG_CYCL_SCENE_COLLECTIBLE, 0x1F, gPlayState->sceneId)
                .randoCheckId;
        default:
            return RC_UNKNOWN;
    }
}

// -----------------------------------------------------------------------------
// Session state
// -----------------------------------------------------------------------------

// Wall-clock deadline: real time keeps "5 minutes" honest regardless of frame
// rate. 0 = inactive.
s64 sDeadlineMs = 0;
DesireCompassCategory sActiveCat = DCOMPASS_CAT_BOSS_SOULS;
s32 sActiveSubcat = DCOMPASS_SUBCAT_ANY;

bool sRoomHasTarget = false; // something of the category is loaded right now
f32 sProximity = 0.0f;       // 0 = far/none, 1 = on top of it
s64 sNextBlipMs = 0;

// "You walked into a room that has something" flash.
s32 sRoomAlertFrames = 0;
s8 sLastRoomNum = -1;
bool sLastRoomHadTarget = false;

// Queued activation, consumed by the tick once gameplay resumes. -1 = none.
s32 sPendingCat = -1;
s32 sPendingSubcat = DCOMPASS_SUBCAT_ANY;
s32 sAttuneTimer = 0;
constexpr s32 kAttuneFrames = 40;

constexpr f32 kNearDist = 150.0f;
constexpr f32 kFarDist = 1200.0f;

inline s64 NowMs() {
    using namespace std::chrono;
    return (s64)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

const char* kCategoryNames[DCOMPASS_CAT_MAX] = {
    "Boss Souls", "Keys", "Other Souls", "Major Items", "Skills", "Junk", "Triforce", "Other",
};

void SpawnAttuneSparkles(Player* p) {
    Vec3f accel = { 0.0f, 0.05f, 0.0f };
    Color_RGBA8 primColor = { 180, 120, 255, 255 };
    Color_RGBA8 envColor = { 80, 40, 200, 255 };

    for (u8 i = 0; i < 3; i++) {
        s16 angle = (s16)(Rand_ZeroOne() * 0xFFFF);
        f32 dist = 15.0f + Rand_ZeroOne() * 25.0f;

        Vec3f pos;
        pos.x = p->actor.world.pos.x + Math_SinS(angle) * dist;
        pos.y = p->actor.world.pos.y + 20.0f + Rand_CenteredFloat(40.0f);
        pos.z = p->actor.world.pos.z + Math_CosS(angle) * dist;

        Vec3f vel;
        vel.x = Math_SinS(angle) * 0.3f;
        vel.y = 1.5f + Rand_ZeroOne() * 1.0f;
        vel.z = Math_CosS(angle) * 0.3f;

        // EffectSsKiraKira_SpawnFocused is only a compat MACRO for the C mods TU
        // (nei_oot_compat.h) — call MM's real spark effect directly.
        EffectSsGSpk_SpawnAccel(gPlayState, &p->actor, &pos, &vel, &accel, &primColor, &envColor, 50, 18);
    }
}

// Nearest loaded, uncollected check of the category.
bool FindNearestLoaded(DesireCompassCategory cat, s32 subcat, f32* outDist) {
    if (gPlayState == nullptr) {
        return false;
    }
    Player* player = GET_PLAYER(gPlayState);
    if (player == nullptr) {
        return false;
    }
    Vec3f playerPos = player->actor.world.pos;

    f32 bestDistSq = -1.0f;

    // (a) Grass patches. Each blade is an ObjGrassElement inside the scene's
    // grass manager — not an actor — and ObjGrass.cpp:428 hangs the check off
    // the element pointer, so the actor sweep below would miss every one.
    if (sGrassManager != nullptr) {
        for (s32 g = 0; g < sGrassManager->activeGrassGroups; g++) {
            ObjGrassGroup* group = &sGrassManager->grassGroups[g];
            for (s32 e = 0; e < group->count; e++) {
                ObjGrassElement* elem = &group->elements[e];
                if (elem->flags & OBJ_GRASS_ELEM_REMOVED) {
                    continue; // already cut
                }
                RandoCheckId rc = Rando::ActorBehavior::GetObjectRandoCheckId(elem);
                if (rc <= RC_UNKNOWN || rc >= RC_MAX || !CheckIsOutstanding(rc) ||
                    !ItemMatchesCategory(RANDO_SAVE_CHECKS[rc].randoItemId, cat, subcat)) {
                    continue;
                }
                f32 dx = elem->pos.x - playerPos.x;
                f32 dz = elem->pos.z - playerPos.z;
                f32 distSq = dx * dx + dz * dz;
                if (bestDistSq < 0.0f || distSq < bestDistSq) {
                    bestDistSq = distSq;
                }
            }
        }
    }

    // (b) Everything that IS a real actor.
    for (s32 category = 0; category < ACTORCAT_MAX; category++) {
        Actor* actor = gPlayState->actorCtx.actorLists[category].first;
        while (actor != nullptr) {
            RandoCheckId rc = ResolveActorCheck(actor);
            if (rc > RC_UNKNOWN && rc < RC_MAX && CheckIsOutstanding(rc) &&
                ItemMatchesCategory(RANDO_SAVE_CHECKS[rc].randoItemId, cat, subcat)) {
                f32 dx = actor->world.pos.x - playerPos.x;
                f32 dz = actor->world.pos.z - playerPos.z;
                f32 distSq = dx * dx + dz * dz;
                if (bestDistSq < 0.0f || distSq < bestDistSq) {
                    bestDistSq = distSq;
                }
            }
            actor = actor->next;
        }
    }
    if (bestDistSq < 0.0f) {
        return false;
    }
    if (outDist != nullptr) {
        *outDist = sqrtf(bestDistSq);
    }
    return true;
}

void ResetSignals() {
    sRoomHasTarget = false;
    sProximity = 0.0f;
    sRoomAlertFrames = 0;
    sLastRoomNum = -1;
    sLastRoomHadTarget = false;
}

} // namespace

// =============================================================================
// Public C API
// =============================================================================

extern "C" const char* Rando_DesireCompass_CategoryName(DesireCompassCategory cat) {
    if (cat < 0 || cat >= DCOMPASS_CAT_MAX) {
        return "?";
    }
    return kCategoryNames[cat];
}

extern "C" s32 Rando_DesireCompass_SubcategoryCount(DesireCompassCategory cat) {
    return (cat == DCOMPASS_CAT_KEYS) ? 2 : 1;
}

extern "C" u8 Rando_DesireCompass_IsAvailable(void) {
    return IS_RANDO ? 1 : 0;
}

extern "C" u8 Rando_DesireCompass_IsOwned(void) {
    NeiSaveData* nei = Nei_Save();
    return (nei != nullptr && nei->quartzOwned) ? 1 : 0;
}

extern "C" s32 Rando_DesireCompass_CountRemaining(DesireCompassCategory cat, s32 subcat) {
    if (!IS_RANDO || cat < 0 || cat >= DCOMPASS_CAT_MAX) {
        return 0;
    }
    s32 count = 0;
    for (s32 i = RC_UNKNOWN + 1; i < RC_MAX; i++) {
        RandoCheckId rc = (RandoCheckId)i;
        if (CheckIsOutstanding(rc) && ItemMatchesCategory(RANDO_SAVE_CHECKS[rc].randoItemId, cat, subcat)) {
            count++;
        }
    }
    return count;
}

extern "C" u8 Rando_DesireCompass_RequestActivation(DesireCompassCategory cat, s32 subcat) {
    if (!IS_RANDO || cat < 0 || cat >= DCOMPASS_CAT_MAX) {
        return 0;
    }
    if (!Rando_DesireCompass_IsOwned()) {
        return 0;
    }
    // Must survive the 3 hearts. Checked BEFORE the menu closes so a refusal can
    // keep the list open instead of silently doing nothing after the fade-out.
    if (gSaveContext.save.saveInfo.playerData.health <= DCOMPASS_HEALTH_COST) {
        return 0;
    }
    sPendingCat = (s32)cat;
    sPendingSubcat = subcat;

    NeiSaveData* nei = Nei_Save();
    if (nei != nullptr) {
        nei->quartzCategory = (uint8_t)cat;
        nei->quartzSubcat = (uint8_t)subcat;
    }
    return 1;
}

extern "C" void Rando_DesireCompass_Cancel(void) {
    sDeadlineMs = 0;
    sPendingCat = -1;
    sAttuneTimer = 0;
    ResetSignals();
}

extern "C" u8 Rando_DesireCompass_IsAttuning(void) {
    return (sAttuneTimer > 0) ? 1 : 0;
}

extern "C" u8 Rando_DesireCompass_IsActive(void) {
    return (sDeadlineMs != 0 && NowMs() < sDeadlineMs) ? 1 : 0;
}

extern "C" s32 Rando_DesireCompass_GetRemainingSeconds(void) {
    if (sDeadlineMs == 0) {
        return 0;
    }
    s64 remainMs = sDeadlineMs - NowMs();
    if (remainMs <= 0) {
        return 0;
    }
    return (s32)((remainMs + 999) / 1000);
}

extern "C" DesireCompassCategory Rando_DesireCompass_GetActiveCategory(void) {
    return sActiveCat;
}

extern "C" u8 Rando_DesireCompass_RoomHasTarget(void) {
    return sRoomHasTarget ? 1 : 0;
}

extern "C" s32 Rando_DesireCompass_RoomAlertFrames(void) {
    return sRoomAlertFrames;
}

extern "C" f32 Rando_DesireCompass_GetProximity(void) {
    return sProximity;
}

// =============================================================================
// Per-frame tick. The Quartz is not an equippable item, so this hook is its
// only heartbeat.
// =============================================================================

namespace {

void AttuneTick() {
    if (gPlayState == nullptr || gPlayState->pauseCtx.state != 0) {
        return; // wait until the menu has actually closed
    }
    Player* p = GET_PLAYER(gPlayState);
    if (p == nullptr) {
        return;
    }

    if (sAttuneTimer == 0) { // first gameplay frame after confirming
        sAttuneTimer = kAttuneFrames;
        Audio_PlaySfx(NA_SE_SY_TRE_BOX_APPEAR);
    }

    p->stateFlags1 |= PLAYER_STATE1_20; // in-item-cutscene lock
    p->actor.speed = 0.0f;              // MM has no Player::linearVelocity

    if ((sAttuneTimer % 3) == 0) {
        SpawnAttuneSparkles(p);
    }
    if ((sAttuneTimer % 6) == 0) {
        func_800AA000(50.0f, 80, 8, 4);
    }

    sAttuneTimer--;
    if (sAttuneTimer > 0) {
        return;
    }

    p->stateFlags1 &= ~PLAYER_STATE1_20;
    const DesireCompassCategory cat = (DesireCompassCategory)sPendingCat;
    const s32 subcat = sPendingSubcat;
    sPendingCat = -1;

    // Health could have dropped while attuning; refuse rather than kill.
    if (gSaveContext.save.saveInfo.playerData.health <= DCOMPASS_HEALTH_COST) {
        Audio_PlaySfx(NA_SE_SY_ERROR);
        return;
    }
    gSaveContext.save.saveInfo.playerData.health -= DCOMPASS_HEALTH_COST;

    sActiveCat = cat;
    sActiveSubcat = subcat;
    sDeadlineMs = NowMs() + (s64)DCOMPASS_DURATION_SECONDS * 1000;
    sNextBlipMs = 0;
    ResetSignals();

    Audio_PlaySfx(NA_SE_SY_CORRECT_CHIME);
}

void SensorTick() {
    if (sPendingCat >= 0) {
        AttuneTick();
        return;
    }
    if (sDeadlineMs == 0) {
        return;
    }
    if (NowMs() >= sDeadlineMs) { // expired
        sDeadlineMs = 0;
        ResetSignals();
        return;
    }
    if (gPlayState == nullptr || gPlayState->pauseCtx.state != 0) {
        return; // don't scan while paused
    }

    if (sRoomAlertFrames > 0) {
        sRoomAlertFrames--;
    }

    f32 dist = 0.0f;
    sRoomHasTarget = FindNearestLoaded(sActiveCat, sActiveSubcat, &dist);

    // Signal 1: entering a room that holds something announces itself once.
    const s8 room = gPlayState->roomCtx.curRoom.num;
    if (room != sLastRoomNum) {
        sLastRoomNum = room;
        sLastRoomHadTarget = false;
    }
    if (sRoomHasTarget && !sLastRoomHadTarget) {
        sLastRoomHadTarget = true;
        sRoomAlertFrames = 90; // ~1.5 s of on-screen indicator
        Audio_PlaySfx(NA_SE_SY_ATTENTION_ON);
        func_800AA000(120.0f, 150, 20, 40);
    }

    if (!sRoomHasTarget) {
        sProximity = 0.0f;
        return;
    }

    // Signal 2: hot/cold. Proximity 0..1, blip rate follows it.
    f32 t = (kFarDist - dist) / (kFarDist - kNearDist);
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }
    sProximity = t;

    const s64 periodMs = (s64)(900.0f - 780.0f * t); // 900 ms far -> 120 ms close
    const s64 now = NowMs();
    if (now >= sNextBlipMs) {
        // The urgent variant once you are basically on top of it.
        Audio_PlaySfx(t > 0.85f ? NA_SE_SY_WARNING_COUNT_E : NA_SE_SY_WARNING_COUNT_N);
        func_800AA000(50.0f, (u8)(60 + 120 * t), 8, 4);
        sNextBlipMs = now + periodMs;
    }
}

void RegisterDesireCompassTick() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayDrawWorldEnd>([]() { SensorTick(); });
}

static RegisterShipInitFunc dcTickInitFunc(RegisterDesireCompassTick, {});

} // namespace
