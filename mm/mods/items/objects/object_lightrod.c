/**
 * object_lightrod.c - Light Rod 3D model and draw functions
 *
 * Draws the golden/yellow glowing rod and light projectiles.
 * Projectiles use OoT's real gPhantomEnergyBallDL (from oot.o2r, 1:1 with SoH) when available,
 * falling back to the native gEffFire1DL flame placeholder when oot.o2r isn't mounted.
 */

#include "z64.h"
#include "../custom_items.h"
#include "../logic/item_rod_light.h"
#include "macros.h"
#include "functions.h"
#include <math.h>

// OoT's object_fhg (Phantom Ganon energy ball) has no MM equivalent in gameplay_keep, so we pull
// the REAL gPhantomEnergyBallDL from oot.o2r for a 1:1 SoH look. If oot.o2r isn't mounted we fall
// back to the native gEffFire1DL flame placeholder (glitchy but visible). Skijer's NEI
#include "objects/gameplay_keep/gameplay_keep.h"
#include "mods/oot_asset_loader/oot_asset_loader.h"

// Cached OoT Phantom Ganon energy ball (real light-rod orb). NULL until resolved / if unavailable.
static Gfx* sLightOrbDL = NULL;
static u8 sLightOrbTried = 0;

static Gfx* LightRod_GetOrbDL(void) {
    if (!sLightOrbTried) {
        sLightOrbTried = 1;
        sLightOrbDL = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_fhg/gPhantomEnergyBallDL");
    }
    return sLightOrbDL;
}

// Light Rod model from light_rodDL folder
#include "light_rodDL/header.h"
#include "light_rodDL/Cylinder_002.c" // 2ship: bring the DL data into the unity TU

// Public display list reference for draw.cpp (give item)
Gfx* gLightRodGiveDL = g_light_rod_dl;

// ============================================================================
// LIGHT ROD ORB VISUAL CONSTANTS (same style as Dominion Rod)
// ============================================================================

// 5.5f was for SoH's gPhantomEnergyBallDL; 2ship draws the orb with gEffFire1DL (base geometry
// ~1000 units, like fire/ice), so it needs ~0.01 (LIGHT_ROD_PROJ_RADIUS 10 / 1000). At 5.5 the orbs
// were screen-filling flames. Tunable 0.008–0.012. Skijer's NEI
#define LIGHTROD_ORB_SCALE 0.01f

// Colors - golden/yellow variant of Dominion Rod's orange
#define LIGHTROD_ORB_PRIM_R 255
#define LIGHTROD_ORB_PRIM_G 255
#define LIGHTROD_ORB_PRIM_B 255
#define LIGHTROD_ORB_PRIM_A 200

#define LIGHTROD_ORB_ENV_R 255
#define LIGHTROD_ORB_ENV_G 255 // More yellow than Dominion Rod's 215
#define LIGHTROD_ORB_ENV_B 50
#define LIGHTROD_ORB_ENV_A 0

// ============================================================================
// DRAW FUNCTION - Draws Light Rod and active projectiles
// Uses exact same rendering approach as Dominion Rod
// ============================================================================

void CustomItems_DrawLightRod(Player* player, PlayState* play) {
    if (!lightRodActive)
        return;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Opa(play->state.gfxCtx);

    // Get forearm and hand positions to calculate hand direction
    Vec3f forearmPos = player->bodyPartsPos[PLAYER_BODYPART_L_FOREARM];
    Vec3f handPos = player->bodyPartsPos[PLAYER_BODYPART_L_HAND];

    // Calculate direction vector from forearm to hand
    f32 dx = handPos.x - forearmPos.x;
    f32 dy = handPos.y - forearmPos.y;
    f32 dz = handPos.z - forearmPos.z;

    // Calculate yaw and pitch from direction
    f32 handYaw = atan2f(dx, dz);
    f32 horizDist = sqrtf(dx * dx + dz * dz);
    f32 handPitch = atan2f(dy, horizDist);

    // Position at hand
    Matrix_Translate(handPos.x, handPos.y, handPos.z, MTXMODE_NEW);

    // Apply hand rotation
    Matrix_RotateY(handYaw, MTXMODE_APPLY);
    Matrix_RotateX(-handPitch, MTXMODE_APPLY);
    Matrix_RotateY(BINANG_TO_RAD(0x4000), MTXMODE_APPLY);

    // Slight offset in local X, Y and Z
    Matrix_Translate(-0.5f, 5.0f, 0.5f, MTXMODE_APPLY);

    Matrix_Scale(0.05f, 0.05f, 0.05f, MTXMODE_APPLY);

    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_OPA_DISP++, g_light_rod_dl);

    // Draw transparent parts (light crystal) with same matrix
    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, g_light_rod_xlu_dl);

    // Draw active light ball sets. Local play uses sLightProjSets[]. Remote
    // dummies fall back to gCustomItemState (single set, mirrored from sync).
    u8 hasLocalSets = LightRod_HasAnyActiveSet();
    u8 hasRemoteSync = !hasLocalSets && lightRodProjActive && gCustomItemState.lightRodProjCount > 0;

    if (hasLocalSets || hasRemoteSync) {
        s16 rotZ = (play->gameplayFrames * 0x1000) + (s16)(Rand_ZeroOne() * 0x4000);

        // 1:1 with SoH when oot.o2r is mounted: real gPhantomEnergyBallDL at scale 5.5 (no seg-8 hack).
        // Fallback: MM gEffFire1DL flame at ~0.01 with the seg-8 texture bind (glitchy but visible).
        Gfx* orbDL = LightRod_GetOrbDL();
        u8 useReal = (orbDL != NULL);
        Gfx* ballDL = useReal ? orbDL : (Gfx*)gEffFire1DL;
        f32 ballScale = useReal ? 5.5f : LIGHTROD_ORB_SCALE;
        f32 trailMin = useReal ? 1.0f : 0.0005f;

        Gfx_SetupDL_25Xlu(play->state.gfxCtx);
        // The gEffFire1DL fallback reads its texture from segment 0x08; the real energy ball is
        // self-textured (via its own OTR refs) and doesn't need it. Skijer's NEI
        if (!useReal) {
            gSPSegment(POLY_XLU_DISP++, 0x08,
                       Gfx_TwoTexScroll(play->state.gfxCtx, 0, 0, 0, 0x20, 0x40, 1, 0,
                                        (play->gameplayFrames * -20) & 0x1FF, 0x20, 0x80));
        }
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, LIGHTROD_ORB_PRIM_R, LIGHTROD_ORB_PRIM_G, LIGHTROD_ORB_PRIM_B,
                        LIGHTROD_ORB_PRIM_A);
        gDPSetEnvColor(POLY_XLU_DISP++, LIGHTROD_ORB_ENV_R, LIGHTROD_ORB_ENV_G, LIGHTROD_ORB_ENV_B, LIGHTROD_ORB_ENV_A);
        gDPPipeSync(POLY_XLU_DISP++);

        if (hasLocalSets) {
            RodProjSet* sets = LightRod_GetProjSets();
            for (s32 s = 0; s < ROD_MAX_PROJ_SETS; s++) {
                RodProjSet* set = &sets[s];
                if (!set->active)
                    continue;

                for (s32 p = 0; p < set->count; p++) {
                    Matrix_Translate(set->pos[p].x, set->pos[p].y, set->pos[p].z, MTXMODE_NEW);
                    Matrix_ReplaceRotation(&play->billboardMtxF);
                    Matrix_Scale(ballScale, ballScale, ballScale, MTXMODE_APPLY);
                    Matrix_RotateZ(((rotZ + (p * 0x5555)) / (f32)0x8000) * M_PI, MTXMODE_APPLY);
                    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPDisplayList(POLY_XLU_DISP++, ballDL);
                }

                for (s32 i = 1; i < 4; i++) {
                    Vec3f* trailPos = &set->trail[i];
                    f32 trailScale = ballScale * (1.0f - (i * 0.25f));
                    if (trailScale < trailMin)
                        continue;

                    u8 trailAlpha = (u8)(LIGHTROD_ORB_PRIM_A * (1.0f - (i * 0.3f)));
                    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, LIGHTROD_ORB_PRIM_R, LIGHTROD_ORB_PRIM_G,
                                    LIGHTROD_ORB_PRIM_B, trailAlpha);

                    Matrix_Translate(trailPos->x, trailPos->y, trailPos->z, MTXMODE_NEW);
                    Matrix_ReplaceRotation(&play->billboardMtxF);
                    Matrix_Scale(trailScale, trailScale, trailScale, MTXMODE_APPLY);
                    Matrix_RotateZ(((rotZ - (i * 0x2000)) / (f32)0x8000) * M_PI, MTXMODE_APPLY);
                    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPDisplayList(POLY_XLU_DISP++, ballDL);
                }
            }
        } else {
            Vec3f remotePos[3] = { lightRodProjPos, gCustomItemState.lightRodProjPos2,
                                   gCustomItemState.lightRodProjPos3 };
            s32 remoteCount = gCustomItemState.lightRodProjCount;
            if (remoteCount > 3)
                remoteCount = 3;

            for (s32 p = 0; p < remoteCount; p++) {
                Matrix_Translate(remotePos[p].x, remotePos[p].y, remotePos[p].z, MTXMODE_NEW);
                Matrix_ReplaceRotation(&play->billboardMtxF);
                Matrix_Scale(ballScale, ballScale, ballScale, MTXMODE_APPLY);
                Matrix_RotateZ(((rotZ + (p * 0x5555)) / (f32)0x8000) * M_PI, MTXMODE_APPLY);
                gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_XLU_DISP++, ballDL);
            }

            for (s32 i = 1; i < 4; i++) {
                Vec3f* trailPos = &lightRodProjTrail[i];
                f32 trailScale = ballScale * (1.0f - (i * 0.25f));
                if (trailScale < trailMin)
                    continue;

                u8 trailAlpha = (u8)(LIGHTROD_ORB_PRIM_A * (1.0f - (i * 0.3f)));
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, LIGHTROD_ORB_PRIM_R, LIGHTROD_ORB_PRIM_G, LIGHTROD_ORB_PRIM_B,
                                trailAlpha);

                Matrix_Translate(trailPos->x, trailPos->y, trailPos->z, MTXMODE_NEW);
                Matrix_ReplaceRotation(&play->billboardMtxF);
                Matrix_Scale(trailScale, trailScale, trailScale, MTXMODE_APPLY);
                Matrix_RotateZ(((rotZ - (i * 0x2000)) / (f32)0x8000) * M_PI, MTXMODE_APPLY);
                gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_XLU_DISP++, ballDL);
            }
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
