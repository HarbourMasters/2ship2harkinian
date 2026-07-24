#include "Rando/Rando.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/Rando/DrawFuncs.h"
#include "2s2h_assets.h"
#include "mods/nei_save.h"                     // NeiSaveData chain tiers for progressive get-item draws
#include "2s2h/FleetShipCombo/FleetComboIds.h" // FC_OOT_SWORD_* registry indices (chain tiers)

extern "C" {
#include "variables.h"
#include "functions.h"
#include "objects/gameplay_keep/gameplay_keep.h"
#include "objects/object_gi_melody/object_gi_melody.h"
#include "assets/objects/object_gi_key/object_gi_key.h"
#include "assets/objects/object_gi_bosskey/object_gi_bosskey.h"
#include "objects/object_gi_hearts/object_gi_hearts.h"
#include "objects/object_gi_liquid/object_gi_liquid.h"
#include "objects/object_sek/object_sek.h"
#include "objects/object_st/object_st.h"

#include "assets/overlays/ovl_Arrow_Ice/ovl_Arrow_Ice.h"
#include "assets/objects/object_gi_purse/object_gi_purse.h"

Gfx* ResourceMgr_LoadGfxByName(const char* path);
u8 ResourceMgr_FileExists(const char* resName);
void* OotAssets_LoadGfx(const char* otrPath); // Skijer's NEI — resolve an OoT model DL from oot.o2r
void* OotAssets_LoadGfxDirect(const char* otrPath); // Skijer's NEI — archive-scoped load (defeats MM shadowing)
void* OotAssets_LoadTexOrDList(const char* otrPath); // Skijer's NEI — texture/DL resource (Climb ladder seg-8 tex)
uint8_t Nei_GetOwnedItem(uint8_t slot);       // mods/nei_save.cpp — Roc chain level for its draw
}

s32 StrayFairyOverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx,
                               Gfx** gfx) {
    if (limbIndex == STRAY_FAIRY_LIMB_RIGHT_FACING_HEAD) {
        *dList = NULL;
    }

    return false;
}

void DrawStrayFairy(RandoItemId randoItemId) {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    switch (randoItemId) {
        case RI_WOODFALL_STRAY_FAIRY:
            AnimatedMat_Draw(gPlayState, (AnimatedMaterial*)&gStrayFairyWoodfallTexAnim);
            break;
        case RI_SNOWHEAD_STRAY_FAIRY:
            AnimatedMat_Draw(gPlayState, (AnimatedMaterial*)&gStrayFairySnowheadTexAnim);
            break;
        case RI_GREAT_BAY_STRAY_FAIRY:
            AnimatedMat_Draw(gPlayState, (AnimatedMaterial*)&gStrayFairyGreatBayTexAnim);
            break;
        case RI_STONE_TOWER_STRAY_FAIRY:
            AnimatedMat_Draw(gPlayState, (AnimatedMaterial*)&gStrayFairyStoneTowerTexAnim);
            break;
        default: // STRAY_FAIRY_AREA_CLOCK_TOWN
            AnimatedMat_Draw(gPlayState, (AnimatedMaterial*)&gStrayFairyClockTownTexAnim);
            break;
    }

    Matrix_ReplaceRotation(&gPlayState->billboardMtxF);
    Matrix_Scale(0.03f, 0.03f, 0.03f, MTXMODE_APPLY);

    // Kind of a hack to draw the stray fairy, the drawback of this is that all stray fairies in the scene will animate
    // together, but worse is that the more there are the faster their animation will play (because of the
    // SkelAnime_Update below). This is still better than the previous solution which hand drew the fairy with DL
    // calls...
    static bool initialized = false;
    static SkelAnime skelAnime;
    static Vec3s jointTable[STRAY_FAIRY_LIMB_MAX];
    static u32 lastUpdate = 0;
    if (!initialized) {
        initialized = true;
        SkelAnime_InitFlex(gPlayState, &skelAnime, (FlexSkeletonHeader*)&gStrayFairySkel,
                           (AnimationHeader*)&gStrayFairyFlyingAnim, jointTable, jointTable, STRAY_FAIRY_LIMB_MAX);
    }
    if (gPlayState != NULL && lastUpdate != gPlayState->state.frames) {
        lastUpdate = gPlayState->state.frames;
        SkelAnime_Update(&skelAnime);
    }
    POLY_XLU_DISP = SkelAnime_DrawFlex(gPlayState, skelAnime.skeleton, skelAnime.jointTable, skelAnime.dListCount,
                                       StrayFairyOverrideLimbDraw, NULL, NULL, POLY_XLU_DISP);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawSong(RandoItemId randoItemId) {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);

    switch (randoItemId) {
        case RI_SONG_SUN:
            gDPSetEnvColor(POLY_XLU_DISP++, 237, 231, 62, 255);
            break;
        case RI_SONG_DOUBLE_TIME:
        case RI_SONG_INVERTED_TIME:
        case RI_SONG_TIME:
            gDPSetEnvColor(POLY_XLU_DISP++, 98, 177, 211, 255);
            break;
        case RI_SONG_HEALING:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 150, 230, 255);
            break;
        case RI_SONG_STORMS:
            gDPSetEnvColor(POLY_XLU_DISP++, 146, 146, 146, 255);
            break;
        case RI_SONG_SARIA:
        case RI_SONG_SONATA:
            gDPSetEnvColor(POLY_XLU_DISP++, 98, 255, 98, 255);
            break;
        case RI_SONG_SOARING:
            gDPSetEnvColor(POLY_XLU_DISP++, 200, 160, 255, 255);
            break;
        case RI_SONG_ELEGY:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 98, 0, 255);
            break;
        case RI_SONG_LULLABY_INTRO:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 100, 100, 255);
            break;
        case RI_SONG_LULLABY:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 20, 20, 255);
            break;
        case RI_SONG_OATH:
            gDPSetEnvColor(POLY_XLU_DISP++, 98, 0, 98, 255);
            break;
        case RI_SONG_EPONA:
            gDPSetEnvColor(POLY_XLU_DISP++, 146, 87, 49, 255);
            break;
        case RI_SONG_NOVA:
            gDPSetEnvColor(POLY_XLU_DISP++, 20, 20, 255, 255);
            break;
        // Skijer's NEI — OoT (SoH) warp songs. MM renders every song as one note (gGiSongNoteDL) tinted by
        // env color, so the OoT warp songs reuse that exact note model, tinted to each sage's color.
        case RI_OOT_SONG_MINUET_OF_FOREST:
            gDPSetEnvColor(POLY_XLU_DISP++, 98, 255, 98, 255);
            break;
        case RI_OOT_SONG_BOLERO_OF_FIRE:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 60, 0, 255);
            break;
        case RI_OOT_SONG_SERENADE_OF_WATER:
            gDPSetEnvColor(POLY_XLU_DISP++, 85, 180, 223, 255);
            break;
        case RI_OOT_SONG_REQUIEM_OF_SPIRIT:
            gDPSetEnvColor(POLY_XLU_DISP++, 222, 158, 47, 255);
            break;
        case RI_OOT_SONG_NOCTURNE_OF_SHADOW:
            gDPSetEnvColor(POLY_XLU_DISP++, 160, 40, 210, 255);
            break;
        case RI_OOT_SONG_PRELUDE_OF_LIGHT:
            gDPSetEnvColor(POLY_XLU_DISP++, 237, 231, 62, 255);
            break;
        // Skijer's NEI — the 3 NEI custom songs (no get-item model on the OoT side either); note tinted to
        // each song's SoH quest-page ring color (soh z_kaleido_collect.c sMmPageSongs ring colors).
        case RI_OOT_SONG_FUGUE_OF_HOME:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 170, 50, 255); // amber
            break;
        case RI_OOT_SONG_COMMAND_MELODY:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 120, 255, 255); // magenta
            break;
        case RI_OOT_SONG_BALLAD_OF_THE_HERO:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 230, 120, 255); // gold
            break;
        default:
            break;
    }

    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)&gGiSongNoteDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawDoubleDefense() {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gDPSetGrayscaleColor(POLY_XLU_DISP++, 255, 255, 255, 255);
    gSPGrayscale(POLY_XLU_DISP++, true);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)&gGiHeartBorderDL);
    gDPSetGrayscaleColor(POLY_XLU_DISP++, 255, 0, 0, 100);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)&gGiHeartContainerDL);
    gSPGrayscale(POLY_XLU_DISP++, false);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawMilkRefill() {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, G_TX_RENDERTILE, -gPlayState->state.frames,
                                             gPlayState->state.frames, 32, 32, 1, -gPlayState->state.frames,
                                             gPlayState->state.frames, 32, 32, -1, 1, -1, 1));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    // Container Color
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(POLY_OPA_DISP++, 200, 200, 200, 255);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gGiPotionContainerPotDL);
    // Liquid Color
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(POLY_OPA_DISP++, 200, 200, 200, 255);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gGiPotionContainerLiquidDL);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    // Pattern Color
    // Milk
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 13, 33, 255, 255);
    gDPSetEnvColor(POLY_XLU_DISP++, 100, 100, 255, 255);
    gDPLoadTextureBlock(POLY_XLU_DISP++, gGiPotionContainerBluePatternTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 32, 0,
                        G_TX_MIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_CLAMP, 4, 5, G_TX_NOLOD, G_TX_NOLOD);
    // Chateau
    // gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 250, 225, 78, 255);
    // gDPSetEnvColor(POLY_XLU_DISP++, 184, 42, 119, 255);
    // gDPLoadTextureBlock(POLY_XLU_DISP++, gGiPotionContainerRedPatternTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 32, 0,
    // G_TX_MIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_CLAMP, 4, 5, G_TX_NOLOD, G_TX_NOLOD);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGiPotionContainerPatternDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawOwlStatue() {
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    Matrix_Translate(0, -3000, 0, MTXMODE_APPLY);
    Gfx_DrawDListOpa(gPlayState, (Gfx*)gOwlStatueOpenedDL);
}

static Gfx gGiSmallKeyCopyDL[75];

void DrawSmallKey(RandoItemId randoItemId) {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    switch (randoItemId) {
        case RI_WOODFALL_SMALL_KEY:
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 236, 120, 186, 255);
            break;
        case RI_SNOWHEAD_SMALL_KEY:
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 129, 173, 70, 255);
            break;
        case RI_GREAT_BAY_SMALL_KEY:
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 99, 90, 183, 255);
            break;
        case RI_STONE_TOWER_SMALL_KEY:
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 177, 165, 83, 255);
            break;
        default:
            break;
    }

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gGiSmallKeyCopyDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

static Gfx gGiBossKeyCopyDL[87];

void DrawBossKey(RandoItemId randoItemId) {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    switch (randoItemId) {
        case RI_WOODFALL_BOSS_KEY:
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 236, 120, 186, 255);
            break;
        case RI_SNOWHEAD_BOSS_KEY:
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 129, 173, 70, 255);
            break;
        case RI_GREAT_BAY_BOSS_KEY:
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 99, 90, 183, 255);
            break;
        case RI_STONE_TOWER_BOSS_KEY:
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_OPA_DISP++, 177, 165, 83, 255);
            break;
        default:
            break;
    }

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, gGiBossKeyCopyDL);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGiBossKeyGemDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

static Gfx gSkulltulaTokenFlameCopyDL[76];

void DrawSkulltulaToken(RandoItemId randoItemId, Actor* actor) {
    // It is not known why this happens, but the eyes on the skulltula tokens disappear if they are are perfectly
    // parallel with the camera. This most likely a problem in our Fast3D (maybe z-index stuff?).
    // Tilting the token down by 16 units seems to be enough to get it to always render the eyes without being
    // noticeable that it is tilted. This issue was most prevalent for tokens in shops.
    Matrix_RotateXS(16, MTXMODE_APPLY);

    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gSkulltulaTokenDL);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    if (randoItemId == RI_GS_TOKEN_OCEAN) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0x80, 0, 255, 255, 255);
        gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 255, 255);
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0x80, 0, 255, 170, 255);
        gDPSetEnvColor(POLY_XLU_DISP++, 0, 255, 0, 255);
    }

    gSPSegment(POLY_XLU_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, G_TX_RENDERTILE, gPlayState->state.frames * 0,
                                             -(gPlayState->state.frames * 5), 32, 32, 1, gPlayState->state.frames * 0,
                                             gPlayState->state.frames * 0, 32, 64, 0, -5, 0, 0));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gSkulltulaTokenFlameCopyDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawTrapModel() {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    Matrix_Scale(0.03f, 0.03f, 0.03f, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gTrapDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawTriforcePiece(RandoItemId randoItemId) {
    Gfx* triforcePieceModels[3] = {
        (Gfx*)gTriforcePiece0DL,
        (Gfx*)gTriforcePiece1DL,
        (Gfx*)gTriforcePiece2DL,
    };

    u16 currentTriforcePieces = gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces;

    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    Matrix_Scale(0.03f, 0.03f, 0.03f, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    if (currentTriforcePieces >= RANDO_SAVE_OPTIONS[RO_TRIFORCE_PIECES_REQUIRED]) {
        gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gTriforcePieceCompletedDL);
    } else {
        if (randoItemId == RI_TRIFORCE_PIECE_PREVIOUS) {
            gSPDisplayList(POLY_XLU_DISP++, (Gfx*)triforcePieceModels[(currentTriforcePieces - 1) % 3]);
        } else {
            gSPDisplayList(POLY_XLU_DISP++, (Gfx*)triforcePieceModels[currentTriforcePieces % 3]);
        }
    }

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawAbilityItem(RandoItemId randoItemId, Actor* actor) {
    Gfx* abilityItemModel[1] = {
        (Gfx*)gGiFlippersDL,
    };

    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)abilityItemModel[randoItemId - RI_ABILITY_SWIM]);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawOcarinaButtonItem(RandoItemId randoItemId, Actor* actor) {
    Gfx* ocarinaButtonModel[5] = {
        (Gfx*)gOcarinaAButtonDL,     (Gfx*)gOcarinaCDownButtonDL, (Gfx*)gOcarinaCRightButtonDL,
        (Gfx*)gOcarinaCLeftButtonDL, (Gfx*)gOcarinaCUpButtonDL,
    };

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)ocarinaButtonModel[randoItemId - RI_OCARINA_BUTTON_A]);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// clang-format off
std::unordered_map<RandoItemId, std::function<void()>> soulDrawMap = {
    { RI_SOUL_ENEMY_ALIEN,          DrawAlien },
    { RI_SOUL_ENEMY_ARMOS,          DrawArmos },
    { RI_SOUL_ENEMY_BAD_BAT,        DrawBat },
    { RI_SOUL_ENEMY_BEAMOS,         DrawBeamos },
    { RI_SOUL_ENEMY_BUBBLE,         DrawBubble },
    { RI_SOUL_ENEMY_BOE,            DrawBoe },
    { RI_SOUL_ENEMY_CHUCHU,         DrawChuchu },
    { RI_SOUL_ENEMY_CAPTAIN_KEETA,  DrawCaptainKeeta },
    { RI_SOUL_ENEMY_DEATH_ARMOS,    DrawDeathArmos },
    { RI_SOUL_ENEMY_DEEP_PYTHON,    DrawDeepPython },
    { RI_SOUL_ENEMY_DEKU_BABA,      DrawDekuBaba },
    { RI_SOUL_ENEMY_DEXIHAND,       DrawDexihand },
    { RI_SOUL_ENEMY_DINOLFOS,       DrawDinolfos },
    { RI_SOUL_ENEMY_DODONGO,        DrawDodongo },
    { RI_SOUL_ENEMY_DRAGONFLY,      DrawDragonfly },
    { RI_SOUL_ENEMY_EENO,           DrawEeno },
    { RI_SOUL_ENEMY_EYEGORE,        DrawEyegore },
    { RI_SOUL_ENEMY_FREEZARD,       DrawFreezard },
    { RI_SOUL_ENEMY_GARO,           DrawGaro },
    { RI_SOUL_ENEMY_GEKKO,          DrawGekko },
    { RI_SOUL_ENEMY_GIANT_BEE,      DrawGiantBee },
    { RI_SOUL_ENEMY_GOMESS,         DrawGomess },
    { RI_SOUL_ENEMY_GUAY,           DrawGuay },
    { RI_SOUL_ENEMY_HIPLOOP,        DrawHiploop },
    { RI_SOUL_ENEMY_IGOS_DU_IKANA,  DrawIgosDuIkana },
    { RI_SOUL_ENEMY_IRON_KNUCKLE,   DrawIronKnuckle },
    { RI_SOUL_ENEMY_KEESE,          DrawKeese },
    { RI_SOUL_ENEMY_LEEVER,         DrawLeever },
    { RI_SOUL_ENEMY_LIKE_LIKE,      DrawLikeLike },
    { RI_SOUL_ENEMY_MAD_SCRUB,      DrawMadScrub },
    { RI_SOUL_ENEMY_NEJIRON,        DrawNejiron },
    { RI_SOUL_ENEMY_OCTOROK,        DrawOctorok },
    { RI_SOUL_ENEMY_PEAHAT,         DrawPeahat },
    { RI_SOUL_ENEMY_PIRATE,         DrawPirate },
    { RI_SOUL_ENEMY_POE,            DrawPoe },
    { RI_SOUL_ENEMY_REDEAD,         DrawRedead },
    { RI_SOUL_ENEMY_SHELLBLADE,     DrawShellBlade },
    { RI_SOUL_ENEMY_SKULLFISH,      DrawSkullfish },
    { RI_SOUL_ENEMY_SKULLTULA,      DrawSkulltula },
    { RI_SOUL_ENEMY_SNAPPER,        DrawSnapper },
    { RI_SOUL_ENEMY_STALCHILD,      DrawStalchild },
    { RI_SOUL_ENEMY_TAKKURI,        DrawTakkuri },
    { RI_SOUL_ENEMY_TEKTITE,        DrawTektite },
    { RI_SOUL_ENEMY_WALLMASTER,     DrawWallmaster },
    { RI_SOUL_ENEMY_WART,           DrawWart },
    { RI_SOUL_ENEMY_WIZROBE,        DrawWizrobe },
    { RI_SOUL_ENEMY_WOLFOS,         DrawWolfos },
};
// clang-format on

void DrawSoul(RandoItemId randoItemId) {
    auto it = soulDrawMap.find(randoItemId);
    if (it != soulDrawMap.end()) {
        it->second();
    }
}

void DrawSparkles(RandoItemId randoItemId, Actor* actor) {
    if (actor == NULL) {
        return;
    }

    if (gGameState->frames % 2 == 0) {
        return;
    }

    static Vec3f sVelocity = { 0.0f, 0.0f, 0.0f };
    static Vec3f sAccel = { 0.0f, 0.0f, 0.0f };
    static Color_RGBA8 sPrimColor = { 255, 255, 255, 255 };
    static Color_RGBA8 sEnvColor = { 255, 128, 0, 255 };
    Vec3f newPos;

    newPos.x = Rand_CenteredFloat(10.0f) + actor->world.pos.x;
    newPos.y = (Rand_ZeroOne() * 10.0f) + actor->world.pos.y;
    newPos.z = Rand_CenteredFloat(10.0f) + actor->world.pos.z;

    if (actor->id == ACTOR_EN_SI) {
        newPos.y = (Rand_ZeroOne() * 10.0f) + actor->world.pos.y - 5.0f;
    } else if (actor->id == ACTOR_EN_ITEM00) {
        newPos.x = Rand_CenteredFloat(20.0f) + actor->world.pos.x;
        newPos.y = (Rand_ZeroOne() * 10.0f) + actor->world.pos.y + 10.0f;
        newPos.z = Rand_CenteredFloat(20.0f) + actor->world.pos.z;
    }

    EffectSsKirakira_SpawnDispersed(gPlayState, &newPos, &sVelocity, &sAccel, &sPrimColor, &sEnvColor, 2000, 16);
}

void DrawTycoonWallet() {
    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);

    // Wallet body - use Giant's color DL for render state, then override to purple
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gGiGiantsWalletColorDL);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x80, 150, 0, 200, 255);
    gDPSetEnvColor(POLY_OPA_DISP++, 80, 0, 120, 255);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gGiWalletDL);

    // Rupee outer - keep Giant's Wallet default colors
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gGiGiantsWalletRupeeOuterColorDL);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gGiWalletRupeeOuterDL);

    // String - keep Giant's Wallet default colors
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gGiGiantsWalletStringColorDL);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gGiWalletStringDL);

    // Rupee inner - keep Giant's Wallet default colors
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gGiGiantsWalletRupeeInnerColorDL);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)&gGiWalletRupeeInnerDL);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Skijer's NEI — OoT-only get-item models MM has no native GID/GI for (Deku Seeds slingshot ammo,
// Fairy Slingshot). The get-item DL is pulled from the mounted oot.o2r by OTR path, cached, and drawn
// exactly like MM's GetItem_DrawOpa0 (self-contained OoT get-item DLs). If oot.o2r isn't mounted the
// cache stays NULL and the model is simply skipped (nothing to draw), retried on the next frame.
static void DrawOotGetItemOpa(const char* otrPath, Gfx** cache) {
    if (*cache == NULL) {
        *cache = (Gfx*)OotAssets_LoadGfx(otrPath);
    }
    if (*cache == NULL) {
        return;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, *cache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void DrawDekuSeeds() {
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_seed/gGiSeedDL", &sCache);
}

void DrawFairySlingshot() {
    // FC 3-level chain (FCI_SLINGSHOT): the floating model = the tier you'll RECEIVE (OoT parity).
    // Copy 1 (nothing owned) = the slingshot; copies 2/3 = the bigger bullet bag
    // (object_gi_dekupouch is OoT-unique — not in mm/assets, so the path resolves from oot.o2r).
    if (Nei_Save()->slingshotOwned) {
        static Gfx* sBagCache = NULL;
        DrawOotGetItemOpa("__OTR__objects/object_gi_dekupouch/gGiBulletBagDL", &sBagCache);
        return;
    }
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_pachinko/gGiSlingshotDL", &sCache);
}

// Skijer's NEI — OoT (SoH) rando souls ported into MM. These reuse OoT get-item DLs pulled from the
// mounted oot.o2r (cached, retried while NULL). The bean "souls" all draw the OoT magic-bean sprout;
// the boss souls all draw the OoT blue-fire flame, grayscale-tinted per boss (matching SoH draw.cpp).

// All 10 magic-bean souls share one bean model. NOTE: we deliberately do NOT use OoT's
// object_mamenoki/gMagicBeanSeedlingDL — oot.o2r is mounted at the LOWEST priority, and MM also has
// an "objects/object_mamenoki" folder WITHOUT that symbol, so the path never resolves and the item
// renders as nothing. object_gi_bean/gGiBeanDL exists in BOTH games, so it always resolves (to MM's
// bean here) and is actually visible.
void DrawOotBeanSoul() {
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_bean/gGiBeanDL", &sCache);
}

// All 9 boss souls share the one OoT blue-fire flame (object_gi_fire/gGiBlueFireFlameDL), grayscale-
// tinted per boss. Flame colors mirror SoH's Randomizer_DrawBossSoul. The boss skeleton SoH also draws
// is intentionally omitted (that would require every boss object; the tinted flame is the generic soul).
void DrawOotBossSoul(RandoItemId randoItemId) {
    static Gfx* sCache = NULL;
    if (sCache == NULL) {
        sCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_fire/gGiBlueFireFlameDL");
    }
    if (sCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }

    u8 r = 150, g = 150, b = 150; // default: Ganon/grey
    switch (randoItemId) {
        case RI_SOUL_OOT_BOSS_GOHMA:        r = 0;   g = 255; b = 0;   break;
        case RI_SOUL_OOT_BOSS_KING_DODONGO: r = 255; g = 0;   b = 100; break;
        case RI_SOUL_OOT_BOSS_BARINADE:     r = 50;  g = 255; b = 255; break;
        case RI_SOUL_OOT_BOSS_PHANTOM_GANON:r = 4;   g = 195; b = 46;  break;
        case RI_SOUL_OOT_BOSS_VOLVAGIA:     r = 237; g = 95;  b = 95;  break;
        case RI_SOUL_OOT_BOSS_MORPHA:       r = 85;  g = 180; b = 223; break;
        case RI_SOUL_OOT_BOSS_BONGO_BONGO:  r = 126; g = 16;  b = 177; break;
        case RI_SOUL_OOT_BOSS_TWINROVA:     r = 222; g = 158; b = 47;  break;
        case RI_SOUL_OOT_BOSS_GANON:        r = 150; g = 150; b = 150; break;
        default: break;
    }

    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 8,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, 0, 0, 0, 16, 32, 1,
                                             gPlayState->state.frames, -(gPlayState->state.frames * 8), 16, 32, 0, 0, 1,
                                             -8));
    Matrix_Translate(0.0f, -70.0f, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(5.0f, 5.0f, 5.0f, MTXMODE_APPLY);
    Matrix_ReplaceRotation(&gPlayState->billboardMtxF);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gDPSetGrayscaleColor(POLY_XLU_DISP++, r, g, b, 255);
    gSPGrayscale(POLY_XLU_DISP++, true);
    gSPDisplayList(POLY_XLU_DISP++, sCache);
    gSPGrayscale(POLY_XLU_DISP++, false);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Skijer's NEI — OoT (SoH) trade-chain items ported into MM as get-items. MM has no OoT trade slots, so
// these are draw + message only (give is a no-op). Each DL (body + any color/eyes/writing DL) is pulled
// from the mounted oot.o2r by OTR path, cached (retried while NULL), and drawn mirroring OoT z_draw.c's
// get-item draw funcs (GetItem_DrawOpa0 / DrawOpa0Xlu1 / DrawOpa10Xlu2 / DrawEggOrMedallion / DrawGoronSword).

// Two opaque DLs drawn in sequence — OoT GetItem_DrawEggOrMedallion (material DL then geometry DL).
static void DrawOotGetItemOpaOpa(const char* pathA, Gfx** cacheA, const char* pathB, Gfx** cacheB) {
    if (*cacheA == NULL) {
        *cacheA = (Gfx*)OotAssets_LoadGfx(pathA);
    }
    if (*cacheB == NULL) {
        *cacheB = (Gfx*)OotAssets_LoadGfx(pathB);
    }
    if (*cacheA == NULL || *cacheB == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, *cacheA);
    gSPDisplayList(POLY_OPA_DISP++, *cacheB);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// One opaque DL then one translucent DL — OoT GetItem_DrawOpa0Xlu1 (body Opa, eyes/writing/glass Xlu).
static void DrawOotGetItemOpaXlu(const char* opaPath, Gfx** opaCache, const char* xluPath, Gfx** xluCache) {
    if (*opaCache == NULL) {
        *opaCache = (Gfx*)OotAssets_LoadGfx(opaPath);
    }
    if (*xluCache == NULL) {
        *xluCache = (Gfx*)OotAssets_LoadGfx(xluPath);
    }
    if (*opaCache == NULL || *xluCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, *opaCache);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, *xluCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Pocket Egg / Weird Egg — same OoT model (object_gi_egg: material DL + egg DL, both opaque).
void DrawOotEgg() {
    static Gfx* materialCache = NULL;
    static Gfx* eggCache = NULL;
    DrawOotGetItemOpaOpa("__OTR__objects/object_gi_egg/gGiEggMaterialDL", &materialCache,
                         "__OTR__objects/object_gi_egg/gGiEggDL", &eggCache);
}

// Zelda's Letter — object_gi_letter (letter Opa + writing Xlu).
void DrawOotZeldasLetter() {
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_letter/gGiLetterDL", &opaCache,
                         "__OTR__objects/object_gi_letter/gGiLetterWritingDL", &xluCache);
}

// Cojiro — OoT GetItem_DrawOpa10Xlu2 on object_gi_niwatori (color + body Opa, eyes Xlu).
void DrawOotCojiro() {
    static Gfx* colorCache = NULL;
    static Gfx* bodyCache = NULL;
    static Gfx* eyesCache = NULL;
    if (colorCache == NULL) {
        colorCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_niwatori/gGiCojiroColorDL");
    }
    if (bodyCache == NULL) {
        bodyCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_niwatori/gGiChickenDL");
    }
    if (eyesCache == NULL) {
        eyesCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_niwatori/gGiChickenEyesDL");
    }
    if (colorCache == NULL || bodyCache == NULL || eyesCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, colorCache);
    gSPDisplayList(POLY_OPA_DISP++, bodyCache);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, eyesCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Odd Mushroom — object_gi_mushroom (single opaque DL).
void DrawOotOddMushroom() {
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_mushroom/gGiOddMushroomDL", &sCache);
}

// Odd Potion — object_gi_powder (single opaque DL).
void DrawOotOddPotion() {
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_powder/gGiOddPotionDL", &sCache);
}

// Poacher's Saw — object_gi_saw (single opaque DL).
void DrawOotPoachersSaw() {
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_saw/gGiSawDL", &sCache);
}

// Broken Goron's Sword — OoT GetItem_DrawGoronSword (opaque DL with a scrolling shine on segment 0x08).
void DrawOotBrokenGoronsSword() {
    static Gfx* sCache = NULL;
    if (sCache == NULL) {
        sCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_brokensword/gGiBrokenGoronSwordDL");
    }
    if (sCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, 0, gPlayState->state.frames, 0, 32, 32, 1, 0, 0,
                                             32, 32, 1, 0, 0, 0));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Prescription — object_gi_prescription (paper Opa + writing Xlu).
void DrawOotPrescription() {
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_prescription/gGiPrescriptionDL", &opaCache,
                         "__OTR__objects/object_gi_prescription/gGiPrescriptionWritingDL", &xluCache);
}

// Eyeball Frog — object_gi_frog (body Opa + eyes Xlu).
void DrawOotEyeballFrog() {
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_frog/gGiFrogDL", &opaCache,
                         "__OTR__objects/object_gi_frog/gGiFrogEyesDL", &xluCache);
}

// World's Finest Eyedrops — object_gi_eye_lotion (cap Opa + glass bottle Xlu).
void DrawOotEyedrops() {
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_eye_lotion/gGiEyeDropsCapDL", &opaCache,
                         "__OTR__objects/object_gi_eye_lotion/gGiEyeDropsBottleDL", &xluCache);
}

// Claim Check — object_gi_ticketstone (stone Opa + writing Xlu).
void DrawOotClaimCheck() {
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_ticketstone/gGiClaimCheckDL", &opaCache,
                         "__OTR__objects/object_gi_ticketstone/gGiClaimCheckWritingDL", &xluCache);
}

// Skijer's NEI — OoT (SoH) medallions ported into MM as get-items. OoT GetItem_DrawEggOrMedallion draws
// the colored face DL then the shared gold ring (gGiMedallionDL), both opaque. All six share the one ring.
static void DrawOotMedallion(const char* faceOtrPath, Gfx** faceCache) {
    static Gfx* sRingCache = NULL;
    DrawOotGetItemOpaOpa(faceOtrPath, faceCache, "__OTR__objects/object_gi_medal/gGiMedallionDL", &sRingCache);
}
void DrawOotMedallionFire() {
    static Gfx* c = NULL;
    DrawOotMedallion("__OTR__objects/object_gi_medal/gGiFireMedallionFaceDL", &c);
}
void DrawOotMedallionForest() {
    static Gfx* c = NULL;
    DrawOotMedallion("__OTR__objects/object_gi_medal/gGiForestMedallionFaceDL", &c);
}
void DrawOotMedallionLight() {
    static Gfx* c = NULL;
    DrawOotMedallion("__OTR__objects/object_gi_medal/gGiLightMedallionFaceDL", &c);
}
void DrawOotMedallionShadow() {
    static Gfx* c = NULL;
    DrawOotMedallion("__OTR__objects/object_gi_medal/gGiShadowMedallionFaceDL", &c);
}
void DrawOotMedallionSpirit() {
    static Gfx* c = NULL;
    DrawOotMedallion("__OTR__objects/object_gi_medal/gGiSpiritMedallionFaceDL", &c);
}
void DrawOotMedallionWater() {
    static Gfx* c = NULL;
    DrawOotMedallion("__OTR__objects/object_gi_medal/gGiWaterMedallionFaceDL", &c);
}

// Skijer's NEI — OoT (SoH) spiritual stones ported into MM as get-items. OoT GetItem_DrawJewel draws the
// gem (Xlu) over the metal setting (Opa), each tinted by its own prim/env color (matching SoH z_draw.c's
// GetItem_DrawJewel{Kokiri,Goron,Zora}). The animated shine lives on tex-scroll segments 8/9.
static void DrawOotStone(const char* gemPath, Gfx** gemCache, const char* settingPath, Gfx** settingCache, u8 pxR,
                         u8 pxG, u8 pxB, u8 exR, u8 exG, u8 exB, u8 poR, u8 poG, u8 poB, u8 eoR, u8 eoG, u8 eoB) {
    if (*gemCache == NULL) {
        *gemCache = (Gfx*)OotAssets_LoadGfx(gemPath);
    }
    if (*settingCache == NULL) {
        *settingCache = (Gfx*)OotAssets_LoadGfx(settingPath);
    }
    if (*gemCache == NULL || *settingCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 9,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, 0, 0, 255, 64, 64, 1, 0, 255, 16, 16, 0, 0, 0,
                                             0));
    gSPSegment(POLY_OPA_DISP++, 8, (uintptr_t)Gfx_TexScrollEx(gPlayState->state.gfxCtx, 0, 0, 16, 16, 0, 0));
    Matrix_RotateZYX(0, -0x4000, 0x4000, MTXMODE_APPLY);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 128, pxR, pxG, pxB, 255);
    gDPSetEnvColor(POLY_XLU_DISP++, exR, exG, exB, 255);
    gSPDisplayList(POLY_XLU_DISP++, *gemCache);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 128, poR, poG, poB, 255);
    gDPSetEnvColor(POLY_OPA_DISP++, eoR, eoG, eoB, 255);
    gSPDisplayList(POLY_OPA_DISP++, *settingCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}
void DrawOotStoneKokiriEmerald() {
    static Gfx* gemCache = NULL;
    static Gfx* settingCache = NULL;
    DrawOotStone("__OTR__objects/object_gi_jewel/gGiKokiriEmeraldGemDL", &gemCache,
                 "__OTR__objects/object_gi_jewel/gGiKokiriEmeraldSettingDL", &settingCache, 255, 255, 160, 0, 255, 0,
                 255, 255, 170, 150, 120, 0);
}
void DrawOotStoneGoronRuby() {
    static Gfx* gemCache = NULL;
    static Gfx* settingCache = NULL;
    DrawOotStone("__OTR__objects/object_gi_jewel/gGiGoronRubyGemDL", &gemCache,
                 "__OTR__objects/object_gi_jewel/gGiGoronRubySettingDL", &settingCache, 255, 170, 255, 255, 0, 100, 255,
                 255, 170, 150, 120, 0);
}
void DrawOotStoneZoraSapphire() {
    static Gfx* gemCache = NULL;
    static Gfx* settingCache = NULL;
    DrawOotStone("__OTR__objects/object_gi_jewel/gGiZoraSapphireGemDL", &gemCache,
                 "__OTR__objects/object_gi_jewel/gGiZoraSapphireSettingDL", &settingCache, 50, 255, 255, 50, 0, 150, 255,
                 255, 170, 150, 120, 0);
}

// Skijer's NEI — OoT (SoH) per-dungeon items ported into MM as get-items.
//
// Small keys / key rings / boss keys now use SoH's REAL custom key models (soh.o2r —
// objects/object_key / object_keyring / object_bosskey), replicating SoH draw.cpp
// Randomizer_DrawSmallKey / Randomizer_DrawKeyRing / Randomizer_DrawBossKey with the
// "CustomKeyModels" path and SoH's DEFAULT colors (2ship has no key-cosmetic CVars):
// body env white (boss key yellow), per-dungeon emblem env colors below, gem env red.
// All DLs are pulled via OotAssets_LoadGfxDirect (archive-scoped; soh.o2r is mounted
// lowest-priority by mm_asset_loader's LoadOotO2rs). NULL-safe fallback while soh.o2r
// is absent = the previous generic draws (gGiSmallKeyDL / gGiBossKeyDL+gem).
//
// SoH slot order (shared by all three per-dungeon path tables + emblem colors):
// 0 Forest, 1 Fire, 2 Water, 3 Spirit, 4 Shadow, 5 Well, 6 GTG, 7 Fortress, 8 Ganon, 9 TCG.
static const Color_RGB8 sOotKeyEmblemColors[10] = {
    { 4, 195, 46 },    // Forest
    { 237, 95, 95 },   // Fire
    { 85, 180, 223 },  // Water
    { 222, 158, 47 },  // Spirit
    { 126, 16, 177 },  // Shadow
    { 227, 110, 255 }, // Bottom of the Well
    { 221, 212, 60 },  // Gerudo Training Ground
    { 255, 255, 255 }, // Gerudo Fortress
    { 80, 80, 80 },    // Ganon's Castle
    { 255, 255, 255 }, // Treasure Chest Game
};
static const char* sOotSmallKeyIconPaths[10] = {
    "__OTR__objects/object_key/gSmallKeyIconForestTempleDL",
    "__OTR__objects/object_key/gSmallKeyIconFireTempleDL",
    "__OTR__objects/object_key/gSmallKeyIconWaterTempleDL",
    "__OTR__objects/object_key/gSmallKeyIconSpiritTempleDL",
    "__OTR__objects/object_key/gSmallKeyIconShadowTempleDL",
    "__OTR__objects/object_key/gSmallKeyIconBottomoftheWellDL",
    "__OTR__objects/object_key/gSmallKeyIconGerudoTrainingGroundDL",
    "__OTR__objects/object_key/gSmallKeyIconGerudoFortressDL",
    "__OTR__objects/object_key/gSmallKeyIconGanonsCastleDL",
    "__OTR__objects/object_key/gSmallKeyIconTreasureChestGameDL",
};
// Non-MQ key layouts only: MM has no Master Quest dungeons, so SoH's MQ variant fork is moot.
static const char* sOotKeyringKeysPaths[10] = {
    "__OTR__objects/object_keyring/gKeyringKeysForestTempleDL",
    "__OTR__objects/object_keyring/gKeyringKeysFireTempleDL",
    "__OTR__objects/object_keyring/gKeyringKeysWaterTempleDL",
    "__OTR__objects/object_keyring/gKeyringKeysSpiritTempleDL",
    "__OTR__objects/object_keyring/gKeyringKeysShadowTempleDL",
    "__OTR__objects/object_keyring/gKeyringKeysBottomoftheWellDL",
    "__OTR__objects/object_keyring/gKeyringKeysGerudoTrainingGroundDL",
    "__OTR__objects/object_keyring/gKeyringKeysGerudoFortressDL",
    "__OTR__objects/object_keyring/gKeyringKeysGanonsCastleDL",
    "__OTR__objects/object_keyring/gKeyringKeysTreasureChestGameDL",
};
static const char* sOotKeyringIconPaths[10] = {
    "__OTR__objects/object_keyring/gKeyringIconForestTempleDL",
    "__OTR__objects/object_keyring/gKeyringIconFireTempleDL",
    "__OTR__objects/object_keyring/gKeyringIconWaterTempleDL",
    "__OTR__objects/object_keyring/gKeyringIconSpiritTempleDL",
    "__OTR__objects/object_keyring/gKeyringIconShadowTempleDL",
    "__OTR__objects/object_keyring/gKeyringIconBottomoftheWellDL",
    "__OTR__objects/object_keyring/gKeyringIconGerudoTrainingGroundDL",
    "__OTR__objects/object_keyring/gKeyringIconGerudoFortressDL",
    "__OTR__objects/object_keyring/gKeyringIconGanonsCastleDL",
    "__OTR__objects/object_keyring/gKeyringIconTreasureChestGameDL",
};
// Boss keys exist for 6 dungeons only; same SoH order: Forest, Fire, Water, Spirit, Shadow, Ganon.
static const char* sOotBossKeyIconPaths[6] = {
    "__OTR__objects/object_bosskey/gBossKeyIconForestTempleDL",
    "__OTR__objects/object_bosskey/gBossKeyIconFireTempleDL",
    "__OTR__objects/object_bosskey/gBossKeyIconWaterTempleDL",
    "__OTR__objects/object_bosskey/gBossKeyIconSpiritTempleDL",
    "__OTR__objects/object_bosskey/gBossKeyIconShadowTempleDL",
    "__OTR__objects/object_bosskey/gBossKeyIconGanonsCastleDL",
};

// RI token → SoH slot. Handles all three per-dungeon families (small key, key ring, boss key).
static s32 OotKeySlotFromItem(RandoItemId randoItemId) {
    switch (randoItemId) {
        case RI_OOT_SMALL_KEY_FOREST_TEMPLE:
        case RI_OOT_KEY_RING_FOREST_TEMPLE:
        case RI_OOT_BOSS_KEY_FOREST_TEMPLE:
            return 0;
        case RI_OOT_SMALL_KEY_FIRE_TEMPLE:
        case RI_OOT_KEY_RING_FIRE_TEMPLE:
        case RI_OOT_BOSS_KEY_FIRE_TEMPLE:
            return 1;
        case RI_OOT_SMALL_KEY_WATER_TEMPLE:
        case RI_OOT_KEY_RING_WATER_TEMPLE:
        case RI_OOT_BOSS_KEY_WATER_TEMPLE:
            return 2;
        case RI_OOT_SMALL_KEY_SPIRIT_TEMPLE:
        case RI_OOT_KEY_RING_SPIRIT_TEMPLE:
        case RI_OOT_BOSS_KEY_SPIRIT_TEMPLE:
            return 3;
        case RI_OOT_SMALL_KEY_SHADOW_TEMPLE:
        case RI_OOT_KEY_RING_SHADOW_TEMPLE:
        case RI_OOT_BOSS_KEY_SHADOW_TEMPLE:
            return 4;
        case RI_OOT_SMALL_KEY_BOTTOM_OF_THE_WELL:
        case RI_OOT_KEY_RING_BOTTOM_OF_THE_WELL:
            return 5;
        case RI_OOT_SMALL_KEY_GERUDO_TRAINING_GROUND:
        case RI_OOT_KEY_RING_GERUDO_TRAINING_GROUND:
            return 6;
        case RI_OOT_SMALL_KEY_GERUDO_FORTRESS:
        case RI_OOT_KEY_RING_GERUDO_FORTRESS:
            return 7;
        case RI_OOT_SMALL_KEY_GANONS_CASTLE:
        case RI_OOT_KEY_RING_GANONS_CASTLE:
            return 8;
        case RI_OOT_BOSS_KEY_GANONS_CASTLE:
            return 5; // boss-key table is 6 entries; Ganon is index 5 there
        case RI_OOT_SMALL_KEY_TREASURE_GAME:
        case RI_OOT_KEY_RING_TREASURE_GAME:
            return 9;
        default:
            return 0;
    }
}

// SoH Randomizer_DrawSmallKey (custom path): key body Opa env-white + per-dungeon emblem Xlu.
static bool DrawOotSmallKeyReal(s32 slot) {
    static Gfx* sCustomCache = NULL;
    static Gfx* sIconCache[10] = { NULL };
    if (sCustomCache == NULL) {
        sCustomCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_key/gSmallKeyCustomDL");
    }
    if (sIconCache[slot] == NULL) {
        sIconCache[slot] = (Gfx*)OotAssets_LoadGfxDirect(sOotSmallKeyIconPaths[slot]);
    }
    if (sCustomCache == NULL || sIconCache[slot] == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255); // SoH default key body color
    gSPDisplayList(POLY_OPA_DISP++, sCustomCache);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gDPSetEnvColor(POLY_XLU_DISP++, sOotKeyEmblemColors[slot].r, sOotKeyEmblemColors[slot].g,
                   sOotKeyEmblemColors[slot].b, 255);
    gSPDisplayList(POLY_XLU_DISP++, sIconCache[slot]);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

// SoH Randomizer_DrawKeyRing (custom path): per-dungeon key bundle + ring (both Opa, env-white),
// then a fresh Opa setup for the per-dungeon emblem (SoH draws the keyring emblem Opa, not Xlu).
static bool DrawOotKeyRingReal(s32 slot) {
    static Gfx* sRingCache = NULL;
    static Gfx* sKeysCache[10] = { NULL };
    static Gfx* sIconCache[10] = { NULL };
    if (sRingCache == NULL) {
        sRingCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_keyring/gKeyringRingDL");
    }
    if (sKeysCache[slot] == NULL) {
        sKeysCache[slot] = (Gfx*)OotAssets_LoadGfxDirect(sOotKeyringKeysPaths[slot]);
    }
    if (sIconCache[slot] == NULL) {
        sIconCache[slot] = (Gfx*)OotAssets_LoadGfxDirect(sOotKeyringIconPaths[slot]);
    }
    if (sRingCache == NULL || sKeysCache[slot] == NULL || sIconCache[slot] == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255); // SoH default key color
    gSPDisplayList(POLY_OPA_DISP++, sKeysCache[slot]);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255); // SoH default ring color
    gSPDisplayList(POLY_OPA_DISP++, sRingCache);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, sOotKeyEmblemColors[slot].r, sOotKeyEmblemColors[slot].g,
                   sOotKeyEmblemColors[slot].b, 255);
    gSPDisplayList(POLY_OPA_DISP++, sIconCache[slot]);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

// SoH Randomizer_DrawBossKey (custom path): body Opa env-yellow + per-dungeon gem Xlu env-red.
static bool DrawOotBossKeyReal(s32 slot) {
    static Gfx* sCustomCache = NULL;
    static Gfx* sIconCache[6] = { NULL };
    if (sCustomCache == NULL) {
        sCustomCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_bosskey/gBossKeyCustomDL");
    }
    if (sIconCache[slot] == NULL) {
        sIconCache[slot] = (Gfx*)OotAssets_LoadGfxDirect(sOotBossKeyIconPaths[slot]);
    }
    if (sCustomCache == NULL || sIconCache[slot] == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 0, 255); // SoH default boss-key body color
    gSPDisplayList(POLY_OPA_DISP++, sCustomCache);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 0, 0, 255); // SoH default boss-key gem color
    gSPDisplayList(POLY_XLU_DISP++, sIconCache[slot]);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

void DrawOotSmallKey(RandoItemId randoItemId) { // real per-dungeon custom key; fallback: object_gi_key
    if (DrawOotSmallKeyReal(OotKeySlotFromItem(randoItemId))) {
        return;
    }
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_key/gGiSmallKeyDL", &sCache);
}
void DrawOotKeyRing(RandoItemId randoItemId) { // real ring + key bundle; fallback: single small key
    if (DrawOotKeyRingReal(OotKeySlotFromItem(randoItemId))) {
        return;
    }
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_key/gGiSmallKeyDL", &sCache);
}
void DrawOotBossKey(RandoItemId randoItemId) { // real custom boss key; fallback: gGiBossKeyDL + gem
    if (DrawOotBossKeyReal(OotKeySlotFromItem(randoItemId))) {
        return;
    }
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_bosskey/gGiBossKeyDL", &opaCache,
                         "__OTR__objects/object_gi_bosskey/gGiBossKeyGemDL", &xluCache);
}
void DrawOotDungeonMap() { // object_gi_map, single opaque DL
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_map/gGiDungeonMapDL", &sCache);
}
void DrawOotCompass() { // object_gi_compass, body Opa + glass Xlu (OoT GetItem_DrawCompass)
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_compass/gGiCompassDL", &opaCache,
                         "__OTR__objects/object_gi_compass/gGiCompassGlassDL", &xluCache);
}

// =============================================================================
// Skijer's NEI — second wave: OoT vanilla gear/spells/masks + NEI page-2 customs + extended equipment as
// MM get-items (draw + message, no-op give). Real OoT models are used wherever the OTR path resolves from
// MM (OoT-unique folder in oot.o2r, a same-named symbol in MM's own archive, or an object_nei_* mesh in
// the now-mounted soh.o2r). Items whose SoH model is compiled-in C data (not in ANY .o2r) draw DOCUMENTED
// stand-ins — usually a resolvable base mesh grayscale-tinted per item, mirroring SoH's
// DrawCustomItemDiamondTint (minus the self-rotation: MM's caller already spins freestanding items via
// the actor matrix). Each real-mesh draw keeps its stand-in as the fallback while soh.o2r is absent.
// =============================================================================

// Single opaque DL, grayscale-tinted.
static void DrawOotGetItemOpaTint(const char* otrPath, Gfx** cache, u8 r, u8 g, u8 b) {
    if (*cache == NULL) {
        *cache = (Gfx*)OotAssets_LoadGfx(otrPath);
    }
    if (*cache == NULL) {
        return; // archive not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetGrayscaleColor(POLY_OPA_DISP++, r, g, b, 255);
    gSPGrayscale(POLY_OPA_DISP++, true);
    gSPDisplayList(POLY_OPA_DISP++, *cache);
    gSPGrayscale(POLY_OPA_DISP++, false);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Two opaque DLs drawn in sequence, grayscale-tinted (tinted tunics / paired meshes).
static void DrawOotGetItemOpaOpaTint(const char* pathA, Gfx** cacheA, const char* pathB, Gfx** cacheB, u8 r, u8 g,
                                     u8 b) {
    if (*cacheA == NULL) {
        *cacheA = (Gfx*)OotAssets_LoadGfx(pathA);
    }
    if (*cacheB == NULL) {
        *cacheB = (Gfx*)OotAssets_LoadGfx(pathB);
    }
    if (*cacheA == NULL || *cacheB == NULL) {
        return; // archive not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetGrayscaleColor(POLY_OPA_DISP++, r, g, b, 255);
    gSPGrayscale(POLY_OPA_DISP++, true);
    gSPDisplayList(POLY_OPA_DISP++, *cacheA);
    gSPDisplayList(POLY_OPA_DISP++, *cacheB);
    gSPGrayscale(POLY_OPA_DISP++, false);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// --- Direct-loaded REAL OoT DLs (OotAssets_LoadGfxDirect — archive-scoped, defeats the MM shadowing
// that makes same-path OoT meshes unreachable through the normal priority chain). Each helper returns
// true once drawn so the caller can fall back to its previous (MM-mesh / tinted) draw while the DL is
// unavailable (oot.o2r not mounted, or path genuinely absent — misses are negative-cached upstream). ---
static bool DrawOotDirectOpa(const char* otrPath, Gfx** cache) {
    if (*cache == NULL) {
        *cache = (Gfx*)OotAssets_LoadGfxDirect(otrPath);
    }
    if (*cache == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, *cache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

static bool DrawOotDirectOpaTint(const char* otrPath, Gfx** cache, u8 r, u8 g, u8 b) {
    if (*cache == NULL) {
        *cache = (Gfx*)OotAssets_LoadGfxDirect(otrPath);
    }
    if (*cache == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetGrayscaleColor(POLY_OPA_DISP++, r, g, b, 255);
    gSPGrayscale(POLY_OPA_DISP++, true);
    gSPDisplayList(POLY_OPA_DISP++, *cache);
    gSPGrayscale(POLY_OPA_DISP++, false);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

// Real OoT Mirror Shield — replica of SoH z_draw.c GetItem_DrawMirrorShield: shield body Opa with a
// segment-8 two-tex scroll (the swirling face texture), then the face symbol Xlu on its own matrix.
// Both DLs direct-loaded: gGiMirrorShieldDL is SHADOWED by MM's Ikana shield (same path in mm/assets);
// the symbol DL is OoT-only but comes off the same archive for consistency.
static bool DrawOotMirrorShieldReal(void) {
    static Gfx* sBodyCache = NULL;
    static Gfx* sSymbolCache = NULL;
    if (sBodyCache == NULL) {
        sBodyCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_gi_shield_3/gGiMirrorShieldDL");
    }
    if (sSymbolCache == NULL) {
        sSymbolCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_gi_shield_3/gGiMirrorShieldSymbolDL");
    }
    if (sBodyCache == NULL || sSymbolCache == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, 0, 0 * (gPlayState->state.frames * 0) % 256,
                                             1 * (gPlayState->state.frames * 2) % 256, 64, 64, 1,
                                             0 * (gPlayState->state.frames * 0) % 128,
                                             1 * (gPlayState->state.frames * 1) % 128, 32, 32, 0, 2, 0, 1));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sBodyCache);

    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sSymbolCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

// Biggoron's Sword — the REAL OoT object_gi_longsword mesh, loaded ARCHIVE-SCOPED from oot.o2r
// (deep-patched so its vtx/textures come from oot.o2r, not MM's same-named remnant object). Its blade
// DL CALLS segment 0x08 for the animated reflection (command `gSPDisplayList 0x08000001`), so segment
// 8 MUST be set first — a replica of MM's own GetItem_DrawGoronSword (z_draw.c). DrawOotGetItemOpa did
// NOT set segment 8, so the interpreter jumped to the unresolved 0x08000001 and crashed in the vertex
// handler. Returns false if oot.o2r is unavailable so the caller can fall back.
static bool DrawOotBiggoronSwordReal(void) {
    static Gfx* sCache = NULL;
    if (sCache == NULL) {
        sCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_gi_longsword/gGiBiggoronSwordDL");
    }
    if (sCache == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, G_TX_RENDERTILE, gPlayState->state.frames * 1,
                                             gPlayState->state.frames * 0, 32, 32, 1, gPlayState->state.frames * 0,
                                             gPlayState->state.frames * 0, 32, 32, 1, 0, 0, 0));
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

// --- Real OoT models (single opaque DL — OoT GetItem_DrawOpa0 / DrawMaskOrBombchu) ---
void DrawOotBoomerang() { // object_gi_boomerang (OoT-unique folder)
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_boomerang/gGiBoomerangDL", &sCache);
}
void DrawOotHammer() { // object_gi_hammer (OoT-unique). FC 2-level chain: the floating model = the
                       // tier you'll RECEIVE — L1 Megaton Hammer (base mesh), L2 Iron Knuckle's Axe
                       // (hammer already owned) = the same mesh in a cold steel-blue grayscale tint
                       // (there is no axe GI mesh in either archive; the tint is the tier signal).
    if (Nei_Save()->ootHammerOwned) {
        static Gfx* sAxeCache = NULL;
        DrawOotGetItemOpaTint("__OTR__objects/object_gi_hammer/gGiHammerDL", &sAxeCache, 120, 145, 210);
        return;
    }
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_hammer/gGiHammerDL", &sCache);
}
void DrawOotHoverBoots() { // object_gi_hoverboots (OoT-unique)
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_hoverboots/gGiHoverBootsDL", &sCache);
}
void DrawOotGerudoCard() { // object_gi_gerudo (OoT-unique)
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_gerudo/gGiGerudoCardDL", &sCache);
}
void DrawOotStoneOfAgony() { // MM's own object_gi_map ALSO carries gGiStoneOfAgonyDL — resolves natively
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_map/gGiStoneOfAgonyDL", &sCache);
}
void DrawOotSkullMask() { // object_gi_skj_mask (OoT-unique)
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_skj_mask/gGiSkullMaskDL", &sCache);
}
void DrawOotSpookyMask() { // object_gi_redead_mask (OoT-unique)
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_redead_mask/gGiSpookyMaskDL", &sCache);
}
void DrawOotGerudoMask() { // object_gi_gerudomask (OoT-unique)
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_gerudomask/gGiGerudoMaskDL", &sCache);
}

// Iron Boots — object_gi_boots_2 (OoT-unique), boots Opa + rivets Xlu (OoT GetItem_DrawOpa0Xlu1).
void DrawOotIronBoots() {
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_boots_2/gGiIronBootsDL", &opaCache,
                         "__OTR__objects/object_gi_boots_2/gGiIronBootsRivetsDL", &xluCache);
}

// Tunics — object_gi_clothes (OoT-unique). OoT GetItem_DrawOpa1023: collar color, collar, tunic color,
// tunic — all opaque, collar/tunic geometry shared by both colors.
static void DrawOotTunic(const char* collarColorPath, Gfx** collarColorCache, const char* tunicColorPath,
                         Gfx** tunicColorCache) {
    static Gfx* sCollarCache = NULL;
    static Gfx* sTunicCache = NULL;
    if (*collarColorCache == NULL) {
        *collarColorCache = (Gfx*)OotAssets_LoadGfx(collarColorPath);
    }
    if (*tunicColorCache == NULL) {
        *tunicColorCache = (Gfx*)OotAssets_LoadGfx(tunicColorPath);
    }
    if (sCollarCache == NULL) {
        sCollarCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_clothes/gGiTunicCollarDL");
    }
    if (sTunicCache == NULL) {
        sTunicCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_clothes/gGiTunicDL");
    }
    if (*collarColorCache == NULL || *tunicColorCache == NULL || sCollarCache == NULL || sTunicCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, *collarColorCache);
    gSPDisplayList(POLY_OPA_DISP++, sCollarCache);
    gSPDisplayList(POLY_OPA_DISP++, *tunicColorCache);
    gSPDisplayList(POLY_OPA_DISP++, sTunicCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}
void DrawOotGoronTunic() {
    static Gfx* collarColorCache = NULL;
    static Gfx* tunicColorCache = NULL;
    DrawOotTunic("__OTR__objects/object_gi_clothes/gGiGoronCollarColorDL", &collarColorCache,
                 "__OTR__objects/object_gi_clothes/gGiGoronTunicColorDL", &tunicColorCache);
}
void DrawOotZoraTunic() {
    static Gfx* collarColorCache = NULL;
    static Gfx* tunicColorCache = NULL;
    DrawOotTunic("__OTR__objects/object_gi_clothes/gGiZoraCollarColorDL", &collarColorCache,
                 "__OTR__objects/object_gi_clothes/gGiZoraTunicColorDL", &tunicColorCache);
}

// Grayscale-tinted plain tunic (collar + tunic, no color DLs) — SoH's DrawCustomItemDiamondTint recolor
// used for the "clothing" ext equipment (Magic Cape / Spirit Breastplate / Champion's Tunic) and stand-ins.
static void DrawOotTunicTint(u8 r, u8 g, u8 b) {
    static Gfx* sCollarCache = NULL;
    static Gfx* sTunicCache = NULL;
    DrawOotGetItemOpaOpaTint("__OTR__objects/object_gi_clothes/gGiTunicCollarDL", &sCollarCache,
                             "__OTR__objects/object_gi_clothes/gGiTunicDL", &sTunicCache, r, g, b);
}

// Magic spells — object_gi_goddess (OoT-unique). OoT GetItem_DrawMagicSpell: Xlu with tex-scroll on
// segment 8, drawing diamond, per-spell color DL, then orb.
static void DrawOotMagicSpellSetup(void) {
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, 0, gPlayState->state.frames * 2,
                                             -(gPlayState->state.frames * 6), 32, 32, 1, gPlayState->state.frames,
                                             -(gPlayState->state.frames * 2), 32, 32, 2, -6, 1, -2));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}
static bool LoadOotMagicSpellDiamondOrb(Gfx** diamond, Gfx** orb) {
    static Gfx* sDiamondCache = NULL;
    static Gfx* sOrbCache = NULL;
    if (sDiamondCache == NULL) {
        sDiamondCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_goddess/gGiMagicSpellDiamondDL");
    }
    if (sOrbCache == NULL) {
        sOrbCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_goddess/gGiMagicSpellOrbDL");
    }
    *diamond = sDiamondCache;
    *orb = sOrbCache;
    return sDiamondCache != NULL && sOrbCache != NULL;
}
static void DrawOotMagicSpell(const char* colorPath, Gfx** colorCache) {
    Gfx* diamond;
    Gfx* orb;
    if (*colorCache == NULL) {
        *colorCache = (Gfx*)OotAssets_LoadGfx(colorPath);
    }
    if (!LoadOotMagicSpellDiamondOrb(&diamond, &orb) || *colorCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    DrawOotMagicSpellSetup();
    OPEN_DISPS(gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, diamond);
    gSPDisplayList(POLY_XLU_DISP++, *colorCache);
    gSPDisplayList(POLY_XLU_DISP++, orb);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}
void DrawOotDinsFire() {
    static Gfx* c = NULL;
    DrawOotMagicSpell("__OTR__objects/object_gi_goddess/gGiDinsFireColorDL", &c);
}
void DrawOotFaroresWind() {
    static Gfx* c = NULL;
    DrawOotMagicSpell("__OTR__objects/object_gi_goddess/gGiFaroresWindColorDL", &c);
}
void DrawOotNayrusLove() {
    static Gfx* c = NULL;
    DrawOotMagicSpell("__OTR__objects/object_gi_goddess/gGiNayrusLoveColorDL", &c);
}
// Spell stand-in for the NEI custom "spell" items (their real meshes are soh.o2r-only): the OoT spell
// diamond + orb, grayscale-tinted per item instead of using a spell color DL.
static void DrawOotMagicSpellTint(u8 r, u8 g, u8 b) {
    Gfx* diamond;
    Gfx* orb;
    if (!LoadOotMagicSpellDiamondOrb(&diamond, &orb)) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    DrawOotMagicSpellSetup();
    OPEN_DISPS(gPlayState->state.gfxCtx);
    gDPSetGrayscaleColor(POLY_XLU_DISP++, r, g, b, 255);
    gSPGrayscale(POLY_XLU_DISP++, true);
    gSPDisplayList(POLY_XLU_DISP++, diamond);
    gSPDisplayList(POLY_XLU_DISP++, orb);
    gSPGrayscale(POLY_XLU_DISP++, false);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Real NEI spell give DLs — converted from mods/items magic_spell_giveDL/model.inc.c into XML custom
// assets (object_nei_magic_spell, scratch c2xml2.py; textures = native gameplay_keep gEffUnknown10/12Tex
// in BOTH games). Packs into 2ship.o2r; fallback = the tinted diamond+orb stand-in.
static Gfx* LoadNeiRealGfx(const char* otrPath, Gfx** cache, u8* tried); // defined below (NEI real-mesh loader)
static void DrawOotNeiSpellReal(const char* path, Gfx** cache, u8* tried, u8 r, u8 g, u8 b) {
    Gfx* dl = LoadNeiRealGfx(path, cache, tried);
    if (dl != NULL) {
        DrawOotMagicSpellSetup();
        OPEN_DISPS(gPlayState->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, dl);
        CLOSE_DISPS(gPlayState->state.gfxCtx);
        return;
    }
    DrawOotMagicSpellTint(r, g, b);
}

// Mogma Mitts — stand-in: the real digging gloves OoT has, the Silver Gauntlets (object_gi_gloves,
// OoT-unique). OoT GetItem_DrawOpa10Xlu32: color + gauntlets Opa, plate color + plate Xlu.
// (The REAL object_nei_mogma_mitts mesh is tried first by DrawOotNeiMogmaMitts below.)
void DrawOotSilverGauntlets() {
    static Gfx* colorCache = NULL;
    static Gfx* gauntletsCache = NULL;
    static Gfx* plateColorCache = NULL;
    static Gfx* plateCache = NULL;
    if (colorCache == NULL) {
        colorCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_gloves/gGiSilverGauntletsColorDL");
    }
    if (gauntletsCache == NULL) {
        gauntletsCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_gloves/gGiGauntletsDL");
    }
    if (plateColorCache == NULL) {
        plateColorCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_gloves/gGiSilverGauntletsPlateColorDL");
    }
    if (plateCache == NULL) {
        plateCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_gloves/gGiGauntletsPlateDL");
    }
    if (colorCache == NULL || gauntletsCache == NULL || plateColorCache == NULL || plateCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, colorCache);
    gSPDisplayList(POLY_OPA_DISP++, gauntletsCache);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, plateColorCache);
    gSPDisplayList(POLY_XLU_DISP++, plateCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Water Dragon Scale — SoH parity: OoT scale mesh (object_gi_scale, OoT-unique), Xlu with the scale's
// tex-scroll on segment 8, grayscale blue tint (Randomizer_DrawExtWaterDragonScale).
void DrawOotWaterDragonScale() {
    static Gfx* sCache = NULL;
    if (sCache == NULL) {
        sCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_scale/gGiScaleDL");
    }
    if (sCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, 0, gPlayState->state.frames * 2,
                                             -(gPlayState->state.frames * 2), 64, 64, 1, gPlayState->state.frames * 4,
                                             -(gPlayState->state.frames * 4), 32, 32, 2, -2, 4, -4));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gDPSetGrayscaleColor(POLY_XLU_DISP++, 40, 120, 220, 255);
    gSPGrayscale(POLY_XLU_DISP++, true);
    gSPDisplayList(POLY_XLU_DISP++, sCache);
    gSPGrayscale(POLY_XLU_DISP++, false);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Progressive Master Sword — SoH parity (Randomizer_DrawMasterSword): the Temple of Time pedestal Master
// Sword mesh from vanilla OoT object_toki_objects (OoT-unique folder), scaled/rotated to fit the get-item
// cylinder, with its scrolling shine on segment 8.
void DrawOotMasterSword() {
    static Gfx* sCache = NULL;
    if (sCache == NULL) {
        sCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_toki_objects/object_toki_objects_DL_001BD0");
    }
    if (sCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    // FC 2-level chain (FCI_MASTER_SWORD): base already owned -> this pickup is the TRUE Master
    // Sword (weaponUpgrades bit 3) — same mesh in a radiant gold grayscale tint (tier signal).
    u8 trueTier = Nei_Save()->comboObtained[FC_OOT_SWORD_MASTER] != 0;
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, 0, gPlayState->state.frames, 0, 32, 32, 1, 0, 0,
                                             32, 32, 1, 0, 0, 0));
    Matrix_Scale(0.05f, 0.05f, 0.05f, MTXMODE_APPLY);
    Matrix_RotateZF(2.1f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    if (trueTier) {
        gDPSetGrayscaleColor(POLY_OPA_DISP++, 255, 215, 110, 255);
        gSPGrayscale(POLY_OPA_DISP++, true);
    }
    gSPDisplayList(POLY_OPA_DISP++, sCache);
    if (trueTier) {
        gSPGrayscale(POLY_OPA_DISP++, false);
    }
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Lantern — SoH parity (Randomizer_DrawLantern): the vanilla OoT Poe lantern mesh (object_poh, OoT-unique).
// Modeled at actor scale, so shrink to fit the get-item cylinder.
void DrawOotNeiLantern() {
    static Gfx* sCache = NULL;
    if (sCache == NULL) {
        sCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_poh/gPoeLanternDL");
    }
    if (sCache == NULL) {
        return; // oot.o2r not mounted yet — try again next frame
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Scale(0.025f, 0.025f, 0.025f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// --- REAL NEI meshes (2ship.o2r, packed from mm/assets/custom/objects/object_nei_* + object_somaria).
// NULL-safe loader: existence-gated (ResourceMgr_FileExists) and cached, so an OLDER 2ship.o2r that
// doesn't carry the object yet just returns NULL and the caller falls back to its documented
// stand-in — o2r-version-proof, never a crash on unresolved symbols. Skijer's NEI ---
extern "C" unsigned char OotAssets_PathAllowed(const char* p); // blocklist (oot_asset_loader.cpp):
// custom families whose XML DLs reference textures+palettes by Path= crash 2ship's interpreter —
// those load as NULL here so every call site draws its stand-in fallback instead.

static Gfx* LoadNeiRealGfx(const char* otrPath, Gfx** cache, u8* tried) {
    if (!OotAssets_PathAllowed(otrPath)) {
        return NULL; // blocked family -> stand-in fallback (crash-proof)
    }
    if (!*tried) {
        *tried = 1;
        if (ResourceMgr_FileExists(otrPath)) {
            *cache = ResourceMgr_LoadGfxByName(otrPath);
        }
    }
    return *cache;
}

// Opaque single-DL draw at a per-item scale — the scales mirror SoH's get-item draws for the SAME
// meshes (soh/soh/Enhancements/randomizer/draw.cpp DrawCustomItemDiamond calls). SoH's manual
// RotateY spin is omitted: MM's get-item flow already spins the item. `rotZPi` flips meshes
// authored upside-down (bomb arrows — SoH parity Matrix_RotateZ(M_PI)).
static bool DrawNeiRealOpa(const char* otrPath, Gfx** cache, u8* tried, f32 scale, bool rotZPi) {
    Gfx* dl = LoadNeiRealGfx(otrPath, cache, tried);
    if (dl == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    if (scale > 0.0f) {
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    }
    if (rotZPi) {
        Matrix_RotateZF(3.14159265f, MTXMODE_APPLY);
    }
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, dl);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

// Opa body + Xlu overlay pair (ice rod glass / net netting) — both matrices at the same scale, like
// SoH's Randomizer_DrawNet (a plain Opa draw would drop the translucent half).
static bool DrawNeiRealOpaXlu(const char* opaPath, Gfx** opaCache, u8* opaTried, const char* xluPath, Gfx** xluCache,
                              u8* xluTried, f32 scale) {
    Gfx* opa = LoadNeiRealGfx(opaPath, opaCache, opaTried);
    Gfx* xlu = LoadNeiRealGfx(xluPath, xluCache, xluTried);
    if (opa == NULL || xlu == NULL) {
        return false;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    if (scale > 0.0f) {
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    }
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, opa);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, xlu);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
    return true;
}

// --- Stand-ins for soh.o2r-only NEI meshes (each documented). The "rod" family reuses the Deku Stick
// mesh (object_gi_stick — same symbol in MM's archive, always resolves) tinted per item. ---
static void DrawOotRodStandIn(Gfx** cache, u8 r, u8 g, u8 b) {
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_stick/gGiStickDL", cache, r, g, b);
}
void DrawOotNeiFireRod() { // REAL mesh (object_nei_fire_rod) — fallback: stick tinted fire-red
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_fire_rod/Cylinder_001_opaque_dl", &real, &tried, 0.2f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 255, 70, 30);
}
void DrawOotNeiIceRod() { // REAL mesh (object_nei_ice_rod, Opa body + Xlu crystal) — fallback: ice-blue stick
    static Gfx* opa = NULL;
    static Gfx* xlu = NULL;
    static u8 opaTried = 0;
    static u8 xluTried = 0;
    if (DrawNeiRealOpaXlu("__OTR__objects/object_nei_ice_rod/ice_rod_opaque_dl", &opa, &opaTried,
                          "__OTR__objects/object_nei_ice_rod/ice_rod_transparent_dl", &xlu, &xluTried, 0.2f)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 90, 200, 255);
}
void DrawOotNeiLightRod() { // REAL mesh: converted from light_rodDL/Cylinder_002.c into XML custom
                            // assets (mm/assets/custom/objects/object_nei_light_rod, scratch c2xml.py) —
                            // packs into 2ship.o2r like the fire/ice rods, zero soh.o2r dependency.
                            // Opa body + Xlu crystal at the rod get-item scale (0.2).
    static Gfx* opa = NULL;
    static Gfx* xlu = NULL;
    static u8 opaTried = 0;
    static u8 xluTried = 0;
    if (DrawNeiRealOpaXlu("__OTR__objects/object_nei_light_rod/Cylinder_002_opaque_dl", &opa, &opaTried,
                          "__OTR__objects/object_nei_light_rod/Cylinder_002_transparent_dl", &xlu, &xluTried, 0.2f)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 255, 255, 130);
}
void DrawOotNeiCaneOfSomaria() { // REAL mesh (object_somaria give DL, SoH scale 0.25) — fallback: LTTP-red stick
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_somaria/g_somaria_cane_give_dl", &real, &tried, 0.25f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 220, 60, 50);
}
void DrawOotExtCaneOfByrna() { // REAL mesh (object_somaria blue Byrna give DL) — fallback: LTTP-blue stick
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_somaria/g_byrna_cane_give_dl", &real, &tried, 0.25f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 70, 120, 255);
}
void DrawOotNeiDominionRod() { // stand-in: stick tinted copper. PERMANENT: no dominion-rod mesh exists
                               // anywhere — SoH itself draws a placeholder green cube for this item
                               // (draw.cpp DEFINE_GREEN_CUBE_ITEM(Dominionrod)); the tinted stick is a
                               // strictly better stand-in than porting the cube.
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 205, 160, 60);
}
void DrawOotNeiWhip() { // REAL mesh (object_nei_whip, SoH scale 0.5) — fallback: leather-brown stick
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_whip/whip_give_opaque_dl", &real, &tried, 0.5f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 150, 95, 45);
}
void DrawOotNeiShovel() { // REAL mesh (object_nei_shovel, SoH scale 0.2) — fallback: steel stick
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_shovel/gShovelGiveDL_opaque_dl", &real, &tried, 0.2f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 190, 190, 200);
}
void DrawOotFishingPole() { // REAL mesh now that soh.o2r mounts (object_gi_fishing_pole, SoH
                            // Randomizer_DrawFishingPoleGI rod at scale 0.2; the dangling-lure pass is
                            // skipped — it needs OoT's fishing actor segments). Fallback: rod-brown stick.
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_gi_fishing_pole/gFishingPoleGiDL", &real, &tried, 0.2f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 150, 110, 60);
}
void DrawOotNeiSwitchHook() { // REAL mesh (object_nei_switchhook, SoH scale 0.01) — fallback: blue hookshot
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_switchhook/gSwitchHookGiveDL", &real, &tried, 0.01f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_hookshot/gGiHookshotDL", &c, 90, 140, 255);
}
void DrawOotNeiSpinner() { // REAL mesh (object_nei_spinner, SoH scale 0.3) — fallback: green compass
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_spinner/n0b0_opaque_dl", &real, &tried, 0.3f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_compass/gGiCompassDL", &c, 110, 210, 120);
}
void DrawOotNeiPokeBall() { // REAL mesh (object_nei_pokeball, SoH scale 0.18) — fallback: red bomb
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_pokeball/ItmPokeBall_opaque_dl", &real, &tried, 0.18f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_bomb_1/gGiBombDL", &c, 255, 70, 70);
}
void DrawOotNeiBallAndChain() { // REAL mesh (object_nei_ball_and_chain, SoH scale 0.25) — fallback: steel bomb
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_ball_and_chain/g_ball_and_chain_dl", &real, &tried, 0.25f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_bomb_1/gGiBombDL", &c, 170, 170, 190);
}
void DrawOotNeiBombArrows() { // REAL mesh (object_nei_bombarrows, SoH: scale 0.5 + RotZ(PI) flip) —
                              // fallback composite: MM arrow bundle + bomb, both opaque on one matrix
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_bombarrows/gBombarrowsGiveDL", &real, &tried, 0.5f, true)) {
        return;
    }
    static Gfx* arrowsCache = NULL;
    static Gfx* bombCache = NULL;
    DrawOotGetItemOpaOpa("__OTR__objects/object_gi_arrow/gGiArrowMediumDL", &arrowsCache,
                         "__OTR__objects/object_gi_bomb_1/gGiBombDL", &bombCache);
}
void DrawOotNeiGustJar() { // REAL mesh (object_nei_gust_jar, SoH scale 5.0) — fallback: MM empty bottle
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_gust_jar/jar_model_dl", &real, &tried, 5.0f, false)) {
        return;
    }
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_bottle/gGiEmptyBottleCorkDL", &opaCache,
                         "__OTR__objects/object_gi_bottle/gGiEmptyBottleGlassDL", &xluCache);
}
void DrawOotNeiBeetle() { // REAL mesh (object_nei_beetle, SoH scale 0.3) — fallback: MM bottled bugs
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_beetle/g_beetle_dl", &real, &tried, 0.3f, false)) {
        return;
    }
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_insect/gGiBugContainerContentsDL", &opaCache,
                         "__OTR__objects/object_gi_insect/gGiBugContainerGlassDL", &xluCache);
}
void DrawOotNeiDekuLeaf() { // REAL mesh (object_nei_deku_leaf, SoH scale 0.5) — fallback: OoT grass tuft
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_deku_leaf/g_dekuleaf_dl", &real, &tried, 0.5f, false)) {
        return;
    }
    static Gfx* sCache = NULL;
    DrawOotGetItemOpa("__OTR__objects/object_gi_grass/gGiGrassDL", &sCache);
}
void DrawOotNeiDesireSensor() { // REAL mesh (object_nei_desire_sensor, SoH scale 0.5) — fallback: purple agony
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_desire_sensor/g_desire_sensor_dl", &real, &tried, 0.5f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_map/gGiStoneOfAgonyDL", &c, 200, 110, 255);
}
void DrawOotNeiMinishCap() { // REAL mesh (object_nei_minish_cap, SoH scale 0.5) — fallback: green tunic
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_minish_cap/Cylinder_opaque_dl", &real, &tried, 0.5f, false)) {
        return;
    }
    DrawOotTunicTint(80, 220, 80);
}
void DrawOotProgressiveRoc() { // REAL meshes (object_nei_rocs_feather/cape, SoH scales 0.5/0.6): show the
                               // NEXT chain tier (cape once the Skijer feather sits in SLOT_ROCS).
                               // Fallback: OoT bottled butterfly (a "flying thing").
    uint8_t curRoc = Nei_GetOwnedItem(24 /* SLOT_ROCS */);
    bool wantCape = (curRoc == 0xB6 /* ITEM_ROCS_FEATHER_SKIJER */) || (curRoc == 0xB7 /* ITEM_ROCS_CAPE */);
    if (wantCape) {
        static Gfx* capeReal = NULL;
        static u8 capeTried = 0;
        if (DrawNeiRealOpa("__OTR__objects/object_nei_rocs_cape/rocs_cape_mesh_dl", &capeReal, &capeTried, 0.6f,
                           false)) {
            return;
        }
    } else {
        static Gfx* featherReal = NULL;
        static u8 featherTried = 0;
        if (DrawNeiRealOpa("__OTR__objects/object_nei_rocs_feather/rocs_feather_dl", &featherReal, &featherTried, 0.5f,
                           false)) {
            return;
        }
    }
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_butterfly/gGiButterflyContainerDL", &opaCache,
                         "__OTR__objects/object_gi_butterfly/gGiButterflyGlassDL", &xluCache);
}
void DrawOotNeiMogmaMitts() { // REAL mesh (object_nei_mogma_mitts, SoH scale 0.5) — fallback: Silver Gauntlets
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_mogma_mitts/gMogmaMittsGiveDL", &real, &tried, 0.5f, false)) {
        return;
    }
    DrawOotSilverGauntlets();
}
void DrawOotNeiTimeGate() { // REAL mesh (object_nei_time_gate, SoH scale 0.5) — fallback: teal spell
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_time_gate/g_timegate_dl", &real, &tried, 0.5f, false)) {
        return;
    }
    DrawOotMagicSpellTint(100, 220, 200);
}
// Skijer's NEI bottle rando — Net get-item: REAL mesh (object_nei_net, Opa handle/hoop + Xlu netting,
// SoH Randomizer_DrawNet scale 0.3). Fallback: tan-tinted stick (o2r-version-proof).
void DrawNeiNet() {
    static Gfx* opa = NULL;
    static Gfx* xlu = NULL;
    static u8 opaTried = 0;
    static u8 xluTried = 0;
    if (DrawNeiRealOpaXlu("__OTR__objects/object_nei_net/g_net_dl", &opa, &opaTried,
                          "__OTR__objects/object_nei_net/g_net_xlu_dl", &xlu, &xluTried, 0.3f)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotRodStandIn(&c, 200, 170, 110);
}
void DrawOotSkeletonKey() { // stand-in: small key mesh tinted gold (SoH env color 255,255,170 on its custom key)
    static Gfx* c = NULL;
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_key/gGiSmallKeyDL", &c, 255, 235, 140);
}
void DrawOotExtFourSword() { // Four Sword = Kokiri Sword mesh tinted Four-Sword green (SoH tint).
    // Use MM's OWN Kokiri Sword parts (object_gi_sword_1 gGiKokiriSwordGuardDL + gGiKokiriSwordBladeHiltDL,
    // resolved via the normal chain). Rationale: oot.o2r's COMBINED gGiKokiriSwordDL does not exist in
    // mm.o2r (MM only ships the two part DLs), and OoT get-item meshes are authored at a LARGER unit scale
    // than MM's — direct-loading the OoT combined DL rendered the sword GIANT. MM's parts are authored at
    // MM's get-item scale, so they draw at the correct size. Both children resolve consistently from MM.
    static Gfx* guardCache = NULL;
    static Gfx* bladeCache = NULL;
    DrawOotGetItemOpaOpaTint("__OTR__objects/object_gi_sword_1/gGiKokiriSwordGuardDL", &guardCache,
                             "__OTR__objects/object_gi_sword_1/gGiKokiriSwordBladeHiltDL", &bladeCache, 0, 180, 80);
}
void DrawOotExtDivineShield() { // REAL mesh (object_nei_divine_shield, SoH scale 0.5) — fallback: gold Hero's
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_divine_shield/g_divine_shield_dl", &real, &tried, 0.5f, false)) {
        return;
    }
    static Gfx* shieldCache = NULL;
    static Gfx* emblemCache = NULL;
    DrawOotGetItemOpaOpaTint("__OTR__objects/object_gi_shield_2/gGiHerosShieldDL", &shieldCache,
                             "__OTR__objects/object_gi_shield_2/gGiHerosShieldEmblemDL", &emblemCache, 255, 215, 100);
}
void DrawOotExtSheikahShield() { // REAL mesh (object_nei_kite_shield, SoH scale 0.5) — fallback: slate Deku
    static Gfx* real = NULL;
    static u8 tried = 0;
    if (DrawNeiRealOpa("__OTR__objects/object_nei_kite_shield/g_kite_shield_dl", &real, &tried, 0.5f, false)) {
        return;
    }
    static Gfx* c = NULL;
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_shield_1/gGiDekuShieldDL", &c, 90, 90, 130);
}
void DrawOotDekuShield() { // REAL OoT Deku Shield (object_gi_shield_1/gGiDekuShieldDL — OoT-unique folder),
                           // direct-loaded for archive-scoped certainty. Fallback: MM's Hero's Shield mesh
                           // (shield + emblem DLs) in a wooden-brown grayscale tint.
    static Gfx* sDirectCache = NULL;
    if (DrawOotDirectOpa("__OTR__objects/object_gi_shield_1/gGiDekuShieldDL", &sDirectCache)) {
        return;
    }
    static Gfx* shieldCache = NULL;
    static Gfx* emblemCache = NULL;
    DrawOotGetItemOpaOpaTint("__OTR__objects/object_gi_shield_2/gGiHerosShieldDL", &shieldCache,
                             "__OTR__objects/object_gi_shield_2/gGiHerosShieldEmblemDL", &emblemCache, 165, 115, 60);
}

// ─── Third wave (final cross items) ──────────────────────────────────────────────────────────────

// Climb (SoH RG_CLIMB) — replica of SoH Randomizer_DrawLadder: the Forest Temple ladder DL drawn
// twice (front + back, second rotated pi), segment 8 = the wooden pillar texture the DL samples.
// object_mori_objects is OoT-unique (no MM folder) so the normal chain resolves it from oot.o2r.
void DrawOotClimbLadder() {
    static Gfx* sLadderCache = NULL;
    static void* sTexCache = NULL;
    if (sLadderCache == NULL) {
        sLadderCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_mori_objects/gMoriHashigoLadderDL");
    }
    if (sTexCache == NULL) {
        sTexCache = OotAssets_LoadTexOrDList("__OTR__objects/object_mori_objects/gMoriHashiraTex");
    }
    if (sLadderCache == NULL || sTexCache == NULL) {
        return;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)sTexCache);
    Matrix_Translate(0.0f, -30.0f, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(1.0f, 0.25f, 1.0f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sLadderCache);
    Matrix_RotateYF(M_PIf, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sLadderCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Crawl (SoH RG_CRAWL) — replica of SoH Randomizer_DrawKneePads: two Deku Shields worn as knee pads.
// Same direct-loaded real OoT shield mesh as DrawOotDekuShield; while unavailable, fall back to the
// single-shield draw so the item is never invisible.
void DrawOotKneePads() {
    static Gfx* sShieldCache = NULL;
    if (sShieldCache == NULL) {
        sShieldCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_gi_shield_1/gGiDekuShieldDL");
    }
    if (sShieldCache == NULL) {
        DrawOotDekuShield();
        return;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Push();
    Matrix_Translate(-35.0f, -5.0f, 0.0f, MTXMODE_APPLY);
    Matrix_Scale(0.4f, 0.8f, 1.2f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sShieldCache);
    Matrix_Pop();
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    Matrix_Translate(35.0f, -7.0f, 4.0f, MTXMODE_APPLY);
    Matrix_Scale(0.4f, 0.8f, 1.2f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sShieldCache);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Jabber nuts (SoH RG_SPEAK_*) — SoH's REAL custom per-race nut meshes (soh.o2r objects/object_jabbernut,
// direct load: archive-scoped first, normal chain fallback), env-tinted with SoH's default colors
// (Randomizer_DrawJabberNut). Fallback while soh.o2r is absent: MM's own deku-nut GI model.
void DrawOotJabberNut(RandoItemId randoItemId) {
    struct NutDesc {
        const char* path;
        u8 r, g, b;
    };
    // Order matches the RI_OOT_SPEAK_* enum block (alphabetical: Deku..Zora).
    static const NutDesc sNuts[6] = {
        { "__OTR__objects/object_jabbernut/gGiDekuJabbernutDL", 255, 160, 32 },
        { "__OTR__objects/object_jabbernut/gGiGerudoJabbernutDL", 128, 64, 0 },
        { "__OTR__objects/object_jabbernut/gGiGoronJabbernutDL", 255, 32, 0 },
        { "__OTR__objects/object_jabbernut/gGiHylianJabbernutDL", 255, 255, 0 },
        { "__OTR__objects/object_jabbernut/gGiKokiriJabbernutDL", 128, 216, 48 },
        { "__OTR__objects/object_jabbernut/gGiZoraJabbernutDL", 96, 240, 255 },
    };
    static Gfx* sCaches[6] = { NULL };
    s32 idx = randoItemId - RI_OOT_SPEAK_DEKU;
    if (idx < 0 || idx >= 6) {
        return;
    }
    if (sCaches[idx] == NULL) {
        sCaches[idx] = (Gfx*)OotAssets_LoadGfxDirect(sNuts[idx].path);
    }
    if (sCaches[idx] == NULL) {
        GetItem_Draw(gPlayState, GID_DEKU_NUTS); // MM's own nut model while soh.o2r is absent
        return;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, sNuts[idx].r, sNuts[idx].g, sNuts[idx].b, 255);
    gSPDisplayList(POLY_OPA_DISP++, sCaches[idx]);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// OoT Gold Skulltula Token — the REAL OoT object_gi_sutaru mesh, replica of OoT GetItem_DrawSkullToken
// (token Opa + flame Xlu with the vanilla two-tex scroll). SHADOW VERDICT: mm/assets ALSO carries
// objects/object_gi_sutaru with the SAME symbol names, so the normal chain would return MM's copy —
// both DLs are direct-loaded off the oot.o2r archive handle. Fallback: MM's own token, gold-tinted
// flame (so it still reads as "GS token" while oot.o2r is absent).
void DrawOotGsToken() {
    static Gfx* sTokenCache = NULL;
    static Gfx* sFlameCache = NULL;
    if (sTokenCache == NULL) {
        sTokenCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_gi_sutaru/gGiSkulltulaTokenDL");
    }
    if (sFlameCache == NULL) {
        sFlameCache = (Gfx*)OotAssets_LoadGfxDirect("__OTR__objects/object_gi_sutaru/gGiSkulltulaTokenFlameDL");
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sTokenCache != NULL ? sTokenCache : (Gfx*)gSkulltulaTokenDL);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    if (sFlameCache == NULL) {
        // MM CopyDL fallback has its prim/env stripped — feed it OoT's flame gold.
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0x80, 255, 255, 170, 255);
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 100, 0, 255);
    }
    gSPSegment(POLY_XLU_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, G_TX_RENDERTILE, 0,
                                             -(gPlayState->state.frames * 5), 32, 32, 1, 0, 0, 32, 64, 0, -5, 0, 0));
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sFlameCache != NULL ? sFlameCache : (Gfx*)gSkulltulaTokenFlameCopyDL);
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

// Ruto's Letter — OoT bottle-with-letter (object_gi_bottle_letter, OoT-unique folder): contents Opa +
// bottle glass Xlu, matching OoT z_draw { GetItem_DrawOpa0Xlu1, { gGiLetterBottleContentsDL, gGiLetterBottleDL } }.
void DrawOotRutosLetter() {
    static Gfx* opaCache = NULL;
    static Gfx* xluCache = NULL;
    DrawOotGetItemOpaXlu("__OTR__objects/object_gi_bottle_letter/gGiLetterBottleContentsDL", &opaCache,
                         "__OTR__objects/object_gi_bottle_letter/gGiLetterBottleDL", &xluCache);
}

// Bottle with Blue Fire — no MM analog anywhere. Replica of OoT GetItem_DrawBlueFire (object_gi_fire,
// OoT-unique folder): chamberstick Opa, flame Xlu billboarded on a two-tex scroll (segment 8).
void DrawOotBlueFireBottle() {
    static Gfx* sStickCache = NULL;
    static Gfx* sFlameCache = NULL;
    if (sStickCache == NULL) {
        sStickCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_fire/gGiBlueFireChamberstickDL");
    }
    if (sFlameCache == NULL) {
        sFlameCache = (Gfx*)OotAssets_LoadGfx("__OTR__objects/object_gi_fire/gGiBlueFireFlameDL");
    }
    if (sStickCache == NULL || sFlameCache == NULL) {
        return;
    }
    OPEN_DISPS(gPlayState->state.gfxCtx);
    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, sStickCache);
    Gfx_SetupDL25_Xlu(gPlayState->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScrollEx(gPlayState->state.gfxCtx, G_TX_RENDERTILE, 0, 0, 16, 32, 1,
                                             gPlayState->state.frames, -(gPlayState->state.frames * 8), 16, 32, 0, 0,
                                             1, -8));
    Matrix_Push();
    Matrix_Translate(-8.0f, -2.0f, 0.0f, MTXMODE_APPLY);
    Matrix_ReplaceRotation(&gPlayState->billboardMtxF);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, gPlayState->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, sFlameCache);
    Matrix_Pop();
    CLOSE_DISPS(gPlayState->state.gfxCtx);
}
void DrawOotExtMagicCape() { // SoH parity: tunic tinted red/purple
    DrawOotTunicTint(180, 40, 120);
}
void DrawOotExtSpiritBreastplate() { // SoH parity: tunic tinted orange
    DrawOotTunicTint(235, 110, 20);
}
void DrawOotExtChampionsTunic() { // SoH parity: tunic tinted BotW champion blue
    DrawOotTunicTint(0, 120, 215);
}
void DrawOotExtPegasusAnklet() { // SoH parity: Hover Boots mesh tinted red
    static Gfx* c = NULL;
    DrawOotGetItemOpaTint("__OTR__objects/object_gi_hoverboots/gGiHoverBootsDL", &c, 220, 40, 40);
}

void Rando::DrawItem(RandoItemId randoItemId, RandoCheckId randoCheckId, Actor* actor) {
    // Apply hilites with actor world pos before drawing
    if (actor != NULL) {
        func_800B8118(actor, gPlayState, 0);
        func_800B8050(actor, gPlayState, 0);
    }

    switch (randoItemId) {
        case RI_JUNK:
            Rando::DrawItem(Rando::CurrentJunkItem(randoCheckId), randoCheckId, actor);
            break;
        case RI_GREAT_BAY_SMALL_KEY:
        case RI_SNOWHEAD_SMALL_KEY:
        case RI_STONE_TOWER_SMALL_KEY:
        case RI_WOODFALL_SMALL_KEY:
            DrawSmallKey(randoItemId);
            break;
        case RI_GREAT_BAY_BOSS_KEY:
        case RI_SNOWHEAD_BOSS_KEY:
        case RI_STONE_TOWER_BOSS_KEY:
        case RI_WOODFALL_BOSS_KEY:
            DrawBossKey(randoItemId);
            break;
        case RI_SONG_TIME:
        case RI_SONG_STORMS:
        case RI_SONG_SUN:
        case RI_SONG_HEALING:
        case RI_SONG_SARIA:
        case RI_SONG_SOARING:
        case RI_SONG_SONATA:
        case RI_SONG_ELEGY:
        case RI_SONG_LULLABY_INTRO:
        case RI_SONG_LULLABY:
        case RI_SONG_OATH:
        case RI_SONG_EPONA:
        case RI_SONG_NOVA:
        case RI_SONG_DOUBLE_TIME:
        case RI_SONG_INVERTED_TIME:
        // Skijer's NEI — OoT (SoH) warp songs reuse MM's own note model, tinted per sage (see DrawSong).
        case RI_OOT_SONG_BOLERO_OF_FIRE:
        case RI_OOT_SONG_MINUET_OF_FOREST:
        case RI_OOT_SONG_NOCTURNE_OF_SHADOW:
        case RI_OOT_SONG_PRELUDE_OF_LIGHT:
        case RI_OOT_SONG_REQUIEM_OF_SPIRIT:
        case RI_OOT_SONG_SERENADE_OF_WATER:
        // Skijer's NEI — the 3 NEI custom songs also reuse the tinted note model (see DrawSong).
        case RI_OOT_SONG_BALLAD_OF_THE_HERO:
        case RI_OOT_SONG_COMMAND_MELODY:
        case RI_OOT_SONG_FUGUE_OF_HOME:
            DrawSong(randoItemId);
            break;
        case RI_CLOCK_TOWN_STRAY_FAIRY:
        case RI_WOODFALL_STRAY_FAIRY:
        case RI_SNOWHEAD_STRAY_FAIRY:
        case RI_GREAT_BAY_STRAY_FAIRY:
        case RI_STONE_TOWER_STRAY_FAIRY:
            DrawStrayFairy(randoItemId);
            break;
        case RI_DOUBLE_DEFENSE:
            DrawDoubleDefense();
            break;
        case RI_MILK_REFILL:
            DrawMilkRefill();
            break;
        case RI_GS_TOKEN_SWAMP:
        case RI_GS_TOKEN_OCEAN:
            DrawSkulltulaToken(randoItemId, actor);
            break;
        case RI_OWL_CLOCK_TOWN_SOUTH:
        case RI_OWL_GREAT_BAY_COAST:
        case RI_OWL_IKANA_CANYON:
        case RI_OWL_MILK_ROAD:
        case RI_OWL_MOUNTAIN_VILLAGE:
        case RI_OWL_SNOWHEAD:
        case RI_OWL_SOUTHERN_SWAMP:
        case RI_OWL_STONE_TOWER:
        case RI_OWL_WOODFALL:
        case RI_OWL_ZORA_CAPE:
            DrawOwlStatue();
            break;
        case RI_TIME_DAY_1:
        case RI_TIME_NIGHT_1:
        case RI_TIME_DAY_2:
        case RI_TIME_NIGHT_2:
        case RI_TIME_DAY_3:
        case RI_TIME_NIGHT_3:
        case RI_TIME_PROGRESSIVE:
            DrawClock(randoItemId, actor);
            break;
        case RI_WALLET_TYCOON:
            DrawTycoonWallet();
            break;
        case RI_PROGRESSIVE_LULLABY:
        case RI_PROGRESSIVE_MAGIC:
        case RI_PROGRESSIVE_BOW:
        case RI_PROGRESSIVE_BOMB_BAG:
        case RI_PROGRESSIVE_SWORD:
        case RI_PROGRESSIVE_WALLET:
            Rando::DrawItem(Rando::ConvertItem(randoItemId, randoCheckId), randoCheckId, actor);
            break;
        case RI_SOUL_ENEMY_ALIEN:
        case RI_SOUL_ENEMY_ARMOS:
        case RI_SOUL_ENEMY_BAD_BAT:
        case RI_SOUL_ENEMY_BEAMOS:
        case RI_SOUL_ENEMY_BOE:
        case RI_SOUL_ENEMY_BUBBLE:
        case RI_SOUL_ENEMY_CAPTAIN_KEETA:
        case RI_SOUL_ENEMY_CHUCHU:
        case RI_SOUL_ENEMY_DEATH_ARMOS:
        case RI_SOUL_ENEMY_DEEP_PYTHON:
        case RI_SOUL_ENEMY_DEKU_BABA:
        case RI_SOUL_ENEMY_DEXIHAND:
        case RI_SOUL_ENEMY_DINOLFOS:
        case RI_SOUL_ENEMY_DODONGO:
        case RI_SOUL_ENEMY_DRAGONFLY:
        case RI_SOUL_ENEMY_EENO:
        case RI_SOUL_ENEMY_EYEGORE:
        case RI_SOUL_ENEMY_FREEZARD:
        case RI_SOUL_ENEMY_GARO:
        case RI_SOUL_ENEMY_GEKKO:
        case RI_SOUL_ENEMY_GIANT_BEE:
        case RI_SOUL_ENEMY_GOMESS:
        case RI_SOUL_ENEMY_GUAY:
        case RI_SOUL_ENEMY_HIPLOOP:
        case RI_SOUL_ENEMY_IGOS_DU_IKANA:
        case RI_SOUL_ENEMY_IRON_KNUCKLE:
        case RI_SOUL_ENEMY_KEESE:
        case RI_SOUL_ENEMY_LEEVER:
        case RI_SOUL_ENEMY_LIKE_LIKE:
        case RI_SOUL_ENEMY_MAD_SCRUB:
        case RI_SOUL_ENEMY_NEJIRON:
        case RI_SOUL_ENEMY_OCTOROK:
        case RI_SOUL_ENEMY_PEAHAT:
        case RI_SOUL_ENEMY_PIRATE:
        case RI_SOUL_ENEMY_POE:
        case RI_SOUL_ENEMY_REDEAD:
        case RI_SOUL_ENEMY_SHELLBLADE:
        case RI_SOUL_ENEMY_SKULLFISH:
        case RI_SOUL_ENEMY_SKULLTULA:
        case RI_SOUL_ENEMY_SNAPPER:
        case RI_SOUL_ENEMY_STALCHILD:
        case RI_SOUL_ENEMY_TAKKURI:
        case RI_SOUL_ENEMY_TEKTITE:
        case RI_SOUL_ENEMY_WALLMASTER:
        case RI_SOUL_ENEMY_WART:
        case RI_SOUL_ENEMY_WIZROBE:
        case RI_SOUL_ENEMY_WOLFOS:
            DrawSoul(randoItemId);
            break;
        case RI_SOUL_BOSS_GOHT:
            DrawGoht();
            break;
        case RI_SOUL_BOSS_GYORG:
            DrawGyorg();
            break;
        case RI_SOUL_BOSS_MAJORA:
            DrawMajora();
            break;
        case RI_SOUL_BOSS_ODOLWA:
            DrawOdolwa();
            break;
        case RI_SOUL_BOSS_TWINMOLD:
            DrawTwinmold();
            break;
        case RI_FROG_BLUE:
        case RI_FROG_CYAN:
        case RI_FROG_PINK:
        case RI_FROG_WHITE:
            DrawMinifrog(randoItemId, actor);
            break;
        case RI_ABILITY_SWIM:
            DrawAbilityItem(randoItemId, actor);
            break;
        case RI_TRIFORCE_PIECE_PREVIOUS:
        case RI_TRIFORCE_PIECE:
            DrawTriforcePiece(randoItemId);
            break;
        case RI_TRAP:
            Rando::DrawItem(Rando::CurrentTrapItem(randoCheckId), randoCheckId, actor);
            break;
        case RI_MAX_TRAP:
            DrawTrapModel();
            break;
        case RI_OCARINA_BUTTON_A:
        case RI_OCARINA_BUTTON_C_DOWN:
        case RI_OCARINA_BUTTON_C_LEFT:
        case RI_OCARINA_BUTTON_C_RIGHT:
        case RI_OCARINA_BUTTON_C_UP:
            DrawOcarinaButtonItem(randoItemId, actor);
            break;
        case RI_DEKU_SEEDS: // Skijer's NEI — OoT slingshot ammo (object_gi_seed)
            DrawDekuSeeds();
            break;
        case RI_FAIRY_SLINGSHOT: // Skijer's NEI — OoT Fairy Slingshot (object_gi_pachinko)
            DrawFairySlingshot();
            break;
        // Skijer's NEI — OoT (SoH) bean souls: all draw the OoT magic-bean sprout.
        case RI_SOUL_OOT_BEAN_DEATH_MOUNTAIN_CRATER:
        case RI_SOUL_OOT_BEAN_DEATH_MOUNTAIN_TRAIL:
        case RI_SOUL_OOT_BEAN_DESERT_COLOSSUS:
        case RI_SOUL_OOT_BEAN_GERUDO_VALLEY:
        case RI_SOUL_OOT_BEAN_GRAVEYARD:
        case RI_SOUL_OOT_BEAN_KOKIRI_FOREST:
        case RI_SOUL_OOT_BEAN_LAKE_HYLIA:
        case RI_SOUL_OOT_BEAN_LOST_WOODS:
        case RI_SOUL_OOT_BEAN_LOST_WOODS_BRIDGE:
        case RI_SOUL_OOT_BEAN_ZORAS_RIVER:
            DrawOotBeanSoul();
            break;
        // Skijer's NEI — OoT (SoH) boss souls: all draw the OoT blue-fire flame, tinted per boss.
        case RI_SOUL_OOT_BOSS_BARINADE:
        case RI_SOUL_OOT_BOSS_BONGO_BONGO:
        case RI_SOUL_OOT_BOSS_GANON:
        case RI_SOUL_OOT_BOSS_GOHMA:
        case RI_SOUL_OOT_BOSS_KING_DODONGO:
        case RI_SOUL_OOT_BOSS_MORPHA:
        case RI_SOUL_OOT_BOSS_PHANTOM_GANON:
        case RI_SOUL_OOT_BOSS_TWINROVA:
        case RI_SOUL_OOT_BOSS_VOLVAGIA:
            DrawOotBossSoul(randoItemId);
            break;
        // Skijer's NEI — OoT (SoH) trade-chain items, drawn from oot.o2r models.
        case RI_OOT_TRADE_POCKET_EGG:
        case RI_OOT_TRADE_WEIRD_EGG:
            DrawOotEgg();
            break;
        case RI_OOT_TRADE_ZELDAS_LETTER:
            DrawOotZeldasLetter();
            break;
        case RI_OOT_TRADE_COJIRO:
            DrawOotCojiro();
            break;
        case RI_OOT_TRADE_ODD_MUSHROOM:
            DrawOotOddMushroom();
            break;
        case RI_OOT_TRADE_ODD_POTION:
            DrawOotOddPotion();
            break;
        case RI_OOT_TRADE_POACHERS_SAW:
            DrawOotPoachersSaw();
            break;
        case RI_OOT_TRADE_BROKEN_GORONS_SWORD:
            DrawOotBrokenGoronsSword();
            break;
        case RI_OOT_TRADE_PRESCRIPTION:
            DrawOotPrescription();
            break;
        case RI_OOT_TRADE_EYEBALL_FROG:
            DrawOotEyeballFrog();
            break;
        case RI_OOT_TRADE_EYEDROPS:
            DrawOotEyedrops();
            break;
        case RI_OOT_TRADE_CLAIM_CHECK:
            DrawOotClaimCheck();
            break;
        // Skijer's NEI — OoT (SoH) per-dungeon items. Small keys / key rings / boss keys draw SoH's
        // REAL per-dungeon custom key models from soh.o2r (see DrawOotSmallKey / DrawOotKeyRing /
        // DrawOotBossKey); maps/compasses share one model per type like before.
        case RI_OOT_SMALL_KEY_BOTTOM_OF_THE_WELL:
        case RI_OOT_SMALL_KEY_FIRE_TEMPLE:
        case RI_OOT_SMALL_KEY_FOREST_TEMPLE:
        case RI_OOT_SMALL_KEY_GANONS_CASTLE:
        case RI_OOT_SMALL_KEY_GERUDO_FORTRESS:
        case RI_OOT_SMALL_KEY_GERUDO_TRAINING_GROUND:
        case RI_OOT_SMALL_KEY_SHADOW_TEMPLE:
        case RI_OOT_SMALL_KEY_SPIRIT_TEMPLE:
        case RI_OOT_SMALL_KEY_TREASURE_GAME:
        case RI_OOT_SMALL_KEY_WATER_TEMPLE:
            DrawOotSmallKey(randoItemId);
            break;
        case RI_OOT_KEY_RING_BOTTOM_OF_THE_WELL:
        case RI_OOT_KEY_RING_FIRE_TEMPLE:
        case RI_OOT_KEY_RING_FOREST_TEMPLE:
        case RI_OOT_KEY_RING_GANONS_CASTLE:
        case RI_OOT_KEY_RING_GERUDO_FORTRESS:
        case RI_OOT_KEY_RING_GERUDO_TRAINING_GROUND:
        case RI_OOT_KEY_RING_SHADOW_TEMPLE:
        case RI_OOT_KEY_RING_SPIRIT_TEMPLE:
        case RI_OOT_KEY_RING_TREASURE_GAME:
        case RI_OOT_KEY_RING_WATER_TEMPLE:
            DrawOotKeyRing(randoItemId);
            break;
        case RI_OOT_BOSS_KEY_FIRE_TEMPLE:
        case RI_OOT_BOSS_KEY_FOREST_TEMPLE:
        case RI_OOT_BOSS_KEY_GANONS_CASTLE:
        case RI_OOT_BOSS_KEY_SHADOW_TEMPLE:
        case RI_OOT_BOSS_KEY_SPIRIT_TEMPLE:
        case RI_OOT_BOSS_KEY_WATER_TEMPLE:
            DrawOotBossKey(randoItemId);
            break;
        case RI_OOT_MAP_BOTTOM_OF_THE_WELL:
        case RI_OOT_MAP_DEKU_TREE:
        case RI_OOT_MAP_DODONGOS_CAVERN:
        case RI_OOT_MAP_FIRE_TEMPLE:
        case RI_OOT_MAP_FOREST_TEMPLE:
        case RI_OOT_MAP_ICE_CAVERN:
        case RI_OOT_MAP_JABU_JABUS_BELLY:
        case RI_OOT_MAP_SHADOW_TEMPLE:
        case RI_OOT_MAP_SPIRIT_TEMPLE:
        case RI_OOT_MAP_WATER_TEMPLE:
            DrawOotDungeonMap();
            break;
        case RI_OOT_COMPASS_BOTTOM_OF_THE_WELL:
        case RI_OOT_COMPASS_DEKU_TREE:
        case RI_OOT_COMPASS_DODONGOS_CAVERN:
        case RI_OOT_COMPASS_FIRE_TEMPLE:
        case RI_OOT_COMPASS_FOREST_TEMPLE:
        case RI_OOT_COMPASS_ICE_CAVERN:
        case RI_OOT_COMPASS_JABU_JABUS_BELLY:
        case RI_OOT_COMPASS_SHADOW_TEMPLE:
        case RI_OOT_COMPASS_SPIRIT_TEMPLE:
        case RI_OOT_COMPASS_WATER_TEMPLE:
            DrawOotCompass();
            break;
        // Skijer's NEI — OoT (SoH) medallions & spiritual stones, drawn from oot.o2r models.
        case RI_OOT_MEDALLION_FIRE:
            DrawOotMedallionFire();
            break;
        case RI_OOT_MEDALLION_FOREST:
            DrawOotMedallionForest();
            break;
        case RI_OOT_MEDALLION_LIGHT:
            DrawOotMedallionLight();
            break;
        case RI_OOT_MEDALLION_SHADOW:
            DrawOotMedallionShadow();
            break;
        case RI_OOT_MEDALLION_SPIRIT:
            DrawOotMedallionSpirit();
            break;
        case RI_OOT_MEDALLION_WATER:
            DrawOotMedallionWater();
            break;
        case RI_OOT_STONE_KOKIRI_EMERALD:
            DrawOotStoneKokiriEmerald();
            break;
        case RI_OOT_STONE_GORON_RUBY:
            DrawOotStoneGoronRuby();
            break;
        case RI_OOT_STONE_ZORA_SAPPHIRE:
            DrawOotStoneZoraSapphire();
            break;
        // Skijer's NEI — second wave: OoT vanilla gear/spells/masks (real oot.o2r models).
        case RI_OOT_BOOMERANG:
            DrawOotBoomerang();
            break;
        case RI_OOT_PROGRESSIVE_HAMMER:
            DrawOotHammer();
            break;
        case RI_OOT_PROGRESSIVE_MASTER_SWORD:
            DrawOotMasterSword();
            break;
        case RI_OOT_HOVER_BOOTS:
            DrawOotHoverBoots();
            break;
        case RI_OOT_IRON_BOOTS:
            DrawOotIronBoots();
            break;
        case RI_OOT_GORON_TUNIC:
            DrawOotGoronTunic();
            break;
        case RI_OOT_ZORA_TUNIC:
            DrawOotZoraTunic();
            break;
        case RI_OOT_DINS_FIRE:
            DrawOotDinsFire();
            break;
        case RI_OOT_FARORES_WIND:
            DrawOotFaroresWind();
            break;
        case RI_OOT_NAYRUS_LOVE:
            DrawOotNayrusLove();
            break;
        case RI_OOT_GERUDO_MEMBERSHIP_CARD:
            DrawOotGerudoCard();
            break;
        case RI_OOT_STONE_OF_AGONY:
            DrawOotStoneOfAgony();
            break;
        case RI_OOT_MASK_SKULL:
            DrawOotSkullMask();
            break;
        case RI_OOT_MASK_SPOOKY:
            DrawOotSpookyMask();
            break;
        case RI_OOT_MASK_GERUDO:
            DrawOotGerudoMask();
            break;
        case RI_OOT_DEKU_SHIELD: // real OoT object_gi_shield_1 mesh (direct load; Hero's Shield brown fallback)
            DrawOotDekuShield();
            break;
        // Third wave (final cross items). The 8 MM-native bottled contents (big poe / potions / bugs /
        // fairy / fish / mushroom / poe) draw through the DEFAULT GetItem_Draw path via their MM GIDs.
        case RI_OOT_ABILITY_CLIMB: // SoH ladder draw replica (object_mori_objects)
            DrawOotClimbLadder();
            break;
        case RI_OOT_ABILITY_CRAWL: // SoH knee-pads replica (two deku shields)
            DrawOotKneePads();
            break;
        case RI_OOT_SPEAK_DEKU:
        case RI_OOT_SPEAK_GERUDO:
        case RI_OOT_SPEAK_GORON:
        case RI_OOT_SPEAK_HYLIAN:
        case RI_OOT_SPEAK_KOKIRI:
        case RI_OOT_SPEAK_ZORA: // SoH's real per-race jabber-nut meshes (soh.o2r), SoH default tints
            DrawOotJabberNut(randoItemId);
            break;
        case RI_OOT_GS_TOKEN: // real OoT token (direct load — MM shadows object_gi_sutaru)
            DrawOotGsToken();
            break;
        case RI_OOT_RUTOS_LETTER: // OoT bottle-with-letter (object_gi_bottle_letter)
            DrawOotRutosLetter();
            break;
        case RI_OOT_BOTTLE_BLUE_FIRE: // OoT blue-fire chamberstick+flame (no MM analog)
            DrawOotBlueFireBottle();
            break;
        // MM-native models via GetItem_Draw (documented stand-ins / intended MM skins).
        case RI_OOT_MIRROR_SHIELD: {
            // OoT's own Mirror Shield — the REAL object_gi_shield_3 mesh (body Opa w/ tex scroll +
            // symbol Xlu, SoH GetItem_DrawMirrorShield replica), direct-loaded off the oot.o2r
            // archive handle: the path is SHADOWED (mm/assets ships its own objects/object_gi_shield_3
            // with the same gGiMirrorShieldDL symbol = MM's Ikana mesh), so only the archive-scoped
            // loader can reach it. Fallback while unavailable: the previous differentiation — the
            // (MM-resolved) Ikana mesh in a warm red/silver grayscale tint.
            if (DrawOotMirrorShieldReal()) {
                break;
            }
            static Gfx* sCache = NULL;
            DrawOotGetItemOpaTint("__OTR__objects/object_gi_shield_3/gGiMirrorShieldDL", &sCache, 255, 125, 115);
            break;
        }
        case RI_HOOKSHOT: {
            // MM-native hookshot pickup + FC 3-level chain (FCI_HOOKSHOT): the floating model = the
            // tier you'll RECEIVE, drawn with the REAL OoT get-item meshes direct-loaded off the
            // oot.o2r archive handle (gGiHookshotDL is shadowed by MM's same-path mesh; gGiLongshotDL
            // only exists in OoT's object_gi_hookshot). Tier 1 = OoT Hookshot, tier 2 = OoT Longshot,
            // tier 3 (Ultrashot — no distinct GI mesh anywhere) = the Longshot mesh in the violet
            // tier-signal tint. Fallbacks while the direct load is unavailable = the previous draws:
            // MM's native hookshot mesh (plain / gold tint / violet tint).
            u8 lvl = Nei_Save()->ootHookshotLevel;
            if (lvl == 0 && INV_CONTENT(ITEM_HOOKSHOT) == ITEM_HOOKSHOT) {
                lvl = 1; // pre-chain native hookshot with no level recorded (same idiom as FleetSync)
            }
            if (lvl == 0) {
                static Gfx* sHookCache = NULL; // next tier = Hookshot (real OoT mesh)
                if (!DrawOotDirectOpa("__OTR__objects/object_gi_hookshot/gGiHookshotDL", &sHookCache)) {
                    GetItem_Draw(gPlayState, GID_HOOKSHOT); // fallback: MM's own hookshot model
                }
            } else if (lvl == 1) {
                static Gfx* sLongDirectCache = NULL; // next tier = Longshot (real OoT mesh, natural colors)
                if (!DrawOotDirectOpa("__OTR__objects/object_gi_hookshot/gGiLongshotDL", &sLongDirectCache)) {
                    static Gfx* sLongCache = NULL; // fallback: MM hookshot mesh tinted gold
                    DrawOotGetItemOpaTint("__OTR__objects/object_gi_hookshot/gGiHookshotDL", &sLongCache, 255, 215,
                                          110);
                }
            } else {
                static Gfx* sUltraDirectCache = NULL; // next tier = Ultrashot (OoT Longshot mesh, violet tier tint)
                if (!DrawOotDirectOpaTint("__OTR__objects/object_gi_hookshot/gGiLongshotDL", &sUltraDirectCache, 195,
                                          125, 255)) {
                    static Gfx* sUltraCache = NULL; // fallback: MM hookshot mesh tinted violet
                    DrawOotGetItemOpaTint("__OTR__objects/object_gi_hookshot/gGiHookshotDL", &sUltraCache, 195, 125,
                                          255);
                }
            }
            break;
        }
        case RI_CLAWSHOT: {
            // Clawshot is its own item but shares GID_HOOKSHOT in the static table — differentiate
            // it from RI_HOOKSHOT (the OoT progressive chain tier 1, which draws the natural-color
            // OoT hookshot mesh). Chosen: the REAL OoT object_gi_hookshot mesh, direct-loaded off
            // the mounted OoT archive handles (same-path shadowing makes it unreachable through the
            // normal chain), in a slight red claw tint — enough to read "clawshot" next to the
            // chain tier-1's untinted copy while still being the real model. Fallback while the
            // direct load is unavailable: the previous draw — the (MM-resolved) hookshot mesh in
            // the same red tint.
            static Gfx* sDirectCache = NULL;
            if (DrawOotDirectOpaTint("__OTR__objects/object_gi_hookshot/gGiHookshotDL", &sDirectCache, 255, 150, 140)) {
                break;
            }
            static Gfx* sCache = NULL;
            DrawOotGetItemOpaTint("__OTR__objects/object_gi_hookshot/gGiHookshotDL", &sCache, 255, 110, 95);
            break;
        }
        case RI_GREAT_FAIRY_SWORD:
            // FC 2-level chain (FCI_BIGGORON_SWORD): the floating model = the tier you'll RECEIVE.
            // First copy = Biggoron's Sword — the REAL OoT mesh from oot.o2r (its blade DL calls
            // segment 8 for the animated reflection, so it needs DrawOotBiggoronSwordReal which sets
            // that segment up; drawing it with a plain draw crashed). Second copy = the actual Great
            // Fairy's Sword (native GID). If oot.o2r is unavailable, fall back to MM's own native
            // Biggoron's Sword draw (GID_SWORD_BGS = GetItem_DrawGoronSword, which also sets segment 8).
            if (Nei_Save()->comboObtained[FC_OOT_SWORD_BIGGORON] == 0) {
                if (!DrawOotBiggoronSwordReal()) {
                    GetItem_Draw(gPlayState, GID_SWORD_BGS);
                }
            } else {
                GetItem_Draw(gPlayState, GID_SWORD_GREAT_FAIRY);
            }
            break;
        case RI_OOT_GREG: // SoH draws Greg as the vanilla green rupee — MM's green rupee here
            GetItem_Draw(gPlayState, GID_RUPEE_GREEN);
            break;
        case RI_OOT_BOMBCHU_BAG: // SoH's bag mesh is soh.o2r-only — MM's bombchu model stands in
            GetItem_Draw(gPlayState, GID_BOMBCHU);
            break;
        // NEI page-2 / ext equipment / misc (real model or documented stand-in per draw func).
        case RI_OOT_NEI_BALL_AND_CHAIN:
            DrawOotNeiBallAndChain();
            break;
        case RI_OOT_NEI_BEETLE:
            DrawOotNeiBeetle();
            break;
        case RI_OOT_NEI_BOMB_ARROWS:
            DrawOotNeiBombArrows();
            break;
        case RI_OOT_NEI_CANE_OF_SOMARIA:
            DrawOotNeiCaneOfSomaria();
            break;
        case RI_OOT_NEI_DEKU_LEAF:
            DrawOotNeiDekuLeaf();
            break;
        case RI_OOT_NEI_DEMISE_DESTRUCTION: { // REAL converted mesh (black core) + tint fallback
            static Gfx* c = NULL;
            static u8 tr = 0;
            DrawOotNeiSpellReal("__OTR__objects/object_nei_magic_spell/gDemiseDestructionGiveDL", &c, &tr, 150, 30, 30);
            break;
        }
        case RI_OOT_NEI_DESIRE_SENSOR:
            DrawOotNeiDesireSensor();
            break;
        case RI_OOT_NEI_DOMINION_ROD:
            DrawOotNeiDominionRod();
            break;
        case RI_OOT_NEI_FIRE_ROD:
            DrawOotNeiFireRod();
            break;
        case RI_OOT_NEI_GUST_JAR:
            DrawOotNeiGustJar();
            break;
        case RI_OOT_NEI_HYLIAS_GRACE: { // REAL converted mesh (pink/violet spell) + tint fallback
            static Gfx* c = NULL;
            static u8 tr = 0;
            DrawOotNeiSpellReal("__OTR__objects/object_nei_magic_spell/gHyliaGraceGiveDL", &c, &tr, 255, 240, 150);
            break;
        }
        case RI_OOT_NEI_ICE_ROD:
            DrawOotNeiIceRod();
            break;
        case RI_OOT_NEI_LANTERN:
            DrawOotNeiLantern();
            break;
        case RI_OOT_NEI_LIGHT_ROD:
            DrawOotNeiLightRod();
            break;
        case RI_OOT_NEI_MINISH_CAP:
            DrawOotNeiMinishCap();
            break;
        case RI_OOT_NEI_MOGMA_MITTS:
            DrawOotNeiMogmaMitts();
            break;
        case RI_OOT_NEI_POKE_BALL:
            DrawOotNeiPokeBall();
            break;
        case RI_OOT_NEI_SHOVEL:
            DrawOotNeiShovel();
            break;
        case RI_OOT_NEI_SPINNER:
            DrawOotNeiSpinner();
            break;
        case RI_OOT_NEI_SWITCH_HOOK:
            DrawOotNeiSwitchHook();
            break;
        case RI_OOT_NEI_TIME_GATE:
            DrawOotNeiTimeGate();
            break;
        // Skijer's NEI bottle rando — Net (real object_nei_net mesh) + Bottomless Bottle (MM's own
        // empty-bottle get-item model; identity carried by the icon/name, matching the kaleido cell).
        case RI_NET:
            DrawNeiNet();
            break;
        case RI_BOTTOMLESS_BOTTLE:
            GetItem_Draw(gPlayState, GID_BOTTLE);
            break;
        case RI_OOT_NEI_WHIP:
            DrawOotNeiWhip();
            break;
        case RI_OOT_NEI_ZONAI_PERMAFROST: { // REAL converted mesh (turquoise core) + tint fallback
            static Gfx* c = NULL;
            static u8 tr = 0;
            DrawOotNeiSpellReal("__OTR__objects/object_nei_magic_spell/gZonaiPermafrostGiveDL", &c, &tr, 150, 230,
                                255);
            break;
        }
        case RI_OOT_PROGRESSIVE_ROC:
            DrawOotProgressiveRoc();
            break;
        case RI_OOT_SKELETON_KEY:
            DrawOotSkeletonKey();
            break;
        case RI_OOT_FISHING_POLE:
            DrawOotFishingPole();
            break;
        case RI_OOT_EXT_CANE_OF_BYRNA:
            DrawOotExtCaneOfByrna();
            break;
        case RI_OOT_EXT_CHAMPIONS_TUNIC:
            DrawOotExtChampionsTunic();
            break;
        case RI_OOT_EXT_DIVINE_SHIELD:
            DrawOotExtDivineShield();
            break;
        case RI_OOT_EXT_FOUR_SWORD:
            DrawOotExtFourSword();
            break;
        case RI_OOT_EXT_MAGIC_CAPE:
            DrawOotExtMagicCape();
            break;
        case RI_OOT_EXT_PEGASUS_ANKLET:
            DrawOotExtPegasusAnklet();
            break;
        case RI_OOT_EXT_SHEIKAH_SHIELD:
            DrawOotExtSheikahShield();
            break;
        case RI_OOT_EXT_SPIRIT_BREASTPLATE:
            DrawOotExtSpiritBreastplate();
            break;
        case RI_OOT_EXT_WATER_DRAGON_SCALE:
            DrawOotWaterDragonScale();
            break;
        case RI_NONE:
        case RI_UNKNOWN:
            break;
        default:
            GetItem_Draw(gPlayState, Rando::StaticData::Items[randoItemId].drawId);
            break;
    }

    switch (randoItemId) {
        case RI_NONE:
        case RI_ABILITY_SWIM:
        case RI_PROGRESSIVE_MAGIC:
        case RI_SINGLE_MAGIC:
        case RI_DOUBLE_MAGIC:
        case RI_TIME_PROGRESSIVE:
            DrawSparkles(randoItemId, actor);
            break;
        default:
            break;
    }
}

static RegisterShipInitFunc initializeGICopyDLs(
    []() {
        // Small keys
        Gfx* baseDL = ResourceMgr_LoadGfxByName(gGiSmallKeyDL);
        memcpy(gGiSmallKeyCopyDL, baseDL, sizeof(gGiSmallKeyCopyDL));
        gGiSmallKeyCopyDL[5] = gsDPNoOp();
        gGiSmallKeyCopyDL[6] = gsDPNoOp();

        // Boss keys
        baseDL = ResourceMgr_LoadGfxByName(gGiBossKeyDL);
        memcpy(gGiBossKeyCopyDL, baseDL, sizeof(gGiBossKeyCopyDL));
        gGiBossKeyCopyDL[5] = gsDPNoOp();
        gGiBossKeyCopyDL[6] = gsDPNoOp();

        // Token Flame
        baseDL = ResourceMgr_LoadGfxByName(gSkulltulaTokenFlameDL);
        memcpy(gSkulltulaTokenFlameCopyDL, baseDL, sizeof(gSkulltulaTokenFlameCopyDL));
        gSkulltulaTokenFlameCopyDL[5] = gsDPNoOp();
        gSkulltulaTokenFlameCopyDL[6] = gsDPNoOp();
    },
    {});
