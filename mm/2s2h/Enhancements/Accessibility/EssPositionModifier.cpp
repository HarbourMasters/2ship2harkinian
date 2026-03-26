#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenPort.h"
#include <math.h>

extern "C" {
#include "variables.h"
}

#define CVAR_NAME "gEnhancements.Player.EssPosition.Enable"
#define CVAR_BTN_NAME "gEnhancements.Player.EssPosition.Btn"
#define CVAR CVarGetInteger(CVAR_NAME, 0)
#define CVAR_BTN CVarGetInteger(CVAR_BTN_NAME, BTN_CUSTOM_MODIFIER2)

#define ESS_MAGNITUDE 17

void RegisterEssPositionModifier() {
    COND_HOOK(OnPassPlayerInputs, CVAR, [](Input* input) {
        if (CHECK_BTN_ALL(input->cur.button, CVAR_BTN)) {
            s8 x = input->cur.stick_x;
            s8 y = input->cur.stick_y;

            if (x != 0 || y != 0) {
                float mag = sqrtf((float)(x * x + y * y));
                s8 essX = (s8)((x / mag) * ESS_MAGNITUDE);
                s8 essY = (s8)((y / mag) * ESS_MAGNITUDE);

                input->cur.stick_x = essX;
                input->cur.stick_y = essY;
                input->rel.stick_x = essX;
                input->rel.stick_y = essY;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterEssPositionModifier, { CVAR_NAME });
