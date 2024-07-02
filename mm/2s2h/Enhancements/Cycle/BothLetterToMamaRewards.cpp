#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "functions.h"
#include "z64message.h"
#include "variables.h"
extern PlayState* gPlayState;
}

void RegisterBothLetterToMamaRewards() {

    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(ACTOR_EN_AL, [](Actor* actor) {
        if (CVarGetInteger("gEnhancements.Cycle.BothLetterToMamaRewards", 0)) {
            /*
             * Player has given letter to Madame Aroma this cycle but does not have the Postman's Hat. This mirrors
             * how the Postman himself directly checks the inventory for the Postman's Hat, although there does
             * exist the Bomber's Notebook flag marking the Postman's quest as complete
             * (WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_POSTMANS_HAT). Also wait until msgMode is free,
             * otherwise this can softlock when executed during the dialog that sets Madame Aroma's flags.
             */
            if ((CHECK_WEEKEVENTREG(WEEKEVENTREG_57_04) && INV_CONTENT(ITEM_MASK_POSTMAN) != ITEM_MASK_POSTMAN) &&
                gPlayState->msgCtx.msgMode == MSGMODE_NONE) {
                Actor_OfferGetItemFar(actor, gPlayState, GI_MASK_POSTMAN);
                Message_BombersNotebookQueueEvent(gPlayState, BOMBERS_NOTEBOOK_EVENT_RECEIVED_POSTMANS_HAT);
                Message_BombersNotebookQueueEvent(gPlayState, BOMBERS_NOTEBOOK_EVENT_MET_POSTMAN);
            }
        }
    });

    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(ACTOR_EN_PM, [](Actor* actor) {
        if (CVarGetInteger("gEnhancements.Cycle.BothLetterToMamaRewards", 0)) {
            /*
             * Postman is fleeing, but the player has not obtained Madame Aroma's reward. Also wait until msgMode
             * is free, otherwise this can softlock when executed during the dialog that sets the Postman's flags.
             */
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_90_01) && !CHECK_WEEKEVENTREG(WEEKEVENTREG_57_08) &&
                gPlayState->msgCtx.msgMode == MSGMODE_NONE) {
                SET_WEEKEVENTREG(WEEKEVENTREG_57_04); // Cycle flag that marks Madame Aroma's reward as obtained
                SET_WEEKEVENTREG(WEEKEVENTREG_57_08); // Persistent flag that marks Madame Aroma's reward as obtained
                Actor_OfferGetItemFar(actor, gPlayState, GI_CHATEAU_BOTTLE);
                Message_BombersNotebookQueueEvent(gPlayState, BOMBERS_NOTEBOOK_EVENT_DELIVERED_PRIORITY_MAIL);
                Message_BombersNotebookQueueEvent(gPlayState, BOMBERS_NOTEBOOK_EVENT_MET_MADAME_AROMA);
            }
        }
    });
}
