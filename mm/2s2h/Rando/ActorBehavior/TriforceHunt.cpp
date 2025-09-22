#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

bool isGameplayPaused() {
    return (Player_InBlockingCsMode(gPlayState, GET_PLAYER(gPlayState)) || gPlayState->pauseCtx.state != 0 ||
            gPlayState->msgCtx.msgMode != MSGMODE_NONE);
}

bool creditsWarpActive = false;

void Rando::ActorBehavior::InitTriforceHuntBehavior() {
    bool shouldRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_TRIFORCE_PIECES] == RO_GENERIC_YES;

    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, shouldRegister, [](Actor* actor) {
        if (!gPlayState) {
            return;
        }

        if (gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces >=
            RANDO_SAVE_OPTIONS[RO_TRIFORCE_PIECES_REQUIRED]) {
            creditsWarpActive = true;
        }

        if (creditsWarpActive && !isGameplayPaused()) {
            creditsWarpActive = false;
            gPlayState->nextEntrance = ENTRANCE(TERMINA_FIELD, 0);
            gSaveContext.nextCutsceneIndex = 0xFFF7;
            gPlayState->transitionTrigger = TRANS_TRIGGER_START;
        }

        // Blocks the ability to beat the game through killing Majora until all Triforce Pieces are found.
        if (gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces >=
                RANDO_SAVE_OPTIONS[RO_TRIFORCE_PIECES_REQUIRED] &&
            !Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_MAJORA)) {
            Rando::GiveItem(RI_SOUL_MAJORA);
        }
    });
}
