#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Timesavers.SkipBottleCatchCutscene"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSkipBottleCatchCutscene() {
    // Player_Action_68: Catching items in bottle.
    // When catching items in a bottle, the game plays a hold-up animation, shows a textbox, and plays a fanfare.
    // Skip the presentation entirely -- the item goes into the bottle and the swing animation finishes naturally into
    // idle.
    COND_VB_SHOULD(VB_PLAY_BOTTLE_CATCH_CS, CVAR, { *should = false; });
}

static RegisterShipInitFunc initFunc(RegisterSkipBottleCatchCutscene, { CVAR_NAME });