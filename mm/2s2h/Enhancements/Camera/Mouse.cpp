#include "Mouse.h"

#include "Context.h"
#include "ShipInit.hpp"
#include "GameInteractor/GameInteractor.h"
#include "public/bridge/consolevariablebridge.h"

#define MOUSE_ENABLED (Ship::Context::GetInstance()->GetWindow()->IsMouseCaptured() && CVarGetInteger("gEnhancements.Camera.Mouse.Enabled", 0))

static MouseCoords current;

#ifdef __cplusplus
extern "C" {
#endif

void Mouse_Update() {
    Ship::Coords coords = Ship::Context::GetInstance()->GetWindow()->GetMouseDelta();
    current.x = coords.x;
    current.y = coords.y;
}

MouseCoords Mouse_GetDelta() {
    return current;
}

MouseCoords Mouse_GetPos() {
    Ship::Coords coords = Ship::Context::GetInstance()->GetWindow()->GetMousePos();
    return { coords.x, coords.y };
}

void Mouse_SetCursorPos(s32 x, s32 y) {
    Ship::Context::GetInstance()->GetWindow()->SetMousePos({ x, y });
}

bool Mouse_IsCaptured() {
    return Ship::Context::GetInstance()->GetWindow()->IsMouseCaptured();
}

static s8 iterMouse = 0;
static f32 mouseQuickspinX[5] = {};
static f32 mouseQuickspinY[5] = {};
static u8 quickspinCount = 0;

// copypasted logic from SoH
void UpdateQuickspinCount(Input* input) {
    quickspinCount = (quickspinCount + 1) % 5;
    mouseQuickspinX[quickspinCount] = current.x;
    mouseQuickspinY[quickspinCount] = current.y;
}

bool HandleQuickspin(bool* should, s8* iter2, s8* sp3C) {
    s8 temp1;
    s8 temp2;
    s32 i;

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
        MOUSE_ENABLED && CVarGetInteger("gEnhancements.Mouse.Quickspin.Enable", 0),
        { HandleQuickspin(should, va_arg(args, s8*), va_arg(args, s8*)); }
    );
    COND_HOOK(
        OnPassPlayerInputs,
        MOUSE_ENABLED && CVarGetInteger("gEnhancements.Mouse.Quickspin.Enable", 0),
        UpdateQuickspinCount
    );
}

// OnGameStateMainStart (capture toggle on start?)
// OnKaleidoUpdate (forced disable capture in kaleido menu)

static RegisterShipInitFunc initFunc(RegisterQuickspinFunc, { "gEnhancements.Mouse.Quickspin.Enable" });

#ifdef __cplusplus
} // extern "C"
#endif
