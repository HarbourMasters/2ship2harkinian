#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
}

#define CVAR_NAME "gEnhancements.Fixes.DekuButlerFixShockLoopAnimation"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterDekuButlerFixShockLoopAnimation() {
    COND_VB_SHOULD(VB_DEKU_BUTLER_FIX_SHOCK_ANIM, CVAR, { *should = false; });
}

static RegisterShipInitFunc initFunc(RegisterDekuButlerFixShockLoopAnimation, { CVAR_NAME });
