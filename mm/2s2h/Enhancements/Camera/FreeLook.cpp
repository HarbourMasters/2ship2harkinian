#include "FreeLook.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "CameraUtils.h"

extern "C" {
#include <macros.h>
#include <functions.h>
extern PlayState* gPlayState;
extern PlayState* sCamPlayState;
extern f32 Camera_ScaledStepToCeilF(f32 target, f32 cur, f32 stepScale, f32 minDiff);
extern s16 Camera_ScaledStepToCeilS(s16 target, s16 cur, f32 stepScale, s16 minDiff);
extern s32 Camera_BgCheckInfo(Camera* camera, Vec3f* from, CameraCollision* to);
extern void Camera_ResetActionFuncState(Camera* camera, s32 mode);
extern void func_800CBFA4(Camera* camera, Vec3f* arg1, Vec3f* arg2, s32 arg3);
extern CameraSetting sCameraSettings[];
extern s32 sCameraInterfaceFlags;
}

// Static Data Used For Free Camera
static bool sCanFreeLook = false;

void UpdateFreeLookState(Camera* camera) {
    if (CAM_MODE_TARGET <= camera->mode && camera->mode <= CAM_MODE_BATTLE) {
        sCanFreeLook = false;
    }

    if (CAM_MODE_FIRSTPERSON <= camera->mode && camera->mode <= CAM_MODE_CLIMBZ) {
        sCanFreeLook = false;
    }

    if (camera->mode == CAM_MODE_HANGZ) {
        sCanFreeLook = false;
    }

    if (CAM_MODE_BOOMERANG <= camera->mode && camera->mode <= CAM_MODE_ZORAFINZ) {
        sCanFreeLook = false;
    }

    if (gPlayState != nullptr && Player_InCsMode(gPlayState)) {
        sCanFreeLook = false;
    }
}

// Function based on several camera functions, including Camera_Parallel1
bool Camera_FreeLook(Camera* camera) {
    Vec3f* eye = &camera->eye;
    Vec3f* at = &camera->at;
    Vec3f* eyeNext = &camera->eyeNext;
    Player* player = GET_PLAYER(gPlayState);
    VecGeo eyeAdjustment = OLib_Vec3fDiffToVecGeo(at, eye);
    Parallel1ReadOnlyData* roData = &camera->paramData.para1.roData;
    f32 playerHeight;
    f32 pitch = eyeAdjustment.pitch;
    f32 yaw = eyeAdjustment.yaw;

    // Smooth step camera 'at' towards player
    f32 playerYOffset = Player_GetHeight(player) / 1.2f;
    if (player->rideActor != NULL) {
        playerYOffset /= 2;
    }
    at->x = Camera_ScaledStepToCeilF(player->actor.world.pos.x, at->x, 0.5f, 1.0f);
    at->y = Camera_ScaledStepToCeilF(player->actor.world.pos.y + playerYOffset, at->y, 0.5f, 1.0f);
    at->z = Camera_ScaledStepToCeilF(player->actor.world.pos.z, at->z, 0.5f, 1.0f);

    // Equivalent to 'Camera_GetFocalActorHeight'
    playerHeight = Player_GetHeight(player);

    // Camera param reloading based on Camera_Parallel1, only updating roData as only that seems necessary
    if (RELOAD_PARAMS(camera)) {
        CameraModeValue* values = sCameraSettings[camera->setting].cameraModes[camera->mode].values;
        f32 yNormal = (0.8f - ((68.0f / playerHeight) * -0.2f));

        roData->unk_00 = GET_NEXT_SCALED_RO_DATA(values) * playerHeight * yNormal;
        roData->unk_04 = GET_NEXT_SCALED_RO_DATA(values) * playerHeight * yNormal;
        roData->unk_08 = GET_NEXT_SCALED_RO_DATA(values) * playerHeight * yNormal;
        roData->unk_20 = CAM_DEG_TO_BINANG(GET_NEXT_RO_DATA(values));
        roData->unk_22 = CAM_DEG_TO_BINANG(GET_NEXT_RO_DATA(values));
        roData->unk_0C = GET_NEXT_RO_DATA(values);
        roData->unk_10 = GET_NEXT_RO_DATA(values);
        roData->unk_14 = GET_NEXT_RO_DATA(values);
        roData->unk_18 = GET_NEXT_SCALED_RO_DATA(values);
        roData->interfaceFlags = GET_NEXT_RO_DATA(values);
        roData->unk_1C = GET_NEXT_SCALED_RO_DATA(values);
        roData->unk_24 = GET_NEXT_RO_DATA(values);
    }

    // 1 acts as a default value of sorts for sCameraInterfaceFlags
    sCameraInterfaceFlags = 1;

    Camera_ResetActionFuncState(camera, camera->mode);

    f32 yawDiff = -sCamPlayState->state.input[0].cur.right_stick_x * 10.0f *
                  (CVarGetFloat("gEnhancements.Camera.RightStick.CameraSensitivity.X", 1.0f));
    f32 pitchDiff = sCamPlayState->state.input[0].cur.right_stick_y * 10.0f *
                    (CVarGetFloat("gEnhancements.Camera.RightStick.CameraSensitivity.Y", 1.0f));

    yaw += yawDiff * GameInteractor_InvertControl(GI_INVERT_CAMERA_RIGHT_STICK_X);
    pitch += pitchDiff * -GameInteractor_InvertControl(GI_INVERT_CAMERA_RIGHT_STICK_Y);

    s16 maxPitch = DEG_TO_BINANG(CVarGetFloat("gEnhancements.Camera.FreeLook.MaxPitch", 72.0f));
    s16 minPitch = DEG_TO_BINANG(CVarGetFloat("gEnhancements.Camera.FreeLook.MinPitch", -49.0f));

    if (pitch > maxPitch) {
        pitch = maxPitch;
    }
    if (pitch < minPitch) {
        pitch = minPitch;
    }

    f32 distTarget = CVarGetInteger("gEnhancements.Camera.FreeLook.MaxCameraDistance", roData->unk_04);
    f32 transitionSpeed = CVarGetInteger("gEnhancements.Camera.FreeLook.TransitionSpeed", 25);
    // Smooth step camera away to max camera distance. Camera collision is calculated later
    camera->dist = Camera_ScaledStepToCeilF(distTarget, camera->dist,
                                            transitionSpeed / (ABS(distTarget - camera->dist) + transitionSpeed), 0.0f);

    // Setup new camera angle based on the calculations from stick inputs
    eyeAdjustment.r = camera->dist;
    eyeAdjustment.yaw = yaw;
    eyeAdjustment.pitch = pitch;

    *eyeNext = OLib_AddVecGeoToVec3f(at, &eyeAdjustment);
    // Apply new camera angle only when camera active
    if (camera->status == CAM_STATUS_ACTIVE) {
        *eye = *eyeNext;
        // Adjust camera for collision with floors, walls and ceilings.
        func_800CBFA4(camera, at, eye, 0);
    }

    // 65.0f based on value from SoH
    f32 fovTarget = 65.0f;
    camera->fov = Camera_ScaledStepToCeilF(fovTarget, camera->fov, camera->fovUpdateRate, 0.1f);
    camera->roll = Camera_ScaledStepToCeilS(0, camera->roll, 0.5f, 5);

    return true;
}

bool Camera_CanFreeLook(Camera* camera) {
    f32 camX = sCamPlayState->state.input[0].cur.right_stick_x * 10.0f;
    f32 camY = sCamPlayState->state.input[0].cur.right_stick_y * 10.0f;
    if (!sCanFreeLook && (fabsf(camX) >= 15.0f || fabsf(camY) >= 15.0f)) {
        sCanFreeLook = true;
    }
    return sCanFreeLook;
}

static uint32_t freeLookCameraSettingChangeHookId = 0;
static uint32_t freeLookCameraVBHookId = 0;

void RegisterCameraFreeLook() {
    if (freeLookCameraVBHookId) {
        GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(
            freeLookCameraVBHookId);
        freeLookCameraVBHookId = 0;
    }

    if (freeLookCameraSettingChangeHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnCameraChangeModeFlags>(
            freeLookCameraSettingChangeHookId);
        freeLookCameraSettingChangeHookId = 0;
    }

    if (CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0)) {
        freeLookCameraVBHookId = REGISTER_VB_SHOULD(GI_VB_USE_CUSTOM_CAMERA, {
            Camera* camera = static_cast<Camera*>(opt);
            switch (sCameraSettings[camera->setting].cameraModes[camera->mode].funcId) {
                case CAM_FUNC_NORMAL0:
                case CAM_FUNC_NORMAL1:
                case CAM_FUNC_NORMAL3:
                case CAM_FUNC_NORMAL4:
                case CAM_FUNC_JUMP2:
                case CAM_FUNC_JUMP3:
                case CAM_FUNC_BATTLE1:
                case CAM_FUNC_UNIQUE2:
                    if (Camera_CanFreeLook(camera)) {
                        Camera_FreeLook(camera);
                        *should = false;
                    }
                    break;
                default:
                    break;
            }
        });
        freeLookCameraSettingChangeHookId =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnCameraChangeModeFlags>(
                [](Camera* camera) { UpdateFreeLookState(camera); });
    }
}
