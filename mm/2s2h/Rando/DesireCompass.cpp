// =============================================================================
// Desire Compass — brain implementation. See DesireCompass.h for the contract.
//
// Locate strategy (per the design): the compass only scans while active, so we
// iterate the live actor lists every call rather than maintaining a registry.
// Each loaded actor is resolved to its RandoCheckId by a single recognizer that
// covers every locatable carrier:
//   - Obj* families / EnCow / ObjTsubo  -> Rando::ActorBehavior::GetObjectRandoCheckId
//   - the floating rando item           -> ACTOR_EN_ITEM00 w/ params==ITEM00_NOTHING, RC in home.rot.z
//   - chests                            -> ACTOR_EN_BOX, RC in home.rot.x
//   - EnGamelupy / EnElforg / EnTakaraya-> RC in home.rot.x
// Checks with no in-scene carrier (songs, minigame rewards, hardcoded-RC NPCs)
// are intentionally not locatable and simply never match.
// =============================================================================

#include "2s2h/Rando/Rando.h"
#include "2s2h/Rando/StaticData/StaticData.h"
#include "2s2h/Rando/ActorBehavior/ActorBehavior.h"
#include "2s2h/Rando/DesireCompass.h"

extern "C" {
#include "z64.h"
#include "variables.h"
#include "functions.h"
#include "macros.h"
extern PlayState* gPlayState;
}

// Mirror of the private macros in the owning behaviors (kept local so we don't
// have to pull their headers). CustomItem stashes its RandoCheckId in home.rot.z
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
    // "Skills" = ability items. Only RI_ABILITY_SWIM exists today; the range
    // check keeps future RI_ABILITY_* additions covered automatically.
    return id == RI_ABILITY_SWIM;
}

// Does the item placed at this check belong to (cat, subcat)? Uses the raw
// placed RandoItemId (progressive resolution isn't needed to classify).
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
            // Major = the RITYPE_MAJOR/MASK bucket minus the slices that get
            // their own dedicated categories (souls / triforce / skills).
            return (type == RITYPE_MAJOR || type == RITYPE_MASK) && !IsBossSoul(id) &&
                   !IsOtherSoul(id) && !IsTriforce(id) && !IsSkill(id);
        case DCOMPASS_CAT_OTHER:
            // Everything not claimed by one of the 7 buckets above
            // (RITYPE_LESSER / HEALTH / SKULLTULA_TOKEN / STRAY_FAIRY, etc.).
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

// A check is a live target if it's shuffled into the seed and not yet obtained
// (permanently or this 3-day cycle).
bool CheckIsOutstanding(RandoCheckId rc) {
    if (rc <= RC_UNKNOWN || rc >= RC_MAX) {
        return false;
    }
    const RandoSaveCheck& sc = RANDO_SAVE_CHECKS[rc];
    return sc.shuffled && !sc.obtained && !sc.cycleObtained;
}

// -----------------------------------------------------------------------------
// Actor -> RandoCheckId recognizer
// -----------------------------------------------------------------------------

RandoCheckId ResolveActorCheck(Actor* actor) {
    // 1. Object-extension: the general Actor*->RC attachment used by every
    //    Obj* family, EnCow, and the ObjTsubo fallback.
    RandoCheckId rc = Rando::ActorBehavior::GetObjectRandoCheckId(actor);
    if (rc > RC_UNKNOWN && rc < RC_MAX) {
        return rc;
    }

    switch (actor->id) {
        case ACTOR_EN_ITEM00:
            // The floating rando replacement item — the dominant carrier for
            // freestanding / pot-drops / tree-drops / collectibles.
            if (actor->params == ITEM00_NOTHING) {
                return DCOMPASS_RC_FROM_ROT_Z(actor);
            }
            return RC_UNKNOWN;
        case ACTOR_EN_BOX:       // chests
        case ACTOR_EN_GAMELUPY:  // Deku playground rupees
        case ACTOR_EN_ELFORG:    // stray fairies
        case ACTOR_EN_TAKARAYA:  // treasure chest game
            return DCOMPASS_RC_FROM_ROT_X(actor);
        default:
            return RC_UNKNOWN;
    }
}

} // namespace

// =============================================================================
// Public C API
// =============================================================================

extern "C" u8 Rando_DesireCompass_IsAvailable(void) {
    return IS_RANDO ? 1 : 0;
}

extern "C" s32 Rando_DesireCompass_SubcategoryCount(DesireCompassCategory cat) {
    switch (cat) {
        case DCOMPASS_CAT_KEYS:
            return 2; // small keys, boss keys
        default:
            return 1; // no meaningful subdivision yet
    }
}

extern "C" u8 Rando_DesireCompass_LocateNearest(DesireCompassCategory cat, s32 subcat, Vec3f* outPos,
                                                f32* outDist) {
    if (!IS_RANDO || gPlayState == NULL || cat < 0 || cat >= DCOMPASS_CAT_MAX) {
        return 0;
    }
    Player* player = GET_PLAYER(gPlayState);
    if (player == NULL) {
        return 0;
    }
    Vec3f playerPos = player->actor.world.pos;

    f32 bestDistSq = -1.0f;
    Vec3f bestPos = { 0.0f, 0.0f, 0.0f };

    for (s32 category = 0; category < ACTORCAT_MAX; category++) {
        Actor* actor = gPlayState->actorCtx.actorLists[category].first;
        while (actor != NULL) {
            RandoCheckId rc = ResolveActorCheck(actor);
            if (rc > RC_UNKNOWN && rc < RC_MAX && CheckIsOutstanding(rc) &&
                ItemMatchesCategory(RANDO_SAVE_CHECKS[rc].randoItemId, cat, subcat)) {
                f32 dx = actor->world.pos.x - playerPos.x;
                f32 dz = actor->world.pos.z - playerPos.z;
                f32 distSq = dx * dx + dz * dz;
                if (bestDistSq < 0.0f || distSq < bestDistSq) {
                    bestDistSq = distSq;
                    bestPos = actor->world.pos;
                }
            }
            actor = actor->next;
        }
    }

    if (bestDistSq < 0.0f) {
        return 0;
    }
    if (outPos != NULL) {
        *outPos = bestPos;
    }
    if (outDist != NULL) {
        *outDist = sqrtf(bestDistSq);
    }
    return 1;
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
