#include "Mouse.h"

#include "ship/Context.h"
#include "ShipInit.hpp"
#include "GameInteractor/GameInteractor.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "z64player.h"
#include "z64camera.h"
#include "functions.h"
#include "macros.h"
#include <libultraship/bridge/consolevariablebridge.h>

static MouseCoords current = {};
static MouseCaptureGameState gameState = {};
static bool mPrevShieldHandled = false;

#ifdef __cplusplus
extern "C" {
#endif

void Mouse_Update() {
    Ship::Coords coords = Ship::Context::GetRawInstance()->GetWindow()->GetMouseDelta();
    current.x = coords.x;
    current.y = coords.y;
}

MouseCoords Mouse_GetDelta() {
    return current;
}

MouseCoords Mouse_GetPos() {
    Ship::Coords coords = Ship::Context::GetRawInstance()->GetWindow()->GetMousePos();
    return { coords.x, coords.y };
}

void Mouse_SetCursorPos(s32 x, s32 y) {
    Ship::Context::GetRawInstance()->GetWindow()->SetMousePos({ x, y });
}

bool Mouse_IsCaptured() {
    return Ship::Context::GetRawInstance()->GetWindow()->IsMouseCaptured();
}

void Mouse_SetForceCapture(bool isEnabled, bool value) {
    gameState.isCaptureForced = isEnabled;
    gameState.forcedCaptureValue = value;
    Mouse_UpdateCaptureByState();
}

bool InferCaptureFromState(MouseCaptureGameState state) {
    bool inBenMenu = Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetMenuOrMenubarVisible();

    if (state.isCaptureForced) {
        return state.forcedCaptureValue;
    } else if (inBenMenu || state.inKaleido) {
        return false;
    } else if (!state.gameStarted) {
        return false;
    } else {
        return true;
    }
}

void HandleForcing() {
    if (
        !gameState.isCaptureForced
        || !Ship::Context::GetRawInstance()->GetWindow()->GetMouseStateManager()->ShouldAutoCaptureMouse()
    ) {
        gameState.isCaptureForced = true;
        gameState.forcedCaptureValue = !Ship::Context::GetRawInstance()->GetWindow()->IsMouseCaptured();
        return;
    }

    // Detect if we are forcing the opposite of what the game state would normally dictate
    MouseCaptureGameState unforcedState = gameState;
    unforcedState.isCaptureForced = false;
    bool isOppositeForced = (gameState.forcedCaptureValue == InferCaptureFromState(unforcedState));

    if (isOppositeForced) {
        gameState.forcedCaptureValue = !gameState.forcedCaptureValue;
    } else {
        gameState.isCaptureForced = false;
    }
}

void Mouse_ForceToggleCapture() {
    HandleForcing();
    Mouse_UpdateCaptureByState();
}

void Mouse_UpdateCaptureByState() {
    bool capture = InferCaptureFromState(gameState);

    if (
        gameState.isCaptureForced
        || Ship::Context::GetRawInstance()->GetWindow()->GetMouseStateManager()->ShouldAutoCaptureMouse()
        || !capture
    ) {
        Ship::Context::GetRawInstance()->GetWindow()->SetMouseCapture(capture);
    }
}

void HandleKaleidoCapture(PauseContext* pauseCtx) {
    bool prev = gameState.inKaleido;
    switch (pauseCtx->state) {
        case PAUSE_STATE_MAIN: {
            gameState.inKaleido = true;
            break;
        }
        case PAUSE_STATE_UNPAUSE_SETUP: {
            gameState.inKaleido = false;
            break;
        }
    }
    if (prev != gameState.inKaleido) {
        Mouse_UpdateCaptureByState();
    }
}

static void HandleShieldAim(Player* player, PlayState* play, f32* xStick, f32* yStick, bool* handled) {
    if (!Mouse_IsCaptured()) {
        *handled = mPrevShieldHandled = false;
        return;
    }

    MouseCoords mouseDelta = Mouse_GetDelta();
    bool hasDelta = (mouseDelta.x != 0 || mouseDelta.y != 0);

    if (!hasDelta) {
        *handled = mPrevShieldHandled = (mPrevShieldHandled && *xStick == 0 && *yStick == 0);
        return;
    }

    bool hasFocusActor = (player->focusActor != NULL);
    bool camRotate =
        CVarGetInteger("gEnhancements.Mouse.Shielding.CameraControl", 1) && !hasFocusActor;

    if (camRotate) {
        // Hook shield to camera view
        Camera* camera = GET_ACTIVE_CAM(play);
        VecGeo viewOffset = OLib_Vec3fDiffToVecGeo(&camera->at, &camera->eye);
        s16 camYaw = viewOffset.yaw + 0x8000;
        s16 camPitch = viewOffset.pitch;

        player->actor.shape.rot.y = camYaw;
        player->yaw = camYaw;
        player->upperLimbRot.y = 0;
        player->upperLimbRot.x = camPitch;
    } else {
        // Plain aim
        f32 xDelta = ((f32)mouseDelta.x) * 60 *
                     CVarGetFloat("gEnhancements.Camera.FirstPerson.GyroSensitivityX", 1.0f) *
                     GameInteractor_InvertControl(GI_INVERT_SHIELD_X);
        f32 yDelta = -((f32)mouseDelta.y) * 60 *
                     CVarGetFloat("gEnhancements.Camera.FirstPerson.GyroSensitivityY", 1.0f) *
                     GameInteractor_InvertControl(GI_INVERT_SHIELD_Y);

        s16 rotYTarget = CLAMP(player->upperLimbRot.y + (s16)xDelta, -60 * 120, 60 * 120);
        s16 rotXTarget = CLAMP(player->upperLimbRot.x + (s16)yDelta, -60 * 180, 0xDAC);

        player->upperLimbRot.y = rotYTarget;
        player->upperLimbRot.x = rotXTarget;
    }
    *handled = mPrevShieldHandled = true;
}

void HandleShieldCameraControl(Camera* camera, s16 viewYaw) {
    if (!mPrevShieldHandled || !CVarGetInteger("gEnhancements.Mouse.Shielding.CameraControl", 1)) {
        return;
    }

    f32 shoulderOffset = CVarGetFloat("gEnhancements.Mouse.Shielding.ShoulderOffset", -12.0f);
    VecGeo lateral = { .r = shoulderOffset, .pitch = 0, .yaw = viewYaw + 0x4000 };
    Vec3f offset = OLib_VecGeoToVec3f(&lateral);

    camera->at.x += offset.x;
    camera->at.z += offset.z;
}

static void HandleTelescopeAim(s16* inputX, s16* inputY) {
    if (!Mouse_IsCaptured()) {
        return;
    }
    MouseCoords d = Mouse_GetDelta();
    *inputX -= (s16)(d.x * 12.0f * CVarGetFloat("gEnhancements.Camera.FirstPerson.GyroSensitivityX", 1.0f) *
                     GameInteractor_InvertControl(GI_INVERT_FIRST_PERSON_GYRO_X));
    *inputY += (s16)(d.y * 12.0f * CVarGetFloat("gEnhancements.Camera.FirstPerson.GyroSensitivityY", 1.0f) *
                     GameInteractor_InvertControl(GI_INVERT_FIRST_PERSON_GYRO_Y));
}

static void HandleOvershoulderAim(bool* should, Player* player) {
    if (!Mouse_IsCaptured()) { return; }
    MouseCoords d = Mouse_GetDelta();
    if (d.y != 0) {
        // FIXME: to remove? Why was it there?
        // player->actor.focus.rot.x += d.y * 8;
        f32 yBuf = d.y * 12.0f * CVarGetFloat("gEnhancements.Camera.FirstPerson.GyroSensitivityY", 1.0f);
        yBuf *= -GameInteractor_InvertControl(GI_INVERT_FIRST_PERSON_GYRO_Y);
        player->actor.focus.rot.x = CLAMP(player->actor.focus.rot.x - (s16)yBuf, -60 * 240, 60 * 240);
        *should = false;
    }
}

static void HandleDekuCharge(Player* player) {
    if (!Mouse_IsCaptured()) { return; }
    MouseCoords d = Mouse_GetDelta();
    if (d.x != 0) {
        player->yaw -= (s16)(d.x * 40 *
                             CVarGetFloat("gEnhancements.Camera.RightStick.CameraSensitivity.X", 1.0f) *
                             GameInteractor_InvertControl(GI_INVERT_CAMERA_RIGHT_STICK_X));
    }
}

void RegisterMouseRelatedHooks() {
    {
        static bool autoCapture = false;
        bool newAutoCapture = CVarGetInteger("gSettings.EnableMouse", 0) && CVarGetInteger("gSettings.AutoCaptureMouse", 1);
        if (autoCapture != newAutoCapture) {
            autoCapture = newAutoCapture;
            Ship::Context::GetRawInstance()->GetWindow()->SetAutoCaptureMouse(autoCapture);
        }
    }
    COND_HOOK(
        OnKaleidoUpdate,
        true,
        HandleKaleidoCapture
    );
    COND_HOOK(
        OnSaveLoad,
        true,
        [](s16 fileNum) {
            gameState.gameStarted = true;
            Mouse_UpdateCaptureByState();
        }
    );
    COND_HOOK(
        OnConsoleLogoUpdate,
        true,
        []() {
            if (gameState.gameStarted) {
                gameState.gameStarted = false;
                gameState.inKaleido = false;
                Mouse_UpdateCaptureByState();
            }
        }
    );
    COND_HOOK(
        OnPlayerShieldControl,
        CVarGetInteger("gSettings.EnableMouse", 0) && CVarGetInteger("gEnhancements.Mouse.Shielding.Enabled", 0),
        HandleShieldAim
    );
    COND_HOOK(
        OnPlayerTelescopeAim,
        CVarGetInteger("gSettings.EnableMouse", 0) && CVarGetInteger("gEnhancements.Camera.FirstPerson.GyroEnabled", 0),
        HandleTelescopeAim
    );
    COND_VB_SHOULD(
        VB_SHOULD_OVERSHOULDER_AIM,
        CVarGetInteger("gSettings.EnableMouse", 0) && CVarGetInteger("gEnhancements.Camera.FirstPerson.GyroEnabled", 0),
        { HandleOvershoulderAim(should, va_arg(args, Player*)); }
    );
    COND_HOOK(
        OnPlayerDekuCharge,
        CVarGetInteger("gSettings.EnableMouse", 0) && !CVarGetInteger("gEnhancements.Camera.Mouse.DisableThirdPerson", 0),
        HandleDekuCharge
    );
}

static RegisterShipInitFunc initFunc(
    RegisterMouseRelatedHooks,
    {
        "gSettings.EnableMouse",
        "gSettings.AutoCaptureMouse",
        "gEnhancements.Mouse.Shielding.Enabled",
        "gEnhancements.Camera.FirstPerson.GyroEnabled",
        "gEnhancements.Camera.Mouse.DisableThirdPerson"
    }
);

#ifdef __cplusplus
} // extern "C"
#endif
