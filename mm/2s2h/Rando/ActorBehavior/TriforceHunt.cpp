#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "variables.h"
#include "functions.h"
}

bool isGameplayPaused() {
    return (Player_InBlockingCsMode(gPlayState, GET_PLAYER(gPlayState)) || gPlayState->pauseCtx.state != 0 ||
            gPlayState->msgCtx.msgMode != 0)
               ? true
               : false;
}

bool creditsWarpActive = false;

void Rando::ActorBehavior::InitTriforceHuntBehavior() {
    bool shouldRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_TRIFORCE_PIECES] == RO_GENERIC_YES;

    REGISTER_VB_SHOULD(VB_WARP_TO_CREDITS, {
        if (!gPlayState) {
            return;
        }

        if (gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces ==
            gSaveContext.save.shipSaveInfo.rando.requiredTriforcePieces) {
            creditsWarpActive = true;
        }
    });

    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, shouldRegister, [](Actor* actor) {
        if (creditsWarpActive && !isGameplayPaused()) {
            creditsWarpActive = false;
            gPlayState->nextEntrance = 0x5400;
            gSaveContext.nextCutsceneIndex = 0xFFF7;
            gPlayState->transitionTrigger = TRANS_TRIGGER_START;
        }
        if (gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces ==
            gSaveContext.save.shipSaveInfo.rando.requiredTriforcePieces && 
            !Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_MAJORA)) {
            Rando::GiveItem(RI_SOUL_MAJORA);
        }
    });
}
