#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Cycle.DisableMagicDropsWithChateau"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterDisableMagicDropsWithChateau() {
    COND_VB_SHOULD(VB_ITEM00_GET_DROP_ID, CVAR, {
        if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
            return;
        }

        s16* dropId = va_arg(args, s16*);

        if (*dropId == ITEM00_MAGIC_JAR_SMALL || *dropId == ITEM00_MAGIC_JAR_BIG) {
            *dropId = ITEM00_RECOVERY_HEART;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterDisableMagicDropsWithChateau, { CVAR_NAME });
