#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "src/overlays/actors/ovl_En_Invadepoh/z_en_invadepoh.h"

void EnInvadepoh_InvasionHandler_SuccessCutscene(EnInvadepoh* enInvadepoh, PlayState* play);
void EnInvadepoh_InvasionHandler_SuccessEnd(EnInvadepoh* enInvadepoh, PlayState* play);
}

#define CVAR_NAME "gEnhancements.Minigames.SkipRanchInvasion"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// Needed to prevent debug warp from always skipping to 5:15 AM
static bool sInvasionSkipped = false;

static void EnInvadepoh_SkipToReward(Actor* actor, bool* should) {
    if ((CURRENT_DAY != 1) || (CURRENT_TIME >= CLOCK_TIME(5, 15)) || (CURRENT_TIME < CLOCK_TIME(2, 30)) ||
        (EN_INVADEPOH_GET_TYPE(actor) != EN_INVADEPOH_TYPE_INVASION_HANDLER)) {
        return;
    }

    EnInvadepoh* enInvadepoh = (EnInvadepoh*)actor;
    if (enInvadepoh->actionFunc == EnInvadepoh_InvasionHandler_SuccessEnd) {
        return;
    }

    EnInvadepoh_InvasionHandler_SuccessCutscene(enInvadepoh, gPlayState);
    SET_WEEKEVENTREG(WEEKEVENTREG_DEFENDED_AGAINST_ALIENS);
    sInvasionSkipped = true;
    *should = false;
}

static void AdvanceToEnd(s16 sceneId, s8 spawnNum) {
    if (sInvasionSkipped) {
        gSaveContext.save.time = CLOCK_TIME(5, 15);
        sInvasionSkipped = false;
    }
}

static void RegisterSkipRanchInvasion() {
    COND_ID_HOOK(ShouldActorInit, ACTOR_EN_INVADEPOH, CVAR, EnInvadepoh_SkipToReward);

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EN_INVADEPOH, CVAR, EnInvadepoh_SkipToReward);

    COND_ID_HOOK(OnSceneInit, SCENE_F01, CVAR, AdvanceToEnd);
}

static RegisterShipInitFunc initFunc(RegisterSkipRanchInvasion, { CVAR_NAME });
