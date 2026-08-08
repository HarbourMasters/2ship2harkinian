// TODO: Maybe this file should be part of BenPort
#pragma once

#include "ultratypes.h"
#include "z64camera.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MouseCoords {
    s32 x;
    s32 y;
} MouseCoords;

typedef struct MouseCaptureGameState {
    bool inKaleido;
    bool gameStarted;
    bool isCaptureForced;
    bool forcedCaptureValue;
} MouseCaptureGameState;

void Mouse_Update();
MouseCoords Mouse_GetDelta();
MouseCoords Mouse_GetPos();
void Mouse_SetCursorPos(s32 x, s32 y);
bool Mouse_IsCaptured();
void Mouse_SetForceCapture(bool isEnabled, bool value);
void Mouse_ForceToggleCapture();
void Mouse_UpdateCaptureByState();
void HandleShieldCameraControl(Camera* camera, s16 viewYaw);

#ifdef __cplusplus
} // extern "C"
#endif
