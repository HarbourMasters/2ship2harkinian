#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

extern "C" {
#include "variables.h"
extern f32 D_8085D958[2];
}

#define CVAR_NAME CVAR_CHEAT("LongerFlowerGlide")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterLongerFlowerGlide() {
    if (CVAR) {
        D_8085D958[0] = 99999.9f;
        D_8085D958[1] = 99999.9f;
    } else {
        D_8085D958[0] = 600.0f;
        D_8085D958[1] = 960.0f;
    }
}

static RegisterShipInitFunc initFunc(RegisterLongerFlowerGlide, { CVAR_NAME });
