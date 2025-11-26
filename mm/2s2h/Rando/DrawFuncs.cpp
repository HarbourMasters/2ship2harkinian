#include "DrawFuncs.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "BenPort.h"

extern "C" {
#include <functions.h>
#include "objects/gameplay_keep/gameplay_keep.h"

// Soul Effects
#include "src/overlays/actors/ovl_Obj_Moon_Stone/z_obj_moon_stone.h"
#include "assets/objects/object_gi_reserve00/object_gi_reserve00.h"

s32 EnMinifrog_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* enMini);

// clang-format off
// Boss Includes
/* Goht */      #include "objects/object_boss_hakugin/object_boss_hakugin.h"
/* Gyorg */     #include "objects/object_boss03/object_boss03.h"
/* Odolwa */    #include "objects/object_boss01/object_boss01.h"
/* Twinmold */  #include "objects/object_boss02/object_boss02.h"
/* Majora */    #include "objects/object_boss07/object_boss07.h"

// Enemy Includes
/* Armos */         #include "assets/objects/object_am/object_am.h" 
/* Bad Bat */       #include "assets/objects/object_bat/object_bat.h"
/* Beamos */        #include "assets/objects/object_vm/object_vm.h"
/* Boe */           #include "assets/objects/object_mkk/object_mkk.h"
/* Rat */           #include "assets/objects/object_rat/object_rat.h"
/* Bubble */        #include "assets/objects/object_bb/object_bb.h"
/* Death Armos */   #include "assets/objects/object_famos/object_famos.h"
/* Deku Baba */     #include "assets/objects/object_dekubaba/object_dekubaba.h"
/* Dinolfos */      #include "assets/objects/object_dinofos/object_dinofos.h"
/* Dodongo */       #include "assets/objects/object_dodongo/object_dodongo.h"
/* Eeno */          #include "assets/objects/object_snowman/object_snowman.h"
/* Flying Pot */    #include "assets/objects/gameplay_dangeon_keep/gameplay_dangeon_keep.h"
/* Freezard */      #include "assets/objects/object_fz/object_fz.h"
/* Grasshopper */   #include "assets/objects/object_grasshopper/object_grasshopper.h"
/* Guay */          #include "assets/objects/object_crow/object_crow.h"
/* Hiploop */       #include "assets/objects/object_pp/object_pp.h"
/* Iron Knuckle */  #include "assets/objects/object_ik/object_ik.h"
/* Keese */         #include "assets/objects/object_firefly/object_firefly.h"
/* Leever */        #include "assets/objects/object_rb/object_rb.h"
/* Like Like */     #include "assets/objects/object_rr/object_rr.h"
/* Mad Scrub */     #include "assets/objects/object_dekunuts/object_dekunuts.h"
/* Nejiron */       #include "assets/objects/object_gmo/object_gmo.h"
/* Octorok */       #include "assets/objects/object_okuta/object_okuta.h"
/* Peehat */        #include "assets/objects/object_ph/object_ph.h"
/* Redead */        #include "assets/objects/object_rd/object_rd.h"
/* Shellblade */    #include "assets/objects/object_sb/object_sb.h"
/* Skullfish */     #include "assets/objects/object_pr/object_pr.h"
/* Skulltula */     #include "assets/objects/object_st/object_st.h"
/* Slimes */        #include "assets/objects/object_slime/object_slime.h"
/* Snapper */       #include "assets/objects/object_tl/object_tl.h"
/* Stalchild */     #include "assets/objects/object_skb/object_skb.h"
/* Tektite */       #include "assets/objects/object_tite/object_tite.h"
/* Wallmaster */    #include "assets/objects/object_wallmaster/object_wallmaster.h"
/* Wolfos */        #include "assets/objects/object_wf/object_wf.h"

// Other Actor Includes
/* Minifrog */  #include "objects/object_fr/object_fr.h"
// clang-format on
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

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sp68);

    CLOSE_DISPS(gPlayState->state.gfxCtx);

    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        unk_144++;
    }
}

// Limb Override Functions
void EnMinifrogPostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    if ((limbIndex == FROG_LIMB_RIGHT_EYE) || (limbIndex == FROG_LIMB_LEFT_EYE)) {
        OPEN_DISPS(play->state.gfxCtx);

        Matrix_ReplaceRotation(&play->billboardMtxF);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, *dList);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}

void DrawEnFirefly_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* firefly) {
    static Color_RGBA8 auraPrimColor[2] = { { 255, 255, 100, 255 }, { 100, 200, 255, 255 } };
    static Color_RGBA8 auraEnvColor[2] = { { 255, 50, 0, 0 }, { 0, 0, 255, 0 } };
    static uint32_t dustUpdate = 0;
    static bool auraColor = false;
    static Vec3f auraVelocity = { 0, 0.5f, 0 };
    static Vec3f auraAccel = { 0, 0.5f, 0 };
    static Vec3f auraPos;
    if (firefly != NULL) {
        auraPos = firefly->world.pos;
    }

    Matrix_MultZero(&auraPos);
    auraPos.x += Rand_ZeroOne() * 0.5f;
    auraPos.y += Rand_ZeroOne() * 0.5f;
    auraPos.z += Rand_ZeroOne() * 0.5f;

    if (gPlayState != NULL && dustUpdate != gPlayState->state.frames) {
        if (dustUpdate == gPlayState->state.frames - 20) {
            dustUpdate = gPlayState->state.frames;
            auraColor = !auraColor;
        }
    }

    if (limbIndex == FIRE_KEESE_LIMB_HEAD) {
        Gfx* gfx = play->state.gfxCtx->polyXlu.p;
        Scene_SetRenderModeXlu(play, 1, 2);
        MATRIX_FINALIZE_AND_LOAD(gfx++, play->state.gfxCtx);
        gSPDisplayList(gfx++, (Gfx*)&gKeeseRedEyesDL);
    }
    if (limbIndex == FIRE_KEESE_LIMB_LEFT_WING_END || limbIndex == FIRE_KEESE_LIMB_RIGHT_WING_END_ROOT) {
        EffectSsDust_Spawn(gPlayState, 2, &auraPos, &auraVelocity, &auraAccel, &auraPrimColor[auraColor],
                           &auraEnvColor[auraColor], 100, -40, 3, 0);
    }
}

void DrawEnRealBombchu_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* rat) {
    // Gfx* gfx = play->state.gfxCtx->polyOpa.p;

    if (limbIndex == REAL_BOMBCHU_LIMB_TAIL_END) {
        OPEN_DISPS(play->state.gfxCtx);
        Matrix_ReplaceRotation(&play->billboardMtxF);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gBombCapDL);
        Matrix_RotateZYX(0x4000, 0, 0, MTXMODE_APPLY);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 80, 255);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 10, 0, 40, 255);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gBombBodyDL);
        CLOSE_DISPS(play->state.gfxCtx);
    }
}

s32 DrawEnSkb_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    s16 sins;
    if (limbIndex == 11) {
        OPEN_DISPS(play->state.gfxCtx);

        sins = fabsf(Math_SinS(play->gameplayFrames * 6000) * 95.0f) + 160.0f;

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetEnvColor(POLY_OPA_DISP++, sins, sins, sins, 255);

        CLOSE_DISPS(play->state.gfxCtx);
    }
    return false;
}

void DrawEnIk_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    if (limbIndex == IRON_KNUCKLE_LIMB_HELMET_ARMOR) {
        OPEN_DISPS(play->state.gfxCtx);

        Gfx* xlu = POLY_XLU_DISP;

        MATRIX_FINALIZE_AND_LOAD(&xlu[0], play->state.gfxCtx);
        gSPDisplayList(&xlu[1], (Gfx*)gIronKnuckleHelmetMarkingDL);
        POLY_XLU_DISP = &xlu[2];

        CLOSE_DISPS(play->state.gfxCtx);
    }
}

// Enemy Soul Draw Functions
extern void DrawArmos() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[OBJECT_AM_LIMB_MAX];
    static Vec3s morphTable[OBJECT_AM_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3100, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&object_am_Skel_005948,
                       (AnimationHeader*)&gArmosHopAnim, jointTable, morphTable, OBJECT_AM_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawBat() {
    static u32 lastUpdate = 0;
    static u32 wingAnim = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);

    static Gfx* sWingsDLs[] = {
        (Gfx*)&gBadBatWingsFrame0DL, (Gfx*)&gBadBatWingsFrame1DL, (Gfx*)&gBadBatWingsFrame2DL,
        (Gfx*)&gBadBatWingsFrame3DL, (Gfx*)&gBadBatWingsFrame4DL, (Gfx*)&gBadBatWingsFrame5DL,
        (Gfx*)&gBadBatWingsFrame6DL, (Gfx*)&gBadBatWingsFrame7DL, (Gfx*)&gBadBatWingsFrame8DL,
    };

    Gfx* gfx = POLY_OPA_DISP;

    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        if (wingAnim == 8) {
            wingAnim = 0;
        } else {
            wingAnim++;
        }
    }

    gSPDisplayList(&gfx[0], gSetupDLs[SETUPDL_25]);
    MATRIX_FINALIZE_AND_LOAD(&gfx[1], gPlayState->state.gfxCtx);
    gSPDisplayList(&gfx[2], (Gfx*)&gBadBatSetupDL);
    gSPDisplayList(&gfx[3], (Gfx*)&gBadBatBodyDL);
    gSPDisplayList(&gfx[4], sWingsDLs[wingAnim]);

    POLY_OPA_DISP = &gfx[5];

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 6.0f, 6.0f, 6.0f });
}

extern void DrawBeamos() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[11];
    static Vec3s morphTable[11];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3200, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gBeamosSkel, (AnimationHeader*)&gBeamosAnim,
                       jointTable, morphTable, 11);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawBoe() {
    static Gfx* sBoeDLists[4] = {
        (Gfx*)gBlackBoeBodyMaterialDL,
        (Gfx*)gBlackBoeBodyModelDL,
        (Gfx*)gBlackBoeEndDL,
        (Gfx*)gBlackBoeEyesDL,
    };
    static Color_RGBA8 D_80A4F7C4 = { 0, 0, 0, 255 };

    OPEN_DISPS(gPlayState->state.gfxCtx);

    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -1200, 0, MTXMODE_APPLY);

    gSPDisplayList(POLY_OPA_DISP++, gSetupDLs[SETUPDL_25]);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0xFF, 0, 0, 0, 255);
    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)D_801AEFA0);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gBlackBoeEndDL);

    gSPDisplayList(POLY_XLU_DISP++, gSetupDLs[SETUPDL_25]);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, 255);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gBlackBoeBodyMaterialDL);
    Matrix_ReplaceRotation(&gPlayState->billboardMtxF);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gBlackBoeBodyModelDL);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0xFF, 245, 97, 0, 255);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gBlackBoeEyesDL);
    Matrix_Scale(0.009f, 0.009f, 0.009f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0xFF, 245, 214, 0, 255);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gBlackBoeEyesDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 1000.0f, 1000.0f, 1000.0f });
}

extern void DrawRealBombchu() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[REAL_BOMBCHU_LIMB_MAX];
    static Vec3s morphTable[REAL_BOMBCHU_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Gfx_SetupDL60_XluNoCD(gPlayState->state.gfxCtx);
    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);
    Matrix_Translate(0, -1500.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gRealBombchuSkel,
                           (AnimationHeader*)&gRealBombchuRunAnim, jointTable, morphTable, REAL_BOMBCHU_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL,
                          DrawEnRealBombchu_PostLimbDraw, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 6.0f, 6.0f, 6.0f });
}

extern void DrawBubble() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[BUBBLE_LIMB_MAX];
    static Vec3s morphTable[BUBBLE_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gBubbleSkel, (AnimationHeader*)&gBubbleFlyingAnim,
                       jointTable, morphTable, BUBBLE_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawDeathArmos() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[FAMOS_LIMB_MAX];
    static Vec3s morphTable[FAMOS_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    AnimatedMat_Draw(gPlayState, (AnimatedMaterial*)gFamosNormalGlowingEmblemTexAnim);
    Matrix_Scale(0.008f, 0.008f, 0.008f, MTXMODE_APPLY);
    Matrix_Translate(0, -4100, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gFamosSkel, (AnimationHeader*)&gFamosIdleAnim,
                       jointTable, morphTable, FAMOS_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawDekuBaba() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[DEKUBABA_LIMB_MAX];
    static Vec3s morphTable[DEKUBABA_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);
    Matrix_Translate(0, 0, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gDekuBabaSkel,
                       (AnimationHeader*)&gDekuBabaFastChompAnim, jointTable, morphTable, DEKUBABA_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 6.0f, 6.0f, 6.0f });
}

extern void DrawDinolfos() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[DINOLFOS_LIMB_MAX];
    static Vec3s morphTable[DINOLFOS_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    Matrix_Scale(0.014f, 0.014f, 0.014f, MTXMODE_APPLY);
    Matrix_Translate(0, -2200.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gDinolfosSkel,
                           (AnimationHeader*)&gDinolfosIdleAnim, jointTable, morphTable, DINOLFOS_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    Scene_SetRenderModeXlu(gPlayState, 0, 1);
    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)&gDinolfosEyeOpenTex);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawDodongo() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[OBJECT_DODONGO_LIMB_MAX];
    static Vec3s morphTable[OBJECT_DODONGO_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);
    Matrix_Translate(0, -1500.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&object_dodongo_Skel_008318,
                       (AnimationHeader*)&object_dodongo_Anim_004C20, jointTable, morphTable, OBJECT_DODONGO_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawEeno() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[EENO_LIMB_MAX];
    static Vec3s morphTable[EENO_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3000.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gEenoSkel, (AnimationHeader*)&gEenoIdleAnim,
                           jointTable, morphTable, EENO_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    Scene_SetRenderModeXlu(gPlayState, 0, 1);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawFlyingPot() {
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Matrix_Scale(0.3f, 0.3f, 0.3f, MTXMODE_APPLY);
    Matrix_Translate(0, -100.0f, 0, MTXMODE_APPLY);

    Gfx_DrawDListOpa(gPlayState, (Gfx*)gameplay_dangeon_keep_DL_017EA0);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 0.4f, 0.4f, 0.4f });
}

extern void DrawFreezard() {
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Matrix_Scale(0.006f, 0.006f, 0.006f, MTXMODE_APPLY);
    Matrix_Translate(0, -4100.0f, 0, MTXMODE_APPLY);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScroll(gPlayState->state.gfxCtx, 0, 0, gPlayState->state.frames % 128, 0x20, 0x20,
                                           1, 0, (gPlayState->state.frames * 2) % 128, 0x20, 0x20));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gDPSetCombineLERP(POLY_XLU_DISP++, TEXEL1, PRIMITIVE, PRIM_LOD_FRAC, TEXEL0, TEXEL1, TEXEL0, PRIMITIVE, TEXEL0,
                      PRIMITIVE, ENVIRONMENT, COMBINED, ENVIRONMENT, COMBINED, 0, ENVIRONMENT, 0);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0x80, 155, 255, 255, 255);
    gDPSetEnvColor(POLY_XLU_DISP++, 200, 200, 200, 255);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)object_fz_DL_001130);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 20.0f, 20.0f, 20.0f });
}

extern void DrawGrasshopper() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[DRAGONFLY_LIMB_MAX];
    static Vec3s morphTable[DRAGONFLY_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -700.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gDragonflySkel, (AnimationHeader*)&gDragonflyFlyAnim,
                       jointTable, morphTable, DRAGONFLY_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawGuay() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[OBJECT_CROW_LIMB_MAX];
    static Vec3s morphTable[OBJECT_CROW_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);
    Matrix_Translate(0, 0, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gGuaySkel, (AnimationHeader*)&gGuayFlyAnim,
                           jointTable, morphTable, OBJECT_CROW_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 6.0f, 6.0f, 6.0f });
}

extern void DrawHiploop() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[HIPLOOP_LIMB_MAX];
    static Vec3s morphTable[HIPLOOP_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);
    Matrix_Translate(0, -1400.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gHiploopSkel,
                           (AnimationHeader*)&gHiploopChargeAnim, jointTable, morphTable, HIPLOOP_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    Scene_SetRenderModeXlu(gPlayState, 0, 1);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawIronKnuckle() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[IRON_KNUCKLE_LIMB_MAX];
    static Vec3s morphTable[IRON_KNUCKLE_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -2900.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gIronKnuckleSkel,
                           (AnimationHeader*)&gIronKnuckleWalkAnim, jointTable, morphTable, IRON_KNUCKLE_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    Gfx* gfx = POLY_XLU_DISP;
    gSPDisplayList(&gfx[0], gSetupDLs[SETUPDL_25]);
    POLY_XLU_DISP = &gfx[1];
    gfx = POLY_OPA_DISP;
    gSPDisplayList(&gfx[0], gSetupDLs[SETUPDL_25]);
    gSPSegment(&gfx[1], 0x08, (uintptr_t)gIronKnuckleBlackArmorMaterialDL);
    gSPSegment(&gfx[2], 0x09, (uintptr_t)gIronKnuckleBrownArmorMaterialDL);
    gSPSegment(&gfx[3], 0x0A, (uintptr_t)gIronKnuckleBrownArmorMaterialDL);
    POLY_OPA_DISP = &gfx[4];

    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL,
                          DrawEnIk_PostLimbDraw, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 12.0f, 12.0f, 12.0f });
}

extern void DrawKeese() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[FIRE_KEESE_LIMB_MAX];
    static Vec3s morphTable[FIRE_KEESE_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -700.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gFireKeeseSkel, (AnimationHeader*)&gFireKeeseFlyAnim,
                       jointTable, morphTable, FIRE_KEESE_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, DrawEnFirefly_PostLimbDraw, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawLeever() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[LEEVER_LIMB_MAX];
    static Vec3s morphTable[LEEVER_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.05f, 0.05f, 0.05f, MTXMODE_APPLY);
    Matrix_Translate(0, -700.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gLeeverSkel, (AnimationHeader*)&gLeeverSpinAnim,
                       jointTable, morphTable, LEEVER_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x01, 255, 255, 255, 255);
    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 3.0f, 3.0f, 3.0f });
}

extern void DrawLikeLike() {
    static bool initialized = false;
    static u32 lastUpdate = 0;
    static s16 textureScroll = 0;
    static struct {
        f32 unk_08;
        f32 unk_10;
        f32 unk_00;
        Vec3s unk_1A;
    } segments[5];

    if (!initialized) {
        initialized = true;
        for (int i = 0; i < 5; i++) {
            segments[i].unk_08 = 0.8f;
            segments[i].unk_10 = 0.0f;
            segments[i].unk_00 = 0.0f;
            segments[i].unk_1A.x = 0;
            segments[i].unk_1A.y = 0;
            segments[i].unk_1A.z = 0;
        }
    }

    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        textureScroll++;

        f32 phase = gPlayState->state.frames * (2500.0f * (2.0f * M_PI / 65536.0f));

        for (int j = 0; j < 5; j++) {
            f32 segmentPhase = phase + (j * 0x4000) * (2.0f * M_PI / 65536.0f);
            segments[j].unk_10 = cosf(segmentPhase) * 0.15f;
            segments[j].unk_00 = 0.0f;
        }

        for (int j = 1; j < 5; j++) {
            segments[j].unk_1A.x = (s16)(cosf(phase + (j * 0x3000) * (2.0f * M_PI / 65536.0f)) * 2048.0f);
            segments[j].unk_1A.z = (s16)(sinf(phase + (j * 0x1000) * (2.0f * M_PI / 65536.0f)) * 2048.0f);
        }
    }

    Mtx* mtx = (Mtx*)GRAPH_ALLOC(gPlayState->state.gfxCtx, 4 * sizeof(Mtx));
    s32 i;
    f32 temp_f20;

    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3000.0f, 0, MTXMODE_APPLY);
    Matrix_Push();

    gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)mtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScroll(gPlayState->state.gfxCtx, 0, 0, 0, 0x20, 0x10, 1, (textureScroll * 0) & 0x3F,
                                           (textureScroll * -6) & 0x7F, 0x20, 0x10));

    Matrix_Push();
    Matrix_Scale((1.0f + segments[0].unk_10) * segments[0].unk_08, 1.0f,
                 (1.0f + segments[0].unk_10) * segments[0].unk_08, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);

    Matrix_Pop();

    for (i = 1; i < 5; i++) {
        temp_f20 = segments[i].unk_08 * (segments[i].unk_10 + 1.0f);

        Matrix_Translate(0.0f, segments[i].unk_00 + 1000.0f, 0.0f, MTXMODE_APPLY);
        Matrix_RotateZYX(segments[i].unk_1A.x, segments[i].unk_1A.y, segments[i].unk_1A.z, MTXMODE_APPLY);
        Matrix_Push();
        Matrix_Scale(temp_f20, 1.0f, temp_f20, MTXMODE_APPLY);
        Matrix_ToMtx(mtx);
        Matrix_Pop();
        mtx++;
    }

    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gLikeLikeDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    Matrix_Pop();
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawMadScrub() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[DEKU_SCRUB_LIMB_MAX];
    static Vec3s morphTable[DEKU_SCRUB_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -2300, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gDekuScrubSkel,
                       (AnimationHeader*)&gDekuScrubLookAroundAnim, jointTable, morphTable, DEKU_SCRUB_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawNejiron() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[NEJIRON_LIMB_MAX];
    static Vec3s morphTable[NEJIRON_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    Matrix_Scale(0.015f, 0.015f, 0.015f, MTXMODE_APPLY);
    Matrix_Translate(0, 0, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gNejironSkel, (AnimationHeader*)&gNejironIdleAnim,
                       jointTable, morphTable, NEJIRON_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    gSPSegment(POLY_OPA_DISP++, 8, (uintptr_t)gNejironEyeOpenTex);
    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 13.0f, 13.0f, 13.0f });
}

extern void DrawOctorok() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[16];
    static Vec3s morphTable[16];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.007f, 0.007f, 0.007f, MTXMODE_APPLY);
    Matrix_Translate(0, -700.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gOctorokSkel, (AnimationHeader*)&gOctorokFloatAnim,
                       jointTable, morphTable, 16);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    Gfx* gfxPtr = POLY_OPA_DISP;
    gSPDisplayList(&gfxPtr[0], gSetupDLs[SETUPDL_25]);
    gSPSegment(&gfxPtr[1], 0x08, (uintptr_t)D_801AEFA0);
    POLY_OPA_DISP = &gfxPtr[2];
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x01, 255, 255, 255, 255);
    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawPeahat() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[24];
    static Vec3s morphTable[24];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&object_ph_Skel_001C80,
                       (AnimationHeader*)&object_ph_Anim_0009C4, jointTable, morphTable, 24);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawRedead() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[REDEAD_LIMB_MAX];
    static Vec3s morphTable[REDEAD_LIMB_MAX];
    static u32 lastUpdate = 0;
    static u32 animUpdate = 0;
    static uint32_t rdAnimID = 0;
    static AnimationHeader* currentAnim = (AnimationHeader*)gGibdoRedeadIdleAnim;

    std::vector<AnimationHeader*> rdAnims = {
        (AnimationHeader*)gGibdoRedeadSquattingDanceAnim,
        (AnimationHeader*)gGibdoRedeadClappingDanceAnim,
        (AnimationHeader*)gGibdoRedeadPirouetteAnim,
    };

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Gfx_SetupDL60_XluNoCD(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -2900.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gRedeadSkel,
                           (AnimationHeader*)gGibdoRedeadPirouetteAnim, jointTable, morphTable, REDEAD_LIMB_MAX);
    }

    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        Player* player = GET_PLAYER(gPlayState);
        if (player->currentMask == PLAYER_MASK_GIBDO) {
            if (animUpdate != gPlayState->state.frames) {
                if (animUpdate <= gPlayState->state.frames - 35) {
                    animUpdate = gPlayState->state.frames;
                    currentAnim = rdAnims[rdAnimID];
                    if (rdAnimID >= rdAnims.size() - 1) {
                        rdAnimID = 0;
                    } else {
                        rdAnimID++;
                    }
                    Animation_MorphToLoop(&skelAnime, currentAnim, -6.0f);
                }
            }
        } else {
            currentAnim = (AnimationHeader*)gGibdoRedeadIdleAnim;
            Animation_MorphToLoop(&skelAnime, currentAnim, -6.0f);
        }
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)D_801AEFA0);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawShellBlade() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[9];
    static Vec3s morphTable[9];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    Matrix_Scale(0.007f, 0.007f, 0.007f, MTXMODE_APPLY);
    Matrix_Translate(0, -3500.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&object_sb_Skel_002BF0,
                           (AnimationHeader*)&object_sb_Anim_000194, jointTable, morphTable, 9);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawSkullfish() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[5];
    static Vec3s morphTable[5];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.02f, 0.02f, 0.02f, MTXMODE_APPLY);
    Matrix_Translate(0, 0, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&object_pr_Skel_004188,
                           (AnimationHeader*)&object_pr_Anim_004340, jointTable, morphTable, 5);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    Scene_SetRenderModeXlu(gPlayState, 0, 1);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 5.0f, 5.0f, 5.0f });
}

extern void DrawSkulltula() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[30];
    static Vec3s morphTable[30];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.03f, 0.03f, 0.03f, MTXMODE_APPLY);
    Matrix_Translate(0, 0, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&object_st_Skel_005298,
                       (AnimationHeader*)&object_st_Anim_000304, jointTable, morphTable, 30);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 5.0f, 5.0f, 5.0f });
}

extern void DrawSlime() {
    static int16_t timer = 25;
    f32 timerFactor = sqrtf(timer) * 0.2f;
    AnimatedMaterial* sSlimeTexAnim = (AnimatedMaterial*)Lib_SegmentedToVirtual((void*)gChuchuSlimeFlowTexAnim);

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Matrix_Scale(
        0.01f,
        ((((coss(RAD_TO_BINANG(timer * (2.0f * M_PI / 5.0f))) * SHT_MINV) * (0.07f * timerFactor)) + 1.0f) * 0.01f),
        0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -2700.0f, 0, MTXMODE_APPLY);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    AnimatedMat_Draw(gPlayState, sSlimeTexAnim);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 100, 255, 255, 200, 255);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 180, 0, 255);

    if (timer == 0) {
        timer = 25;
    }

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    Scene_SetRenderModeXlu(gPlayState, 1, 2);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gChuchuBodyDL);
    gSPSegment(POLY_XLU_DISP++, 9, (uintptr_t)gChuchuEyeOpenTex);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gChuchuEyesDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
    timer--;
}

extern void DrawSnapper() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[SNAPPER_LIMB_MAX];
    static Vec3s morphTable[SNAPPER_LIMB_MAX];
    static u32 lastUpdate = 0;
    TexturePtr eyeTexture = Lib_SegmentedToVirtual((TexturePtr)gSnapperEyeOpenTex);

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3100.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gSnapperSkel,
                           (AnimationHeader*)&gSnapperIdleAnim, jointTable, morphTable, SNAPPER_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)eyeTexture);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawStalchild() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[STALCHILD_LIMB_MAX];
    static Vec3s morphTable[STALCHILD_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3200, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&gStalchildSkel, (AnimationHeader*)&gStalchildIdleAnim,
                       jointTable, morphTable, STALCHILD_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, DrawEnSkb_OverrideLimbDraw, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawTektite() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[25];
    static Vec3s morphTable[25];
    static u32 lastUpdate = 0;

    static TexturePtr D_80896B24[2][3] = {
        { (TexturePtr*)&object_tite_Tex_001300, (TexturePtr*)&object_tite_Tex_001700,
          (TexturePtr*)&object_tite_Tex_001900 },
        { (TexturePtr*)&object_tite_Tex_001B00, (TexturePtr*)&object_tite_Tex_001F00,
          (TexturePtr*)&object_tite_Tex_002100 },
    };

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -2900.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_Init(gPlayState, &skelAnime, (SkeletonHeader*)&object_tite_Skel_003A20,
                       (AnimationHeader*)&object_tite_Anim_0012E4, jointTable, morphTable, 25);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    Gfx* gfx = POLY_OPA_DISP;

    gSPDisplayList(&gfx[0], gSetupDLs[SETUPDL_25]);

    gSPSegment(&gfx[1], 0x08, (uintptr_t)D_80896B24[0][0]);
    gSPSegment(&gfx[2], 0x09, (uintptr_t)D_80896B24[0][1]);
    gSPSegment(&gfx[3], 0x0A, (uintptr_t)D_80896B24[0][2]);

    POLY_OPA_DISP = &gfx[4];

    SkelAnime_DrawOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawWallmaster() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[WALLMASTER_LIMB_MAX];
    static Vec3s morphTable[WALLMASTER_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3500.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gWallmasterSkel,
                           (AnimationHeader*)&gWallmasterIdleAnim, jointTable, morphTable, WALLMASTER_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
}

extern void DrawWolfos() {
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[WOLFOS_NORMAL_LIMB_MAX];
    static Vec3s morphTable[WOLFOS_NORMAL_LIMB_MAX];
    static u32 lastUpdate = 0;

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3000.0f, 0, MTXMODE_APPLY);

    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gWolfosNormalSkel,
                           (AnimationHeader*)&gWolfosWaitAnim, jointTable, morphTable, WOLFOS_NORMAL_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)&gWolfosNormalEyeOpenTex);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount, NULL, NULL, NULL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
    DrawEnLight({ 155, 155, 155 }, { 10.0f, 10.0f, 10.0f });
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
    Matrix_ReplaceRotation(&gPlayState->billboardMtxF);

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

// Other Actors
extern void DrawMinifrog(RandoItemId randoItemId, Actor* actor) {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Translate(0.0f, -20.0f, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(0.03f, 0.03f, 0.03f, MTXMODE_APPLY);

    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[FROG_LIMB_MAX];
    static Vec3s otherTable[FROG_LIMB_MAX];
    Color_RGBA8 envColor = { 200, 170, 0, 255 }; // FROG_YELLOW
    static u32 lastUpdate = 0;
    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gFrogSkel, (AnimationHeader*)&gFrogIdleAnim,
                           jointTable, otherTable, FROG_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }

    switch (randoItemId) {
        case RI_FROG_BLUE:
            envColor = { 120, 130, 230, 255 };
            break;
        case RI_FROG_CYAN:
            envColor = { 0, 170, 200, 255 };
            break;
        case RI_FROG_PINK:
            envColor = { 210, 120, 100, 255 };
            break;
        case RI_FROG_WHITE:
            envColor = { 190, 190, 190, 255 };
            break;
    }

    gDPSetEnvColor(POLY_OPA_DISP++, envColor.r, envColor.g, envColor.b, envColor.a);

    Mtx* mtxHead = (Mtx*)GRAPH_ALLOC(gPlayState->state.gfxCtx, 23 * sizeof(Mtx));
    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)gFrogIrisOpenTex);
    gSPSegment(POLY_OPA_DISP++, 0x09, (uintptr_t)gFrogIrisOpenTex);
    SkelAnime_DrawFlexOpa(gPlayState, skelAnime.skeleton, skelAnime.jointTable, FROG_LIMB_MAX,
                          EnMinifrog_OverrideLimbDraw, EnMinifrogPostLimbDraw, actor);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}