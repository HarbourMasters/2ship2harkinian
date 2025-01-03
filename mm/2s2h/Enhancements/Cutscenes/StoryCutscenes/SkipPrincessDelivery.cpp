#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
#include "overlays/actors/ovl_En_Dnp/z_en_dnp.h"
}

#define CVAR_NAME "gEnhancements.Cutscenes.SkipStoryCutscenes"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSkipPrincessDelivery() {
    COND_ID_HOOK(OnActorUpdate, ACTOR_EN_DNP, CVAR, [](Actor* actor) {
        EnDnp* princess = (EnDnp*)actor;

        // Check if princess is released from bottle and has grown to full size
        if (DEKU_PRINCESS_GET_TYPE(actor) == DEKU_PRINCESS_TYPE_RELEASED_FROM_BOTTLE &&
            (s32)(actor->scale.x * 10000.0f) >= 85) {

            // Set the flag that would normally be set after the dialogue
            SET_WEEKEVENTREG(WEEKEVENTREG_23_20);

            // Set up transition to Deku Palace throne room
            gPlayState->nextEntrance = ENTRANCE(DEKU_KINGS_CHAMBER, 3);
            gSaveContext.nextCutsceneIndex = 0;
            gPlayState->transitionTrigger = TRANS_TRIGGER_START;
            gPlayState->transitionType = TRANS_TYPE_FADE_BLACK;
            gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterSkipPrincessDelivery, { CVAR_NAME });
