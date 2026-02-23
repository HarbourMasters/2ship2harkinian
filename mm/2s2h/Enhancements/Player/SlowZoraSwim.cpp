#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/Enhancements/Enhancements.h"

#define CVAR_NAME "gEnhancements.Player.SlowZoraSwim"
#define CVAR CVarGetInteger(CVAR_NAME, SLOW_ZORA_SWIM_OFF)

static void HandleSlowZoraSwim(f32* speed, f32* speedTarget) {
    bool isZPressed = CHECK_BTN_ALL(gPlayState->state.input[0].cur.button, BTN_Z);
    bool shouldSwimSlow = (CVAR == SLOW_ZORA_SWIM_HOLD_SLOW) ? isZPressed : !isZPressed;
    if (shouldSwimSlow) {
        if (*speed == 16.0f) {
            *speed = 8.0f;
        }
        *speedTarget = 6.0f;
    }
}

static void RegisterSlowZoraSwim() {
    COND_VB_SHOULD(VB_ZORA_LINK_SWIM_SLOWLY, CVAR, {
        f32* speed = va_arg(args, f32*);
        f32* speedTarget = va_arg(args, f32*);
        HandleSlowZoraSwim(speed, speedTarget);
    });
}

static RegisterShipInitFunc initFunc(RegisterSlowZoraSwim, { CVAR_NAME });
