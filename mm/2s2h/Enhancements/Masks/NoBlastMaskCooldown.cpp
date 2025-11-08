#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

#define CVAR_NAME CVAR_ENHANCEMENT("Masks.NoBlastMaskCooldown")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterNoBlastMaskCooldown() {
    COND_VB_SHOULD(VB_SET_BLAST_MASK_COOLDOWN_TIMER, CVAR, { *should = false; });
}

static RegisterShipInitFunc initFunc(RegisterNoBlastMaskCooldown, { CVAR_NAME });
