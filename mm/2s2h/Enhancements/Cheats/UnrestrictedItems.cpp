#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gCheats.UnrestrictedItems"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterUnrestrictedItems() {
    COND_VB_SHOULD(VB_ITEM_BE_RESTRICTED, CVAR, { *should = false; });
}

REGISTER_SHIP_INIT_FUNC(RegisterUnrestrictedItems, { CVAR_NAME });
