#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

void Rando::ActorBehavior::InitTriforceHuntBehavior() {
    bool shouldRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_TRIFORCE_PIECES] == RO_GENERIC_YES;

    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, shouldRegister, [](Actor* actor) {
        if (!gPlayState) {
            return;
        }

        if (gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces ==
            RANDO_SAVE_OPTIONS[RO_TRIFORCE_PIECES_REQUIRED]) {
            GameInteractor::Instance->events.emplace_back(GIEventTransition{ .entrance = ENTRANCE(TERMINA_FIELD, 0),
                                                                             .cutsceneIndex = 0xFFF7,
                                                                             .transitionTrigger = TRANS_TRIGGER_START,
                                                                             .transitionType = TRANS_TYPE_FADE_BLACK });
        }

        // Blocks the ability to beat the game through killing Majora until all Triforce Pieces are found.
        if (gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces >=
                RANDO_SAVE_OPTIONS[RO_TRIFORCE_PIECES_REQUIRED] &&
            !Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_MAJORA)) {
            Rando::GiveItem(RI_SOUL_MAJORA);
        }
    });
}
