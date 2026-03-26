#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#ifdef __APPLE__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

extern "C" {
#include "variables.h"
}

#define CVAR_NAME "gEnhancements.Player.HalfStickEnable"
#define CVAR_HALF_LEFT_KEY "gEnhancements.Player.HalfStickLeftKey"
#define CVAR_HALF_RIGHT_KEY "gEnhancements.Player.HalfStickRightKey"
#define CVAR_HALF_UP_KEY "gEnhancements.Player.HalfStickUpKey"
#define CVAR_HALF_DOWN_KEY "gEnhancements.Player.HalfStickDownKey"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define HALF_STICK_VALUE 35

static inline int clampStick(int val) {
    if (val > 127)
        return 127;
    if (val < -128)
        return -128;
    return val;
}

void RegisterHalfStickModifier() {
    COND_HOOK(OnPassPlayerInputs, CVAR, [](Input* input) {
        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        int32_t halfLeftKey = CVarGetInteger(CVAR_HALF_LEFT_KEY, 0);
        int32_t halfRightKey = CVarGetInteger(CVAR_HALF_RIGHT_KEY, 0);
        int32_t halfUpKey = CVarGetInteger(CVAR_HALF_UP_KEY, 0);
        int32_t halfDownKey = CVarGetInteger(CVAR_HALF_DOWN_KEY, 0);

        int sx = input->cur.stick_x;
        int sy = input->cur.stick_y;

        if (halfLeftKey > 0 && keystate[halfLeftKey]) {
            sx -= HALF_STICK_VALUE;
        }
        if (halfRightKey > 0 && keystate[halfRightKey]) {
            sx += HALF_STICK_VALUE;
        }
        if (halfUpKey > 0 && keystate[halfUpKey]) {
            sy += HALF_STICK_VALUE;
        }
        if (halfDownKey > 0 && keystate[halfDownKey]) {
            sy -= HALF_STICK_VALUE;
        }

        input->cur.stick_x = (s8)clampStick(sx);
        input->cur.stick_y = (s8)clampStick(sy);
    });
}

static RegisterShipInitFunc initFunc(RegisterHalfStickModifier, { CVAR_NAME });
