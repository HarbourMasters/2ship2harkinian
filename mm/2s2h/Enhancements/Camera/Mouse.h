// TODO: Maybe this file should be part of BenPort
#pragma once

#include "ultratypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MouseDelta {
    s32 x;
    s32 y;
} MouseDelta;

void Mouse_Update();
MouseDelta Mouse_GetDelta();
void Mouse_SetCursorPos(s32 x, s32 y);
bool Mouse_IsCaptured();

#ifdef __cplusplus
} // extern "C"
#endif
