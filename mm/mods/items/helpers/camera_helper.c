/**
 * camera_helper.c - First-person aiming and camera utilities
 */

#include "camera_helper.h"
#include "functions.h"
#include "macros.h"
#include "objects/object_link_boy/object_link_boy.h"
#include "2s2h/GameInteractor/GameInteractor.h" // GameInteractor_InvertControl + GI_INVERT_FIRST_PERSON_AIM_*

extern bool Player_IsZTargeting(Player* this);

// Champion's Tunic Bullet Time factor (defined in extended_equipment.c).
// When < 1.0f, Bullet Time is active — skip first-person camera so third-person
// Z-target view is kept and items fall back to shape.rot.y for aim direction.
extern f32 gChampionSlowFactor;

// First-person aim state for the custom-item reticle. OoT poked Player.unk_6AD
// (aim mode) and Player.unk_834 (aim-ready timer); MM's Player has neither, and
// MM drives first-person via its own camera — so these live as module state
// (there is only one logical player) instead of corrupting native Player fields.
s8 gNeiFpAimMode;       // was player->unk_6AD; shared aim signal read by equip_champion.c
static s16 sFpAimTimer; // was player->unk_834 (gates the reticle draw)

// Untextured reticle triangle. OoT's gLinkAdultHookshotReticle texture/vtx have
// no MM equivalent, so the reticle is drawn as a flat prim-colored triangle.
static Vtx sReticleVtx[3] = {
    { { { -22, -18, 0 }, 0, { 0, 0 }, { 0, 0, 0, 255 } } },
    { { { 22, -18, 0 }, 0, { 0, 0 }, { 0, 0, 0, 255 } } },
    { { { 0, 22, 0 }, 0, { 0, 0 }, { 0, 0, 0, 255 } } },
};

void FirstPerson_Init(Player* player, PlayState* play) {
    gNeiFpAimMode = 2; // weapon aiming mode
    // Skip FIRST_PERSON flag during Bullet Time — our third-person camera handles it
    if (gChampionSlowFactor >= 1.0f) {
        player->stateFlags1 |= PLAYER_STATE1_FIRST_PERSON;
    }
    player->stateFlags1 |= PLAYER_STATE1_ITEM_IN_HAND;
    player->stateFlags1 |= PLAYER_STATE1_READY_TO_FIRE;
    sFpAimTimer = 14;
    Player_ZeroSpeedXZ(player);
}

void FirstPerson_Update(Player* player, PlayState* play) {
    gNeiFpAimMode = 2;

    if (sFpAimTimer > 10) {
        sFpAimTimer--;
    } else if (sFpAimTimer == 0) {
        sFpAimTimer = 1;
    }

    // Skip FIRST_PERSON flag during Bullet Time — prevents flip-flop with our code
    if (gChampionSlowFactor >= 1.0f) {
        player->stateFlags1 |= PLAYER_STATE1_FIRST_PERSON;
    }
    player->stateFlags1 |= PLAYER_STATE1_READY_TO_FIRE;
}

void FirstPerson_Exit(Player* player, PlayState* play) {
    gNeiFpAimMode = 0;
    player->stateFlags1 &= ~PLAYER_STATE1_FIRST_PERSON;
    player->stateFlags1 &= ~PLAYER_STATE1_ITEM_IN_HAND;
    player->stateFlags1 &= ~PLAYER_STATE1_READY_TO_FIRE;
    sFpAimTimer = 0;
}

s16 FirstPerson_GetAimYaw(Player* player) {
    return player->actor.focus.rot.y;
}

s16 FirstPerson_GetAimPitch(Player* player) {
    return player->actor.focus.rot.x;
}

void FirstPerson_DrawReticle(Player* player, PlayState* play, f32 range, u8 r, u8 g, u8 b) {
    if (!(player->stateFlags1 & PLAYER_STATE1_ITEM_IN_HAND) || sFpAimTimer == 0)
        return;

    CollisionPoly* colPoly;
    s32 bgId;
    Vec3f rayStart, rayEnd, hitPos;
    Vec3f screenPos;
    f32 screenW;

    rayStart.x = player->actor.focus.pos.x;
    rayStart.y = player->actor.focus.pos.y;
    rayStart.z = player->actor.focus.pos.z;

    f32 cosY = Math_CosS(player->actor.focus.rot.y);
    f32 sinY = Math_SinS(player->actor.focus.rot.y);
    f32 cosX = Math_CosS(player->actor.focus.rot.x);
    f32 sinX = Math_SinS(player->actor.focus.rot.x);

    f32 maxRange = (range > 0) ? range : 10000.0f;

    rayEnd.x = rayStart.x + (sinY * cosX * maxRange);
    rayEnd.y = rayStart.y + (-sinX * maxRange);
    rayEnd.z = rayStart.z + (cosY * cosX * maxRange);

    if (BgCheck_AnyLineTest3(&play->colCtx, &rayStart, &rayEnd, &hitPos, &colPoly, 1, 1, 1, 1, &bgId)) {
        OPEN_DISPS(play->state.gfxCtx);

        OVERLAY_DISP = Gfx_SetupDL(OVERLAY_DISP, 0x07);

        SkinMatrix_Vec3fMtxFMultXYZW(&play->viewProjectionMtxF, &hitPos, &screenPos, &screenW);
        f32 scale = (screenW < 200.0f) ? 0.08f : (screenW / 200.0f) * 0.08f;

        Matrix_Translate(hitPos.x, hitPos.y, hitPos.z, MTXMODE_NEW);
        // Billboard the reticle so it faces the camera. The original code multiplied in a segment-1
        // billboard matrix via SEG_ADDR(1, 0), but segment 1 is NOT set up at this OVERLAY_DISP draw
        // point in 2ship — the segmented address resolved to garbage and crashed GfxSpMatrix
        // (0xc0000005) on use (e.g. the Switch Hook aim reticle). Matrix_ReplaceRotation applies MM's
        // camera billboard directly, no segment needed. (Skijer's NEI reticle crash fix.)
        Matrix_ReplaceRotation(&play->billboardMtxF);
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);

        gSPMatrix(OVERLAY_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPTexture(OVERLAY_DISP++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, r, g, b, 255);

        gSPVertex(OVERLAY_DISP++, sReticleVtx, 3, 0);
        gSP1Triangle(OVERLAY_DISP++, 0, 1, 2, 0);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}

void ItemCamera_Init(ItemCameraState* state, Player* player, PlayState* play) {
    if (Player_IsZTargeting(player)) {
        state->mode = CAMERA_MODE_Z_TARGET;
        state->firstPersonActive = 0;
    } else {
        state->mode = CAMERA_MODE_FIRST_PERSON;
        FirstPerson_Init(player, play);
        state->firstPersonActive = 1;
    }
}

void ItemCamera_Update(ItemCameraState* state, Player* player, PlayState* play) {
    if (state->mode == CAMERA_MODE_FREE) {
        if (state->firstPersonActive) {
            FirstPerson_Exit(player, play);
            state->firstPersonActive = 0;
        }
        return;
    }

    u8 isZTargeting = Player_IsZTargeting(player);

    if (state->mode == CAMERA_MODE_FIRST_PERSON && isZTargeting) {
        FirstPerson_Exit(player, play);
        state->firstPersonActive = 0;
        state->mode = CAMERA_MODE_Z_TARGET;
    } else if (state->mode == CAMERA_MODE_Z_TARGET && !isZTargeting) {
        FirstPerson_Init(player, play);
        state->firstPersonActive = 1;
        state->mode = CAMERA_MODE_FIRST_PERSON;
    }

    if (state->firstPersonActive) {
        FirstPerson_Update(player, play);
    }
}

void ItemCamera_Exit(ItemCameraState* state, Player* player, PlayState* play) {
    if (state->firstPersonActive) {
        FirstPerson_Exit(player, play);
        state->firstPersonActive = 0;
    }
    state->mode = CAMERA_MODE_FIRST_PERSON;
}

void ItemCamera_ToggleFirstPerson(ItemCameraState* state, Player* player, PlayState* play) {
    if (state->firstPersonActive) {
        FirstPerson_Exit(player, play);
        state->firstPersonActive = 0;
        state->mode = Player_IsZTargeting(player) ? CAMERA_MODE_Z_TARGET : CAMERA_MODE_FREE;
    } else {
        FirstPerson_Init(player, play);
        state->firstPersonActive = 1;
        state->mode = CAMERA_MODE_FIRST_PERSON;
    }
}

s16 ItemCamera_GetAimYaw(ItemCameraState* state, Player* player, PlayState* play) {
    switch (state->mode) {
        case CAMERA_MODE_FIRST_PERSON:
            return state->firstPersonActive ? FirstPerson_GetAimYaw(player) : player->actor.shape.rot.y;

        case CAMERA_MODE_Z_TARGET:
            if (Player_IsZTargeting(player) && player->focusActor != NULL) {
                return Math_Vec3f_Yaw(&player->actor.world.pos, &player->focusActor->focus.pos);
            }
            return player->actor.shape.rot.y;

        case CAMERA_MODE_FREE:
        default:
            return player->actor.shape.rot.y;
    }
}

s16 ItemCamera_GetAimPitch(ItemCameraState* state, Player* player) {
    if (state->mode == CAMERA_MODE_FIRST_PERSON && state->firstPersonActive) {
        return FirstPerson_GetAimPitch(player);
    }
    return 0;
}

void ItemCamera_SetFreeMode(ItemCameraState* state, Player* player, PlayState* play) {
    if (state->firstPersonActive) {
        FirstPerson_Exit(player, play);
        state->firstPersonActive = 0;
    }
    state->mode = CAMERA_MODE_FREE;
}

s16 Camera_GetDirectionYaw(PlayState* play) {
    Camera* cam = play->cameraPtrs[play->activeCamId];
    return cam->camDir.y;
}

s16 Camera_GetDirectionPitch(PlayState* play) {
    Camera* cam = play->cameraPtrs[play->activeCamId];
    return -cam->camDir.x;
}

void Camera_InterpolateToDirection(s16* currentYaw, s16* currentPitch, PlayState* play, s16 yawSpeed, s16 pitchSpeed) {
    s16 targetYaw = Camera_GetDirectionYaw(play);
    s16 targetPitch = Camera_GetDirectionPitch(play);
    Math_ScaledStepToS(currentYaw, targetYaw, yawSpeed);
    Math_ScaledStepToS(currentPitch, targetPitch, pitchSpeed);
}

void Input_GetStickDirection(PlayState* play, s16* outYawDelta, s16* outPitchDelta, s16 sensitivity) {
    s8 stickX = play->state.input[0].cur.stick_x;
    s8 stickY = play->state.input[0].cur.stick_y;
    *outYawDelta = -stickX * sensitivity;
    *outPitchDelta = stickY * sensitivity;
}

void Projectile_UpdateDirectionFromStick(s16* yaw, s16* pitch, PlayState* play, s16 turnSpeed, s16 pitchMax) {
    s16 yawDelta, pitchDelta;
    Input_GetStickDirection(play, &yawDelta, &pitchDelta, turnSpeed);
    *yaw += yawDelta;
    *pitch += pitchDelta;
    if (*pitch > pitchMax)
        *pitch = pitchMax;
    if (*pitch < -pitchMax)
        *pitch = -pitchMax;
}

void Projectile_UpdateRotationFromStick(s16* yaw, s16* pitch, PlayState* play, s16 turnSpeed, s16 pitchMax) {
    Input* input = &play->state.input[0];
    f32 rawX = input->rel.stick_x;
    f32 rawY = input->rel.stick_y;
    f32 magnitude = sqrtf(SQ(rawX) + SQ(rawY));

    if (magnitude > 20.0f) {
        // Steer 1:1 with MM's first-person AIM controls (Ship_HandleFirstPersonAiming, z_player.c):
        // stick X → yaw, stick Y → pitch, honoring the SAME invert CVars the aim camera uses
        // (GI_INVERT_FIRST_PERSON_AIM_X/Y). Default Y is negated so pushing UP flies UP; the consumer's
        // +pitch means DOWN (Beetle_Move: pos.y -= sin(pitch)), which lines up with aiming's default
        // (aim's stickY is likewise `-invert`, and +focus.rot.x looks down). Users flip either axis
        // with the First-Person Aim invert options — exactly like aiming. The old angle-decomposition
        // math (atan2(relY,-relX) + sin/cos) had SWAPPED the axes (horizontal moved pitch, vertical
        // moved yaw). Skijer's NEI
        f32 relX = rawX * GameInteractor_InvertControl(GI_INVERT_FIRST_PERSON_AIM_X);
        f32 relY = rawY * -GameInteractor_InvertControl(GI_INVERT_FIRST_PERSON_AIM_Y);

        *yaw += (s16)((relX / 60.0f) * turnSpeed);
        *pitch += (s16)((relY / 60.0f) * turnSpeed);

        if (*pitch > pitchMax)
            *pitch = pitchMax;
        if (*pitch < -pitchMax)
            *pitch = -pitchMax;
    }
}
