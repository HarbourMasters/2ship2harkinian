#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenPort.h"

extern "C" {
#include "variables.h"
#include "z64player.h"
}

#define CVAR_NAME "gEnhancements.Player.InstantQuickSpin.Enable"
#define CVAR_BTN_NAME "gEnhancements.Player.InstantQuickSpin.Btn"
#define CVAR CVarGetInteger(CVAR_NAME, 0)
#define CVAR_BTN CVarGetInteger(CVAR_BTN_NAME, BTN_CUSTOM_MODIFIER2)

void RegisterInstantQuickSpin() {
    COND_VB_SHOULD(VB_PLAYER_CAN_SPIN_ATTACK, CVAR, {
        if (CHECK_BTN_ALL(gPlayState->state.input[0].cur.button, CVAR_BTN)) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterInstantQuickSpin, { CVAR_NAME });
