#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Player.PreventDiveOverWater"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterPreventDiveOverWater() {
    COND_VB_SHOULD(VB_LINK_DIVE_OVER_WATER, CVAR, { *should = false; });
}

REGISTER_SHIP_INIT_FUNC(RegisterPreventDiveOverWater, { CVAR_NAME });
