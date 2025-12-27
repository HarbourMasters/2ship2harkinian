#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

#define CVAR_NAME CVAR_ENHANCEMENT("A11y.NoScreenFlashForEnemyKill")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

static void RegisterDisableScreenFlash() {
    COND_VB_SHOULD(VB_FLASH_SCREEN_FOR_ENEMY_KILL, CVAR, { *should = false; });
}

static RegisterShipInitFunc initFunc(RegisterDisableScreenFlash, { CVAR_NAME });
