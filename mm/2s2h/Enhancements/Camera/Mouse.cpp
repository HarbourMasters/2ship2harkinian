#include "Mouse.h"

#include "Context.h"

static MouseDelta current;

#ifdef __cplusplus
extern "C" {
#endif

void Mouse_Update() {
    Ship::Coords coords = Ship::Context::GetInstance()->GetWindow()->GetMouseDelta();
    current.x = coords.x;
    current.y = coords.y;
}

MouseDelta Mouse_GetDelta() {
    return current;
}

void Mouse_SetCursorPos(s32 x, s32 y) {
    Ship::Context::GetInstance()->GetWindow()->SetMousePos({ x, y });
}

bool Mouse_IsCaptured() {
    return Ship::Context::GetInstance()->GetWindow()->IsMouseCaptured();
}

#ifdef __cplusplus
} // extern "C"
#endif
