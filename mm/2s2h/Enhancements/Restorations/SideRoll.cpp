#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Restorations.SideRoll"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSideRoll() {
    COND_VB_SHOULD(VB_PATCH_SIDEROLL, CVAR, { *should = false; });
}

REGISTER_SHIP_INIT_FUNC(RegisterSideRoll, { CVAR_NAME });
