#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
}

#define CVAR_NAME "gEnhancements.Cutscenes.SkipMiscInteractions"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSkipDekuSalesman() {
    // prevents him from doing his "fly in" animation
    COND_ID_HOOK(ShouldActorInit, ACTOR_EN_SELLNUTS, CVAR, [](Actor* actor, bool* should) {
        if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_73_04)) {
            SET_WEEKEVENTREG(WEEKEVENTREG_73_04);
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterSkipDekuSalesman, { CVAR_NAME });
