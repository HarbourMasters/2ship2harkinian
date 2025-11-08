#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

#define CVAR_NAME CVAR_ENHANCEMENT("Graphics.DisableBlackBars")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterDisableBlackBars() {
    COND_VB_SHOULD(VB_DISABLE_LETTERBOX, CVAR, { *should = true; });
}

static RegisterShipInitFunc initFunc(RegisterDisableBlackBars, { CVAR_NAME });
