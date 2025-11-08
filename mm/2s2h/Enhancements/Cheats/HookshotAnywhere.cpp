#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

#define CVAR_NAME CVAR_CHEAT("HookshotAnywhere")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterHookshotAnywhere() {
    COND_VB_SHOULD(VB_BE_HOOKSHOT_SURFACE, CVAR, { *should = true; });
}

static RegisterShipInitFunc initFunc(RegisterHookshotAnywhere, { CVAR_NAME });
