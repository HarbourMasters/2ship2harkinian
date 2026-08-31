#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "src/overlays/actors/ovl_En_Invadepoh/z_en_invadepoh.h"
#include "z64horse.h"

void EnInvadepoh_InvasionHandler_SetupSuccessEnd(EnInvadepoh* enInvadepoh);
}

#define CVAR_NAME "gEnhancements.Minigames.SkipRanchInvasion"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

static void EnInvadepoh_SkipToReward(EnInvadepoh* enInvadepoh) {
    gPlayState->nextEntrance = ENTRANCE(ROMANI_RANCH, 6);
    gSaveContext.nextCutsceneIndex = 0;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_73;
    gSaveContext.nextTransitionType = TRANS_TYPE_72;
    D_801BDAA0 = true;
    gHorseIsMounted = false;
    EnInvadepoh_InvasionHandler_SetupSuccessEnd(enInvadepoh);
    SET_WEEKEVENTREG(WEEKEVENTREG_DEFENDED_AGAINST_ALIENS);
}

static void AdvanceToEnd(s16 sceneId, s8 spawnNum) {
    if (spawnNum == 6) {
        gSaveContext.save.time = CLOCK_TIME(5, 15);
    }
}

static void RegisterSkipRanchInvasion() {
    COND_VB_SHOULD(VB_ALIENS_INVADE_RANCH, CVAR, {
        if (*should) {
            EnInvadepoh* enInvadepoh = va_arg(args, EnInvadepoh*);
            EnInvadepoh_SkipToReward(enInvadepoh);
            *should = false;
        }
    });

    COND_ID_HOOK(OnSceneInit, SCENE_F01, CVAR, AdvanceToEnd);
}

static RegisterShipInitFunc initFunc(RegisterSkipRanchInvasion, { CVAR_NAME });
