#include "FreeLook.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

typedef struct {
    /* 0x0 */ s16 val;
    /* 0x2 */ s16 param;
} CameraModeValue; // size = 0x4

typedef struct {
    /* 0x0 */ s16 funcId;
    /* 0x2 */ s16 numValues;
    /* 0x4 */ CameraModeValue* values;
} CameraMode; // size = 0x8

/**
 * Flags:
 * (flags & 0xF): Priority (lower value has higher priority)
 * (flags & 0x40000000): Store previous setting and bgCamData, also ignores water checks
 * (flags & 0x80000000): Set camera setting based on bg/scene data and reset action function state
 */
typedef struct {
    /* 0x0 */ u32 validModes;
    /* 0x4 */ u32 flags;
    /* 0x8 */ CameraMode* cameraModes;
} CameraSetting; // size = 0xC


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

// Camera will reload its paramData. Usually that means setting the read-only data from what is stored in
// CameraModeValue arrays. Although sometimes some read-write data is reset as well
#define RELOAD_PARAMS(camera) ((camera->animState == 0) || (camera->animState == 10) || (camera->animState == 20))

/**
 * Camera data is stored in both read-only data and OREG as s16, and then converted to the appropriate type during
 * runtime. If a small f32 is being stored as an s16, it is common to store that value 100 times larger than the
 * original value. This is then scaled back down during runtime with the CAM_RODATA_UNSCALE macro.
 */
#define CAM_RODATA_SCALE(x) ((x)*100.0f)
#define CAM_RODATA_UNSCALE(x) ((x)*0.01f)

// Load the next value from camera read-only data stored in CameraModeValue
#define GET_NEXT_RO_DATA(values) ((values++)->val)
// Load the next value and scale down from camera read-only data stored in CameraModeValue
#define GET_NEXT_SCALED_RO_DATA(values) CAM_RODATA_UNSCALE(GET_NEXT_RO_DATA(values))

// Static Data Used For Free Camera
static f32 sCamX = 0.0f;
static f32 sCamY = 0.0f;
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
    VecGeo eyeAdjustment;
    Parallel1ReadOnlyData* roData = &camera->paramData.para1.roData;
    f32 playerHeight;

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

    f32 camDiffX = -sCamPlayState->state.input[0].cur.right_stick_x * 10.0f * (CVarGetFloat("gEnhancements.Camera.FreeLook.CameraSensitivity.X", 1.0f));
    f32 camDiffY = sCamPlayState->state.input[0].cur.right_stick_y * 10.0f * (CVarGetFloat("gEnhancements.Camera.FreeLook.CameraSensitivity.Y", 1.0f));

    sCamX += camDiffX * (CVarGetInteger("gEnhancements.Camera.FreeLook.InvertXAxis", 0) ? -1 : 1);
    sCamY += camDiffY * (CVarGetInteger("gEnhancements.Camera.FreeLook.InvertYAxis", 1) ? 1 : -1);

    s16 maxY = DEG_TO_BINANG(CVarGetFloat("gEnhancements.Camera.FreeLook.MaxY", 72.0f));
    s16 minY = DEG_TO_BINANG(CVarGetFloat("gEnhancements.Camera.FreeLook.MinY", -49.0f));

    if (sCamY > maxY) {
        sCamY = maxY;
    }
    if (sCamY < minY) {
        sCamY = minY;
    }

    f32 distTarget = CVarGetInteger("gEnhancements.Camera.FreeLook.MaxCameraDistance", roData->unk_04);
    f32 transitionSpeed = CVarGetInteger("gEnhancements.Camera.FreeLook.TransitionSpeed", 25);
    // Smooth step camera away to max camera distance. Camera collision is calculated later
    camera->dist = Camera_ScaledStepToCeilF(distTarget, camera->dist, transitionSpeed / (ABS(distTarget - camera->dist) + transitionSpeed), 0.0f);

    // Setup new camera angle based on the calculations from stick inputs
    eyeAdjustment = OLib_Vec3fDiffToVecGeo(at, eyeNext);
    eyeAdjustment.r = camera->dist;
    eyeAdjustment.yaw = sCamX;
    eyeAdjustment.pitch = sCamY;

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

static uint32_t freeLookCameraSettingChangeHookId = 0;
static uint32_t freeLookCameraVBHookId = 0;

void RegisterCameraFreeLook() {
    if (freeLookCameraVBHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::ShouldVanillaBehavior>(freeLookCameraVBHookId);
        freeLookCameraVBHookId = 0;
    }

    if (CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0)) {
        freeLookCameraVBHookId = REGISTER_VB_SHOULD(GI_VB_FREE_LOOK, {
                Camera* camera = static_cast<Camera*>(opt);
                f32 camX = sCamPlayState->state.input[0].cur.right_stick_x * 10.0f;
                f32 camY = sCamPlayState->state.input[0].cur.right_stick_y * 10.0f;
                // Set sCamX/sCamY to camera positions after mode which disables free cam
                if (!sCanFreeLook && (fabsf(camX) >= 15.0f || fabsf(camY) >= 15.0f)) {
                    VecGeo eyeAdjustment = OLib_Vec3fDiffToVecGeo(&camera->at, &camera->eye);
                    sCamX = eyeAdjustment.yaw;
                    sCamY = eyeAdjustment.pitch;
                    sCanFreeLook = true;
                }
                *should = sCanFreeLook;
        });
    }

    if (freeLookCameraSettingChangeHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnCameraChangeModeFlags>(freeLookCameraSettingChangeHookId);
        freeLookCameraSettingChangeHookId = 0;
    }

    if (CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0)) {
        freeLookCameraSettingChangeHookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnCameraChangeModeFlags>([](Camera* camera) {
            UpdateFreeLookState(camera);
        });
    }
}
