#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/Enhancements/Enhancements.h"

#define CVAR_NAME "gEnhancements.Cycle.DisableMagicDropsWithChateau"
#define CVAR CVarGetInteger(CVAR_NAME, DISABLE_MAGIC_DROPS_OFF)

void RegisterDisableMagicDropsWithChateau() {
    COND_VB_SHOULD(VB_ITEM00_GET_DROP_ID, CVAR != DISABLE_MAGIC_DROPS_OFF, {
        if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI)) {
            return;
        }

        s16* dropId = va_arg(args, s16*);

        if (*dropId == ITEM00_MAGIC_JAR_SMALL || *dropId == ITEM00_MAGIC_JAR_BIG) {
            if (CVAR == DISABLE_MAGIC_DROPS_RECOVERY_HEART) {
                *dropId = ITEM00_RECOVERY_HEART;
            } else if (CVAR == DISABLE_MAGIC_DROPS_GREEN_RUPEE) {
                *dropId = ITEM00_RUPEE_GREEN;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterDisableMagicDropsWithChateau, { CVAR_NAME });
