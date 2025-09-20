#include "DrawFuncs.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "Rando/Types.h"
#include "BenPort.h"

extern "C" {
#include <functions.h>
#include "objects/gameplay_keep/gameplay_keep.h"
#include "objects/object_obj_tokeidai/object_obj_tokeidai.h"

// clang-format off
// Boss Includes
/* Goht */      #include "objects/object_boss_hakugin/object_boss_hakugin.h"
/* Gyorg */     #include "objects/object_boss03/object_boss03.h"
/* Odolwa */    #include "objects/object_boss01/object_boss01.h"
/* Twinmold */  #include "objects/object_boss02/object_boss02.h"
/* Majora */    #include "objects/object_boss07/object_boss07.h"
// clang-format on

// Soul Effects
#include "src/overlays/actors/ovl_Obj_Moon_Stone/z_obj_moon_stone.h"
#include "assets/objects/object_gi_reserve00/object_gi_reserve00.h"
}

// Soul Effects
void DrawEnLight(Color_RGB8 flameColor, Vec3f flameSize) {
    Gfx* sp68;
    static s8 unk_144 = (s8)(Rand_ZeroOne() * 255.0f);
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    Matrix_ReplaceRotation(&gPlayState->billboardMtxF);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScroll(gPlayState->state.gfxCtx, 0, 0, 0, 0x10, 0x20, 1, (unk_144 * 2) & 0x3F,
                                           (unk_144 * -6) & 0x7F, 0x10, 0x20));
    sp68 = (Gfx*)gameplay_keep_DL_01ACF0;
    gDPSetPrimColor(POLY_XLU_DISP++, 0xC0, 0xC0, flameColor.r, flameColor.g, flameColor.b, 0);
    gDPSetEnvColor(POLY_XLU_DISP++, flameColor.r, flameColor.g, flameColor.b, 0);
    Matrix_Scale(flameSize.x, flameSize.y, flameSize.z, MTXMODE_APPLY);

    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(gPlayState->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, sp68);

    CLOSE_DISPS(gPlayState->state.gfxCtx);

    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        unk_144++;
    }
}

// Boss Souls
extern void DrawGoht() {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Translate(0.0f, -20.0f, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(0.005f, 0.005f, 0.005f, MTXMODE_APPLY);

    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[33];
    static Vec3s otherTable[33];
    static u32 lastUpdate = 0;
    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gGohtSkel, (AnimationHeader*)&gGohtRunAnim,
                           jointTable, otherTable, 33);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)gGohtMetalPlateWithCirclePatternTex);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 10, 138, 46 }, { 30.0f, 30.0f, 30.0f });
}

extern void DrawGyorg() {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Translate(0.0f, -20.0f, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(0.05f, 0.05f, 0.05f, MTXMODE_APPLY);

    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[15];
    static Vec3s otherTable[15];
    static u32 lastUpdate = 0;
    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gGyorgSkel,
                           (AnimationHeader*)&gGyorgGentleSwimmingAnim, jointTable, otherTable, 15);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 19, 99, 165 }, { 3.0f, 3.0f, 3.0f });
}

extern void DrawOdolwa() {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    Matrix_Translate(0.0f, -20.0f, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(0.005f, 0.005f, 0.005f, MTXMODE_APPLY);

    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[52];
    static Vec3s otherTable[52];
    static u32 lastUpdate = 0;
    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gOdolwaSkel,
                           (AnimationHeader*)&gOdolwaReadyAnim, jointTable, otherTable, 52);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 145, 20, 133 }, { 25.0f, 25.0f, 25.0f });
}

extern void DrawTwinmold() {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.06f, 0.06f, 0.06f, MTXMODE_APPLY);

    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[13];
    static Vec3s otherTable[13];
    static u32 lastUpdate = 0;
    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gTwinmoldHeadSkel,
                           (AnimationHeader*)&gTwinmoldHeadFlyAnim, jointTable, otherTable, 13);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    Mtx* mtxHead = (Mtx*)GRAPH_ALLOC(gPlayState->state.gfxCtx, 23 * sizeof(Mtx));
    gSPSegment(POLY_OPA_DISP++, 0x0D, (uintptr_t)mtxHead);
    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)gTwinmoldBlueSkinTex);
    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 168, 180, 20 }, { 3.0f, 3.0f, 3.0f });
}

extern void DrawMajora() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[MAJORAS_MASK_LIMB_MAX];
    static Vec3s morphTable[MAJORAS_MASK_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    Matrix_Scale(0.05f, 0.05f, 0.05f, MTXMODE_APPLY);
    Matrix_Translate(0, 0, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gMajorasMaskSkel,
                       (AnimationHeader*)&gMajorasMaskFloatingAnim, jointTable, morphTable, MAJORAS_MASK_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    gSPSegment(POLY_OPA_DISP++, 8, (uintptr_t)gMajorasMaskWithNormalEyesTex);
    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gMajorasMaskTentacleMaterialDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 232, 128, 21 }, { 3.0f, 3.0f, 3.0f });
}

// Clock function implementation
extern void DrawClock(RandoItemId randoItemId) {
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    // Collectible scale (half vanilla wall clock size)
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);

    // Determine visual properties based on clock type
    bool isNightClock =
        (randoItemId == RI_CLOCK_NIGHT_1 || randoItemId == RI_CLOCK_NIGHT_2 || randoItemId == RI_CLOCK_NIGHT_3);
    s16 sunMoonRotation = isNightClock ? 0x8000 : 0; // Moon for night, sun for day
    s32 hour = isNightClock ? 18 : 12;               // 6 PM for night, 12 PM for day
    s16 clockRotation = (s16)(s32)(hour * (0x10000 / 24.0f));

    // Minute ring (rotates with clock face)
    Matrix_Push();
    Matrix_RotateZS(-clockRotation * 2, MTXMODE_APPLY);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(gPlayState->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gClockTowerMinuteRingDL);
    Matrix_Pop();

    // Clock center and hands
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(gPlayState->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gClockTowerClockCenterAndHandDL);

    // Clock face with rotation and colors
    Matrix_RotateZS(-clockRotation * 2, MTXMODE_APPLY);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(gPlayState->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0xFF, 255, 255, 255, 255);
    gDPSetEnvColor(POLY_OPA_DISP++, 100, 100, 120, 255);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gWallClockClockFaceDL);

    // Sun/Moon panel with positioning and colors
    Matrix_Translate(0.0f, -1112.0f, -19.6f, MTXMODE_APPLY);
    Matrix_RotateYS(sunMoonRotation, MTXMODE_APPLY);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(gPlayState->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0xFF, 255, 220, 180, 255);
    gDPSetEnvColor(POLY_OPA_DISP++, 150, 100, 50, 255);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gClockTowerSunAndMoonPanelDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}