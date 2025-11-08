#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

#define CVAR_NAME CVAR_ENHANCEMENT("Restorations.SideRoll")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSideRoll() {
    COND_VB_SHOULD(VB_PATCH_SIDEROLL, CVAR, { *should = false; });
}

static RegisterShipInitFunc initFunc(RegisterSideRoll, { CVAR_NAME });
