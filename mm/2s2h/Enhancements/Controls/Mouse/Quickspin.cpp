#include <libultraship/bridge/consolevariablebridge.h>
#include "GameInteractor/GameInteractor.h"
#include "ShipInit.hpp"
#include "Mouse.h"

#define MOUSE_ENABLED (Mouse_IsCaptured() && CVarGetInteger("gEnhancements.Camera.Mouse.Enabled", 0))

#ifdef __cplusplus
extern "C" {
#endif

static s8 iterMouse = 0;
static f32 mouseQuickspinX[5] = {};
static f32 mouseQuickspinY[5] = {};
static u8 quickspinCount = 0;

// copypasted logic from SoH
void UpdateQuickspinCount(Input* input) {
    MouseCoords current = Mouse_GetDelta();
    quickspinCount = (quickspinCount + 1) % 5;
    mouseQuickspinX[quickspinCount] = current.x;
    mouseQuickspinY[quickspinCount] = current.y;
}

bool HandleQuickspin(bool* should, s8* iter2, s8* sp3C) {
    s8 temp1;
    s8 temp2;
    s32 i;
    if (!MOUSE_ENABLED) {
        return *should = false;
    }

    for (i = 0; i < 4; i++, iter2++) {
        f32 relY = mouseQuickspinY[i + 1] - mouseQuickspinY[i];
        f32 relX = mouseQuickspinX[i + 1] - mouseQuickspinX[i];
        s16 aTan = Math_Atan2S(relY, -relX);
        iterMouse = (u16)(aTan + 0x2000) >> 9;
        if ((*iter2 = iterMouse) < 0) {
            return *should = false;
        }
        *iter2 *= 2;
    }
    temp1 = sp3C[0] - sp3C[1];
    if (ABS(temp1) < 10) {
        return *should = false;
    }
    iter2 = &sp3C[1];
    for (i = 1; i < 3; i++, iter2++) {
        temp2 = *iter2 - *(iter2 + 1);
        if ((ABS(temp2) < 10) || (temp2 * temp1 < 0)) {
            return *should = false;
        }
    }

    return *should = true;
}

void RegisterQuickspinFunc() {
    COND_VB_SHOULD(
        VB_SHOULD_QUICKSPIN,
        CVarGetInteger("gEnhancements.Mouse.Quickspin.Enable", 0),
        { HandleQuickspin(should, va_arg(args, s8*), va_arg(args, s8*)); }
    );
    COND_HOOK(
        OnPassPlayerInputs,
        CVarGetInteger("gEnhancements.Mouse.Quickspin.Enable", 0),
        UpdateQuickspinCount
    );
}

static RegisterShipInitFunc initFunc(RegisterQuickspinFunc, { "gEnhancements.Mouse.Quickspin.Enable" });

#ifdef __cplusplus
} // extern "C"
#endif
