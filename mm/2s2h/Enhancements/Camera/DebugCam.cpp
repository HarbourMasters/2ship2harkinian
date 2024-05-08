#include "DebugCam.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "CameraUtils.h"

extern "C" {
#include <macros.h>
#include <functions.h>
extern PlayState* gPlayState;
extern PlayState* sCamPlayState;
extern f32 Camera_ScaledStepToCeilF(f32 target, f32 cur, f32 stepScale, f32 minDiff);
extern Vec3f Camera_CalcUpVec(s16 pitch, s16 yaw, s16 roll);
}

// Static Data Used For Free Camera
static f32 sDebugCamDistTarget = 185;
static f32 sDebugCamPitch = 0.0f;
static f32 sDebugCamYaw = 0.0f;
static f32 sDebugCamRoll = 0.0f;

void Camera_DebugCam(Camera* camera) {

    Vec3f* eye = &camera->eye;
    Vec3f* at = &camera->at;
    Vec3f* eyeNext = &camera->eyeNext;
    VecGeo eyeAdjustment;
    VecGeo originalEyeGeo = OLib_Vec3fDiffToVecGeo(at, eye);

    // "If Refresh Static Params"
    if (true) {
        sDebugCamPitch = originalEyeGeo.pitch;
        sDebugCamYaw = originalEyeGeo.yaw;
        sDebugCamRoll = camera->roll;
        sDebugCamDistTarget = camera->dist;
    }

    s32 controllerPort = CVarGetInteger("gEnhancements.Camera.DebugCam.Port", 2) - 1;
    if (controllerPort == 0) {
        // Disable Link Inputs
        GET_PLAYER(gPlayState)->stateFlags1 |= PLAYER_STATE1_20;
    }

    /* 
        Camera Speed
     */

    f32 camSpeed = CVarGetFloat("gEnhancements.Camera.DebugCam.CameraSpeed", 0.5f);
    if (CHECK_BTN_ANY(sCamPlayState->state.input[controllerPort].cur.button, BTN_A | BTN_B | BTN_L)) {
        camSpeed *= 3.0f;
    }

    /* 
        Camera Roll
     */

    sDebugCamRoll += (CHECK_BTN_ANY(sCamPlayState->state.input[controllerPort].cur.button, BTN_DLEFT) - CHECK_BTN_ANY(sCamPlayState->state.input[controllerPort].cur.button, BTN_DRIGHT)) * 1200.0f * camSpeed;

    if (sDebugCamRoll > DEG_TO_BINANG(180.0f)) {
        sDebugCamRoll -= DEG_TO_BINANG(360.0f);
    }
    if (sDebugCamRoll < 0) {
        sDebugCamRoll += DEG_TO_BINANG(360.0f);
    }

    camera->roll = sDebugCamRoll;

    /* 
        Camera distance
     */

    // Transition speed set to be responsive
    f32 transitionSpeed = camSpeed * 200.0f;
    sDebugCamDistTarget += (CHECK_BTN_ANY(sCamPlayState->state.input[controllerPort].cur.button, BTN_DDOWN) - CHECK_BTN_ANY(sCamPlayState->state.input[controllerPort].cur.button, BTN_DUP)) * 1200.0f * camSpeed;
    sDebugCamDistTarget = CLAMP_MIN(sDebugCamDistTarget, 1.0f);
    // Smooth step camera away to max camera distance.
    camera->dist = Camera_ScaledStepToCeilF(sDebugCamDistTarget, camera->dist, transitionSpeed / (ABS(sDebugCamDistTarget - camera->dist) + transitionSpeed), 0.0f);

    /* 
        Set Camera Pitch and Yaw
     */
    f32 yawDiff = -sCamPlayState->state.input[controllerPort].cur.right_stick_x * 10.0f * (CVarGetFloat("gEnhancements.Camera.FreeLook.CameraSensitivity.X", 1.0f));
    f32 pitchDiff = sCamPlayState->state.input[controllerPort].cur.right_stick_y * 10.0f * (CVarGetFloat("gEnhancements.Camera.FreeLook.CameraSensitivity.Y", 1.0f));

    sDebugCamYaw += yawDiff * (CVarGetInteger("gEnhancements.Camera.FreeLook.InvertXAxis", 0) ? -1 : 1);
    sDebugCamPitch += pitchDiff * (CVarGetInteger("gEnhancements.Camera.FreeLook.InvertYAxis", 1) ? 1 : -1);

    s16 maxPitch = DEG_TO_BINANG(89.0f);
    s16 minPitch = DEG_TO_BINANG(-89.0f);

    if (sDebugCamPitch > maxPitch) {
        sDebugCamPitch = maxPitch;
    }
    if (sDebugCamPitch < minPitch) {
        sDebugCamPitch = minPitch;
    }

    // Setup new camera angle based on the calculations from right stick inputs
    eyeAdjustment = OLib_Vec3fDiffToVecGeo(at, eyeNext);
    eyeAdjustment.r = camera->dist;
    eyeAdjustment.yaw = sDebugCamYaw;
    eyeAdjustment.pitch = sDebugCamPitch;

    *eyeNext = OLib_AddVecGeoToVec3f(at, &eyeAdjustment);

    /* 
        Camera Movement
     */
    
    // Movement differences from the point of view of the camera. Use max stick value for buttons scale
    Vec3f posDiff;
    posDiff.x = sCamPlayState->state.input[controllerPort].cur.stick_x * camSpeed;
    posDiff.y = (CHECK_BTN_ANY(sCamPlayState->state.input[controllerPort].cur.button, BTN_Z) - CHECK_BTN_ANY(sCamPlayState->state.input[controllerPort].cur.button, BTN_R)) * 120.0f * camSpeed;
    posDiff.z = -sCamPlayState->state.input[controllerPort].cur.stick_y * camSpeed;

    // Adjust movement to camera's current direction
    Vec3f camPosDiff;
    camPosDiff.x = camPosDiff.y = camPosDiff.z = 0.0f;

    VecGeo diffGeo = OLib_Vec3fDiffToVecGeo(at, eyeNext);
    camera->up = Camera_CalcUpVec(diffGeo.pitch, diffGeo.yaw, camera->roll);
    Vec3f normX;
    Vec3f normY = camera->up;
    Vec3f normZ = OLib_Vec3fDistNormalize(at, eyeNext);

    // Cross Product
    normX.x = normY.y * normZ.z - normY.z * normZ.y;
    normX.y = normY.z * normZ.x - normY.x * normZ.z;
    normX.z = normY.x * normZ.y - normY.y * normZ.x;

    // Account for flipped normal calculation due to camera roll
    if (normY.y < 0.0f) {
        posDiff.x = -posDiff.x;
        posDiff.y = -posDiff.y;
    }

    Math_Vec3f_Scale(&normX, posDiff.x);
    Math_Vec3f_Scale(&normY, posDiff.y);
    Math_Vec3f_Scale(&normZ, posDiff.z);

    Math_Vec3f_Sum(&camPosDiff, &normX, &camPosDiff);
    Math_Vec3f_Sum(&camPosDiff, &normY, &camPosDiff);
    Math_Vec3f_Sum(&camPosDiff, &normZ, &camPosDiff);

    // "Move" camera eye around with controls
    Math_Vec3f_Sum(at, &camPosDiff, at);
    Math_Vec3f_Sum(eyeNext, &camPosDiff, eyeNext);

    *eye = *eyeNext;
}

static uint32_t freeCamVBHookId = 0;

void RegisterDebugCam() {
    if (freeCamVBHookId) {
        GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(freeCamVBHookId);
        freeCamVBHookId = 0;
        // Re-enable Player Inputs
        GET_PLAYER(gPlayState)->stateFlags1 &= ~PLAYER_STATE1_20;
    }

    if (CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0)) {
        freeCamVBHookId = REGISTER_VB_SHOULD(GI_VB_USE_CUSTOM_CAMERA, {
            Camera* camera = static_cast<Camera*>(opt);
            Camera_DebugCam(camera);
            *should = false;
        });
    }
}
