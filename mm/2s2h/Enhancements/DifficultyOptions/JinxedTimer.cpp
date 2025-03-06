#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.DifficultyOptions.JinxedTimer"
#define CVAR CVarGetInteger(CVAR_NAME, 60)

void RegisterJinxedTimer() {
    COND_VB_SHOULD(VB_MODIFY_JINX_TIMER, CVAR < 60, {
        s32* timer = va_arg(args, s32*);

        if (CVAR == 0) {
            *timer = 0;
        } else {
            // Prevent the timer from exceeding CVAR * 20 and resetting to 1200 if hit again while jinxed.
            if (*timer > CVAR * 20) {
                *timer = CVAR * 20;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterJinxedTimer, { CVAR_NAME });
