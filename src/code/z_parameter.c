#include "global.h"
#include "z64snap.h"
#include "z64view.h"
#include "interface/parameter_static/parameter_static.h"
#include "interface/do_action_static/do_action_static.h"
#include "misc/story_static/story_static.h"

#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "overlays/actors/ovl_En_Mm3/z_en_mm3.h"

extern Gfx D_0E0001C8[];      // Display List
extern Gfx D_0E0002E0[];      // gScreenFillDL
extern TexturePtr D_08095AC0; // gMagicArrowEquipEffectTex

typedef enum {
    /* 0 */ PICTOGRAPH_STATE_OFF,         // Not using the pictograph
    /* 1 */ PICTOGRAPH_STATE_LENS,        // Looking through the lens of the pictograph
    /* 2 */ PICTOGRAPH_STATE_SETUP_PHOTO, // Looking at the photo currently taken
    /* 3 */ PICTOGRAPH_STATE_PHOTO
} PictographState;

// TODO extract this information from the texture definitions themselves
#define DO_ACTION_TEX_WIDTH 48
#define DO_ACTION_TEX_HEIGHT 16
#define DO_ACTION_TEX_SIZE ((DO_ACTION_TEX_WIDTH * DO_ACTION_TEX_HEIGHT) / 2) // (sizeof(gCheckDoActionENGTex))

typedef struct {
    /* 0x00 */ u8 scene;
    /* 0x01 */ u8 flags1;
    /* 0x02 */ u8 flags2;
    /* 0x03 */ u8 flags3;
} RestrictionFlags;

Input sPostmanTimerInput[4];

// TODO: Better way to do this?
#define SET_RESTRICTIONS(hGauge, bButton, aButton, tradeItems, unk_312, unk_313, unk_314, songOfSoaring, songOfStorms, \
                         unk_317, pictoBox, all)                                                                       \
    ((((hGauge)&3) << 6) | (((bButton)&3) << 4) | (((aButton)&3) << 2) | (((tradeItems)&3) << 0)),                     \
        ((((unk_312)&3) << 6) | (((unk_313)&3) << 4) | (((unk_314)&3) << 2) | (((songOfSoaring)&3) << 0)),             \
        ((((songOfStorms)&3) << 6) | (((unk_317)&3) << 4) | (((pictoBox)&3) << 2) | (((all)&3) << 0))

RestrictionFlags sRestrictionFlags[] = {
    { SCENE_20SICHITAI2, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_1, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_2, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_3, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_4, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_5, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_6, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_KAKUSIANA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_SPOT00, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_9, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_WITCH_SHOP, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_LAST_BS, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0) },
    { SCENE_HAKASHITA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_AYASHIISHOP, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_UNSET_E, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_F, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_OMOYA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0) },
    { SCENE_BOWLING, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_SONCHONOIE, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_IKANA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_KAIZOKU, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_MILK_BAR, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_INISIE_N, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_TAKARAYA, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_INISIE_R, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_OKUJOU, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 0, 0) },
    { SCENE_OPENINGDAN, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_MITURIN, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_13HUBUKINOMITI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_CASTLE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_DEKUTES, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1) },
    { SCENE_MITURIN_BS, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_SYATEKI_MIZU, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_HAKUGIN, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_ROMANYMAE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_PIRATE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_SYATEKI_MORI, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_SINKAI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_YOUSEI_IZUMI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_KINSTA1, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0) },
    { SCENE_KINDAN2, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0) },
    { SCENE_TENMON_DAI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0) },
    { SCENE_LAST_DEKU, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0) },
    { SCENE_22DEKUCITY, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_KAJIYA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 0) },
    { SCENE_00KEIKOKU, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_POSTHOUSE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_LABO, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_DANPEI2TEST, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0) },
    { SCENE_UNSET_31, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_16GORON_HOUSE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_33ZORACITY, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0) },
    { SCENE_8ITEMSHOP, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_F01, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_INISIE_BS, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_30GYOSON, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_31MISAKI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_TAKARAKUJI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_UNSET_3A, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_TORIDE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_FISHERMAN, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_GORONSHOP, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_DEKU_KING, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 0) },
    { SCENE_LAST_GORON, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0) },
    { SCENE_24KEMONOMITI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_F01_B, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0) },
    { SCENE_F01C, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0) },
    { SCENE_BOTI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_HAKUGIN_BS, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_20SICHITAI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_21MITURINMAE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_LAST_ZORA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0) },
    { SCENE_11GORONNOSATO2, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_SEA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_35TAKI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_REDEAD, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_BANDROOM, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 0) },
    { SCENE_11GORONNOSATO, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_GORON_HAKA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_SECOM, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 0, 3, 0, 0, 0) },
    { SCENE_10YUKIYAMANOMURA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_TOUGITES, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 0, 0) },
    { SCENE_DANPEI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0) },
    { SCENE_IKANAMAE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_DOUJOU, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_MUSICHOUSE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 0) },
    { SCENE_IKNINSIDE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0) },
    { SCENE_MAP_SHOP, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_F40, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_F41, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_10YUKIYAMANOMURA2, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_14YUKIDAMANOMITI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_12HAKUGINMAE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_17SETUGEN, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_17SETUGEN2, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_SEA_BS, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_RANDOM, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_YADOYA, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_KONPEKI_ENT, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_INSIDETOWER, SET_RESTRICTIONS(0, 0, 0, 0, 3, 3, 3, 3, 3, 0, 0, 0) },
    { SCENE_26SARUNOMORI, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0) },
    { SCENE_LOST_WOODS, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_LAST_LINK, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0) },
    { SCENE_SOUGEN, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0) },
    { SCENE_BOMYA, SET_RESTRICTIONS(0, 1, 0, 0, 0, 3, 0, 0, 3, 0, 0, 1) },
    { SCENE_KYOJINNOMA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_KOEPONARACE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0) },
    { SCENE_GORONRACE, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_TOWN, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_ICHIBA, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_BACKTOWN, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_CLOCKTOWER, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
    { SCENE_ALLEY, SET_RESTRICTIONS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) },
};

s16 sPictographState = PICTOGRAPH_STATE_OFF;
s16 sPictographPhotoBeingTaken = false; // pictoBox related

s16 sHBAScoreTier = 0; // Remnant of OoT, non-functional

u16 sMinigameScoreDigits[] = { 0, 0, 0, 0 };

u16 sCUpInvisible = 0;
u16 sCUpTimer = 0;

s16 sMagicMeterOutlinePrimRed = 255;
s16 sMagicMeterOutlinePrimGreen = 255;
s16 sMagicMeterOutlinePrimBlue = 255;
s16 sMagicBorderRatio = 2;
s16 sMagicBorderStep = 1;

s16 sExtraItemBases[] = {
    ITEM_STICK,   // ITEM_STICKS_5
    ITEM_STICK,   // ITEM_STICKS_10
    ITEM_NUT,     // ITEM_NUTS_5
    ITEM_NUT,     // ITEM_NUTS_10
    ITEM_BOMB,    // ITEM_BOMBS_5
    ITEM_BOMB,    // ITEM_BOMBS_10
    ITEM_BOMB,    // ITEM_BOMBS_20
    ITEM_BOMB,    // ITEM_BOMBS_30
    ITEM_BOW,     // ITEM_ARROWS_10
    ITEM_BOW,     // ITEM_ARROWS_30
    ITEM_BOW,     // ITEM_ARROWS_40
    ITEM_BOMBCHU, // ITEM_ARROWS_50 !@bug this data is missing an ITEM_BOW, offsetting the rest by 1
    ITEM_BOMBCHU, // ITEM_BOMBCHUS_20
    ITEM_BOMBCHU, // ITEM_BOMBCHUS_10
    ITEM_BOMBCHU, // ITEM_BOMBCHUS_1
    ITEM_STICK,   // ITEM_BOMBCHUS_5
    ITEM_STICK,   // ITEM_STICK_UPGRADE_20
    ITEM_NUT,     // ITEM_STICK_UPGRADE_30
    ITEM_NUT,     // ITEM_NUT_UPGRADE_30
};

s16 sEnvTimerType = PLAYER_ENV_TIMER_NONE;
s16 sEnvTimerActive = false;
s16 sPostmanBunnyHoodState = POSTMAN_MINIGAME_BUNNY_HOOD_OFF;
OSTime sTimerPausedOsTime = 0;
OSTime sBottleTimerPausedOsTime = 0;
OSTime D_801BF8F8[] = {
    0, 0, 0, 0, 0, 0, 0,
};
OSTime D_801BF930[] = {
    0, 0, 0, 0, 0, 0, 0,
};
u8 sIsTimerPaused = false;
u8 sIsBottleTimerPaused = false;
s16 sTimerId = TIMER_ID_NONE;

s16 D_801BF974 = 0;
s16 D_801BF978 = 10;
s16 D_801BF97C = 255;
f32 D_801BF980 = 1.0f;
s32 D_801BF984 = 0;

Gfx gScreenFillSetupDL[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetOtherMode(G_AD_DISABLE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_1PRIMITIVE,
                     G_AC_NONE | G_ZS_PIXEL | G_RM_CLD_SURF | G_RM_CLD_SURF2),
    gsDPSetCombineMode(G_CC_PRIMITIVE, G_CC_PRIMITIVE),
    gsSPEndDisplayList(),
};

s16 D_801BF9B0 = 0;
f32 D_801BF9B4[] = { 100.0f, 109.0f };
s16 D_801BF9BC[] = { 0x226, 0x2A8, 0x2A8, 0x2A8 };
s16 D_801BF9C4[] = { 0x9E, 0x9B };
s16 D_801BF9C8[] = { 0x17, 0x16 };
f32 D_801BF9CC[] = { -380.0f, -350.0f };
s16 D_801BF9D4[] = { 0xA7, 0xE3 };
s16 D_801BF9D8[] = { 0xF9, 0x10F };
s16 D_801BF9DC[] = { 0x11, 0x12 };
s16 D_801BF9E0[] = { 0x22, 0x12 };
s16 D_801BF9E4[] = { 0x23F, 0x26C, 0x26C, 0x26C };

s16 sFinalHoursClockDigitsRed = 0;
s16 sFinalHoursClockFrameEnvRed = 0;
s16 sFinalHoursClockFrameEnvGreen = 0;
s16 sFinalHoursClockFrameEnvBlue = 0;
s16 sFinalHoursClockColorTimer = 15;
s16 sFinalHoursClockColorTargetIndex = 0;

Gfx* Gfx_DrawTexRectRGBA16(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                           s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

Gfx* Gfx_DrawTexRectIA8(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                        s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

Gfx* Gfx_DrawTexRectIA8_DropShadow(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft,
                                   s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy, s16 r, s16 g, s16 b,
                                   s16 a) {
    s16 dropShadowAlpha = a;

    if (a > 100) {
        dropShadowAlpha = 100;
    }

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, dropShadowAlpha);

    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(gfx++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, r, g, b, a);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

Gfx* Gfx_DrawRect_DropShadow(Gfx* gfx, s16 rectLeft, s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy,
                             s16 r, s16 g, s16 b, s16 a) {
    s16 dropShadowAlpha = a;

    if (a > 100) {
        dropShadowAlpha = 100;
    }

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, dropShadowAlpha);
    gSPTextureRectangle(gfx++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, r, g, b, a);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

Gfx* Gfx_DrawTexRectIA8_DropShadowOffset(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight,
                                         s16 rectLeft, s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy,
                                         s16 r, s16 g, s16 b, s16 a, s32 masks, s32 rectS) {
    s16 dropShadowAlpha = a;

    if (a > 100) {
        dropShadowAlpha = 100;
    }

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, dropShadowAlpha);

    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_MIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, masks, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(gfx++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, rectS, 0, dsdx, dtdy);

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, r, g, b, a);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, rectS, 0, dsdx, dtdy);

    return gfx;
}

Gfx* Gfx_DrawTexRectI8(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                       s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_I, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return gfx;
}

Gfx* Gfx_DrawTexRect4b(Gfx* gfx, TexturePtr texture, s32 fmt, s16 textureWidth, s16 textureHeight, s16 rectLeft,
                       s16 rectTop, s16 rectWidth, s16 rectHeight, s32 cms, s32 masks, s32 s, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock_4b(gfx++, texture, fmt, textureWidth, textureHeight, 0, cms, G_TX_NOMIRROR | G_TX_WRAP, masks,
                           G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(gfx++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4, (rectTop + rectHeight) * 4,
                        G_TX_RENDERTILE, s, 0, dsdx, dtdy);

    return gfx;
}

Gfx* Gfx_DrawTexQuadIA8(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, u16 point) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSP1Quadrangle(gfx++, point, point + 2, point + 3, point + 1, 0);

    return gfx;
}

Gfx* Gfx_DrawTexQuad4b(Gfx* gfx, TexturePtr texture, s32 fmt, s16 textureWidth, s16 textureHeight, u16 point) {
    gDPLoadTextureBlock_4b(gfx++, texture, fmt, textureWidth, textureHeight, 0, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSP1Quadrangle(gfx++, point, point + 2, point + 3, point + 1, 0);

    return gfx;
}

s16 sActionVtxPosX[] = {
    -14, // A Button
    -14, // A Button Background
    -24, // A Button Do-Action
    -8,  // Three-Day Clock's Star (for the Minute Tracker)
    -12, // Three-Day Clock's Sun (for the Day-Time Hours Tracker)
    -12, // Three-Day Clock's Moon (for the Night-Time Hours Tracker)
    -7,  // Three-Day Clock's Hour Digit Above the Sun
    -8,  // Three-Day Clock's Hour Digit Above the Sun
    -7,  // Three-Day Clock's Hour Digit Above the Moon
    -8,  // Three-Day Clock's Hour Digit Above the Moon
    -12, // Minigame Countdown Textures
};
s16 sActionVtxWidths[] = {
    28, // A Button
    28, // A Button Background
    48, // A Button Do-Action
    16, // Three-Day Clock's Star (for the Minute Tracker)
    24, // Three-Day Clock's Sun (for the Day-Time Hours Tracker)
    24, // Three-Day Clock's Moon (for the Night-Time Hours Tracker)
    16, // Three-Day Clock's Hour Digit Above the Sun
    16, // Three-Day Clock's Hour Digit Above the Sun
    16, // Three-Day Clock's Hour Digit Above the Moon
    16, // Three-Day Clock's Hour Digit Above the Moon
    24, // Minigame Countdown Textures
};
s16 sActionVtxPosY[] = {
    14,  // A Button
    14,  // A Button Background
    8,   // A Button Do-Action
    24,  // Three-Day Clock's Star (for the Minute Tracker)
    -82, // Three-Day Clock's Sun (for the Day-Time Hours Tracker)
    -82, // Three-Day Clock's Moon (for the Night-Time Hours Tracker)
    58,  // Three-Day Clock's Hour Digit Above the Sun
    59,  // Three-Day Clock's Hour Digit Above the Sun
    58,  // Three-Day Clock's Hour Digit Above the Moon
    59,  // Three-Day Clock's Hour Digit Above the Moon
    32,  // Minigame Countdown Textures
};
s16 sActionVtxHeights[] = {
    28, // A Button
    28, // A Button Background
    16, // A Button Do-Action
    16, // Three-Day Clock's Star (for the Minute Tracker)
    24, // Three-Day Clock's Sun (for the Day-Time Hours Tracker)
    24, // Three-Day Clock's Moon (for the Night-Time Hours Tracker)
    11, // Three-Day Clock's Hour Digit Above the Sun
    11, // Three-Day Clock's Hour Digit Above the Sun
    11, // Three-Day Clock's Hour Digit Above the Moon
    11, // Three-Day Clock's Hour Digit Above the Moon
    32, // Minigame Countdown Textures
};

#define MINIGAME_PERFECT_VTX_WIDTH 32
#define MINIGAME_PERFECT_VTX_HEIGHT 33

// Both minigame-perfect vtx data indexed as the 8 letters of:
// 'p', 'e', 'r', 'f', 'e', 'c', 't', '!'
s16 sMinigamePerfectVtxPosX[] = {
    -61, -45, 29, 104, -117, -42, 32, 55,
};
s16 sMinigamePerfectVtxPosY[] = {
    1, -70, -99, -70, 71, 101, 72, 1,
};

/**
 * interfaceCtx->actionVtx[0]   -> A Button
 * interfaceCtx->actionVtx[4]   -> A Button Shadow
 * interfaceCtx->actionVtx[8]   -> A Button Do-Action
 * interfaceCtx->actionVtx[12]  -> Three-Day Clock's Star (for the Minute Tracker)
 * interfaceCtx->actionVtx[16]  -> Three-Day Clock's Sun (for the Day-Time Hours Tracker)
 * interfaceCtx->actionVtx[20]  -> Three-Day Clock's Moon (for the Night-Time Hours Tracker)
 * interfaceCtx->actionVtx[24]  -> Three-Day Clock's Hour Digit Above the Sun (uses 8 vertices)
 * interfaceCtx->actionVtx[32]  -> Three-Day Clock's Hour Digit Above the Moon (uses 8 vertices)
 * interfaceCtx->actionVtx[40]  -> Minigame Countdown Textures
 * interfaceCtx->actionVtx[44]  -> Minigame Perfect Letter P Shadow
 * interfaceCtx->actionVtx[48]  -> Minigame Perfect Letter E Shadow
 * interfaceCtx->actionVtx[52]  -> Minigame Perfect Letter R Shadow
 * interfaceCtx->actionVtx[56]  -> Minigame Perfect Letter F Shadow
 * interfaceCtx->actionVtx[60]  -> Minigame Perfect Letter E Shadow
 * interfaceCtx->actionVtx[64]  -> Minigame Perfect Letter C Shadow
 * interfaceCtx->actionVtx[68]  -> Minigame Perfect Letter T Shadow
 * interfaceCtx->actionVtx[72]  -> Minigame Perfect Letter ! Shadow
 * interfaceCtx->actionVtx[76]  -> Minigame Perfect Letter P Colored
 * interfaceCtx->actionVtx[80]  -> Minigame Perfect Letter E Colored
 * interfaceCtx->actionVtx[84]  -> Minigame Perfect Letter R Colored
 * interfaceCtx->actionVtx[88]  -> Minigame Perfect Letter F Colored
 * interfaceCtx->actionVtx[92]  -> Minigame Perfect Letter E Colored
 * interfaceCtx->actionVtx[96]  -> Minigame Perfect Letter C Colored
 * interfaceCtx->actionVtx[100] -> Minigame Perfect Letter T Colored
 * interfaceCtx->actionVtx[104] -> Minigame Perfect Letter ! Colored
 */

// TODO: Mess with macros
#define MINIGAME_PERFECT_VTX_SHADOW(index) (interfaceCtx->actionVtx[44 + index])
#define MINIGAME_PERFECT_VTX_COLOR(index) (interfaceCtx->actionVtx[76 + index])

#define BEATING_HEART_VTX_X -8
#define BEATING_HEART_VTX_Y -8
#define BEATING_HEART_VTX_WIDTH 16

void Interface_SetVertices(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 i;
    s16 j;
    s16 k;
    s16 letteroffset;

    // 108 = 11 + (2 * 8 minigame perfect)) * 4 vertices per quad
    play->interfaceCtx.actionVtx = GRAPH_ALLOC(play->state.gfxCtx, 108 * sizeof(Vtx));

    // Loop over all non-minigame perfect vertices
    for (k = 0, i = 0; i < 44; i += 4, k++) {
        // Left vertices x Pos
        interfaceCtx->actionVtx[i + 0].v.ob[0] = interfaceCtx->actionVtx[i + 2].v.ob[0] = sActionVtxPosX[k];

        // Right vertices x Pos
        interfaceCtx->actionVtx[i + 1].v.ob[0] = interfaceCtx->actionVtx[i + 3].v.ob[0] =
            interfaceCtx->actionVtx[i].v.ob[0] + sActionVtxWidths[k];

        // Top vertices y Pos
        interfaceCtx->actionVtx[i].v.ob[1] = interfaceCtx->actionVtx[i + 1].v.ob[1] = sActionVtxPosY[k];

        // Bottom vertices y Pos
        interfaceCtx->actionVtx[i + 2].v.ob[1] = interfaceCtx->actionVtx[i + 3].v.ob[1] =
            interfaceCtx->actionVtx[i].v.ob[1] - sActionVtxHeights[k];

        // All vertices z Pos
        interfaceCtx->actionVtx[i + 0].v.ob[2] = interfaceCtx->actionVtx[i + 1].v.ob[2] =
            interfaceCtx->actionVtx[i + 2].v.ob[2] = interfaceCtx->actionVtx[i + 3].v.ob[2] = 0;

        // Unused flag
        interfaceCtx->actionVtx[i + 0].v.flag = interfaceCtx->actionVtx[i + 1].v.flag =
            interfaceCtx->actionVtx[i + 2].v.flag = interfaceCtx->actionVtx[i + 3].v.flag = 0;

        // Left and Top texture coordinates
        interfaceCtx->actionVtx[i + 0].v.tc[0] = interfaceCtx->actionVtx[i + 0].v.tc[1] =
            interfaceCtx->actionVtx[i + 1].v.tc[1] = interfaceCtx->actionVtx[i + 2].v.tc[0] = 0;

        // Right vertices texture coordinates
        interfaceCtx->actionVtx[i + 1].v.tc[0] = interfaceCtx->actionVtx[i + 3].v.tc[0] = sActionVtxWidths[k] << 5;

        // Bottom vertices texture coordinates
        interfaceCtx->actionVtx[i + 2].v.tc[1] = interfaceCtx->actionVtx[i + 3].v.tc[1] = sActionVtxHeights[k] << 5;

        // Set color
        interfaceCtx->actionVtx[i + 0].v.cn[0] = interfaceCtx->actionVtx[i + 1].v.cn[0] =
            interfaceCtx->actionVtx[i + 2].v.cn[0] = interfaceCtx->actionVtx[i + 3].v.cn[0] =
                interfaceCtx->actionVtx[i + 0].v.cn[1] = interfaceCtx->actionVtx[i + 1].v.cn[1] =
                    interfaceCtx->actionVtx[i + 2].v.cn[1] = interfaceCtx->actionVtx[i + 3].v.cn[1] =
                        interfaceCtx->actionVtx[i + 0].v.cn[2] = interfaceCtx->actionVtx[i + 1].v.cn[2] =
                            interfaceCtx->actionVtx[i + 2].v.cn[2] = interfaceCtx->actionVtx[i + 3].v.cn[2] = 255;

        // Set alpha
        interfaceCtx->actionVtx[i + 0].v.cn[3] = interfaceCtx->actionVtx[i + 1].v.cn[3] =
            interfaceCtx->actionVtx[i + 2].v.cn[3] = interfaceCtx->actionVtx[i + 3].v.cn[3] = 255;
    }

    // A button right and top texture coordinates
    interfaceCtx->actionVtx[1].v.tc[0] = interfaceCtx->actionVtx[3].v.tc[0] = interfaceCtx->actionVtx[2].v.tc[1] =
        interfaceCtx->actionVtx[3].v.tc[1] = 32 << 5;

    // A button background right and top texture coordinates
    interfaceCtx->actionVtx[5].v.tc[0] = interfaceCtx->actionVtx[7].v.tc[0] = interfaceCtx->actionVtx[6].v.tc[1] =
        interfaceCtx->actionVtx[7].v.tc[1] = 32 << 5;

    // Loop over vertices for the minigame-perfect letters
    // Outer loop is to loop over 2 sets of letters: shadowed letters and colored letters
    for (j = 0, letteroffset = 2; j < 2; j++, letteroffset -= 2) {
        for (k = 0; k < 8; k++, i += 4) {
            if ((interfaceCtx->minigamePerfectType == MINIGAME_PERFECT_TYPE_1) ||
                ((interfaceCtx->minigamePerfectType == MINIGAME_PERFECT_TYPE_3) &&
                 (interfaceCtx->minigamePerfectState[0] == MINIGAME_PERFECT_STATE_6))) {
                // Left vertices x Pos
                interfaceCtx->actionVtx[i].v.ob[0] = interfaceCtx->actionVtx[i + 2].v.ob[0] =
                    -((sMinigamePerfectVtxPosX[k] - letteroffset) + 16);

                // Right vertices x Pos
                interfaceCtx->actionVtx[i + 1].v.ob[0] = interfaceCtx->actionVtx[i + 3].v.ob[0] =
                    interfaceCtx->actionVtx[i].v.ob[0] + MINIGAME_PERFECT_VTX_WIDTH;

                // Top vertices y Pos
                interfaceCtx->actionVtx[i].v.ob[1] = interfaceCtx->actionVtx[i + 1].v.ob[1] =
                    (sMinigamePerfectVtxPosY[k] - letteroffset) + 16;

                // Bottom vertices y Pos
                interfaceCtx->actionVtx[i + 2].v.ob[1] = interfaceCtx->actionVtx[i + 3].v.ob[1] =
                    interfaceCtx->actionVtx[i].v.ob[1] - MINIGAME_PERFECT_VTX_HEIGHT;

            } else if ((interfaceCtx->minigamePerfectType == MINIGAME_PERFECT_TYPE_2) ||
                       (interfaceCtx->minigamePerfectType == MINIGAME_PERFECT_TYPE_3)) {
                // Left vertices x Pos
                interfaceCtx->actionVtx[i].v.ob[0] = interfaceCtx->actionVtx[i + 2].v.ob[0] =
                    -(interfaceCtx->minigamePerfectVtxOffset[k] - letteroffset + 16);

                // Right vertices x Pos
                interfaceCtx->actionVtx[i + 1].v.ob[0] = interfaceCtx->actionVtx[i + 3].v.ob[0] =
                    interfaceCtx->actionVtx[i].v.ob[0] + MINIGAME_PERFECT_VTX_WIDTH;

                // Top vertices y Pos
                interfaceCtx->actionVtx[i].v.ob[1] = interfaceCtx->actionVtx[i + 1].v.ob[1] = 16 - letteroffset;

                // Bottom vertices y Pos
                interfaceCtx->actionVtx[i + 2].v.ob[1] = interfaceCtx->actionVtx[i + 3].v.ob[1] =
                    interfaceCtx->actionVtx[i].v.ob[1] - MINIGAME_PERFECT_VTX_HEIGHT;

            } else {
                // Left vertices x Pos
                interfaceCtx->actionVtx[i].v.ob[0] = interfaceCtx->actionVtx[i + 2].v.ob[0] = -(216 - letteroffset);

                // Right vertices x Pos
                interfaceCtx->actionVtx[i + 1].v.ob[0] = interfaceCtx->actionVtx[i + 3].v.ob[0] =
                    interfaceCtx->actionVtx[i].v.ob[0] + MINIGAME_PERFECT_VTX_WIDTH;

                // Top vertices y Pos
                interfaceCtx->actionVtx[i].v.ob[1] = interfaceCtx->actionVtx[i + 1].v.ob[1] = 24 - letteroffset;

                // Bottom vertices y Pos
                interfaceCtx->actionVtx[i + 2].v.ob[1] = interfaceCtx->actionVtx[i + 3].v.ob[1] =
                    interfaceCtx->actionVtx[i].v.ob[1] - MINIGAME_PERFECT_VTX_HEIGHT;
            }

            // All vertices z Pos
            interfaceCtx->actionVtx[i].v.ob[2] = interfaceCtx->actionVtx[i + 1].v.ob[2] =
                interfaceCtx->actionVtx[i + 2].v.ob[2] = interfaceCtx->actionVtx[i + 3].v.ob[2] = 0;

            // Unused flag
            interfaceCtx->actionVtx[i].v.flag = interfaceCtx->actionVtx[i + 1].v.flag =
                interfaceCtx->actionVtx[i + 2].v.flag = interfaceCtx->actionVtx[i + 3].v.flag = 0;

            // Left and Top texture coordinates
            interfaceCtx->actionVtx[i].v.tc[0] = interfaceCtx->actionVtx[i].v.tc[1] =
                interfaceCtx->actionVtx[i + 1].v.tc[1] = interfaceCtx->actionVtx[i + 2].v.tc[0] = 0;

            // Right vertices texture coordinates
            interfaceCtx->actionVtx[i + 1].v.tc[0] = interfaceCtx->actionVtx[i + 3].v.tc[0] = MINIGAME_PERFECT_VTX_WIDTH
                                                                                              << 5;

            // Bottom vertices texture coordinates
            interfaceCtx->actionVtx[i + 2].v.tc[1] = interfaceCtx->actionVtx[i + 3].v.tc[1] =
                MINIGAME_PERFECT_VTX_HEIGHT << 5;

            // Set color
            interfaceCtx->actionVtx[i].v.cn[0] = interfaceCtx->actionVtx[i + 1].v.cn[0] =
                interfaceCtx->actionVtx[i + 2].v.cn[0] = interfaceCtx->actionVtx[i + 3].v.cn[0] =
                    interfaceCtx->actionVtx[i].v.cn[1] = interfaceCtx->actionVtx[i + 1].v.cn[1] =
                        interfaceCtx->actionVtx[i + 2].v.cn[1] = interfaceCtx->actionVtx[i + 3].v.cn[1] =
                            interfaceCtx->actionVtx[i].v.cn[2] = interfaceCtx->actionVtx[i + 1].v.cn[2] =
                                interfaceCtx->actionVtx[i + 2].v.cn[2] = interfaceCtx->actionVtx[i + 3].v.cn[2] = 255;

            // Set alpha
            interfaceCtx->actionVtx[i].v.cn[3] = interfaceCtx->actionVtx[i + 1].v.cn[3] =
                interfaceCtx->actionVtx[i + 2].v.cn[3] = interfaceCtx->actionVtx[i + 3].v.cn[3] = 255;
        }
    }

    // Beating Hearts Vertices
    interfaceCtx->beatingHeartVtx = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));

    // Left vertices x Pos
    interfaceCtx->beatingHeartVtx[0].v.ob[0] = interfaceCtx->beatingHeartVtx[2].v.ob[0] = BEATING_HEART_VTX_X;

    // Right vertices x Pos
    interfaceCtx->beatingHeartVtx[1].v.ob[0] = interfaceCtx->beatingHeartVtx[3].v.ob[0] =
        BEATING_HEART_VTX_X + BEATING_HEART_VTX_WIDTH;

    // Top vertices y Pos
    interfaceCtx->beatingHeartVtx[0].v.ob[1] = interfaceCtx->beatingHeartVtx[1].v.ob[1] =
        BEATING_HEART_VTX_Y + BEATING_HEART_VTX_WIDTH;

    // Bottom vertices y Pos
    interfaceCtx->beatingHeartVtx[2].v.ob[1] = interfaceCtx->beatingHeartVtx[3].v.ob[1] = BEATING_HEART_VTX_Y;

    // All vertices z Pos
    interfaceCtx->beatingHeartVtx[0].v.ob[2] = interfaceCtx->beatingHeartVtx[1].v.ob[2] =
        interfaceCtx->beatingHeartVtx[2].v.ob[2] = interfaceCtx->beatingHeartVtx[3].v.ob[2] = 0;

    // unused flag
    interfaceCtx->beatingHeartVtx[0].v.flag = interfaceCtx->beatingHeartVtx[1].v.flag =
        interfaceCtx->beatingHeartVtx[2].v.flag = interfaceCtx->beatingHeartVtx[3].v.flag = 0;

    // Texture Coordinates
    interfaceCtx->beatingHeartVtx[0].v.tc[0] = interfaceCtx->beatingHeartVtx[0].v.tc[1] =
        interfaceCtx->beatingHeartVtx[1].v.tc[1] = interfaceCtx->beatingHeartVtx[2].v.tc[0] = 0;
    interfaceCtx->beatingHeartVtx[1].v.tc[0] = interfaceCtx->beatingHeartVtx[2].v.tc[1] =
        interfaceCtx->beatingHeartVtx[3].v.tc[0] = interfaceCtx->beatingHeartVtx[3].v.tc[1] =
            BEATING_HEART_VTX_WIDTH << 5;

    // Set color
    interfaceCtx->beatingHeartVtx[0].v.cn[0] = interfaceCtx->beatingHeartVtx[1].v.cn[0] =
        interfaceCtx->beatingHeartVtx[2].v.cn[0] = interfaceCtx->beatingHeartVtx[3].v.cn[0] =
            interfaceCtx->beatingHeartVtx[0].v.cn[1] = interfaceCtx->beatingHeartVtx[1].v.cn[1] =
                interfaceCtx->beatingHeartVtx[2].v.cn[1] = interfaceCtx->beatingHeartVtx[3].v.cn[1] =
                    interfaceCtx->beatingHeartVtx[0].v.cn[2] = interfaceCtx->beatingHeartVtx[1].v.cn[2] =
                        interfaceCtx->beatingHeartVtx[2].v.cn[2] = interfaceCtx->beatingHeartVtx[3].v.cn[2] =
                            interfaceCtx->beatingHeartVtx[0].v.cn[3] = interfaceCtx->beatingHeartVtx[1].v.cn[3] =
                                interfaceCtx->beatingHeartVtx[2].v.cn[3] = interfaceCtx->beatingHeartVtx[3].v.cn[3] =
                                    255;
}

s32 sPostmanTimerInputBtnAPressed = false;
void Interface_PostmanTimerCallback(s32 arg0) {
    s32 btnAPressed;

    func_80175E68(&sPostmanTimerInput[0], 0);
    btnAPressed = CHECK_BTN_ALL(sPostmanTimerInput[0].cur.button, BTN_A);
    if ((btnAPressed != sPostmanTimerInputBtnAPressed) && btnAPressed) {
        gSaveContext.postmanTimerStopOsTime = osGetTime();
        gSaveContext.timerStates[TIMER_ID_POSTMAN] = TIMER_STATE_POSTMAN_STOP;
    }

    sPostmanTimerInputBtnAPressed = btnAPressed;
}

void Interface_StartTimer(s16 timerId, s16 seconds) {
    gSaveContext.timerX[timerId] = 115;
    gSaveContext.timerY[timerId] = 80;

    sEnvTimerActive = false;

    gSaveContext.timerCurTimes[timerId] = SECONDS_TO_TIMER(seconds);
    gSaveContext.timerTimeLimits[timerId] = gSaveContext.timerCurTimes[timerId];

    if (gSaveContext.timerCurTimes[timerId] != SECONDS_TO_TIMER(0)) {
        gSaveContext.timerDirections[timerId] = TIMER_COUNT_DOWN;
    } else {
        gSaveContext.timerDirections[timerId] = TIMER_COUNT_UP;
    }

    gSaveContext.timerStates[timerId] = TIMER_STATE_START;
}

void Interface_StartPostmanTimer(s16 seconds, s16 bunnyHoodState) {
    gSaveContext.timerX[TIMER_ID_POSTMAN] = 115;
    gSaveContext.timerY[TIMER_ID_POSTMAN] = 80;

    sPostmanBunnyHoodState = bunnyHoodState;

    gSaveContext.timerCurTimes[TIMER_ID_POSTMAN] = SECONDS_TO_TIMER(seconds);
    gSaveContext.timerTimeLimits[TIMER_ID_POSTMAN] = gSaveContext.timerCurTimes[TIMER_ID_POSTMAN];

    if (gSaveContext.timerCurTimes[TIMER_ID_POSTMAN] != SECONDS_TO_TIMER(0)) {
        gSaveContext.timerDirections[TIMER_ID_POSTMAN] = TIMER_COUNT_DOWN;
    } else {
        gSaveContext.timerDirections[TIMER_ID_POSTMAN] = TIMER_COUNT_UP;
    }
    gSaveContext.timerStates[TIMER_ID_POSTMAN] = TIMER_STATE_POSTMAN_START;

    gSaveContext.timerStopTimes[TIMER_ID_POSTMAN] = 0;
    gSaveContext.timerPausedOsTimes[TIMER_ID_POSTMAN] = 0;
}

// Unused, goron race actually uses TIMER_ID_MINIGAME_2
void Interface_StartGoronRaceTimer(s32 arg0) {
    if (gSaveContext.timerStates[TIMER_ID_GORON_RACE_UNUSED] != TIMER_STATE_OFF) {
        // Goron race started
        if (gSaveContext.eventInf[1] & 1) {
            gSaveContext.timerCurTimes[TIMER_ID_GORON_RACE_UNUSED] = SECONDS_TO_TIMER_PRECISE(2, 39);
        } else {
            gSaveContext.timerCurTimes[TIMER_ID_GORON_RACE_UNUSED] = SECONDS_TO_TIMER_PRECISE(0, 1);
        }
    }
}

void Interface_StartBottleTimer(s16 seconds, s16 timerId) {
    gSaveContext.bottleTimerStates[timerId] = BOTTLE_TIMER_STATE_COUNTING;
    gSaveContext.bottleTimerCurTimes[timerId] = SECONDS_TO_TIMER(seconds);
    gSaveContext.bottleTimerTimeLimits[timerId] = gSaveContext.bottleTimerCurTimes[timerId];
    gSaveContext.bottleTimerStartOsTimes[timerId] = osGetTime();
    gSaveContext.bottleTimerPausedOsTimes[timerId] = 0;
    sBottleTimerPausedOsTime = 0;
}

u32 Interface_GetCompressedTimerDigits(s16 timerId) {
    u64 time;
    s16 timerArr[6];

    time = gSaveContext.timerCurTimes[timerId];

    // 6 minutes
    timerArr[0] = time / SECONDS_TO_TIMER(360);
    time -= timerArr[0] * SECONDS_TO_TIMER(360);

    // minutes
    timerArr[1] = time / SECONDS_TO_TIMER(60);
    time -= timerArr[1] * SECONDS_TO_TIMER(60);

    // 10 seconds
    timerArr[2] = time / SECONDS_TO_TIMER(10);
    time -= timerArr[2] * SECONDS_TO_TIMER(10);

    // seconds
    timerArr[3] = time / SECONDS_TO_TIMER(1);
    time -= timerArr[3] * SECONDS_TO_TIMER(1);

    // 100 milliseconds
    timerArr[4] = time / SECONDS_TO_TIMER_PRECISE(0, 10);
    time -= timerArr[4] * SECONDS_TO_TIMER_PRECISE(0, 10);

    // 10 milliseconds
    timerArr[5] = time;

    return (timerArr[0] << 0x14) | (timerArr[1] << 0x10) | (timerArr[2] << 0xC) | (timerArr[3] << 8) |
           (timerArr[4] << 4) | timerArr[5];
}

void Interface_NewDay(PlayState* play, s32 day) {
    s32 pad;
    s16 i = day - 1;

    // i is used to store dayMinusOne
    if ((i < 0) || (i >= 3)) {
        i = 0;
    }

    // Loads day number from week_static for the three-day clock
    DmaMgr_SendRequest0((u32)play->interfaceCtx.doActionSegment + 0x780,
                        (u32)SEGMENT_ROM_START(week_static) + i * 0x510, 0x510);

    // i is used to store sceneId
    for (i = 0; i < 120; i++) {
        gSaveContext.save.permanentSceneFlags[i].chest = gSaveContext.cycleSceneFlags[i].chest;
        gSaveContext.save.permanentSceneFlags[i].switch0 = gSaveContext.cycleSceneFlags[i].switch0;
        gSaveContext.save.permanentSceneFlags[i].switch1 = gSaveContext.cycleSceneFlags[i].switch1;
        gSaveContext.save.permanentSceneFlags[i].clearedRoom = gSaveContext.cycleSceneFlags[i].clearedRoom;
        gSaveContext.save.permanentSceneFlags[i].collectible = gSaveContext.cycleSceneFlags[i].collectible;
    }
}

void Interface_SetHudVisibility(u16 hudVisibility) {
    if (gSaveContext.hudVisibility != hudVisibility) {
        gSaveContext.hudVisibility = hudVisibility;
        gSaveContext.nextHudVisibility = hudVisibility;
        gSaveContext.hudVisibilityTimer = 1;
    }
}

/**
 * Sets the button alphas to be dimmed for disabled buttons, or to the requested alpha for non-disabled buttons
 */
void Interface_UpdateButtonAlphasByStatus(PlayState* play, s16 risingAlpha) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    if ((gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) || (gSaveContext.bButtonStatus == BTN_DISABLED)) {
        if (interfaceCtx->bAlpha != 70) {
            interfaceCtx->bAlpha = 70;
        }
    } else {
        if (interfaceCtx->bAlpha != 255) {
            interfaceCtx->bAlpha = risingAlpha;
        }
    }

    if (gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] == BTN_DISABLED) {
        if (interfaceCtx->cLeftAlpha != 70) {
            interfaceCtx->cLeftAlpha = 70;
        }
    } else {
        if (interfaceCtx->cLeftAlpha != 255) {
            interfaceCtx->cLeftAlpha = risingAlpha;
        }
    }

    if (gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] == BTN_DISABLED) {
        if (interfaceCtx->cDownAlpha != 70) {
            interfaceCtx->cDownAlpha = 70;
        }
    } else {
        if (interfaceCtx->cDownAlpha != 255) {
            interfaceCtx->cDownAlpha = risingAlpha;
        }
    }

    if (gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] == BTN_DISABLED) {
        if (interfaceCtx->cRightAlpha != 70) {
            interfaceCtx->cRightAlpha = 70;
        }
    } else {
        if (interfaceCtx->cRightAlpha != 255) {
            interfaceCtx->cRightAlpha = risingAlpha;
        }
    }

    if (gSaveContext.buttonStatus[EQUIP_SLOT_A] == BTN_DISABLED) {
        if (interfaceCtx->aAlpha != 70) {
            interfaceCtx->aAlpha = 70;
        }
    } else {
        if (interfaceCtx->aAlpha != 255) {
            interfaceCtx->aAlpha = risingAlpha;
        }
    }
}

/**
 * Lower button alphas on the HUD to the requested value
 * If (gSaveContext.hudVisibilityForceButtonAlphasByStatus), then instead update button alphas
 * depending on button status
 */
void Interface_UpdateButtonAlphas(PlayState* play, s16 dimmingAlpha, s16 risingAlpha) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    if (gSaveContext.hudVisibilityForceButtonAlphasByStatus) {
        Interface_UpdateButtonAlphasByStatus(play, risingAlpha);
        return;
    }

    if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > dimmingAlpha)) {
        interfaceCtx->bAlpha = dimmingAlpha;
    }

    if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
        interfaceCtx->aAlpha = dimmingAlpha;
    }

    if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
        interfaceCtx->cLeftAlpha = dimmingAlpha;
    }

    if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
        interfaceCtx->cDownAlpha = dimmingAlpha;
    }

    if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
        interfaceCtx->cRightAlpha = dimmingAlpha;
    }
}

void Interface_UpdateHudAlphas(PlayState* play, s16 dimmingAlpha) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 risingAlpha = 255 - dimmingAlpha;

    switch (gSaveContext.nextHudVisibility) {
        case HUD_VISIBILITY_NONE:
        case HUD_VISIBILITY_NONE_ALT:
        case HUD_VISIBILITY_B:
            if (gSaveContext.nextHudVisibility == HUD_VISIBILITY_B) {
                if (interfaceCtx->bAlpha != 255) {
                    interfaceCtx->bAlpha = risingAlpha;
                }
            } else {
                if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > dimmingAlpha)) {
                    interfaceCtx->bAlpha = dimmingAlpha;
                }
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > dimmingAlpha)) {
                interfaceCtx->healthAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            break;

        case HUD_VISIBILITY_HEARTS_WITH_OVERWRITE:
            // aAlpha is immediately overwritten in Interface_UpdateButtonAlphas
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            Interface_UpdateButtonAlphas(play, dimmingAlpha, risingAlpha + 0);

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_A:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > dimmingAlpha)) {
                interfaceCtx->bAlpha = dimmingAlpha;
            }

            // aAlpha is immediately overwritten below
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > dimmingAlpha)) {
                interfaceCtx->healthAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_A_HEARTS_MAGIC_WITH_OVERWRITE:
            Interface_UpdateButtonAlphas(play, dimmingAlpha, risingAlpha);

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            // aAlpha overwrites the value set in Interface_UpdateButtonAlphas
            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_A_HEARTS_MAGIC_MINIMAP_WITH_OVERWRITE:
            Interface_UpdateButtonAlphas(play, dimmingAlpha, risingAlpha);

            // aAlpha overwrites the value set in Interface_UpdateButtonAlphas
            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (play->sceneId == SCENE_SPOT00) {
                if (interfaceCtx->minimapAlpha < 170) {
                    interfaceCtx->minimapAlpha = risingAlpha;
                } else {
                    interfaceCtx->minimapAlpha = 170;
                }
            } else if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_ALL_NO_MINIMAP_W_DISABLED:
            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            Interface_UpdateButtonAlphasByStatus(play, risingAlpha);

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_HEARTS_MAGIC:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > dimmingAlpha)) {
                interfaceCtx->bAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_B_ALT:
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > dimmingAlpha)) {
                interfaceCtx->healthAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_HEARTS:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > dimmingAlpha)) {
                interfaceCtx->bAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_A_B_MINIMAP:
            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            if ((gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) ||
                (gSaveContext.bButtonStatus == BTN_DISABLED)) {
                if (interfaceCtx->bAlpha != 70) {
                    interfaceCtx->bAlpha = 70;
                }
            } else {
                if (interfaceCtx->bAlpha != 255) {
                    interfaceCtx->bAlpha = risingAlpha;
                }
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = risingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > dimmingAlpha)) {
                interfaceCtx->healthAlpha = dimmingAlpha;
            }

            break;

        case HUD_VISIBILITY_HEARTS_MAGIC_WITH_OVERWRITE:
            Interface_UpdateButtonAlphas(play, dimmingAlpha, risingAlpha);

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            // aAlpha overwrites the value set in Interface_UpdateButtonAlphas
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_HEARTS_MAGIC_C:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > dimmingAlpha)) {
                interfaceCtx->bAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if (interfaceCtx->cLeftAlpha != 255) {
                interfaceCtx->cLeftAlpha = risingAlpha;
            }

            if (interfaceCtx->cDownAlpha != 255) {
                interfaceCtx->cDownAlpha = risingAlpha;
            }

            if (interfaceCtx->cRightAlpha != 255) {
                interfaceCtx->cRightAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_ALL_NO_MINIMAP:
            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = risingAlpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            if (interfaceCtx->cLeftAlpha != 255) {
                interfaceCtx->cLeftAlpha = risingAlpha;
            }

            if (interfaceCtx->cDownAlpha != 255) {
                interfaceCtx->cDownAlpha = risingAlpha;
            }

            if (interfaceCtx->cRightAlpha != 255) {
                interfaceCtx->cRightAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_A_B_C:
            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > dimmingAlpha)) {
                interfaceCtx->healthAlpha = dimmingAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = risingAlpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            if (interfaceCtx->cLeftAlpha != 255) {
                interfaceCtx->cLeftAlpha = risingAlpha;
            }

            if (interfaceCtx->cDownAlpha != 255) {
                interfaceCtx->cDownAlpha = risingAlpha;
            }

            if (interfaceCtx->cRightAlpha != 255) {
                interfaceCtx->cRightAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_B_MINIMAP:
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > dimmingAlpha)) {
                interfaceCtx->healthAlpha = dimmingAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = risingAlpha;
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_HEARTS_MAGIC_MINIMAP:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > dimmingAlpha)) {
                interfaceCtx->bAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_A_HEARTS_MAGIC_MINIMAP:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > dimmingAlpha)) {
                interfaceCtx->bAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_B_MAGIC:
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > dimmingAlpha)) {
                interfaceCtx->aAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > dimmingAlpha)) {
                interfaceCtx->healthAlpha = dimmingAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            break;

        case HUD_VISIBILITY_A_B:
            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = risingAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > dimmingAlpha)) {
                interfaceCtx->minimapAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > dimmingAlpha)) {
                interfaceCtx->magicAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > dimmingAlpha)) {
                interfaceCtx->healthAlpha = dimmingAlpha;
            }

            break;

        case HUD_VISIBILITY_A_B_HEARTS_MAGIC_MINIMAP:
            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > dimmingAlpha)) {
                interfaceCtx->cLeftAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > dimmingAlpha)) {
                interfaceCtx->cDownAlpha = dimmingAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > dimmingAlpha)) {
                interfaceCtx->cRightAlpha = dimmingAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = risingAlpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = risingAlpha;
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            break;
    }

    if ((play->roomCtx.curRoom.unk3 == 1) && (interfaceCtx->minimapAlpha >= 255)) {
        interfaceCtx->minimapAlpha = 255;
    }
}

/**
 * A continuation of the if-else chain from Interface_UpdateButtonsPart1
 * Also used directly when opening the pause menu i.e. skips part 1
 */
void Interface_UpdateButtonsPart2(PlayState* play) {
    MessageContext* msgCtx = &play->msgCtx;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    Player* player = GET_PLAYER(play);
    s16 i;
    s16 restoreHudVisibility = false;

    if (gSaveContext.eventInf[4] & 2) {
        // Related to swamp boat (non-minigame)?
        for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
            if ((GET_CUR_FORM_BTN_ITEM(i) != ITEM_PICTO_BOX) || (msgCtx->msgMode != 0)) {
                if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                    restoreHudVisibility = true;
                }
                gSaveContext.buttonStatus[i] = BTN_DISABLED;
            } else {
                if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                    restoreHudVisibility = true;
                }
                gSaveContext.buttonStatus[i] = BTN_ENABLED;
            }
        }

        if (sPictographState == PICTOGRAPH_STATE_OFF) {
            if (gSaveContext.buttonStatus[EQUIP_SLOT_B] != BTN_DISABLED) {
                restoreHudVisibility = true;
            }
            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
        } else {
            if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) {
                restoreHudVisibility = true;
            }
            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
        }
    } else if (gSaveContext.save.weekEventReg[90] & 0x20) {
        // Fishermans's jumping minigame
        for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
            if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                gSaveContext.buttonStatus[i] = BTN_DISABLED;
            }
        }

        Interface_SetHudVisibility(HUD_VISIBILITY_B);
    } else if (gSaveContext.save.weekEventReg[82] & 8) {
        // Swordsman's log minigame
        for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
            if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                gSaveContext.buttonStatus[i] = BTN_DISABLED;
            }
        }

        Interface_SetHudVisibility(HUD_VISIBILITY_A_B_HEARTS_MAGIC_MINIMAP);
    } else if (gSaveContext.save.weekEventReg[84] & 0x20) {
        // Related to moon child
        if (player->currentMask == PLAYER_MASK_FIERCE_DEITY) {
            for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                if ((GET_CUR_FORM_BTN_ITEM(i) == ITEM_MASK_FIERCE_DEITY) ||
                    ((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_BOTTLE) && (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_OBABA_DRINK))) {
                    if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                        restoreHudVisibility = true;
                        gSaveContext.buttonStatus[i] = BTN_ENABLED;
                    }
                } else {
                    if (gSaveContext.buttonStatus[i] != BTN_DISABLED) {
                        gSaveContext.buttonStatus[i] = BTN_DISABLED;
                        restoreHudVisibility = true;
                    }
                }
            }
        } else {
            for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                if ((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MASK_DEKU) && (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_MASK_ZORA)) {
                    if (gSaveContext.buttonStatus[i] != BTN_DISABLED) {
                        restoreHudVisibility = true;
                    }
                    gSaveContext.buttonStatus[i] = BTN_DISABLED;
                } else {
                    if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                        restoreHudVisibility = true;
                    }
                    gSaveContext.buttonStatus[i] = BTN_ENABLED;
                }
            }
        }
    } else if ((play->sceneId == SCENE_SPOT00) && (gSaveContext.sceneLayer == 6)) {
        // Unknown cutscene
        for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
            if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                restoreHudVisibility = true;
            }
            gSaveContext.buttonStatus[i] = BTN_DISABLED;
        }
    } else if (gSaveContext.eventInf[3] & 0x10) {
        // Deku playground minigame
        if (player->stateFlags3 & PLAYER_STATE3_1000000) {
            if (gSaveContext.save.inventory.items[SLOT_NUT] == ITEM_NUT) {
                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_NUT;
                Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
            } else {
                gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
                restoreHudVisibility = true;
            }
        } else {
            if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) {
                gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
                restoreHudVisibility = true;
            }

            for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                    restoreHudVisibility = true;
                }
                gSaveContext.buttonStatus[i] = BTN_DISABLED;
            }
        }

        if (restoreHudVisibility || (gSaveContext.hudVisibility != HUD_VISIBILITY_A_B_MINIMAP)) {
            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
            Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
            restoreHudVisibility = false;
        }
    } else if (player->stateFlags3 & PLAYER_STATE3_1000000) {
        // Nuts on B (from flying as Deku Link)
        if (gSaveContext.save.inventory.items[SLOT_NUT] == ITEM_NUT) {
            if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NUT) {
                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_NUT;
                Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
                gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
                restoreHudVisibility = true;
            }
        } else if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_ENABLED) {
            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
            restoreHudVisibility = true;
        }

        if (restoreHudVisibility) {
            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
        }
    } else if (!gSaveContext.save.playerData.isMagicAcquired && (CUR_FORM == PLAYER_FORM_DEKU) &&
               (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_NUT)) {
        // Nuts on B (as Deku Link)
        BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_FD;
        gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
    } else if ((Player_GetEnvTimerType(play) >= PLAYER_ENV_TIMER_UNDERWATER_FLOOR) &&
               (Player_GetEnvTimerType(play) <= PLAYER_ENV_TIMER_UNDERWATER_FREE)) {
        // Swimming underwater
        if (CUR_FORM != PLAYER_FORM_ZORA) {
            if ((player->currentMask == PLAYER_MASK_BLAST) && (player->blastMaskTimer == 0)) {
                if (gSaveContext.bButtonStatus == BTN_DISABLED) {
                    restoreHudVisibility = true;
                }
                gSaveContext.bButtonStatus = BTN_ENABLED;
            } else if ((interfaceCtx->bButtonDoAction == DO_ACTION_EXPLODE) &&
                       (player->currentMask == PLAYER_MASK_BLAST)) {
                if (gSaveContext.bButtonStatus != BTN_DISABLED) {
                    gSaveContext.bButtonStatus = BTN_DISABLED;
                    restoreHudVisibility = true;
                }
            } else {
                if (gSaveContext.buttonStatus[EQUIP_SLOT_B] != BTN_DISABLED) {
                    restoreHudVisibility = true;
                }
                gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
            }
        } else {
            if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) {
                restoreHudVisibility = true;
            }
            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
        }

        for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
            if (GET_CUR_FORM_BTN_ITEM(i) != ITEM_MASK_ZORA) {
                if (Player_GetEnvTimerType(play) == PLAYER_ENV_TIMER_UNDERWATER_FLOOR) {
                    if (!((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_BOTTLE) &&
                          (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_OBABA_DRINK))) {
                        if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                            restoreHudVisibility = true;
                        }
                        gSaveContext.buttonStatus[i] = BTN_DISABLED;
                    } else {
                        if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                            restoreHudVisibility = true;
                        }
                        gSaveContext.buttonStatus[i] = BTN_ENABLED;
                    }
                } else {
                    if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                        restoreHudVisibility = true;
                    }
                    gSaveContext.buttonStatus[i] = BTN_DISABLED;
                }
            } else if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                gSaveContext.buttonStatus[i] = BTN_ENABLED;
                restoreHudVisibility = true;
            }
        }

        if (restoreHudVisibility) {
            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
        }

        if ((play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF)) {
            if (ActorCutscene_GetCurrentIndex() == -1) {
                Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
            }
        }
    } else if (player->stateFlags1 & PLAYER_STATE1_200000) {
        // First person view
        for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
            if (GET_CUR_FORM_BTN_ITEM(i) != ITEM_LENS) {
                if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                    restoreHudVisibility = true;
                }
                gSaveContext.buttonStatus[i] = BTN_DISABLED;
            } else {
                if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                    restoreHudVisibility = true;
                }
                gSaveContext.buttonStatus[i] = BTN_ENABLED;
            }
        }

        if (gSaveContext.buttonStatus[EQUIP_SLOT_B] != BTN_DISABLED) {
            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
            restoreHudVisibility = true;
        }
    } else if (player->stateFlags1 & PLAYER_STATE1_2000) {
        // Hanging from a ledge
        if (gSaveContext.buttonStatus[EQUIP_SLOT_B] != BTN_DISABLED) {
            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
            restoreHudVisibility = true;
            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
        }
    } else {
        // End of special event cases

        // B button
        if ((interfaceCtx->bButtonDoAction == DO_ACTION_EXPLODE) && (player->currentMask == PLAYER_MASK_BLAST) &&
            (player->blastMaskTimer != 0)) {
            // Cooldown period for blast mask
            if (gSaveContext.bButtonStatus != BTN_DISABLED) {
                gSaveContext.bButtonStatus = BTN_DISABLED;
                restoreHudVisibility = true;
            }
        } else {
            // default to enabled
            if (gSaveContext.bButtonStatus == BTN_DISABLED) {
                gSaveContext.bButtonStatus = BTN_ENABLED;
                restoreHudVisibility = true;
            }

            // Apply B button restriction
            if (interfaceCtx->restrictions.bButton == 0) {
                if ((BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOW) ||
                    (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOMB) ||
                    (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOMBCHU)) {
                    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_NONE) {
                        gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
                    }

                    if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_ENABLED) {
                        gSaveContext.buttonStatus[EQUIP_SLOT_B] =
                            ITEM_SWORD_KOKIRI + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI;
                    }

                    BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = gSaveContext.buttonStatus[EQUIP_SLOT_B];

                    if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NONE) {
                        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
                    }
                    restoreHudVisibility = true;
                } else if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_NONE) {
                    if (interfaceCtx->bButtonDoAction != 0) {
                        if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) {
                            restoreHudVisibility = true;
                            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
                        }
                    } else {
                        if (gSaveContext.buttonStatus[EQUIP_SLOT_B] != BTN_DISABLED) {
                            restoreHudVisibility = true;
                            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
                        }
                    }
                } else if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_NONE) {
                    if (gSaveContext.buttonStatus[EQUIP_SLOT_B] != BTN_DISABLED) {
                        restoreHudVisibility = true;
                        gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
                    }
                } else {
                    if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) {
                        restoreHudVisibility = true;
                        gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
                    }
                }
            } else if (interfaceCtx->restrictions.bButton != 0) {
                if ((BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOW) ||
                    (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOMB) ||
                    (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOMBCHU)) {
                    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_NONE) {
                        gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
                    }

                    BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = gSaveContext.buttonStatus[EQUIP_SLOT_B];

                    if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NONE) {
                        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
                    }
                    restoreHudVisibility = true;
                }
                if (gSaveContext.buttonStatus[EQUIP_SLOT_B] != BTN_DISABLED) {
                    gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_DISABLED;
                    restoreHudVisibility = true;
                }
            }
        }

        // C buttons
        if (gSaveContext.save.playerForm == player->transformation) {
            for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                // Individual C button
                if (!gPlayerFormItemRestrictions[(void)0, gSaveContext.save.playerForm][GET_CUR_FORM_BTN_ITEM(i)]) {
                    // Item not usable in current playerForm
                    if (gSaveContext.buttonStatus[i] != BTN_DISABLED) {
                        gSaveContext.buttonStatus[i] = BTN_DISABLED;
                        restoreHudVisibility = true;
                    }
                } else if (player->actor.id != ACTOR_PLAYER) {
                    // Currently not playing as the main player
                    if (gSaveContext.buttonStatus[i] != BTN_DISABLED) {
                        gSaveContext.buttonStatus[i] = BTN_DISABLED;
                        restoreHudVisibility = true;
                    }
                } else if (player->currentMask == PLAYER_MASK_GIANT) {
                    // Currently wearing Giant's Mask
                    if (GET_CUR_FORM_BTN_ITEM(i) != ITEM_MASK_GIANT) {
                        if (gSaveContext.buttonStatus[i] != BTN_DISABLED) {
                            gSaveContext.buttonStatus[i] = BTN_DISABLED;
                            restoreHudVisibility = true;
                        }
                    } else if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                        restoreHudVisibility = true;
                        gSaveContext.buttonStatus[i] = BTN_ENABLED;
                    }
                } else if (GET_CUR_FORM_BTN_ITEM(i) == ITEM_MASK_GIANT) {
                    // Giant's Mask is equipped
                    if (play->sceneId != SCENE_INISIE_BS) {
                        if (gSaveContext.buttonStatus[i] != BTN_DISABLED) {
                            gSaveContext.buttonStatus[i] = BTN_DISABLED;
                            restoreHudVisibility = true;
                        }
                    } else if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                        restoreHudVisibility = true;
                        gSaveContext.buttonStatus[i] = BTN_ENABLED;
                    }
                } else if (GET_CUR_FORM_BTN_ITEM(i) == ITEM_MASK_FIERCE_DEITY) {
                    // Fierce Deity's Mask is equipped
                    if ((play->sceneId != SCENE_MITURIN_BS) && (play->sceneId != SCENE_HAKUGIN_BS) &&
                        (play->sceneId != SCENE_SEA_BS) && (play->sceneId != SCENE_INISIE_BS) &&
                        (play->sceneId != SCENE_LAST_BS)) {
                        if (gSaveContext.buttonStatus[i] != BTN_DISABLED) {
                            gSaveContext.buttonStatus[i] = BTN_DISABLED;
                            restoreHudVisibility = true;
                        }
                    } else if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                        restoreHudVisibility = true;
                        gSaveContext.buttonStatus[i] = BTN_ENABLED;
                    }
                } else {
                    // End of special item cases. Apply restrictions to buttons
                    if (interfaceCtx->restrictions.tradeItems != 0) {
                        if (((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MOON_TEAR) &&
                             (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_PENDANT_OF_MEMORIES)) ||
                            ((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_BOTTLE) &&
                             (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_OBABA_DRINK)) ||
                            (GET_CUR_FORM_BTN_ITEM(i) == ITEM_OCARINA)) {
                            if (gSaveContext.buttonStatus[i] == BTN_ENABLED) {
                                restoreHudVisibility = true;
                            }
                            gSaveContext.buttonStatus[i] = BTN_DISABLED;
                        }
                    } else if (interfaceCtx->restrictions.tradeItems == 0) {
                        if (((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MOON_TEAR) &&
                             (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_PENDANT_OF_MEMORIES)) ||
                            ((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_BOTTLE) &&
                             (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_OBABA_DRINK)) ||
                            (GET_CUR_FORM_BTN_ITEM(i) == ITEM_OCARINA)) {
                            if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                                restoreHudVisibility = true;
                            }
                            gSaveContext.buttonStatus[i] = BTN_ENABLED;
                        }
                    }

                    if (interfaceCtx->restrictions.unk_317 != 0) {
                        if ((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MASK_DEKU) &&
                            (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_MASK_GIANT)) {
                            if (!gSaveContext.buttonStatus[i]) { // == BTN_ENABLED
                                restoreHudVisibility = true;
                            }
                            gSaveContext.buttonStatus[i] = BTN_DISABLED;
                        }
                    } else if (interfaceCtx->restrictions.unk_317 == 0) {
                        if ((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MASK_DEKU) &&
                            (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_MASK_GIANT)) {
                            if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                                restoreHudVisibility = true;
                            }
                            gSaveContext.buttonStatus[i] = BTN_ENABLED;
                        }
                    }

                    if (interfaceCtx->restrictions.pictographBox != 0) {
                        if (GET_CUR_FORM_BTN_ITEM(i) == ITEM_PICTO_BOX) {
                            if (!gSaveContext.buttonStatus[i]) { // == BTN_ENABLED
                                restoreHudVisibility = true;
                            }
                            gSaveContext.buttonStatus[i] = BTN_DISABLED;
                        }
                    } else if (interfaceCtx->restrictions.pictographBox == 0) {
                        if (GET_CUR_FORM_BTN_ITEM(i) == ITEM_PICTO_BOX) {
                            if (gSaveContext.buttonStatus[i] == BTN_DISABLED) {
                                restoreHudVisibility = true;
                            }
                            gSaveContext.buttonStatus[i] = BTN_ENABLED;
                        }
                    }

                    if (interfaceCtx->restrictions.all != 0) {
                        if (!((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MOON_TEAR) &&
                              (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_PENDANT_OF_MEMORIES)) &&
                            !((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_BOTTLE) &&
                              (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_OBABA_DRINK)) &&
                            (GET_CUR_FORM_BTN_ITEM(i) != ITEM_OCARINA) &&
                            !((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MASK_DEKU) &&
                              (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_MASK_GIANT)) &&
                            (GET_CUR_FORM_BTN_ITEM(i) != ITEM_PICTO_BOX)) {

                            if ((gSaveContext.buttonStatus[i] == BTN_ENABLED)) {
                                restoreHudVisibility = true;
                                gSaveContext.buttonStatus[i] = BTN_DISABLED;
                            }
                        }
                    } else if (interfaceCtx->restrictions.all == 0) {
                        if (!((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MOON_TEAR) &&
                              (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_PENDANT_OF_MEMORIES)) &&
                            !((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_BOTTLE) &&
                              (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_OBABA_DRINK)) &&
                            (GET_CUR_FORM_BTN_ITEM(i) != ITEM_OCARINA) &&
                            !((GET_CUR_FORM_BTN_ITEM(i) >= ITEM_MASK_DEKU) &&
                              (GET_CUR_FORM_BTN_ITEM(i) <= ITEM_MASK_GIANT)) &&
                            (GET_CUR_FORM_BTN_ITEM(i) != ITEM_PICTO_BOX)) {

                            if ((gSaveContext.buttonStatus[i] == BTN_DISABLED)) {
                                restoreHudVisibility = true;
                                gSaveContext.buttonStatus[i] = BTN_ENABLED;
                            }
                        }
                    }
                }
            }
        }
    }

    if (restoreHudVisibility && (play->activeCamId == CAM_ID_MAIN) && (play->transitionTrigger == TRANS_TRIGGER_OFF) &&
        (play->transitionMode == TRANS_MODE_OFF)) {
        gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
        Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
    }
}

void Interface_UpdateButtonsPart1(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    Player* player = GET_PLAYER(play);
    s32 pad;
    s32 restoreHudVisibility = false;

    if (gSaveContext.save.cutscene < 0xFFF0) {
        gSaveContext.hudVisibilityForceButtonAlphasByStatus = false;
        if ((player->stateFlags1 & PLAYER_STATE1_800000) || (gSaveContext.save.weekEventReg[8] & 1) ||
            (!(gSaveContext.eventInf[4] & 2) && (play->bButtonAmmoPlusOne >= 2))) {
            // Riding Epona OR Honey & Darling minigame OR Horseback balloon minigame OR related to swamp boat
            // (non-minigame?)
            if ((player->stateFlags1 & PLAYER_STATE1_800000) && (player->currentMask == PLAYER_MASK_BLAST) &&
                (gSaveContext.bButtonStatus == BTN_DISABLED)) {
                // Riding Epona with blast mask?
                restoreHudVisibility = true;
                gSaveContext.bButtonStatus = BTN_ENABLED;
            }

            if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NONE) {
                if ((player->transformation == PLAYER_FORM_DEKU) && (gSaveContext.save.weekEventReg[8] & 1)) {
                    gSaveContext.hudVisibilityForceButtonAlphasByStatus = true;
                    if (play->sceneId == SCENE_BOWLING) {
                        if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) {
                            gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
                            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
                            gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
                            gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
                        }
                    } else if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) {
                        gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_ENABLED;
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_ENABLED;
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_ENABLED;
                    }

                    Interface_SetHudVisibility(HUD_VISIBILITY_B_MAGIC);
                } else {
                    if ((BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_BOW) &&
                        (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_BOMB) &&
                        (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_BOMBCHU)) {
                        gSaveContext.hudVisibilityForceButtonAlphasByStatus = true;
                        gSaveContext.buttonStatus[EQUIP_SLOT_B] = BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B);
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_ENABLED;
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_ENABLED;
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_ENABLED;
                        if (play->sceneId == SCENE_BOWLING) {
                            if (CURRENT_DAY == 1) {
                                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_BOMBCHU;
                            } else if (CURRENT_DAY == 2) {
                                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_BOMB;
                            } else {
                                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_BOW;
                            }
                            Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
                            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
                            gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
                            gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
                        } else {
                            BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_BOW;

                            if (play->bButtonAmmoPlusOne >= 2) {
                                Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
                            } else if (gSaveContext.save.inventory.items[SLOT_BOW] == ITEM_NONE) {
                                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_NONE;
                            } else {
                                Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
                            }

                            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
                            gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
                            gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
                            Interface_SetHudVisibility(HUD_VISIBILITY_A_HEARTS_MAGIC_MINIMAP_WITH_OVERWRITE);
                        }
                    }

                    if (play->transitionMode != TRANS_MODE_OFF) {
                        Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
                    } else if ((gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) &&
                               (gSaveContext.save.entrance == ENTRANCE(ROMANI_RANCH, 0)) &&
                               (Cutscene_GetSceneLayer(play) != 0) && (play->transitionTrigger == TRANS_TRIGGER_OFF)) {
                        Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
                    } else if ((gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) &&
                               (gSaveContext.eventInf[3] & 0x20)) {
                        Interface_SetHudVisibility(HUD_VISIBILITY_B_MINIMAP);
                    } else if (!(gSaveContext.save.weekEventReg[0x52] & 8) &&
                               (gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE)) {
                        Interface_SetHudVisibility(HUD_VISIBILITY_B);
                    } else if (play->bButtonAmmoPlusOne >= 2) {
                        Interface_SetHudVisibility(HUD_VISIBILITY_B);
                    } else if (gSaveContext.save.weekEventReg[8] & 1) {
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
                        Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
                    } else if (player->stateFlags1 & PLAYER_STATE1_800000) {
                        Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
                    }
                }
            } else {
                if (player->stateFlags1 & PLAYER_STATE1_800000) {
                    Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
                }

                if (play->sceneId == SCENE_BOWLING) {
                    if (CURRENT_DAY == 1) {
                        BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_BOMBCHU;
                    } else if (CURRENT_DAY == 2) {
                        BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_BOMB;
                    } else {
                        BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_BOW;
                    }
                    gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
                    gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
                    gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
                } else {
                    BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_BOW;
                }

                if (play->bButtonAmmoPlusOne >= 2) {
                    Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
                } else if (gSaveContext.save.inventory.items[SLOT_BOW] == ITEM_NONE) {
                    BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_NONE;
                } else {
                    Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
                }

                if (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED) {
                    gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
                    restoreHudVisibility = true;
                }

                gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
                gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
                gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
                Interface_SetHudVisibility(HUD_VISIBILITY_A_HEARTS_MAGIC_MINIMAP_WITH_OVERWRITE);

                if (play->transitionMode != TRANS_MODE_OFF) {
                    Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
                } else if ((gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) &&
                           (gSaveContext.save.entrance == ENTRANCE(ROMANI_RANCH, 0)) &&
                           (Cutscene_GetSceneLayer(play) != 0) && (play->transitionTrigger == TRANS_TRIGGER_OFF)) {
                    Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
                } else if (gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) {
                    Interface_SetHudVisibility(HUD_VISIBILITY_B);
                } else if (play->bButtonAmmoPlusOne >= 2) {
                    Interface_SetHudVisibility(HUD_VISIBILITY_B);
                } else if (gSaveContext.save.weekEventReg[8] & 1) {
                    gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
                    gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
                    gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
                    Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
                } else if (player->stateFlags1 & PLAYER_STATE1_800000) {
                    Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
                }
            }
        } else if (sPictographState != PICTOGRAPH_STATE_OFF) {
            // Related to pictograph
            if (sPictographState == PICTOGRAPH_STATE_LENS) {
                if (!(play->actorCtx.flags & ACTORCTX_FLAG_PICTOGRAPH_ON)) {
                    Play_CompressI8ToI5((play->pictoPhotoI8 != NULL) ? play->pictoPhotoI8 : D_801FBB90,
                                        (u8*)((void)0, gSaveContext.pictoPhotoI5),
                                        PICTO_RESOLUTION_X * PICTO_RESOLUTION_Y);
                    interfaceCtx->unk_222 = interfaceCtx->unk_224 = 0;
                    restoreHudVisibility = true;
                    sPictographState = PICTOGRAPH_STATE_OFF;
                } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_B)) {
                    play->actorCtx.flags &= ~ACTORCTX_FLAG_PICTOGRAPH_ON;
                    interfaceCtx->unk_222 = interfaceCtx->unk_224 = 0;
                    restoreHudVisibility = true;
                    sPictographState = PICTOGRAPH_STATE_OFF;
                } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_A) || (func_801A5100() == 1)) {
                    if (!(gSaveContext.eventInf[4] & 2) ||
                        ((gSaveContext.eventInf[4] & 2) && (ActorCutscene_GetCurrentIndex() == -1))) {
                        play_sound(NA_SE_SY_CAMERA_SHUTTER);
                        SREG(89) = 1;
                        play->haltAllActors = true;
                        sPictographState = PICTOGRAPH_STATE_SETUP_PHOTO;
                        sPictographPhotoBeingTaken = true;
                    }
                }
            } else if ((sPictographState >= PICTOGRAPH_STATE_SETUP_PHOTO) && (Message_GetState(&play->msgCtx) == 4) &&
                       Message_ShouldAdvance(play)) {
                play->haltAllActors = false;
                player->stateFlags1 &= ~PLAYER_STATE1_200;
                func_801477B4(play);
                if (play->msgCtx.choiceIndex != 0) {
                    func_8019F230();
                    Interface_LoadBButtonDoActionLabel(play, DO_ACTION_STOP);
                    Interface_SetHudVisibility(HUD_VISIBILITY_A_B);
                    sPictographState = PICTOGRAPH_STATE_LENS;
                    REMOVE_QUEST_ITEM(QUEST_PICTOGRAPH);
                } else {
                    func_8019F208();
                    interfaceCtx->unk_222 = interfaceCtx->unk_224 = 0;
                    restoreHudVisibility = true;
                    Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                    sPictographState = PICTOGRAPH_STATE_OFF;
                    if (sPictographPhotoBeingTaken) {
                        Play_CompressI8ToI5((play->pictoPhotoI8 != NULL) ? play->pictoPhotoI8 : D_801FBB90,
                                            (u8*)((void)0, gSaveContext.pictoPhotoI5),
                                            PICTO_RESOLUTION_X * PICTO_RESOLUTION_Y);
                        Snap_RecordPictographedActors(play);
                    }
                    play->actorCtx.flags &= ~ACTORCTX_FLAG_PICTOGRAPH_ON;
                    SET_QUEST_ITEM(QUEST_PICTOGRAPH);
                    sPictographPhotoBeingTaken = false;
                }
            }
        } else if ((gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) &&
                   (gSaveContext.save.entrance == ENTRANCE(WATERFALL_RAPIDS, 1)) &&
                   (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF)) {
            // Beaver race minigame
            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
            Interface_SetHudVisibility(HUD_VISIBILITY_A_B_MINIMAP);
        } else if ((gSaveContext.save.entrance == ENTRANCE(GORON_RACETRACK, 1)) &&
                   (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF)) {
            // Goron race minigame
            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_DISABLED;
            gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_DISABLED;
            Interface_SetHudVisibility(HUD_VISIBILITY_A_B_HEARTS_MAGIC_MINIMAP);
        } else if (play->actorCtx.flags & ACTORCTX_FLAG_PICTOGRAPH_ON) {
            // Related to pictograph
            if (!CHECK_QUEST_ITEM(QUEST_PICTOGRAPH)) {
                Interface_LoadBButtonDoActionLabel(play, DO_ACTION_STOP);
                Interface_SetHudVisibility(HUD_VISIBILITY_A_B);
                sPictographState = PICTOGRAPH_STATE_LENS;
            } else {
                Play_DecompressI5ToI8((u8*)((void)0, gSaveContext.pictoPhotoI5),
                                      (play->pictoPhotoI8 != NULL) ? play->pictoPhotoI8 : D_801FBB90,
                                      PICTO_RESOLUTION_X * PICTO_RESOLUTION_Y);
                play->haltAllActors = true;
                sPictographState = PICTOGRAPH_STATE_SETUP_PHOTO;
            }
        } else {
            // Continue processing the remaining cases
            Interface_UpdateButtonsPart2(play);
        }
    }

    if (restoreHudVisibility) {
        gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
        if ((play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF)) {
            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
        }
    }
}

void Interface_SetSceneRestrictions(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 i = 0;
    u8 currentScene;

    do {
        currentScene = (u8)play->sceneId;
        if (sRestrictionFlags[i].scene == currentScene) {
            interfaceCtx->restrictions.hGauge = (sRestrictionFlags[i].flags1 & 0xC0) >> 6;
            interfaceCtx->restrictions.bButton = (sRestrictionFlags[i].flags1 & 0x30) >> 4;
            interfaceCtx->restrictions.aButton = (sRestrictionFlags[i].flags1 & 0x0C) >> 2;
            interfaceCtx->restrictions.tradeItems = (sRestrictionFlags[i].flags1 & 0x03) >> 0;
            interfaceCtx->restrictions.unk_312 = (sRestrictionFlags[i].flags2 & 0xC0) >> 6;
            interfaceCtx->restrictions.unk_313 = (sRestrictionFlags[i].flags2 & 0x30) >> 4;
            interfaceCtx->restrictions.unk_314 = (sRestrictionFlags[i].flags2 & 0x0C) >> 2;
            interfaceCtx->restrictions.songOfSoaring = (sRestrictionFlags[i].flags2 & 0x03) >> 0;
            interfaceCtx->restrictions.songOfStorms = (sRestrictionFlags[i].flags3 & 0xC0) >> 6;
            interfaceCtx->restrictions.unk_317 = (sRestrictionFlags[i].flags3 & 0x30) >> 4;
            interfaceCtx->restrictions.pictographBox = (sRestrictionFlags[i].flags3 & 0x0C) >> 2;
            interfaceCtx->restrictions.all = (sRestrictionFlags[i].flags3 & 0x03) >> 0;
            break;
        }
        i++;
    } while (sRestrictionFlags[i].scene != 0xFF);
}

void Interface_Noop(void) {
}

void Interface_InitMinigame(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    gSaveContext.minigameStatus = MINIGAME_STATUS_ACTIVE;
    gSaveContext.minigameScore = 0;
    gSaveContext.minigameHiddenScore = 0;

    sHBAScoreTier = 0;
    interfaceCtx->minigamePoints = interfaceCtx->minigameHiddenPoints = interfaceCtx->minigameUnusedPoints = 0;

    interfaceCtx->minigameAmmo = 20;
}

void Interface_LoadItemIconImpl(PlayState* play, u8 btn) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    func_80178E3C(SEGMENT_ROM_START(icon_item_static_test), GET_CUR_FORM_BTN_ITEM(btn),
                  &interfaceCtx->iconItemSegment[(u32)btn * 0x1000], 0x1000);
}

void Interface_LoadItemIcon(PlayState* play, u8 btn) {
    Interface_LoadItemIconImpl(play, btn);
}

/**
 * @param play PlayState
 * @param flag 0 for default update, 1 for simplified update
 */
void Interface_UpdateButtonsAlt(PlayState* play, u16 flag) {
    if (flag) {
        if ((BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOW) ||
            (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOMB) ||
            (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOMBCHU) ||
            (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_FISHING_ROD) ||
            (gSaveContext.buttonStatus[EQUIP_SLOT_B] == BTN_DISABLED)) {
            if ((BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOW) ||
                (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOMB) ||
                (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_BOMBCHU) ||
                (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_FISHING_ROD)) {
                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = gSaveContext.buttonStatus[EQUIP_SLOT_B];
                Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
            }
        } else if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_NONE) {
            if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NONE) {
                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = gSaveContext.buttonStatus[EQUIP_SLOT_B];
                Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
            }
        }

        gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] =
            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
        Interface_SetHudVisibility(HUD_VISIBILITY_ALL_NO_MINIMAP_W_DISABLED);
    } else {
        gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] =
            gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
        Interface_UpdateButtonsPart1(play);
    }
}

s16 sAmmoRefillCounts[] = { 5, 10, 20, 30 }; // Sticks, nuts, bombs
s16 sArrowRefillCounts[] = { 10, 30, 40, 50 };
s16 sBombchuRefillCounts[] = { 20, 10, 1, 5 };
s16 sRupeeRefillCounts[] = { 1, 5, 10, 20, 50, 100, 200 };

u8 Item_Give(PlayState* play, u8 item) {
    Player* player = GET_PLAYER(play);
    u8 i;
    u8 temp;
    u8 slot;

    slot = SLOT(item);
    if (item >= ITEM_STICKS_5) {
        slot = SLOT(sExtraItemBases[item - ITEM_STICKS_5]);
    }

    if (item == ITEM_SKULL_TOKEN) {
        SET_QUEST_ITEM(item - ITEM_SKULL_TOKEN + QUEST_SKULL_TOKEN);
        Inventory_IncrementSkullTokenCount(play->sceneId);
        return ITEM_NONE;

    } else if (item == ITEM_TINGLE_MAP) {
        return ITEM_NONE;

    } else if (item == ITEM_BOMBERS_NOTEBOOK) {
        SET_QUEST_ITEM(QUEST_BOMBERS_NOTEBOOK);
        return ITEM_NONE;

    } else if ((item == ITEM_HEART_PIECE_2) || (item == ITEM_HEART_PIECE)) {
        gSaveContext.save.inventory.questItems += (1 << QUEST_HEART_PIECE_COUNT);
        if ((gSaveContext.save.inventory.questItems & 0xF0000000) == (4 << QUEST_HEART_PIECE_COUNT)) {
            gSaveContext.save.inventory.questItems ^= (4 << QUEST_HEART_PIECE_COUNT);
            gSaveContext.save.playerData.healthCapacity += 0x10;
            gSaveContext.save.playerData.health += 0x10;
        }
        return ITEM_NONE;

    } else if (item == ITEM_HEART_CONTAINER) {
        gSaveContext.save.playerData.healthCapacity += 0x10;
        gSaveContext.save.playerData.health += 0x10;
        return ITEM_NONE;

    } else if ((item >= ITEM_SONG_SONATA) && (item <= ITEM_SONG_LULLABY_INTRO)) {
        SET_QUEST_ITEM(item - ITEM_SONG_SONATA + QUEST_SONG_SONATA);
        return ITEM_NONE;

    } else if ((item >= ITEM_SWORD_KOKIRI) && (item <= ITEM_SWORD_GILDED)) {
        SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, item - ITEM_SWORD_KOKIRI + EQUIP_VALUE_SWORD_KOKIRI);
        BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = item;
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
        if (item == ITEM_SWORD_RAZOR) {
            gSaveContext.save.playerData.swordHealth = 100;
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_SHIELD_HERO) && (item <= ITEM_SHIELD_MIRROR)) {
        if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) != (u16)(item - ITEM_SHIELD_HERO + EQUIP_VALUE_SHIELD_HERO)) {
            SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, item - ITEM_SHIELD_HERO + EQUIP_VALUE_SHIELD_HERO);
            Player_SetEquipmentData(play, player);
            return ITEM_NONE;
        }
        return item;

    } else if ((item == ITEM_KEY_BOSS) || (item == ITEM_COMPASS) || (item == ITEM_DUNGEON_MAP)) {
        SET_DUNGEON_ITEM(item - ITEM_KEY_BOSS, gSaveContext.mapIndex);
        return ITEM_NONE;

    } else if (item == ITEM_KEY_SMALL) {
        if (DUNGEON_KEY_COUNT(gSaveContext.mapIndex) < 0) {
            DUNGEON_KEY_COUNT(gSaveContext.mapIndex) = 1;
            return ITEM_NONE;
        } else {
            DUNGEON_KEY_COUNT(gSaveContext.mapIndex)++;
            return ITEM_NONE;
        }

    } else if ((item == ITEM_QUIVER_30) || (item == ITEM_BOW)) {
        if (CUR_UPG_VALUE(UPG_QUIVER) == 0) {
            Inventory_ChangeUpgrade(UPG_QUIVER, 1);
            INV_CONTENT(ITEM_BOW) = ITEM_BOW;
            AMMO(ITEM_BOW) = CAPACITY(UPG_QUIVER, 1);
            return ITEM_NONE;
        } else {
            AMMO(ITEM_BOW)++;
            if (AMMO(ITEM_BOW) > (s8)CUR_CAPACITY(UPG_QUIVER)) {
                AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
            }
        }

    } else if (item == ITEM_QUIVER_40) {
        Inventory_ChangeUpgrade(UPG_QUIVER, 2);
        INV_CONTENT(ITEM_BOW) = ITEM_BOW;
        AMMO(ITEM_BOW) = CAPACITY(UPG_QUIVER, 2);
        return ITEM_NONE;

    } else if (item == ITEM_QUIVER_50) {
        Inventory_ChangeUpgrade(UPG_QUIVER, 3);
        INV_CONTENT(ITEM_BOW) = ITEM_BOW;
        AMMO(ITEM_BOW) = CAPACITY(UPG_QUIVER, 3);
        return ITEM_NONE;

    } else if (item == ITEM_BOMB_BAG_20) {
        if (CUR_UPG_VALUE(UPG_BOMB_BAG) == 0) {
            Inventory_ChangeUpgrade(UPG_BOMB_BAG, 1);
            INV_CONTENT(ITEM_BOMB) = ITEM_BOMB;
            AMMO(ITEM_BOMB) = CAPACITY(UPG_BOMB_BAG, 1);
            return ITEM_NONE;

        } else {
            AMMO(ITEM_BOMB)++;
            if (AMMO(ITEM_BOMB) > CUR_CAPACITY(UPG_BOMB_BAG)) {
                AMMO(ITEM_BOMB) = CUR_CAPACITY(UPG_BOMB_BAG);
            }
        }

    } else if (item == ITEM_BOMB_BAG_30) {
        Inventory_ChangeUpgrade(UPG_BOMB_BAG, 2);
        INV_CONTENT(ITEM_BOMB) = ITEM_BOMB;
        AMMO(ITEM_BOMB) = CAPACITY(UPG_BOMB_BAG, 2);
        return ITEM_NONE;

    } else if (item == ITEM_BOMB_BAG_40) {
        Inventory_ChangeUpgrade(UPG_BOMB_BAG, 3);
        INV_CONTENT(ITEM_BOMB) = ITEM_BOMB;
        AMMO(ITEM_BOMB) = CAPACITY(UPG_BOMB_BAG, 3);
        return ITEM_NONE;

    } else if (item == ITEM_WALLET_ADULT) {
        Inventory_ChangeUpgrade(UPG_WALLET, 1);
        return ITEM_NONE;

    } else if (item == ITEM_WALLET_GIANT) {
        Inventory_ChangeUpgrade(UPG_WALLET, 2);
        return ITEM_NONE;

    } else if (item == ITEM_STICK_UPGRADE_20) {
        if (INV_CONTENT(ITEM_STICK) != ITEM_STICK) {
            INV_CONTENT(ITEM_STICK) = ITEM_STICK;
        }
        Inventory_ChangeUpgrade(UPG_STICKS, 2);
        AMMO(ITEM_STICK) = CAPACITY(UPG_STICKS, 2);
        return ITEM_NONE;

    } else if (item == ITEM_STICK_UPGRADE_30) {
        if (INV_CONTENT(ITEM_STICK) != ITEM_STICK) {
            INV_CONTENT(ITEM_STICK) = ITEM_STICK;
        }
        Inventory_ChangeUpgrade(UPG_STICKS, 3);
        AMMO(ITEM_STICK) = CAPACITY(UPG_STICKS, 3);
        return ITEM_NONE;

    } else if (item == ITEM_NUT_UPGRADE_30) {
        if (INV_CONTENT(ITEM_NUT) != ITEM_NUT) {
            INV_CONTENT(ITEM_NUT) = ITEM_NUT;
        }
        Inventory_ChangeUpgrade(UPG_NUTS, 2);
        AMMO(ITEM_NUT) = CAPACITY(UPG_NUTS, 2);
        return ITEM_NONE;

    } else if (item == ITEM_NUT_UPGRADE_40) {
        if (INV_CONTENT(ITEM_NUT) != ITEM_NUT) {
            INV_CONTENT(ITEM_NUT) = ITEM_NUT;
        }
        Inventory_ChangeUpgrade(UPG_NUTS, 3);
        AMMO(ITEM_NUT) = CAPACITY(UPG_NUTS, 3);
        return ITEM_NONE;

    } else if (item == ITEM_STICK) {
        if (INV_CONTENT(ITEM_STICK) != ITEM_STICK) {
            Inventory_ChangeUpgrade(UPG_STICKS, 1);
            AMMO(ITEM_STICK) = 1;
        } else {
            AMMO(ITEM_STICK)++;
            if (AMMO(ITEM_STICK) > CUR_CAPACITY(UPG_STICKS)) {
                AMMO(ITEM_STICK) = CUR_CAPACITY(UPG_STICKS);
            }
        }

    } else if ((item == ITEM_STICKS_5) || (item == ITEM_STICKS_10)) {
        if (INV_CONTENT(ITEM_STICK) != ITEM_STICK) {
            Inventory_ChangeUpgrade(UPG_STICKS, 1);
            AMMO(ITEM_STICK) = sAmmoRefillCounts[item - ITEM_STICKS_5];
        } else {
            AMMO(ITEM_STICK) += sAmmoRefillCounts[item - ITEM_STICKS_5];
            if (AMMO(ITEM_STICK) > CUR_CAPACITY(UPG_STICKS)) {
                AMMO(ITEM_STICK) = CUR_CAPACITY(UPG_STICKS);
            }
        }

        item = ITEM_STICK;

    } else if (item == ITEM_NUT) {
        if (INV_CONTENT(ITEM_NUT) != ITEM_NUT) {
            Inventory_ChangeUpgrade(UPG_NUTS, 1);
            AMMO(ITEM_NUT) = 1;
        } else {
            AMMO(ITEM_NUT)++;
            if (AMMO(ITEM_NUT) > CUR_CAPACITY(UPG_NUTS)) {
                AMMO(ITEM_NUT) = CUR_CAPACITY(UPG_NUTS);
            }
        }

    } else if ((item == ITEM_NUTS_5) || (item == ITEM_NUTS_10)) {
        if (INV_CONTENT(ITEM_NUT) != ITEM_NUT) {
            Inventory_ChangeUpgrade(UPG_NUTS, 1);
            AMMO(ITEM_NUT) += sAmmoRefillCounts[item - ITEM_NUTS_5];
        } else {
            AMMO(ITEM_NUT) += sAmmoRefillCounts[item - ITEM_NUTS_5];
            if (AMMO(ITEM_NUT) > CUR_CAPACITY(UPG_NUTS)) {
                AMMO(ITEM_NUT) = CUR_CAPACITY(UPG_NUTS);
            }
        }
        item = ITEM_NUT;

    } else if (item == ITEM_POWDER_KEG) {
        if (INV_CONTENT(ITEM_POWDER_KEG) != ITEM_POWDER_KEG) {
            INV_CONTENT(ITEM_POWDER_KEG) = ITEM_POWDER_KEG;
        }

        AMMO(ITEM_POWDER_KEG) = 1;
        return ITEM_NONE;

    } else if (item == ITEM_BOMB) {
        if ((AMMO(ITEM_BOMB) += 1) > CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMB) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_BOMBS_5) && (item <= ITEM_BOMBS_30)) {
        if (gSaveContext.save.inventory.items[SLOT_BOMB] != ITEM_BOMB) {
            INV_CONTENT(ITEM_BOMB) = ITEM_BOMB;
            AMMO(ITEM_BOMB) += sAmmoRefillCounts[item - ITEM_BOMBS_5];
            return ITEM_NONE;
        }

        if ((AMMO(ITEM_BOMB) += sAmmoRefillCounts[item - ITEM_BOMBS_5]) > CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMB) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
        return ITEM_NONE;

    } else if (item == ITEM_BOMBCHU) {
        if (INV_CONTENT(ITEM_BOMBCHU) != ITEM_BOMBCHU) {
            INV_CONTENT(ITEM_BOMBCHU) = ITEM_BOMBCHU;
            AMMO(ITEM_BOMBCHU) = 10;
            return ITEM_NONE;
        }
        if ((AMMO(ITEM_BOMBCHU) += 10) > CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_BOMBCHUS_20) && (item <= ITEM_BOMBCHUS_5)) {
        if (gSaveContext.save.inventory.items[SLOT_BOMBCHU] != ITEM_BOMBCHU) {
            INV_CONTENT(ITEM_BOMBCHU) = ITEM_BOMBCHU;
            AMMO(ITEM_BOMBCHU) += sBombchuRefillCounts[item - ITEM_BOMBCHUS_20];

            if (AMMO(ITEM_BOMBCHU) > CUR_CAPACITY(UPG_BOMB_BAG)) {
                AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
            }
            return ITEM_NONE;
        }

        if ((AMMO(ITEM_BOMBCHU) += sBombchuRefillCounts[item - ITEM_BOMBCHUS_20]) > CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_ARROWS_10) && (item <= ITEM_ARROWS_50)) {
        AMMO(ITEM_BOW) += sArrowRefillCounts[item - ITEM_ARROWS_10];

        if ((AMMO(ITEM_BOW) >= CUR_CAPACITY(UPG_QUIVER)) || (AMMO(ITEM_BOW) < 0)) {
            AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        }
        return ITEM_BOW;

    } else if (item == ITEM_OCARINA) {
        INV_CONTENT(ITEM_OCARINA) = ITEM_OCARINA;
        return ITEM_NONE;

    } else if (item == ITEM_MAGIC_BEANS) {
        if (INV_CONTENT(ITEM_MAGIC_BEANS) == ITEM_NONE) {
            INV_CONTENT(item) = item;
            AMMO(ITEM_MAGIC_BEANS) = 1;
        } else if (AMMO(ITEM_MAGIC_BEANS) < 20) {
            AMMO(ITEM_MAGIC_BEANS)++;
        } else {
            AMMO(ITEM_MAGIC_BEANS) = 20;
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_REMAINS_ODOLWA) && (item <= ITEM_REMAINS_TWINMOLD)) {
        SET_QUEST_ITEM(item - ITEM_REMAINS_ODOLWA + QUEST_REMAINS_ODOWLA);
        return ITEM_NONE;

    } else if (item == ITEM_RECOVERY_HEART) {
        Health_ChangeBy(play, 0x10);
        return item;

    } else if (item == ITEM_MAGIC_SMALL) {
        Magic_Add(play, MAGIC_NORMAL_METER / 2);
        if (!(gSaveContext.save.weekEventReg[12] & 0x80)) {
            gSaveContext.save.weekEventReg[12] |= 0x80;
            return ITEM_NONE;
        }
        return item;

    } else if (item == ITEM_MAGIC_LARGE) {
        Magic_Add(play, MAGIC_NORMAL_METER);
        if (!(gSaveContext.save.weekEventReg[12] & 0x80)) {
            gSaveContext.save.weekEventReg[12] |= 0x80;
            return ITEM_NONE;
        }
        return item;

    } else if ((item >= ITEM_RUPEE_GREEN) && (item <= ITEM_RUPEE_HUGE)) {
        Rupees_ChangeBy(sRupeeRefillCounts[item - ITEM_RUPEE_GREEN]);
        return ITEM_NONE;

    } else if (item == ITEM_LONGSHOT) {
        slot = SLOT(item);

        for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
            if (gSaveContext.save.inventory.items[slot + i] == ITEM_NONE) {
                gSaveContext.save.inventory.items[slot + i] = ITEM_POTION_RED;
                return ITEM_NONE;
            }
        }
        return item;

    } else if ((item == ITEM_MILK_BOTTLE) || (item == ITEM_POE) || (item == ITEM_GOLD_DUST) || (item == ITEM_CHATEAU) ||
               (item == ITEM_HYLIAN_LOACH)) {
        slot = SLOT(item);

        for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
            if (gSaveContext.save.inventory.items[slot + i] == ITEM_NONE) {
                gSaveContext.save.inventory.items[slot + i] = item;
                return ITEM_NONE;
            }
        }
        return item;

    } else if (item == ITEM_BOTTLE) {
        slot = SLOT(item);

        for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
            if (gSaveContext.save.inventory.items[slot + i] == ITEM_NONE) {
                gSaveContext.save.inventory.items[slot + i] = item;
                return ITEM_NONE;
            }
        }
        return item;

    } else if (((item >= ITEM_POTION_RED) && (item <= ITEM_OBABA_DRINK)) || (item == ITEM_CHATEAU_2) ||
               (item == ITEM_MILK) || (item == ITEM_GOLD_DUST_2) || (item == ITEM_HYLIAN_LOACH_2) ||
               (item == ITEM_SEAHORSE_CAUGHT)) {
        slot = SLOT(item);

        if ((item != ITEM_MILK_BOTTLE) && (item != ITEM_MILK_HALF)) {
            if (item == ITEM_CHATEAU_2) {
                item = ITEM_CHATEAU;

            } else if (item == ITEM_MILK) {
                item = ITEM_MILK_BOTTLE;

            } else if (item == ITEM_GOLD_DUST_2) {
                item = ITEM_GOLD_DUST;

            } else if (item == ITEM_HYLIAN_LOACH_2) {
                item = ITEM_HYLIAN_LOACH;

            } else if (item == ITEM_SEAHORSE_CAUGHT) {
                item = ITEM_SEAHORSE;
            }
            slot = SLOT(item);

            for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
                if (gSaveContext.save.inventory.items[slot + i] == ITEM_BOTTLE) {
                    if (item == ITEM_HOT_SPRING_WATER) {
                        Interface_StartBottleTimer(60, i);
                    }

                    if ((slot + i) == C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = item;
                        Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_LEFT);
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_ENABLED;
                    } else if ((slot + i) == C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = item;
                        Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_DOWN);
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_ENABLED;
                    } else if ((slot + i) == C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = item;
                        Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_RIGHT);
                        gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_ENABLED;
                    }

                    gSaveContext.save.inventory.items[slot + i] = item;
                    return ITEM_NONE;
                }
            }
        } else {
            for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
                if (gSaveContext.save.inventory.items[slot + i] == ITEM_NONE) {
                    gSaveContext.save.inventory.items[slot + i] = item;
                    return ITEM_NONE;
                }
            }
        }

    } else if ((item >= ITEM_MOON_TEAR) && (item <= ITEM_MASK_GIANT)) {
        temp = INV_CONTENT(item);
        INV_CONTENT(item) = item;
        if ((item >= ITEM_MOON_TEAR) && (item <= ITEM_PENDANT_OF_MEMORIES) && (temp != ITEM_NONE)) {
            for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                if (temp == GET_CUR_FORM_BTN_ITEM(i)) {
                    SET_CUR_FORM_BTN_ITEM(i, item);
                    Interface_LoadItemIconImpl(play, i);
                    return ITEM_NONE;
                }
            }
        }
        return ITEM_NONE;
    }

    temp = gSaveContext.save.inventory.items[slot];
    INV_CONTENT(item) = item;
    return temp;
}

u8 Item_CheckObtainabilityImpl(u8 item) {
    s16 i;
    u8 slot;
    u8 bottleSlot;

    slot = SLOT(item);
    if (item >= ITEM_STICKS_5) {
        slot = SLOT(sExtraItemBases[item - ITEM_STICKS_5]);
    }

    if (item == ITEM_SKULL_TOKEN) {
        return ITEM_NONE;

    } else if (item == ITEM_TINGLE_MAP) {
        return ITEM_NONE;

    } else if (item == ITEM_BOMBERS_NOTEBOOK) {
        return ITEM_NONE;

    } else if ((item >= ITEM_SWORD_KOKIRI) && (item <= ITEM_SWORD_GILDED)) {
        return ITEM_NONE;

    } else if ((item >= ITEM_SHIELD_HERO) && (item <= ITEM_SHIELD_MIRROR)) {
        return ITEM_NONE;

    } else if ((item == ITEM_KEY_BOSS) || (item == ITEM_COMPASS) || (item == ITEM_DUNGEON_MAP)) {
        if (!CHECK_DUNGEON_ITEM(item - ITEM_KEY_BOSS, gSaveContext.mapIndex)) {
            return ITEM_NONE;
        }
        return item;

    } else if (item == ITEM_KEY_SMALL) {
        return ITEM_NONE;

    } else if ((item == ITEM_OCARINA) || (item == ITEM_BOMBCHU) || (item == ITEM_HOOKSHOT) || (item == ITEM_LENS) ||
               (item == ITEM_SWORD_GREAT_FAIRY) || (item == ITEM_PICTO_BOX)) {
        if (INV_CONTENT(item) == ITEM_NONE) {
            return ITEM_NONE;
        }
        return INV_CONTENT(item);

    } else if ((item >= ITEM_BOMBS_5) && (item == ITEM_BOMBS_30)) {
        //! @bug: Should be a range check: (item <= ITEM_BOMBS_30)
        if (CUR_UPG_VALUE(UPG_BOMB_BAG) == 0) {
            return ITEM_NONE;
        }
        return 0;

    } else if ((item >= ITEM_BOMBCHUS_20) && (item <= ITEM_BOMBCHUS_5)) {
        if (CUR_UPG_VALUE(UPG_BOMB_BAG) == 0) {
            return ITEM_NONE;
        }
        return 0;

    } else if ((item == ITEM_QUIVER_30) || (item == ITEM_BOW)) {
        if (CUR_UPG_VALUE(UPG_QUIVER) == 0) {
            return ITEM_NONE;
        }
        return 0;

    } else if ((item == ITEM_QUIVER_40) || (item == ITEM_QUIVER_50)) {
        return ITEM_NONE;

    } else if ((item == ITEM_BOMB_BAG_20) || (item == ITEM_BOMB)) {
        if (CUR_UPG_VALUE(UPG_BOMB_BAG) == 0) {
            return ITEM_NONE;
        }
        return 0;

    } else if ((item >= ITEM_STICK_UPGRADE_20) && (item <= ITEM_NUT_UPGRADE_40)) {
        return ITEM_NONE;

    } else if ((item >= ITEM_BOMB_BAG_30) && (item <= ITEM_WALLET_GIANT)) {
        return ITEM_NONE;

    } else if (item == ITEM_MAGIC_BEANS) {
        return ITEM_NONE;

    } else if (item == ITEM_POWDER_KEG) {
        return ITEM_NONE;

    } else if ((item == ITEM_HEART_PIECE_2) || (item == ITEM_HEART_PIECE)) {
        return ITEM_NONE;

    } else if (item == ITEM_HEART_CONTAINER) {
        return ITEM_NONE;

    } else if (item == ITEM_RECOVERY_HEART) {
        return ITEM_RECOVERY_HEART;

    } else if ((item == ITEM_MAGIC_SMALL) || (item == ITEM_MAGIC_LARGE)) {
        if (!(gSaveContext.save.weekEventReg[12] & 0x80)) {
            return ITEM_NONE;
        }
        return item;

    } else if ((item >= ITEM_RUPEE_GREEN) && (item <= ITEM_RUPEE_HUGE)) {
        return ITEM_NONE;

    } else if ((item >= ITEM_REMAINS_ODOLWA) && (item <= ITEM_REMAINS_TWINMOLD)) {
        return ITEM_NONE;

    } else if (item == ITEM_LONGSHOT) {
        return ITEM_NONE;

    } else if (item == ITEM_BOTTLE) {
        return ITEM_NONE;

    } else if ((item == ITEM_MILK_BOTTLE) || (item == ITEM_POE) || (item == ITEM_GOLD_DUST) || (item == ITEM_CHATEAU) ||
               (item == ITEM_HYLIAN_LOACH)) {
        return ITEM_NONE;

    } else if (((item >= ITEM_POTION_RED) && (item <= ITEM_OBABA_DRINK)) || (item == ITEM_CHATEAU_2) ||
               (item == ITEM_MILK) || (item == ITEM_GOLD_DUST_2) || (item == ITEM_HYLIAN_LOACH_2) ||
               (item == ITEM_SEAHORSE_CAUGHT)) {
        bottleSlot = SLOT(item);

        if ((item != ITEM_MILK_BOTTLE) && (item != ITEM_MILK_HALF)) {
            if (item == ITEM_CHATEAU_2) {
                item = ITEM_CHATEAU;

            } else if (item == ITEM_MILK) {
                item = ITEM_MILK_BOTTLE;

            } else if (item == ITEM_GOLD_DUST_2) {
                item = ITEM_GOLD_DUST;

            } else if (item == ITEM_HYLIAN_LOACH_2) {
                item = ITEM_HYLIAN_LOACH;

            } else if (item == ITEM_SEAHORSE_CAUGHT) {
                item = ITEM_SEAHORSE;
            }
            bottleSlot = SLOT(item);

            for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
                if (gSaveContext.save.inventory.items[bottleSlot + i] == ITEM_BOTTLE) {
                    return ITEM_NONE;
                }
            }
        } else {
            for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
                if (gSaveContext.save.inventory.items[bottleSlot + i] == ITEM_NONE) {
                    return ITEM_NONE;
                }
            }
        }
    } else if ((item >= ITEM_MOON_TEAR) && (item <= ITEM_MASK_GIANT)) {
        return ITEM_NONE;
    }

    return gSaveContext.save.inventory.items[slot];
}

u8 Item_CheckObtainability(u8 item) {
    return Item_CheckObtainabilityImpl(item);
}

void Inventory_DeleteItem(s16 item, s16 slot) {
    s16 btn;

    gSaveContext.save.inventory.items[slot] = ITEM_NONE;

    for (btn = EQUIP_SLOT_C_LEFT; btn <= EQUIP_SLOT_C_RIGHT; btn++) {
        if (GET_CUR_FORM_BTN_ITEM(btn) == item) {
            SET_CUR_FORM_BTN_ITEM(btn, ITEM_NONE);
            SET_CUR_FORM_BTN_SLOT(btn, SLOT_NONE);
        }
    }
}

void Inventory_UnequipItem(s16 item) {
    s16 btn;

    for (btn = EQUIP_SLOT_C_LEFT; btn <= EQUIP_SLOT_C_RIGHT; btn++) {
        if (GET_CUR_FORM_BTN_ITEM(btn) == item) {
            SET_CUR_FORM_BTN_ITEM(btn, ITEM_NONE);
            SET_CUR_FORM_BTN_SLOT(btn, SLOT_NONE);
        }
    }
}

s32 Inventory_ReplaceItem(PlayState* play, u8 oldItem, u8 newItem) {
    u8 i;

    for (i = 0; i < 24; i++) {
        if (gSaveContext.save.inventory.items[i] == oldItem) {
            gSaveContext.save.inventory.items[i] = newItem;

            for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                if (GET_CUR_FORM_BTN_ITEM(i) == oldItem) {
                    SET_CUR_FORM_BTN_ITEM(i, newItem);
                    Interface_LoadItemIconImpl(play, i);
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

void Inventory_UpdateDeitySwordEquip(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    u8 btn;

    if (CUR_FORM == PLAYER_FORM_FIERCE_DEITY) {
        interfaceCtx->bButtonDoActionActive = false;
        interfaceCtx->bButtonDoAction = 0;

        // Is simply checking if (gSaveContext.save.playerForm == PLAYER_FORM_FIERCE_DEITY)
        if ((((gSaveContext.save.playerForm > 0) && (gSaveContext.save.playerForm < 4))
                 ? 1
                 : gSaveContext.save.playerForm >> 1) == 0) {
            BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_SWORD_DEITY;
        } else if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_SWORD_DEITY) {
            if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_NONE) {
                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = ITEM_NONE;
            } else {
                BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) =
                    GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI + ITEM_SWORD_KOKIRI;
            }
        }
    }

    for (btn = EQUIP_SLOT_B; btn <= EQUIP_SLOT_B; btn++) {
        if ((GET_CUR_FORM_BTN_ITEM(btn) != ITEM_NONE) && (GET_CUR_FORM_BTN_ITEM(btn) != ITEM_FD)) {
            Interface_LoadItemIconImpl(play, btn);
        }
    }
}

s32 Inventory_HasEmptyBottle(void) {
    s32 slot;

    for (slot = SLOT_BOTTLE_1; slot <= SLOT_BOTTLE_6; slot++) {
        if (gSaveContext.save.inventory.items[slot] == ITEM_BOTTLE) {
            return true;
        }
    }
    return false;
}

s32 Inventory_HasItemInBottle(u8 item) {
    s32 slot;

    for (slot = SLOT_BOTTLE_1; slot <= SLOT_BOTTLE_6; slot++) {
        if (gSaveContext.save.inventory.items[slot] == item) {
            return true;
        }
    }
    return false;
}

void Inventory_UpdateBottleItem(PlayState* play, u8 item, u8 btn) {
    gSaveContext.save.inventory.items[GET_CUR_FORM_BTN_SLOT(btn)] = item;
    SET_CUR_FORM_BTN_ITEM(btn, item);

    Interface_LoadItemIconImpl(play, btn);

    play->pauseCtx.cursorItem[PAUSE_ITEM] = item;
    gSaveContext.buttonStatus[btn] = BTN_ENABLED;

    if (item == ITEM_HOT_SPRING_WATER) {
        Interface_StartBottleTimer(60, GET_CUR_FORM_BTN_SLOT(btn) - SLOT_BOTTLE_1);
    }
}

s32 Inventory_ConsumeFairy(PlayState* play) {
    u8 bottleSlot = SLOT(ITEM_FAIRY);
    u8 btn;
    u8 i;

    for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
        if (gSaveContext.save.inventory.items[bottleSlot + i] == ITEM_FAIRY) {
            for (btn = EQUIP_SLOT_C_LEFT; btn <= EQUIP_SLOT_C_RIGHT; btn++) {
                if (GET_CUR_FORM_BTN_ITEM(btn) == ITEM_FAIRY) {
                    SET_CUR_FORM_BTN_ITEM(btn, ITEM_BOTTLE);
                    Interface_LoadItemIconImpl(play, btn);
                    bottleSlot = GET_CUR_FORM_BTN_SLOT(btn);
                    i = 0;
                    break;
                }
            }
            gSaveContext.save.inventory.items[bottleSlot + i] = ITEM_BOTTLE;
            return true;
        }
    }

    return false;
}

/**
 * Only used to equip Spring Water when Hot Spring Water timer runs out.
 */
void Inventory_UpdateItem(PlayState* play, s16 slot, s16 item) {
    s16 btn;

    gSaveContext.save.inventory.items[slot] = item;

    for (btn = EQUIP_SLOT_C_LEFT; btn <= EQUIP_SLOT_C_RIGHT; btn++) {
        if (GET_CUR_FORM_BTN_SLOT(btn) == slot) {
            SET_CUR_FORM_BTN_ITEM(btn, item);
            Interface_LoadItemIconImpl(play, btn);
        }
    }
}

void Interface_MemSetZeroed(u32* buf, s32 count) {
    s32 i;

    for (i = 0; i != count; i++) {
        buf[i] = 0;
    }
}

void Interface_LoadAButtonDoActionLabel(InterfaceContext* interfaceCtx, u16 action, s16 loadOffset) {
    static TexturePtr sDoActionTextures[] = {
        gDoActionAttackENGTex,
        gDoActionCheckENGTex,
    };

    if (action >= DO_ACTION_MAX) {
        action = DO_ACTION_NONE;
    }

    if (action != DO_ACTION_NONE) {
        osCreateMesgQueue(&interfaceCtx->loadQueue, &interfaceCtx->loadMsg, 1);
        DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184,
                               (u32)interfaceCtx->doActionSegment + (loadOffset * DO_ACTION_TEX_SIZE),
                               (u32)SEGMENT_ROM_START(do_action_static) + (action * DO_ACTION_TEX_SIZE),
                               DO_ACTION_TEX_SIZE, 0, &interfaceCtx->loadQueue, 0);
        osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);
    } else {
        gSegments[0x09] = PHYSICAL_TO_VIRTUAL(interfaceCtx->doActionSegment);
        Interface_MemSetZeroed(Lib_SegmentedToVirtual(sDoActionTextures[loadOffset]), 0x60);
    }
}

void Interface_SetAButtonDoAction(PlayState* play, u16 aButtonDoAction) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    PauseContext* pauseCtx = &play->pauseCtx;

    if (interfaceCtx->aButtonDoAction != aButtonDoAction) {
        interfaceCtx->aButtonDoAction = aButtonDoAction;
        interfaceCtx->aButtonState = A_BTN_STATE_1;
        interfaceCtx->aButtonRoll = 0.0f;
        Interface_LoadAButtonDoActionLabel(interfaceCtx, aButtonDoAction, 1);
        if (pauseCtx->state != 0) {
            interfaceCtx->aButtonState = A_BTN_STATE_3;
        }
    }
}

void Interface_SetBButtonDoAction(PlayState* play, s16 bButtonDoAction) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    if (((BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) >= ITEM_SWORD_KOKIRI) &&
         (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) <= ITEM_SWORD_GILDED)) ||
        (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_NONE) ||
        (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) == ITEM_NUT)) {
        if ((CUR_FORM == PLAYER_FORM_DEKU) && !gSaveContext.save.playerData.isMagicAcquired) {
            interfaceCtx->bButtonDoAction = 0xFD;
        } else {
            interfaceCtx->bButtonDoAction = bButtonDoAction;
            if (interfaceCtx->bButtonDoAction != DO_ACTION_NONE) {
                osCreateMesgQueue(&interfaceCtx->loadQueue, &interfaceCtx->loadMsg, 1);
                DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184, interfaceCtx->doActionSegment + 0x600,
                                       (bButtonDoAction * 0x180) + SEGMENT_ROM_START(do_action_static), 0x180, 0,
                                       &interfaceCtx->loadQueue, NULL);
                osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);
            }

            interfaceCtx->bButtonDoActionActive = true;
        }
    } else {
        interfaceCtx->bButtonDoActionActive = false;
        interfaceCtx->bButtonDoAction = 0;
    }
}

void Interface_SetTatlCall(PlayState* play, u16 tatlCallState) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    if (((tatlCallState == 0x2A) || (tatlCallState == 0x2B)) && !interfaceCtx->tatlCalling &&
        (play->csCtx.state == 0)) {
        if (tatlCallState == 0x2B) {
            play_sound(NA_SE_VO_NAVY_CALL);
        }
        if (tatlCallState == 0x2A) {
            func_8019FDC8(&gSfxDefaultPos, NA_SE_VO_NA_HELLO_2, 0x20);
        }
        interfaceCtx->tatlCalling = true;
        sCUpInvisible = 0;
        sCUpTimer = 10;
    } else if (tatlCallState == 0x2C) {
        if (interfaceCtx->tatlCalling) {
            interfaceCtx->tatlCalling = false;
        }
    }
}

void Interface_LoadBButtonDoActionLabel(PlayState* play, s16 bButtonDoAction) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    interfaceCtx->unk_224 = bButtonDoAction;

    osCreateMesgQueue(&play->interfaceCtx.loadQueue, &play->interfaceCtx.loadMsg, 1);
    DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184, interfaceCtx->doActionSegment + 0x480,
                           (bButtonDoAction * 0x180) + SEGMENT_ROM_START(do_action_static), 0x180, 0,
                           &interfaceCtx->loadQueue, NULL);
    osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);

    interfaceCtx->unk_222 = 1;
}

/**
 * @return false if player is out of health
 */
s32 Health_ChangeBy(PlayState* play, s16 healthChange) {
    if (healthChange > 0) {
        play_sound(NA_SE_SY_HP_RECOVER);
    } else if (gSaveContext.save.playerData.doubleDefense && (healthChange < 0)) {
        healthChange >>= 1;
    }

    gSaveContext.save.playerData.health += healthChange;

    if (((void)0, gSaveContext.save.playerData.health) > ((void)0, gSaveContext.save.playerData.healthCapacity)) {
        gSaveContext.save.playerData.health = gSaveContext.save.playerData.healthCapacity;
    }

    if (gSaveContext.save.playerData.health <= 0) {
        gSaveContext.save.playerData.health = 0;
        return false;
    } else {
        return true;
    }
}

void Health_GiveHearts(s16 hearts) {
    gSaveContext.save.playerData.healthCapacity += hearts * 0x10;
}

void Rupees_ChangeBy(s16 rupeeChange) {
    gSaveContext.rupeeAccumulator += rupeeChange;
}

void Inventory_ChangeAmmo(s16 item, s16 ammoChange) {
    if (item == ITEM_STICK) {
        AMMO(ITEM_STICK) += ammoChange;

        if (AMMO(ITEM_STICK) >= CUR_CAPACITY(UPG_STICKS)) {
            AMMO(ITEM_STICK) = CUR_CAPACITY(UPG_STICKS);
        } else if (AMMO(ITEM_STICK) < 0) {
            AMMO(ITEM_STICK) = 0;
        }

    } else if (item == ITEM_NUT) {
        AMMO(ITEM_NUT) += ammoChange;

        if (AMMO(ITEM_NUT) >= CUR_CAPACITY(UPG_NUTS)) {
            AMMO(ITEM_NUT) = CUR_CAPACITY(UPG_NUTS);
        } else if (AMMO(ITEM_NUT) < 0) {
            AMMO(ITEM_NUT) = 0;
        }

    } else if (item == ITEM_BOMBCHU) {
        AMMO(ITEM_BOMBCHU) += ammoChange;

        if (AMMO(ITEM_BOMBCHU) >= CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        } else if (AMMO(ITEM_BOMBCHU) < 0) {
            AMMO(ITEM_BOMBCHU) = 0;
        }

    } else if (item == ITEM_BOW) {
        AMMO(ITEM_BOW) += ammoChange;

        if (AMMO(ITEM_BOW) >= CUR_CAPACITY(UPG_QUIVER)) {
            AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        } else if (AMMO(ITEM_BOW) < 0) {
            AMMO(ITEM_BOW) = 0;
        }

    } else if (item == ITEM_BOMB) {
        AMMO(ITEM_BOMB) += ammoChange;

        if (AMMO(ITEM_BOMB) >= CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMB) = CUR_CAPACITY(UPG_BOMB_BAG);
        } else if (AMMO(ITEM_BOMB) < 0) {
            AMMO(ITEM_BOMB) = 0;
        }

    } else if (item == ITEM_MAGIC_BEANS) {
        AMMO(ITEM_MAGIC_BEANS) += ammoChange;

    } else if (item == ITEM_POWDER_KEG) {
        AMMO(ITEM_POWDER_KEG) += ammoChange;
        if (AMMO(ITEM_POWDER_KEG) >= 1) {
            AMMO(ITEM_POWDER_KEG) = 1;
        } else if (AMMO(ITEM_POWDER_KEG) < 0) {
            AMMO(ITEM_POWDER_KEG) = 0;
        }
    }
}

void Magic_Add(PlayState* play, s16 magicToAdd) {
    if (((void)0, gSaveContext.save.playerData.magic) < ((void)0, gSaveContext.magicCapacity)) {
        gSaveContext.magicToAdd += magicToAdd;
        gSaveContext.isMagicRequested = true;
    }
}

void Magic_Reset(PlayState* play) {
    if ((gSaveContext.magicState != MAGIC_STATE_STEP_CAPACITY) && (gSaveContext.magicState != MAGIC_STATE_FILL)) {
        sMagicMeterOutlinePrimRed = sMagicMeterOutlinePrimGreen = sMagicMeterOutlinePrimBlue = 255;
        gSaveContext.magicState = MAGIC_STATE_IDLE;
    }
}

/**
 * Request to consume magic.
 * @param magicToConsume the positive-valued amount to decrease magic by
 * @param type how the magic is consumed.
 * @return false if the request failed
 */
s32 Magic_Consume(PlayState* play, s16 magicToConsume, s16 type) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    // Magic is not acquired yet
    if (!gSaveContext.save.playerData.isMagicAcquired) {
        return false;
    }

    // Not enough magic available to consume
    if ((gSaveContext.save.playerData.magic - magicToConsume) < 0) {
        if (gSaveContext.magicCapacity != 0) {
            play_sound(NA_SE_SY_ERROR);
        }
        return false;
    }

    switch (type) {
        case MAGIC_CONSUME_NOW:
        case MAGIC_CONSUME_NOW_ALT:
            // Consume magic immediately
            // Ex. Deku Bubble
            if ((gSaveContext.magicState == MAGIC_STATE_IDLE) ||
                (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS)) {
                if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                    play->actorCtx.lensActive = false;
                }
                if (gSaveContext.save.weekEventReg[14] & 8) {
                    magicToConsume = 0;
                }
                gSaveContext.magicToConsume = magicToConsume;
                gSaveContext.magicState = MAGIC_STATE_CONSUME_SETUP;
                return true;
            } else {
                play_sound(NA_SE_SY_ERROR);
                return false;
            }

        case MAGIC_CONSUME_WAIT_NO_PREVIEW:
            // Sets consume target but waits to consume.
            // No yellow magic to preview target consumption.
            if ((gSaveContext.magicState == MAGIC_STATE_IDLE) ||
                (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS)) {
                if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                    play->actorCtx.lensActive = false;
                }
                if (gSaveContext.save.weekEventReg[14] & 8) {
                    magicToConsume = 0;
                }
                gSaveContext.magicToConsume = magicToConsume;
                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_3;
                return true;
            } else {
                play_sound(NA_SE_SY_ERROR);
                return false;
            }

        case MAGIC_CONSUME_LENS:
            if (gSaveContext.magicState == MAGIC_STATE_IDLE) {
                if (gSaveContext.save.playerData.magic != 0) {
                    interfaceCtx->magicConsumptionTimer = 80;
                    gSaveContext.magicState = MAGIC_STATE_CONSUME_LENS;
                    return true;
                } else {
                    return false;
                }
            } else if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                return true;
            } else {
                return false;
            }

        case MAGIC_CONSUME_WAIT_PREVIEW:
            // Sets consume target but waits to consume.
            // Preview consumption with a yellow bar
            // Ex. Spin Attack
            if ((gSaveContext.magicState == MAGIC_STATE_IDLE) ||
                (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS)) {
                if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                    play->actorCtx.lensActive = false;
                }
                gSaveContext.magicToConsume = magicToConsume;
                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_2;
                return true;
            } else {
                play_sound(NA_SE_SY_ERROR);
                return false;
            }

        case MAGIC_CONSUME_GORON_ZORA:
            // Zora Shock, Goron Spike Roll
            if (gSaveContext.save.playerData.magic != 0) {
                interfaceCtx->magicConsumptionTimer = 10;
                gSaveContext.magicState = MAGIC_STATE_CONSUME_GORON_ZORA_SETUP;
                return true;
            } else {
                return false;
            }

        case MAGIC_CONSUME_GIANTS_MASK:
            // Wearing Giants Mask
            if (gSaveContext.magicState == MAGIC_STATE_IDLE) {
                if (gSaveContext.save.playerData.magic != 0) {
                    interfaceCtx->magicConsumptionTimer = R_MAGIC_CONSUME_TIMER_GIANTS_MASK;
                    gSaveContext.magicState = MAGIC_STATE_CONSUME_GIANTS_MASK;
                    return true;
                } else {
                    return false;
                }
            }
            if (gSaveContext.magicState == MAGIC_STATE_CONSUME_GIANTS_MASK) {
                return true;
            } else {
                return false;
            }

        case MAGIC_CONSUME_DEITY_BEAM:
            // Using Fierce Deity Beam
            // Consumes magic immediately
            if ((gSaveContext.magicState == MAGIC_STATE_IDLE) ||
                (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS)) {
                if (gSaveContext.magicState == MAGIC_STATE_CONSUME_LENS) {
                    play->actorCtx.lensActive = false;
                }
                if (gSaveContext.save.weekEventReg[14] & 8) {
                    magicToConsume = 0;
                }
                gSaveContext.save.playerData.magic -= magicToConsume;
                return true;
            } else {
                play_sound(NA_SE_SY_ERROR);
                return false;
            }
    }

    return false;
}

void Magic_UpdateAddRequest(void) {
    if (gSaveContext.isMagicRequested) {
        gSaveContext.save.playerData.magic += 4;
        play_sound(NA_SE_SY_GAUGE_UP - SFX_FLAG);

        if (((void)0, gSaveContext.save.playerData.magic) >= ((void)0, gSaveContext.magicCapacity)) {
            gSaveContext.save.playerData.magic = gSaveContext.magicCapacity;
            gSaveContext.magicToAdd = 0;
            gSaveContext.isMagicRequested = false;
        } else {
            gSaveContext.magicToAdd -= 4;
            if (gSaveContext.magicToAdd <= 0) {
                gSaveContext.magicToAdd = 0;
                gSaveContext.isMagicRequested = false;
            }
        }
    }
}

s16 sMagicBorderColors[][3] = {
    { 255, 255, 255 },
    { 150, 150, 150 },
};
s16 sMagicBorderIndices[] = { 0, 1, 1, 0 };
s16 sMagicBorderColorTimerIndex[] = { 2, 1, 2, 1 };

void Magic_FlashMeterBorder(void) {
    s16 borderChangeR;
    s16 borderChangeG;
    s16 borderChangeB;
    s16 index = sMagicBorderIndices[sMagicBorderStep];

    borderChangeR = ABS_ALT(sMagicMeterOutlinePrimRed - sMagicBorderColors[index][0]) / sMagicBorderRatio;
    borderChangeG = ABS_ALT(sMagicMeterOutlinePrimGreen - sMagicBorderColors[index][1]) / sMagicBorderRatio;
    borderChangeB = ABS_ALT(sMagicMeterOutlinePrimBlue - sMagicBorderColors[index][2]) / sMagicBorderRatio;

    if (sMagicMeterOutlinePrimRed >= sMagicBorderColors[index][0]) {
        sMagicMeterOutlinePrimRed -= borderChangeR;
    } else {
        sMagicMeterOutlinePrimRed += borderChangeR;
    }

    if (sMagicMeterOutlinePrimGreen >= sMagicBorderColors[index][1]) {
        sMagicMeterOutlinePrimGreen -= borderChangeG;
    } else {
        sMagicMeterOutlinePrimGreen += borderChangeG;
    }

    if (sMagicMeterOutlinePrimBlue >= sMagicBorderColors[index][2]) {
        sMagicMeterOutlinePrimBlue -= borderChangeB;
    } else {
        sMagicMeterOutlinePrimBlue += borderChangeB;
    }

    sMagicBorderRatio--;
    if (sMagicBorderRatio == 0) {
        sMagicMeterOutlinePrimRed = sMagicBorderColors[index][0];
        sMagicMeterOutlinePrimGreen = sMagicBorderColors[index][1];
        sMagicMeterOutlinePrimBlue = sMagicBorderColors[index][2];

        sMagicBorderRatio = sMagicBorderColorTimerIndex[sMagicBorderStep];

        sMagicBorderStep++;
        if (sMagicBorderStep >= 4) {
            sMagicBorderStep = 0;
        }
    }
}

void Magic_Update(PlayState* play) {
    MessageContext* msgCtx = &play->msgCtx;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 magicCapacityTarget;

    if (gSaveContext.save.weekEventReg[14] & 8) {
        Magic_FlashMeterBorder();
    }

    switch (gSaveContext.magicState) {
        case MAGIC_STATE_STEP_CAPACITY:
            // Step magicCapacity to the capacity determined by magicLevel
            // This changes the width of the magic meter drawn
            magicCapacityTarget = gSaveContext.save.playerData.magicLevel * MAGIC_NORMAL_METER;
            if (gSaveContext.magicCapacity != magicCapacityTarget) {
                if (gSaveContext.magicCapacity < magicCapacityTarget) {
                    gSaveContext.magicCapacity += 0x10;
                    if (gSaveContext.magicCapacity > magicCapacityTarget) {
                        gSaveContext.magicCapacity = magicCapacityTarget;
                    }
                } else {
                    gSaveContext.magicCapacity -= 0x10;
                    if (gSaveContext.magicCapacity <= magicCapacityTarget) {
                        gSaveContext.magicCapacity = magicCapacityTarget;
                    }
                }
            } else {
                // Once the capacity has reached its target,
                // follow up by filling magic to magicFillTarget
                gSaveContext.magicState = MAGIC_STATE_FILL;
            }
            break;

        case MAGIC_STATE_FILL:
            // Add magic until magicFillTarget is reached
            gSaveContext.save.playerData.magic += 0x10;

            if ((gSaveContext.gameMode == 0) && (gSaveContext.sceneLayer < 4)) {
                play_sound(NA_SE_SY_GAUGE_UP - SFX_FLAG);
            }

            if (((void)0, gSaveContext.save.playerData.magic) >= ((void)0, gSaveContext.magicFillTarget)) {
                gSaveContext.save.playerData.magic = gSaveContext.magicFillTarget;
                gSaveContext.magicState = MAGIC_STATE_IDLE;
            }
            break;

        case MAGIC_STATE_CONSUME_SETUP:
            // Sets the speed at which magic border flashes
            sMagicBorderRatio = 2;
            gSaveContext.magicState = MAGIC_STATE_CONSUME;
            break;

        case MAGIC_STATE_CONSUME:
            // Consume magic until target is reached or no more magic is available
            if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                gSaveContext.save.playerData.magic =
                    ((void)0, gSaveContext.save.playerData.magic) - ((void)0, gSaveContext.magicToConsume);
                if (gSaveContext.save.playerData.magic <= 0) {
                    gSaveContext.save.playerData.magic = 0;
                }
                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_1;
                sMagicMeterOutlinePrimRed = sMagicMeterOutlinePrimGreen = sMagicMeterOutlinePrimBlue = 255;
            }
            // fallthrough (flash border while magic is being consumed)
        case MAGIC_STATE_METER_FLASH_1:
        case MAGIC_STATE_METER_FLASH_2:
        case MAGIC_STATE_METER_FLASH_3:
            if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                Magic_FlashMeterBorder();
            }
            break;

        case MAGIC_STATE_RESET:
            sMagicMeterOutlinePrimRed = sMagicMeterOutlinePrimGreen = sMagicMeterOutlinePrimBlue = 255;
            gSaveContext.magicState = MAGIC_STATE_IDLE;
            break;

        case MAGIC_STATE_CONSUME_LENS:
            // Slowly consume magic while lens is on
            if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
                (msgCtx->msgMode == 0) && (play->gameOverCtx.state == GAMEOVER_INACTIVE) &&
                (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF) &&
                !Play_InCsMode(play)) {

                if ((gSaveContext.save.playerData.magic == 0) ||
                    ((Player_GetEnvTimerType(play) >= PLAYER_ENV_TIMER_UNDERWATER_FLOOR) &&
                     (Player_GetEnvTimerType(play) <= PLAYER_ENV_TIMER_UNDERWATER_FREE)) ||
                    ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) != ITEM_LENS) &&
                     (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) != ITEM_LENS) &&
                     (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) != ITEM_LENS)) ||
                    !play->actorCtx.lensActive) {
                    // Force lens off and set magic state to idle
                    play->actorCtx.lensActive = false;
                    play_sound(NA_SE_SY_GLASSMODE_OFF);
                    gSaveContext.magicState = MAGIC_STATE_IDLE;
                    sMagicMeterOutlinePrimRed = sMagicMeterOutlinePrimGreen = sMagicMeterOutlinePrimBlue = 255;
                    break;
                }

                interfaceCtx->magicConsumptionTimer--;
                if (interfaceCtx->magicConsumptionTimer == 0) {
                    if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                        gSaveContext.save.playerData.magic--;
                    }
                    interfaceCtx->magicConsumptionTimer = 80;
                }
            }
            if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                Magic_FlashMeterBorder();
            }
            break;

        case MAGIC_STATE_CONSUME_GORON_ZORA_SETUP:
            if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                gSaveContext.save.playerData.magic -= 2;
            }
            if (gSaveContext.save.playerData.magic <= 0) {
                gSaveContext.save.playerData.magic = 0;
            }
            gSaveContext.magicState = MAGIC_STATE_CONSUME_GORON_ZORA;
            // fallthrough
        case MAGIC_STATE_CONSUME_GORON_ZORA:
            if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
                (msgCtx->msgMode == 0) && (play->gameOverCtx.state == GAMEOVER_INACTIVE) &&
                (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF)) {
                if (!Play_InCsMode(play)) {
                    interfaceCtx->magicConsumptionTimer--;
                    if (interfaceCtx->magicConsumptionTimer == 0) {
                        if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                            gSaveContext.save.playerData.magic--;
                        }
                        if (gSaveContext.save.playerData.magic <= 0) {
                            gSaveContext.save.playerData.magic = 0;
                        }
                        interfaceCtx->magicConsumptionTimer = 10;
                    }
                }
            }
            if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                Magic_FlashMeterBorder();
            }
            break;

        case MAGIC_STATE_CONSUME_GIANTS_MASK:
            if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
                (msgCtx->msgMode == 0) && (play->gameOverCtx.state == GAMEOVER_INACTIVE) &&
                (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF)) {
                if (!Play_InCsMode(play)) {
                    interfaceCtx->magicConsumptionTimer--;
                    if (interfaceCtx->magicConsumptionTimer == 0) {
                        if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                            gSaveContext.save.playerData.magic--;
                        }
                        if (gSaveContext.save.playerData.magic <= 0) {
                            gSaveContext.save.playerData.magic = 0;
                        }
                        interfaceCtx->magicConsumptionTimer = R_MAGIC_CONSUME_TIMER_GIANTS_MASK;
                    }
                }
            }
            if (!(gSaveContext.save.weekEventReg[14] & 8)) {
                Magic_FlashMeterBorder();
            }
            break;

        default:
            gSaveContext.magicState = MAGIC_STATE_IDLE;
            break;
    }
}

void Magic_DrawMeter(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 magicBarY;

    OPEN_DISPS(play->state.gfxCtx);

    if (gSaveContext.save.playerData.magicLevel != 0) {
        if (gSaveContext.save.playerData.healthCapacity > 0xA0) {
            magicBarY = 42; // two rows of hearts
        } else {
            magicBarY = 34; // one row of hearts
        }

        func_8012C654(play->state.gfxCtx);

        gDPSetEnvColor(OVERLAY_DISP++, 100, 50, 50, 255);

        OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(
            OVERLAY_DISP, gMagicMeterEndTex, 8, 16, 18, magicBarY, 8, 16, 1 << 10, 1 << 10, sMagicMeterOutlinePrimRed,
            sMagicMeterOutlinePrimGreen, sMagicMeterOutlinePrimBlue, interfaceCtx->magicAlpha);
        OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(OVERLAY_DISP, gMagicMeterMidTex, 24, 16, 26, magicBarY,
                                                     ((void)0, gSaveContext.magicCapacity), 16, 1 << 10, 1 << 10,
                                                     sMagicMeterOutlinePrimRed, sMagicMeterOutlinePrimGreen,
                                                     sMagicMeterOutlinePrimBlue, interfaceCtx->magicAlpha);
        OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadowOffset(
            OVERLAY_DISP, gMagicMeterEndTex, 8, 16, ((void)0, gSaveContext.magicCapacity) + 26, magicBarY, 8, 16,
            1 << 10, 1 << 10, sMagicMeterOutlinePrimRed, sMagicMeterOutlinePrimGreen, sMagicMeterOutlinePrimBlue,
            interfaceCtx->magicAlpha, 3, 0x100);

        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, PRIMITIVE, PRIMITIVE,
                          ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, PRIMITIVE);
        gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);

        if (gSaveContext.magicState == MAGIC_STATE_METER_FLASH_2) {
            // Yellow part of the meter indicating the amount of magic to be subtracted
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 250, 250, 0, interfaceCtx->magicAlpha);
            gDPLoadTextureBlock_4b(OVERLAY_DISP++, gMagicMeterFillTex, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(OVERLAY_DISP++, 104, (magicBarY + 3) << 2,
                                (((void)0, gSaveContext.save.playerData.magic) + 26) << 2, (magicBarY + 10) << 2,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

            // Fill the rest of the meter with the normal magic color
            gDPPipeSync(OVERLAY_DISP++);
            if (gSaveContext.save.weekEventReg[14] & 8) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 200, interfaceCtx->magicAlpha);
            } else {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 200, 0, interfaceCtx->magicAlpha);
            }

            gSPTextureRectangle(
                OVERLAY_DISP++, 104, (magicBarY + 3) << 2,
                ((((void)0, gSaveContext.save.playerData.magic) - ((void)0, gSaveContext.magicToConsume)) + 26) << 2,
                (magicBarY + 10) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        } else {
            // Fill the whole meter with the normal magic color
            if (gSaveContext.save.weekEventReg[14] & 8) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 200, interfaceCtx->magicAlpha);
            } else {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 200, 0, interfaceCtx->magicAlpha);
            }

            gDPLoadTextureBlock_4b(OVERLAY_DISP++, gMagicMeterFillTex, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(OVERLAY_DISP++, 104, (magicBarY + 3) << 2,
                                (((void)0, gSaveContext.save.playerData.magic) + 26) << 2, (magicBarY + 10) << 2,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Interface_SetPerspectiveView(PlayState* play, s32 topY, s32 bottomY, s32 leftX, s32 rightX) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    Vec3f eye;
    Vec3f lookAt;
    Vec3f up;

    eye.x = eye.y = eye.z = 0.0f;
    lookAt.x = lookAt.y = 0.0f;
    lookAt.z = -1.0f;
    up.x = up.z = 0.0f;
    up.y = 1.0f;

    View_LookAt(&interfaceCtx->view, &eye, &lookAt, &up);

    interfaceCtx->viewport.topY = topY;
    interfaceCtx->viewport.bottomY = bottomY;
    interfaceCtx->viewport.leftX = leftX;
    interfaceCtx->viewport.rightX = rightX;
    View_SetViewport(&interfaceCtx->view, &interfaceCtx->viewport);

    View_SetPerspective(&interfaceCtx->view, 60.0f, 10.0f, 60.0f);
    View_ApplyPerspectiveToOverlay(&interfaceCtx->view);
}

void Interface_SetOrthoView(InterfaceContext* interfaceCtx) {
    SET_FULLSCREEN_VIEWPORT(&interfaceCtx->view);
    View_ApplyOrthoToOverlay(&interfaceCtx->view);
}

void Interface_DrawItemButtons(PlayState* play) {
    static TexturePtr cUpLabelTextures[] = {
        gTatlCUpENGTex, gTatlCUpENGTex, gTatlCUpGERTex, gTatlCUpFRATex, gTatlCUpESPTex,
    };
    static s16 startButtonLeftPos[] = {
        // Remnant of OoT
        130, 136, 136, 136, 136,
    };
    static s16 D_801BFAF4[] = { 0x1D, 0x1B };
    static s16 D_801BFAF8[] = { 0x1B, 0x1B };
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    Player* player = GET_PLAYER(play);
    PauseContext* pauseCtx = &play->pauseCtx;
    MessageContext* msgCtx = &play->msgCtx;
    s16 temp; // Used as both an alpha value and a button index
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    gDPPipeSync(OVERLAY_DISP++);
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    // B Button Color & Texture
    OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(OVERLAY_DISP, gButtonBackgroundTex, 0x20, 0x20, D_801BF9D4[0],
                                                 D_801BF9DC[0], D_801BFAF4[0], D_801BFAF4[0], D_801BF9E4[0] * 2,
                                                 D_801BF9E4[0] * 2, 100, 255, 120, interfaceCtx->bAlpha);
    if (1) {}
    gDPPipeSync(OVERLAY_DISP++);

    // C-Left Button Color & Texture
    OVERLAY_DISP = Gfx_DrawRect_DropShadow(OVERLAY_DISP, D_801BF9D4[1], D_801BF9DC[1], D_801BFAF4[1], D_801BFAF4[1],
                                           D_801BF9E4[1] * 2, D_801BF9E4[1] * 2, 255, 240, 0, interfaceCtx->cLeftAlpha);
    // C-Down Button Color & Texture
    OVERLAY_DISP = Gfx_DrawRect_DropShadow(OVERLAY_DISP, D_801BF9D8[0], D_801BF9E0[0], D_801BFAF8[0], D_801BFAF8[0],
                                           D_801BF9E4[2] * 2, D_801BF9E4[2] * 2, 255, 240, 0, interfaceCtx->cDownAlpha);
    // C-Right Button Color & Texture
    OVERLAY_DISP =
        Gfx_DrawRect_DropShadow(OVERLAY_DISP, D_801BF9D8[1], D_801BF9E0[1], D_801BFAF8[1], D_801BFAF8[1],
                                D_801BF9E4[3] * 2, D_801BF9E4[3] * 2, 255, 240, 0, interfaceCtx->cRightAlpha);

    if ((pauseCtx->state < 8) || (pauseCtx->state >= 19)) {
        if ((play->pauseCtx.state != 0) || (play->pauseCtx.debugEditor != DEBUG_EDITOR_NONE)) {
            OVERLAY_DISP = Gfx_DrawRect_DropShadow(OVERLAY_DISP, 0x88, 0x11, 0x16, 0x16, 0x5B6, 0x5B6, 0xFF, 0x82, 0x3C,
                                                   interfaceCtx->startAlpha);
            // Start Button Texture, Color & Label
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->startAlpha);
            gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);
            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
            gDPLoadTextureBlock_4b(OVERLAY_DISP++, interfaceCtx->doActionSegment + DO_ACTION_TEX_SIZE * 2, G_IM_FMT_IA,
                                   DO_ACTION_TEX_WIDTH, DO_ACTION_TEX_HEIGHT, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(OVERLAY_DISP++, 0x01F8, 0x0054, 0x02D4, 0x009C, G_TX_RENDERTILE, 0, 0, 0x04A6, 0x04A6);
        }
    }

    if (interfaceCtx->tatlCalling && (play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
        (play->csCtx.state == 0) && (sPictographState == PICTOGRAPH_STATE_OFF)) {
        if (sCUpInvisible == 0) {
            // C-Up Button Texture, Color & Label (Tatl Text)
            gDPPipeSync(OVERLAY_DISP++);

            if ((gSaveContext.hudVisibility == HUD_VISIBILITY_NONE) ||
                (gSaveContext.hudVisibility == HUD_VISIBILITY_NONE_ALT) ||
                (gSaveContext.hudVisibility == HUD_VISIBILITY_A_HEARTS_MAGIC_WITH_OVERWRITE) ||
                (msgCtx->msgMode != 0)) {
                temp = 0;
            } else if (player->stateFlags1 & PLAYER_STATE1_200000) {
                temp = 70;
            } else {
                temp = interfaceCtx->aAlpha;
            }

            OVERLAY_DISP =
                Gfx_DrawRect_DropShadow(OVERLAY_DISP, 0xFE, 0x10, 0x10, 0x10, 0x800, 0x800, 0xFF, 0xF0, 0, temp);

            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, temp);
            gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);
            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
            gDPLoadTextureBlock_4b(OVERLAY_DISP++, cUpLabelTextures[gSaveContext.options.language], G_IM_FMT_IA, 32, 12,
                                   0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                   G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(OVERLAY_DISP++, 0x03DC, 0x0048, 0x045C, 0x0078, G_TX_RENDERTILE, 0, 0, 1 << 10,
                                1 << 10);
        }

        sCUpTimer--;
        if (sCUpTimer == 0) {
            sCUpInvisible ^= 1;
            sCUpTimer = 10;
        }
    }

    gDPPipeSync(OVERLAY_DISP++);

    // Empty C Button Arrows
    for (temp = EQUIP_SLOT_C_LEFT; temp <= EQUIP_SLOT_C_RIGHT; temp++) {
        if (GET_CUR_FORM_BTN_ITEM(temp) > 0xF0) {
            if (temp == EQUIP_SLOT_C_LEFT) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 240, 0, interfaceCtx->cLeftAlpha);
            } else if (temp == EQUIP_SLOT_C_DOWN) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 240, 0, interfaceCtx->cDownAlpha);
            } else { // EQUIP_SLOT_C_RIGHT
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 240, 0, interfaceCtx->cRightAlpha);
            }
            OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, ((u8*)gButtonBackgroundTex + ((32 * 32) * (temp + 1))),
                                              0x20, 0x20, D_801BF9D4[temp], D_801BF9DC[temp], D_801BFAF4[temp],
                                              D_801BFAF4[temp], D_801BF9E4[temp] * 2, D_801BF9E4[temp] * 2);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Interface_DrawItemIconTexture(PlayState* play, TexturePtr texture, s16 button) {
    static s16 D_801BFAFC[] = { 30, 24, 24, 24 };

    OPEN_DISPS(play->state.gfxCtx);

    gDPLoadTextureBlock(OVERLAY_DISP++, texture, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(OVERLAY_DISP++, D_801BF9D4[button] << 2, D_801BF9DC[button] << 2,
                        (D_801BF9D4[button] + D_801BFAFC[button]) << 2, (D_801BF9DC[button] + D_801BFAFC[button]) << 2,
                        G_TX_RENDERTILE, 0, 0, D_801BF9BC[button] << 1, D_801BF9BC[button] << 1);

    CLOSE_DISPS(play->state.gfxCtx);
}

s16 D_801BFB04[] = { 0xA2, 0xE4, 0xFA, 0x110 };
s16 D_801BFB0C[] = { 0x23, 0x23, 0x33, 0x23 };

void Interface_DrawAmmoCount(PlayState* play, s16 button, s16 alpha) {
    u8 i;
    u16 ammo;

    OPEN_DISPS(play->state.gfxCtx);

    i = ((void)0, GET_CUR_FORM_BTN_ITEM(button));

    if ((i == ITEM_STICK) || (i == ITEM_NUT) || (i == ITEM_BOMB) || (i == ITEM_BOW) ||
        ((i >= ITEM_BOW_ARROW_FIRE) && (i <= ITEM_BOW_ARROW_LIGHT)) || (i == ITEM_BOMBCHU) || (i == ITEM_POWDER_KEG) ||
        (i == ITEM_MAGIC_BEANS) || (i == ITEM_PICTO_BOX)) {

        if ((i >= ITEM_BOW_ARROW_FIRE) && (i <= ITEM_BOW_ARROW_LIGHT)) {
            i = ITEM_BOW;
        }

        ammo = AMMO(i);

        if (i == ITEM_PICTO_BOX) {
            if (!CHECK_QUEST_ITEM(QUEST_PICTOGRAPH)) {
                ammo = 0;
            } else {
                ammo = 1;
            }
        }

        gDPPipeSync(OVERLAY_DISP++);

        if ((button == EQUIP_SLOT_B) && (gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE)) {
            ammo = play->interfaceCtx.minigameAmmo;
        } else if ((button == EQUIP_SLOT_B) && (play->bButtonAmmoPlusOne > 1)) {
            ammo = play->bButtonAmmoPlusOne - 1;
        } else if (((i == ITEM_BOW) && (AMMO(i) == CUR_CAPACITY(UPG_QUIVER))) ||
                   ((i == ITEM_BOMB) && (AMMO(i) == CUR_CAPACITY(UPG_BOMB_BAG))) ||
                   ((i == ITEM_STICK) && (AMMO(i) == CUR_CAPACITY(UPG_STICKS))) ||
                   ((i == ITEM_NUT) && (AMMO(i) == CUR_CAPACITY(UPG_NUTS))) ||
                   ((i == ITEM_BOMBCHU) && (AMMO(i) == CUR_CAPACITY(UPG_BOMB_BAG))) ||
                   ((i == ITEM_POWDER_KEG) && (ammo == 1)) || ((i == ITEM_PICTO_BOX) && (ammo == 1)) ||
                   ((i == ITEM_MAGIC_BEANS) && (ammo == 20))) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 120, 255, 0, alpha);
        }

        if (!ammo) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 100, 100, alpha);
        }

        for (i = 0; ammo >= 10; i++) {
            ammo -= 10;
        }

        // Draw upper digit (tens)
        if (i) {
            OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, ((u8*)gAmmoDigit0Tex + ((8 * 8) * i)), 8, 8,
                                              D_801BFB04[button], D_801BFB0C[button], 8, 8, 1 << 10, 1 << 10);
        }

        // Draw lower digit (ones)
        OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, ((u8*)gAmmoDigit0Tex + ((8 * 8) * ammo)), 8, 8,
                                          D_801BFB04[button] + 6, D_801BFB0C[button], 8, 8, 1 << 10, 1 << 10);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Interface_DrawBButtonIcons(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    Player* player = GET_PLAYER(play);

    OPEN_DISPS(play->state.gfxCtx);

    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    if ((interfaceCtx->unk_222 == 0) && (player->stateFlags3 & PLAYER_STATE3_1000000)) {
        if (gSaveContext.buttonStatus[EQUIP_SLOT_B] != BTN_DISABLED) {
            Interface_DrawItemIconTexture(play, interfaceCtx->iconItemSegment, EQUIP_SLOT_B);
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

            Interface_DrawAmmoCount(play, EQUIP_SLOT_B, interfaceCtx->bAlpha);
        }
    } else if ((!interfaceCtx->bButtonDoActionActive && (interfaceCtx->unk_222 == 0)) ||
               ((interfaceCtx->bButtonDoActionActive &&
                 ((BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) < ITEM_SWORD_KOKIRI) ||
                  (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) > ITEM_SWORD_GILDED)) &&
                 BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NONE) &&
                (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NUT))) {
        if ((player->transformation == PLAYER_FORM_FIERCE_DEITY) || (player->transformation == PLAYER_FORM_HUMAN)) {
            if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NONE) {
                Interface_DrawItemIconTexture(play, interfaceCtx->iconItemSegment, EQUIP_SLOT_B);
                if ((player->stateFlags1 & PLAYER_STATE1_800000) || (gSaveContext.save.weekEventReg[8] & 1) ||
                    (play->bButtonAmmoPlusOne >= 2)) {
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE,
                                      0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

                    if ((play->sceneId != SCENE_SYATEKI_MIZU) && (play->sceneId != SCENE_SYATEKI_MORI) &&
                        (play->sceneId != SCENE_BOWLING) &&
                        ((gSaveContext.minigameStatus != MINIGAME_STATUS_ACTIVE) ||
                         (gSaveContext.save.entrance != ENTRANCE(ROMANI_RANCH, 0))) &&
                        ((gSaveContext.minigameStatus != MINIGAME_STATUS_ACTIVE) ||
                         !(gSaveContext.eventInf[3] & 0x20)) &&
                        (!(gSaveContext.save.weekEventReg[31] & 0x80) || (play->bButtonAmmoPlusOne != 100))) {
                        Interface_DrawAmmoCount(play, EQUIP_SLOT_B, interfaceCtx->bAlpha);
                    }
                }
            }
        }
    } else if (interfaceCtx->unk_222 != 0) {
        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
        gDPLoadTextureBlock_4b(OVERLAY_DISP++, interfaceCtx->doActionSegment + 0x480, G_IM_FMT_IA, 48, 16, 0,
                               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                               G_TX_NOLOD, G_TX_NOLOD);

        D_801BF9B0 = 1024.0f / (D_801BF9B4[gSaveContext.options.language] / 100.0f);

        gSPTextureRectangle(
            OVERLAY_DISP++, (D_801BF9C4[gSaveContext.options.language] * 4),
            (D_801BF9C8[gSaveContext.options.language] * 4), ((D_801BF9C4[gSaveContext.options.language] + 0x30) << 2),
            ((D_801BF9C8[gSaveContext.options.language] + 0x10) << 2), G_TX_RENDERTILE, 0, 0, D_801BF9B0, D_801BF9B0);
    } else if (interfaceCtx->bButtonDoAction != DO_ACTION_NONE) {
        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
        gDPLoadTextureBlock_4b(OVERLAY_DISP++, interfaceCtx->doActionSegment + 0x600, G_IM_FMT_IA, 48, 16, 0,
                               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                               G_TX_NOLOD, G_TX_NOLOD);

        D_801BF9B0 = 1024.0f / (D_801BF9B4[gSaveContext.options.language] / 100.0f);

        gSPTextureRectangle(
            OVERLAY_DISP++, (D_801BF9C4[gSaveContext.options.language] * 4),
            (D_801BF9C8[gSaveContext.options.language] * 4), ((D_801BF9C4[gSaveContext.options.language] + 0x30) << 2),
            ((D_801BF9C8[gSaveContext.options.language] + 0x10) << 2), G_TX_RENDERTILE, 0, 0, D_801BF9B0, D_801BF9B0);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

/**
 * Draws the icons and ammo for each of the C buttons
 */
void Interface_DrawCButtonIcons(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    OPEN_DISPS(play->state.gfxCtx);

    gDPPipeSync(OVERLAY_DISP++);

    // C-Left Button Icon & Ammo Count
    if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) < 0xF0) {
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->cLeftAlpha);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        Interface_DrawItemIconTexture(play, interfaceCtx->iconItemSegment + 0x1000, EQUIP_SLOT_C_LEFT);
        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        Interface_DrawAmmoCount(play, EQUIP_SLOT_C_LEFT, interfaceCtx->cLeftAlpha);
    }

    gDPPipeSync(OVERLAY_DISP++);

    // C-Down Button Icon & Ammo Count
    if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) < 0xF0) {
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->cDownAlpha);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        Interface_DrawItemIconTexture(play, interfaceCtx->iconItemSegment + 0x2000, EQUIP_SLOT_C_DOWN);
        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        Interface_DrawAmmoCount(play, EQUIP_SLOT_C_DOWN, interfaceCtx->cDownAlpha);
    }

    gDPPipeSync(OVERLAY_DISP++);

    // C-Right Button Icon & Ammo Count
    if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) < 0xF0) {
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->cRightAlpha);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        Interface_DrawItemIconTexture(play, interfaceCtx->iconItemSegment + 0x3000, EQUIP_SLOT_C_RIGHT);
        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        Interface_DrawAmmoCount(play, EQUIP_SLOT_C_RIGHT, interfaceCtx->cRightAlpha);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Interface_DrawAButton(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 aAlpha;

    OPEN_DISPS(play->state.gfxCtx);

    aAlpha = interfaceCtx->aAlpha;

    if (aAlpha > 100) {
        aAlpha = 100;
    }

    func_8012C8D4(play->state.gfxCtx);

    Interface_SetPerspectiveView(play, 25 + R_A_BTN_Y_OFFSET, 70 + R_A_BTN_Y_OFFSET, 192, 237);

    gSPClearGeometryMode(OVERLAY_DISP++, G_CULL_BOTH);
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);

    Matrix_Translate(0.0f, 0.0f, -38.0f, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
    Matrix_RotateXFApply(interfaceCtx->aButtonRoll / 10000.0f);

    // Draw A button Shadow
    gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gDPPipeSync(OVERLAY_DISP++);
    gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[4], 4, 0);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, aAlpha);

    OVERLAY_DISP = Gfx_DrawTexQuadIA8(OVERLAY_DISP, gButtonBackgroundTex, 32, 32, 0);

    // Draw A Button Colored
    gDPPipeSync(OVERLAY_DISP++);
    Interface_SetPerspectiveView(play, 23 + R_A_BTN_Y_OFFSET, 68 + R_A_BTN_Y_OFFSET, 190, 235);
    gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0], 4, 0);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 200, 255, interfaceCtx->aAlpha);
    gSP1Quadrangle(OVERLAY_DISP++, 0, 2, 3, 1, 0);

    // Draw A Button Do-Action
    gDPPipeSync(OVERLAY_DISP++);
    Interface_SetPerspectiveView(play, 23 + R_A_BTN_Y_OFFSET, 68 + R_A_BTN_Y_OFFSET, 190, 235);
    gSPSetGeometryMode(OVERLAY_DISP++, G_CULL_BACK);
    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->aAlpha);
    gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);

    Matrix_Translate(0.0f, 0.0f, D_801BF9CC[gSaveContext.options.language] / 10.0f, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
    Matrix_RotateXFApply(interfaceCtx->aButtonRoll / 10000.0f);
    gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[8], 4, 0);

    // Draw Action Label
    if (((interfaceCtx->aButtonState <= A_BTN_STATE_1) || (interfaceCtx->aButtonState == A_BTN_STATE_3))) {
        OVERLAY_DISP = Gfx_DrawTexQuad4b(OVERLAY_DISP, interfaceCtx->doActionSegment, 3, DO_ACTION_TEX_WIDTH,
                                         DO_ACTION_TEX_HEIGHT, 0);
    } else {
        OVERLAY_DISP = Gfx_DrawTexQuad4b(OVERLAY_DISP, interfaceCtx->doActionSegment + DO_ACTION_TEX_SIZE, 3,
                                         DO_ACTION_TEX_WIDTH, DO_ACTION_TEX_HEIGHT, 0);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static s16 sMagicArrowEffectsR[] = { 255, 100, 255 }; // magicArrowEffectsR
static s16 sMagicArrowEffectsG[] = { 0, 100, 255 };   // magicArrowEffectsG
static s16 sMagicArrowEffectsB[] = { 0, 255, 100 };   // magicArrowEffectsB

void Interface_DrawPauseMenuEquippingIcons(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 temp;

    OPEN_DISPS(play->state.gfxCtx);

    gDPPipeSync(OVERLAY_DISP++);

    // This is needed as `Interface_DrawPauseMenuEquippingIcons` is call immediately
    // after `Interface_DrawAButton`, which sets the view to perspective mode
    Interface_SetOrthoView(interfaceCtx);

    if ((pauseCtx->state == 6) && ((pauseCtx->unk_200 == 3) || (pauseCtx->unk_200 == 0xF))) {
        // Inventory Equip Effects
        gSPSegment(OVERLAY_DISP++, 0x08, pauseCtx->iconItemSegment);
        func_8012C8D4(play->state.gfxCtx);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
        gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        pauseCtx->cursorVtx[16].v.ob[0] = pauseCtx->cursorVtx[18].v.ob[0] = pauseCtx->equipAnimX / 10;
        pauseCtx->cursorVtx[17].v.ob[0] = pauseCtx->cursorVtx[19].v.ob[0] =
            pauseCtx->cursorVtx[16].v.ob[0] + (pauseCtx->equipAnimScale / 10);
        pauseCtx->cursorVtx[16].v.ob[1] = pauseCtx->cursorVtx[17].v.ob[1] = pauseCtx->equipAnimY / 10;
        pauseCtx->cursorVtx[18].v.ob[1] = pauseCtx->cursorVtx[19].v.ob[1] =
            pauseCtx->cursorVtx[16].v.ob[1] - (pauseCtx->equipAnimScale / 10);

        if (pauseCtx->equipTargetItem < 0xB5) {
            // Normal Equip (icon goes from the inventory slot to the C button when equipping it)
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, pauseCtx->equipAnimAlpha);
            gSPVertex(OVERLAY_DISP++, &pauseCtx->cursorVtx[16], 4, 0);
            gDPLoadTextureBlock(OVERLAY_DISP++, gItemIcons[pauseCtx->equipTargetItem], G_IM_FMT_RGBA, G_IM_SIZ_32b, 32,
                                32, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
        } else {
            // Magic Arrow Equip Effect
            temp = pauseCtx->equipTargetItem - 0xB5;
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sMagicArrowEffectsR[temp], sMagicArrowEffectsG[temp],
                            sMagicArrowEffectsB[temp], pauseCtx->equipAnimAlpha);

            if ((pauseCtx->equipAnimAlpha > 0) && (pauseCtx->equipAnimAlpha < 255)) {
                temp = (pauseCtx->equipAnimAlpha / 8) / 2;
                pauseCtx->cursorVtx[16].v.ob[0] = pauseCtx->cursorVtx[18].v.ob[0] =
                    pauseCtx->cursorVtx[16].v.ob[0] - temp;
                pauseCtx->cursorVtx[17].v.ob[0] = pauseCtx->cursorVtx[19].v.ob[0] =
                    pauseCtx->cursorVtx[16].v.ob[0] + temp * 2 + 32;
                pauseCtx->cursorVtx[16].v.ob[1] = pauseCtx->cursorVtx[17].v.ob[1] =
                    pauseCtx->cursorVtx[16].v.ob[1] + temp;
                pauseCtx->cursorVtx[18].v.ob[1] = pauseCtx->cursorVtx[19].v.ob[1] =
                    pauseCtx->cursorVtx[16].v.ob[1] - temp * 2 - 32;
            }

            gSPVertex(OVERLAY_DISP++, &pauseCtx->cursorVtx[16], 4, 0);
            gDPLoadTextureBlock(OVERLAY_DISP++, &D_08095AC0, G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
        }

        gSP1Quadrangle(OVERLAY_DISP++, 0, 2, 3, 1, 0);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

/**
 * Draws either the analog three-day clock or the digital final-hours clock
 */
#ifdef NON_MATCHING
void Interface_DrawClock(PlayState* play) {
    static s16 sThreeDayClockAlpha = 255;
    static s16 D_801BFB30 = 0; // clockAlphaTimer1
    static s16 D_801BFB34 = 0; // clockAlphaTimer2
    static u16 sThreeDayClockHours[] = {
        CLOCK_TIME(0, 0),  CLOCK_TIME(1, 0),  CLOCK_TIME(2, 0),  CLOCK_TIME(3, 0),  CLOCK_TIME(4, 0),
        CLOCK_TIME(5, 0),  CLOCK_TIME(6, 0),  CLOCK_TIME(7, 0),  CLOCK_TIME(8, 0),  CLOCK_TIME(9, 0),
        CLOCK_TIME(10, 0), CLOCK_TIME(11, 0), CLOCK_TIME(12, 0), CLOCK_TIME(13, 0), CLOCK_TIME(14, 0),
        CLOCK_TIME(15, 0), CLOCK_TIME(16, 0), CLOCK_TIME(17, 0), CLOCK_TIME(18, 0), CLOCK_TIME(19, 0),
        CLOCK_TIME(20, 0), CLOCK_TIME(21, 0), CLOCK_TIME(22, 0), CLOCK_TIME(23, 0), CLOCK_TIME(24, 0) - 1,
    };
    static TexturePtr sThreeDayClockHourTextures[] = {
        gThreeDayClockHour12Tex, gThreeDayClockHour1Tex, gThreeDayClockHour2Tex,  gThreeDayClockHour3Tex,
        gThreeDayClockHour4Tex,  gThreeDayClockHour5Tex, gThreeDayClockHour6Tex,  gThreeDayClockHour7Tex,
        gThreeDayClockHour8Tex,  gThreeDayClockHour9Tex, gThreeDayClockHour10Tex, gThreeDayClockHour11Tex,
        gThreeDayClockHour12Tex, gThreeDayClockHour1Tex, gThreeDayClockHour2Tex,  gThreeDayClockHour3Tex,
        gThreeDayClockHour4Tex,  gThreeDayClockHour5Tex, gThreeDayClockHour6Tex,  gThreeDayClockHour7Tex,
        gThreeDayClockHour8Tex,  gThreeDayClockHour9Tex, gThreeDayClockHour10Tex, gThreeDayClockHour11Tex,
    };
    static s16 D_801BFBCC = 0;              // clockInvDiamondPrimRed
    static s16 D_801BFBD0 = 0;              // clockInvDiamondPrimGreen
    static s16 D_801BFBD4 = 255;            // clockInvDiamondPrimBlue
    static s16 D_801BFBD8 = 0;              // clockInvDiamondEnvRed
    static s16 D_801BFBDC = 0;              // clockInvDiamondEnvGreen
    static s16 D_801BFBE0 = 0;              // clockInvDiamondEnvBlue
    static s16 D_801BFBE4 = 15;             // clockInvDiamondTimer
    static s16 D_801BFBE8 = 0;              // clockInvDiamondTargetIndex
    static s16 D_801BFBEC[] = { 100, 0 };   // clockInvDiamondPrimRedTargets
    static s16 D_801BFBF0[] = { 205, 155 }; // clockInvDiamondPrimGreenTargets
    static s16 D_801BFBF4[] = { 255, 255 }; // clockInvDiamondPrimBlueTargets
    static s16 D_801BFBF8[] = { 30, 0 };    // clockInvDiamondEnvRedTargets
    static s16 D_801BFBFC[] = { 30, 0 };    // clockInvDiamondEnvGreenTargets
    static s16 D_801BFC00[] = { 100, 0 };   // clockInvDiamondEnvBlueTargets
    static s16 D_801BFC04[] = { 255, 0 };   // finalHoursClockDigitsRed
    static s16 D_801BFC08[] = { 100, 0 };   // finalHoursClockFrameEnvRedTargets
    static s16 D_801BFC0C[] = { 30, 0 };    // finalHoursClockFrameEnvGreenTargets
    static s16 D_801BFC10[] = { 100, 0 };   // finalHoursClockFrameEnvBlueTargets
    static TexturePtr sFinalHoursDigitTextures[] = {
        gFinalHoursClockDigit0Tex, gFinalHoursClockDigit1Tex, gFinalHoursClockDigit2Tex, gFinalHoursClockDigit3Tex,
        gFinalHoursClockDigit4Tex, gFinalHoursClockDigit5Tex, gFinalHoursClockDigit6Tex, gFinalHoursClockDigit7Tex,
        gFinalHoursClockDigit8Tex, gFinalHoursClockDigit9Tex, gFinalHoursClockColonTex,
    };
    // finalHoursDigitSlotPosXOffset
    static s16 D_801BFC40[] = {
        127, 136, 144, 151, 160, 168, 175, 184,
    };
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    MessageContext* msgCtx = &play->msgCtx;
    s16 sp1E6;
    f32 temp_f14;
    u32 timeBeforeMoonCrash;
    f32 sp1D8;
    f32 timeInMinutes;
    f32 sp1D0;
    f32 sp1CC;
    s32 new_var;
    s16 sp1C6;
    s16 currentHour;
    u16 time;
    s16 temp;
    s16 colorStep;
    s16 finalHoursClockSlots[8]; // sp1AC
    s16 index;

    OPEN_DISPS(play->state.gfxCtx);

    if (R_TIME_SPEED != 0) {
        if ((msgCtx->msgMode == 0) || ((play->actorCtx.flags & ACTORCTX_FLAG_1) && !Play_InCsMode(play)) ||
            (msgCtx->msgMode == 0) || ((msgCtx->currentTextId >= 0x100) && (msgCtx->currentTextId <= 0x200)) ||
            (gSaveContext.gameMode == 3)) {
            if (!FrameAdvance_IsEnabled(&play->state)) {
                if (func_800FE4A8() == 0) {
                    if (((void)0, gSaveContext.save.day) < 4) {
                        /**
                         * Changes Clock's transparancy depending if Player is moving or not and possibly other things
                         */
                        if (((void)0, gSaveContext.hudVisibility) == HUD_VISIBILITY_ALL) {
                            if (func_801234D4(play) != 0) {
                                sThreeDayClockAlpha = 80;
                                D_801BFB30 = 5;
                                D_801BFB34 = 20;
                            } else if (D_801BFB34 != 0) {
                                D_801BFB34--;
                            } else if (D_801BFB30 != 0) {
                                colorStep = ABS_ALT(sThreeDayClockAlpha - 255) / D_801BFB30;
                                sThreeDayClockAlpha += colorStep;

                                if (sThreeDayClockAlpha >= 255) {
                                    sThreeDayClockAlpha = 255;
                                    D_801BFB30 = 0;
                                }
                            } else {
                                if (play->actorCtx.flags & ACTORCTX_FLAG_1) {
                                    sThreeDayClockAlpha = 255;
                                } else {
                                    sThreeDayClockAlpha = interfaceCtx->bAlpha;
                                }
                                D_801BFB34 = 0;
                                D_801BFB30 = 0;
                            }
                        } else {
                            if (play->actorCtx.flags & ACTORCTX_FLAG_1) {
                                sThreeDayClockAlpha = 255;
                            } else {
                                sThreeDayClockAlpha = interfaceCtx->bAlpha;
                            }
                            D_801BFB34 = 0;
                            D_801BFB30 = 0;
                        }

                        if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {

                            func_8012C654(play->state.gfxCtx);

                            /**
                             * Draw Clock's Hour Lines
                             */
                            gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                            gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 130, 130, 130, sThreeDayClockAlpha);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                                              PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

                            OVERLAY_DISP = Gfx_DrawTexRect4b(OVERLAY_DISP, gThreeDayClockHourLinesTex, 4, 64, 35, 96,
                                                             180, 128, 35, 1, 6, 0, 1 << 10, 1 << 10);

                            /**
                             * Draw Clock's Border
                             */
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, sThreeDayClockAlpha);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                                              PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

                            //! @bug A texture height of 50 is given below. The texture is only 48 units height
                            //!      resulting in this reading into the next texture. This results in a white
                            //!      dot in the bottom center of the clock. For the three-day clock, this is
                            //!      covered by the diamond. However, it can be seen by the final-hours clock.
                            OVERLAY_DISP = Gfx_DrawTexRect4b(OVERLAY_DISP, gThreeDayClockBorderTex, 4, 64, 50, 96, 168,
                                                             128, 50, 1, 6, 0, 1 << 10, 1 << 10);

                            if (((CURRENT_DAY >= 4) ||
                                 ((CURRENT_DAY == 3) && (((void)0, gSaveContext.save.time) >= 5) &&
                                  (((void)0, gSaveContext.save.time) < CLOCK_TIME(6, 0))))) {
                                func_8012C8D4(play->state.gfxCtx);
                                gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            } else {
                                /**
                                 * Draw Three-Day Clock's Diamond
                                 */
                                gDPPipeSync(OVERLAY_DISP++);

                                // Time is slowed down to half speed with inverted song of time
                                if (gSaveContext.save.daySpeed == -2) {
                                    // Clock diamond is blue and flashes white
                                    colorStep = ABS_ALT(D_801BFBCC - D_801BFBEC[D_801BFBE8]) / D_801BFBE4;
                                    if (D_801BFBCC >= D_801BFBEC[D_801BFBE8]) {
                                        D_801BFBCC -= colorStep;
                                    } else {
                                        D_801BFBCC += colorStep;
                                    }

                                    colorStep = ABS_ALT(D_801BFBD0 - D_801BFBF0[D_801BFBE8]) / D_801BFBE4;
                                    if (D_801BFBD0 >= D_801BFBF0[D_801BFBE8]) {
                                        D_801BFBD0 -= colorStep;
                                    } else {
                                        D_801BFBD0 += colorStep;
                                    }

                                    colorStep = ABS_ALT(D_801BFBD4 - D_801BFBF4[D_801BFBE8]) / D_801BFBE4;
                                    if (D_801BFBD4 >= D_801BFBF4[D_801BFBE8]) {
                                        D_801BFBD4 -= colorStep;
                                    } else {
                                        D_801BFBD4 += colorStep;
                                    }

                                    colorStep = ABS_ALT(D_801BFBD8 - D_801BFBF8[D_801BFBE8]) / D_801BFBE4;
                                    if (D_801BFBD8 >= D_801BFBF8[D_801BFBE8]) {
                                        D_801BFBD8 -= colorStep;
                                    } else {
                                        D_801BFBD8 += colorStep;
                                    }

                                    colorStep = ABS_ALT(D_801BFBDC - D_801BFBFC[D_801BFBE8]) / D_801BFBE4;
                                    if (D_801BFBDC >= D_801BFBFC[D_801BFBE8]) {
                                        D_801BFBDC -= colorStep;
                                    } else {
                                        D_801BFBDC += colorStep;
                                    }

                                    colorStep = ABS_ALT(D_801BFBE0 - D_801BFC00[D_801BFBE8]) / D_801BFBE4;
                                    if (D_801BFBE0 >= D_801BFC00[D_801BFBE8]) {
                                        D_801BFBE0 -= colorStep;
                                    } else {
                                        D_801BFBE0 += colorStep;
                                    }

                                    D_801BFBE4--;

                                    if (D_801BFBE4 == 0) {
                                        D_801BFBCC = D_801BFBEC[D_801BFBE8];
                                        D_801BFBD0 = D_801BFBF0[D_801BFBE8];
                                        D_801BFBD4 = D_801BFBF4[D_801BFBE8];
                                        D_801BFBD8 = D_801BFBF8[D_801BFBE8];
                                        D_801BFBDC = D_801BFBFC[D_801BFBE8];
                                        D_801BFBE0 = D_801BFC00[D_801BFBE8];
                                        D_801BFBE4 = 15;
                                        D_801BFBE8 ^= 1;
                                    }

                                    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT,
                                                      TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE, ENVIRONMENT, TEXEL0,
                                                      ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, D_801BFBCC, D_801BFBD0, 255,
                                                    sThreeDayClockAlpha);
                                    gDPSetEnvColor(OVERLAY_DISP++, D_801BFBD8, D_801BFBDC, D_801BFBE0, 0);
                                } else {
                                    // Clock diamond is green for regular daySpeed
                                    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 170, 100, sThreeDayClockAlpha);
                                }

                                OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, gThreeDayClockDiamondTex, 40, 32, 140,
                                                                  190, 40, 32, 1 << 10, 1 << 10);

                                /**
                                 * Draw Three-Day Clock's Day-Number over Diamond
                                 */
                                gDPPipeSync(OVERLAY_DISP++);
                                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, sThreeDayClockAlpha);

                                OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, interfaceCtx->doActionSegment + 0x780,
                                                                  48, 27, 137, 192, 48, 27, 1 << 10, 1 << 10);

                                /**
                                 * Draw Three-Day Clock's Star (for the Minute Tracker)
                                 */
                                gDPPipeSync(OVERLAY_DISP++);

                                if (D_801BF974 != 0) {
                                    D_801BF980 += 0.02f;
                                    D_801BF97C += 11;
                                } else {
                                    D_801BF980 -= 0.02f;
                                    D_801BF97C -= 11;
                                }

                                D_801BF978--;
                                if (D_801BF978 == 0) {
                                    D_801BF978 = 10;
                                    D_801BF974 ^= 1;
                                }

                                sp1D0 = gSaveContext.save.time * 1.3183594f;
                                sp1D0 -= ((s16)(sp1D0 / 3600.0f)) * 3600.0f;

                                func_8012C8D4(play->state.gfxCtx);

                                gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                                if (sThreeDayClockAlpha != 255) {
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 110, sThreeDayClockAlpha);
                                } else {
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 110, D_801BF97C);
                                }

                                gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                                gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                                gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

                                Matrix_Translate(0.0f, -86.0f, 0.0f, MTXMODE_NEW);
                                Matrix_Scale(1.0f, 1.0f, D_801BF980, MTXMODE_APPLY);
                                Matrix_RotateZF(-(sp1D0 * 0.0175f) / 10.0f, MTXMODE_APPLY);

                                gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx),
                                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                                gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[12], 4, 0);
                                gDPLoadTextureBlock_4b(OVERLAY_DISP++, gThreeDayClockStarMinuteTex, G_IM_FMT_I, 16, 16,
                                                       0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                                       G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                                gSP1Quadrangle(OVERLAY_DISP++, 0, 2, 3, 1, 0);
                            }

                            /**
                             * Cuts off Three-Day Clock's Sun and Moon when they dip below the clock
                             */
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetScissorFrac(OVERLAY_DISP++, G_SC_NON_INTERLACE, 400, 620, 880, XREG(44) * 4.0f);

                            // determines the current hour
                            for (sp1C6 = 0; sp1C6 <= 24; sp1C6++) {
                                if (((void)0, gSaveContext.save.time) < sThreeDayClockHours[sp1C6 + 1]) {
                                    break;
                                }
                            }

                            /**
                             * Draw Three-Day Clock's Sun (for the Day-Time Hours Tracker)
                             */
                            time = gSaveContext.save.time;
                            sp1D8 = Math_SinS(time) * -40.0f;
                            temp_f14 = Math_CosS(time) * -34.0f;

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 100, 110, sThreeDayClockAlpha);

                            Matrix_Translate(sp1D8, temp_f14, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx),
                                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[16], 4, 0);

                            OVERLAY_DISP = Gfx_DrawTexQuadIA8(OVERLAY_DISP, gThreeDayClockSunHourTex, 24, 24, 0);

                            /**
                             * Draw Three-Day Clock's Moon (for the Night-Time Hours Tracker)
                             */
                            sp1D8 = Math_SinS(time) * 40.0f;
                            temp_f14 = Math_CosS(time) * 34.0f;

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 55, sThreeDayClockAlpha);

                            Matrix_Translate(sp1D8, temp_f14, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx),
                                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[20], 4, 0);

                            OVERLAY_DISP = Gfx_DrawTexQuadIA8(OVERLAY_DISP, gThreeDayClockMoonHourTex, 24, 24, 0);

                            /**
                             * Cuts off Three-Day Clock's Hour Digits when they dip below the clock
                             */
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetScissorFrac(OVERLAY_DISP++, G_SC_NON_INTERLACE, 400, 620, 880, XREG(45) * 4.0f);

                            /**
                             * Draws Three-Day Clock's Hour Digit Above the Sun
                             */
                            sp1CC = gSaveContext.save.time * 0.000096131f;

                            // Rotates Three-Day Clock's Hour Digit To Above the Sun
                            Matrix_Translate(0.0f, XREG(43) / 10.0f, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            Matrix_RotateZF(-(sp1CC - 3.15f), MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx),
                                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                            // Draws Three-Day Clock's Hour Digit Above the Sun
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                                              PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, sThreeDayClockAlpha);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[24], 8, 0);

                            OVERLAY_DISP =
                                Gfx_DrawTexQuad4b(OVERLAY_DISP, sThreeDayClockHourTextures[sp1C6], 4, 16, 11, 0);

                            // Colours the Three-Day Clocks's Hour Digit Above the Sun
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, sThreeDayClockAlpha);
                            gSP1Quadrangle(OVERLAY_DISP++, 4, 6, 7, 5, 0);

                            /**
                             * Draws Three-Day Clock's Hour Digit Above the Moon
                             */
                            // Rotates Three-Day Clock's Hour Digit To Above the Moon
                            Matrix_Translate(0.0f, XREG(43) / 10.0f, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            Matrix_RotateZF(-sp1CC, MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx),
                                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                            // Draws Three-Day Clock's Hour Digit Above the Moon
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                                              PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, sThreeDayClockAlpha);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[32], 8, 0);

                            OVERLAY_DISP =
                                Gfx_DrawTexQuad4b(OVERLAY_DISP, sThreeDayClockHourTextures[sp1C6], 4, 16, 11, 0);

                            // Colours the Three-Day Clocks's Hour Digit Above the Moon
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, sThreeDayClockAlpha);
                            gSP1Quadrangle(OVERLAY_DISP++, 4, 6, 7, 5, 0);

                            // Unknown
                            gSPDisplayList(OVERLAY_DISP++, D_0E0001C8);

                            // Final Hours
                            if ((CURRENT_DAY >= 4) || ((CURRENT_DAY == 3) && (((void)0, gSaveContext.save.time) >= 5) &&
                                                       (((void)0, gSaveContext.save.time) < CLOCK_TIME(6, 0)))) {
                                if (((void)0, gSaveContext.save.time) >= CLOCK_TIME(5, 0)) {
                                    // The Final Hours clock will flash red

                                    colorStep = ABS_ALT(sFinalHoursClockDigitsRed -
                                                        D_801BFC04[sFinalHoursClockColorTargetIndex]) /
                                                sFinalHoursClockColorTimer;
                                    if (sFinalHoursClockDigitsRed >= D_801BFC04[sFinalHoursClockColorTargetIndex]) {
                                        sFinalHoursClockDigitsRed -= colorStep;
                                    } else {
                                        sFinalHoursClockDigitsRed += colorStep;
                                    }

                                    colorStep = ABS_ALT(sFinalHoursClockFrameEnvRed -
                                                        D_801BFC08[sFinalHoursClockColorTargetIndex]) /
                                                sFinalHoursClockColorTimer;
                                    if (sFinalHoursClockFrameEnvRed >= D_801BFC08[sFinalHoursClockColorTargetIndex]) {
                                        sFinalHoursClockFrameEnvRed -= colorStep;
                                    } else {
                                        sFinalHoursClockFrameEnvRed += colorStep;
                                    }

                                    colorStep = ABS_ALT(sFinalHoursClockFrameEnvGreen -
                                                        D_801BFC0C[sFinalHoursClockColorTargetIndex]) /
                                                sFinalHoursClockColorTimer;
                                    if (sFinalHoursClockFrameEnvGreen >= D_801BFC0C[sFinalHoursClockColorTargetIndex]) {
                                        sFinalHoursClockFrameEnvGreen -= colorStep;
                                    } else {
                                        sFinalHoursClockFrameEnvGreen += colorStep;
                                    }

                                    colorStep = ABS_ALT(sFinalHoursClockFrameEnvBlue -
                                                        D_801BFC10[sFinalHoursClockColorTargetIndex]) /
                                                sFinalHoursClockColorTimer;
                                    if (sFinalHoursClockFrameEnvBlue >= D_801BFC10[sFinalHoursClockColorTargetIndex]) {
                                        sFinalHoursClockFrameEnvBlue -= colorStep;
                                    } else {
                                        sFinalHoursClockFrameEnvBlue += colorStep;
                                    }

                                    sFinalHoursClockColorTimer--;

                                    if (sFinalHoursClockColorTimer == 0) {
                                        sFinalHoursClockDigitsRed = D_801BFC04[sFinalHoursClockColorTargetIndex];
                                        sFinalHoursClockFrameEnvRed = D_801BFC08[sFinalHoursClockColorTargetIndex];
                                        sFinalHoursClockFrameEnvGreen = D_801BFC0C[sFinalHoursClockColorTargetIndex];
                                        sFinalHoursClockFrameEnvBlue = D_801BFC10[sFinalHoursClockColorTargetIndex];
                                        sFinalHoursClockColorTimer = 6;
                                        sFinalHoursClockColorTargetIndex ^= 1;
                                    }
                                }

                                sp1E6 = sThreeDayClockAlpha;
                                if (sp1E6 != 0) {
                                    sp1E6 = 255;
                                }

                                func_8012C654(play->state.gfxCtx);

                                /**
                                 * Draws Final-Hours Clock's Frame
                                 */
                                gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                                gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                                gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
                                gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0,
                                                  0, PRIMITIVE, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0,
                                                  0, PRIMITIVE, 0);
                                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 195, sp1E6);
                                gDPSetEnvColor(OVERLAY_DISP++, sFinalHoursClockFrameEnvRed,
                                               sFinalHoursClockFrameEnvGreen, sFinalHoursClockFrameEnvBlue, 0);

                                OVERLAY_DISP = Gfx_DrawTexRect4b(OVERLAY_DISP, gFinalHoursClockFrameTex, 3, 80, 13, 119,
                                                                 202, 80, 13, 0, 0, 0, 1 << 10, 1 << 10);

                                finalHoursClockSlots[0] = 0;

                                timeBeforeMoonCrash = (4 - CURRENT_DAY) * DAY_LENGTH -
                                                      (u16)(((void)0, gSaveContext.save.time) - CLOCK_TIME(6, 0));

                                timeInMinutes = TIME_TO_MINUTES_F(timeBeforeMoonCrash);

                                finalHoursClockSlots[1] = timeInMinutes / 60.0f;
                                finalHoursClockSlots[2] = timeInMinutes / 60.0f;

                                temp = (s32)timeInMinutes % 60;

                                // digits for hours
                                while (finalHoursClockSlots[1] >= 10) {
                                    finalHoursClockSlots[0]++;
                                    finalHoursClockSlots[1] -= 10;
                                }

                                finalHoursClockSlots[3] = 0;
                                finalHoursClockSlots[4] = temp;

                                // digits for minutes
                                while (finalHoursClockSlots[4] >= 10) {
                                    finalHoursClockSlots[3]++;
                                    finalHoursClockSlots[4] -= 10;
                                }

                                finalHoursClockSlots[6] = 0;
                                finalHoursClockSlots[7] =
                                    timeBeforeMoonCrash -
                                    (u32)((finalHoursClockSlots[2] * 2730.6667f) + // 2^13 / 3 (ft1 - e6c4)
                                          (((void)0, temp) * 45.511112f));         // 2^13 / 3 / 60 (ft0 - e758)

                                // digits for seconds
                                while (finalHoursClockSlots[7] >= 10) {
                                    finalHoursClockSlots[6]++;
                                    finalHoursClockSlots[7] -= 10;
                                }

                                // Colon separating hours from minutes and minutes from seconds
                                finalHoursClockSlots[2] = finalHoursClockSlots[5] = 10;

                                /**
                                 * Draws Final-Hours Clock's Digits
                                 */
                                gDPPipeSync(OVERLAY_DISP++);
                                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sFinalHoursClockDigitsRed, 0, 0, sp1E6);
                                gDPSetEnvColor(OVERLAY_DISP++, sFinalHoursClockDigitsRed, 0, 0, 0);

                                for (sp1C6 = 0; sp1C6 < 8; sp1C6++) {
                                    index = D_801BFC40[sp1C6];

                                    OVERLAY_DISP = Gfx_DrawTexRectI8(
                                        OVERLAY_DISP, sFinalHoursDigitTextures[finalHoursClockSlots[sp1C6]], 8, 8,
                                        index, 205, 8, 8, 1 << 10, 1 << 10);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    CLOSE_DISPS(play->state.gfxCtx);
}
#else
s16 sThreeDayClockAlpha = 255;
s16 D_801BFB30 = 0;
s16 D_801BFB34 = 0;
u16 sThreeDayClockHours[] = {
    CLOCK_TIME(0, 0),  CLOCK_TIME(1, 0),  CLOCK_TIME(2, 0),  CLOCK_TIME(3, 0),  CLOCK_TIME(4, 0),
    CLOCK_TIME(5, 0),  CLOCK_TIME(6, 0),  CLOCK_TIME(7, 0),  CLOCK_TIME(8, 0),  CLOCK_TIME(9, 0),
    CLOCK_TIME(10, 0), CLOCK_TIME(11, 0), CLOCK_TIME(12, 0), CLOCK_TIME(13, 0), CLOCK_TIME(14, 0),
    CLOCK_TIME(15, 0), CLOCK_TIME(16, 0), CLOCK_TIME(17, 0), CLOCK_TIME(18, 0), CLOCK_TIME(19, 0),
    CLOCK_TIME(20, 0), CLOCK_TIME(21, 0), CLOCK_TIME(22, 0), CLOCK_TIME(23, 0), CLOCK_TIME(24, 0) - 1,
};
TexturePtr sThreeDayClockHourTextures[] = {
    gThreeDayClockHour12Tex, gThreeDayClockHour1Tex, gThreeDayClockHour2Tex,  gThreeDayClockHour3Tex,
    gThreeDayClockHour4Tex,  gThreeDayClockHour5Tex, gThreeDayClockHour6Tex,  gThreeDayClockHour7Tex,
    gThreeDayClockHour8Tex,  gThreeDayClockHour9Tex, gThreeDayClockHour10Tex, gThreeDayClockHour11Tex,
    gThreeDayClockHour12Tex, gThreeDayClockHour1Tex, gThreeDayClockHour2Tex,  gThreeDayClockHour3Tex,
    gThreeDayClockHour4Tex,  gThreeDayClockHour5Tex, gThreeDayClockHour6Tex,  gThreeDayClockHour7Tex,
    gThreeDayClockHour8Tex,  gThreeDayClockHour9Tex, gThreeDayClockHour10Tex, gThreeDayClockHour11Tex,
};
s16 D_801BFBCC = 0;   // color R
s16 D_801BFBD0 = 155; // color G
s16 D_801BFBD4 = 255;
s16 D_801BFBD8 = 0;
s16 D_801BFBDC = 0;
s16 D_801BFBE0 = 0;
s16 D_801BFBE4 = 0xF;
u32 D_801BFBE8 = 0;
s16 D_801BFBEC[] = { 100, 0 };
s16 D_801BFBF0[] = { 205, 155 };
s16 D_801BFBF4[] = { 255, 255 };
s16 D_801BFBF8[] = { 30, 0 };
s16 D_801BFBFC[] = { 30, 0 };
s16 D_801BFC00[] = { 100, 0 };
s16 D_801BFC04[] = { 255, 0 };
s16 D_801BFC08[] = { 100, 0 };
s16 D_801BFC0C[] = { 30, 0 };
s16 D_801BFC10[] = { 100, 0 };
TexturePtr sFinalHoursDigitTextures[] = {
    gFinalHoursClockDigit0Tex, gFinalHoursClockDigit1Tex, gFinalHoursClockDigit2Tex, gFinalHoursClockDigit3Tex,
    gFinalHoursClockDigit4Tex, gFinalHoursClockDigit5Tex, gFinalHoursClockDigit6Tex, gFinalHoursClockDigit7Tex,
    gFinalHoursClockDigit8Tex, gFinalHoursClockDigit9Tex, gFinalHoursClockColonTex,
};
s16 D_801BFC40[] = {
    0x007F, 0x0088, 0x0090, 0x0097, 0x00A0, 0x00A8, 0x00AF, 0x00B8,
};
void Interface_DrawClock(PlayState* play);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_DrawClock.s")
#endif

void Interface_SetMinigamePerfect(PlayState* play, s16 minigamePerfectType) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 i;

    interfaceCtx->isMinigamePerfect = true;
    interfaceCtx->minigamePerfectType = minigamePerfectType;

    interfaceCtx->minigamePerfectPrimColor[0] = 255;
    interfaceCtx->minigamePerfectPrimColor[1] = 165;
    interfaceCtx->minigamePerfectPrimColor[2] = 55;
    interfaceCtx->minigamePerfectPrimColor[3] = 255;
    interfaceCtx->minigamePerfectColorTimer = 20;

    interfaceCtx->minigamePerfectColorTargetIndex = 0;
    interfaceCtx->minigamePerfectLetterCount = 1;
    interfaceCtx->minigamePerfectTimer = 0;

    for (i = 0; i < 8; i++) {
        interfaceCtx->minigamePerfectLetterDirection[i] = 0;
        interfaceCtx->minigamePerfectState[i] = interfaceCtx->minigamePerfectVtxOffset[i] = 0;
        if (interfaceCtx->minigamePerfectType == MINIGAME_PERFECT_TYPE_1) {
            interfaceCtx->minigamePerfectLetterPosX[i] = 140.0f;
            interfaceCtx->minigamePerfectLetterPosY[i] = 100.0f;
        } else {
            interfaceCtx->minigamePerfectLetterPosY[i] = 200.0f;
            interfaceCtx->minigamePerfectLetterPosX[i] = 200.0f;
        }
    }
    interfaceCtx->minigamePerfectState[0] = MINIGAME_PERFECT_STATE_INIT;
}

u16 D_801BFC50[] = {
    0xC000, 0xE000, 0x0000, 0x2000, 0xA000, 0x8000, 0x6000, 0x4000,
};
s16 sMinigamePerfectType1PrimColorTargets[2][3] = {
    { 255, 255, 255 },
    { 255, 165, 55 },
};

void Interface_UpdateMinigamePerfectType1(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 i;
    s16 count;
    s16 colorStepR;
    s16 colorStepG;
    s16 colorStepB;

    for (count = 0, i = 0; i < interfaceCtx->minigamePerfectLetterCount; i++, count += 4) {
        if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_INIT) {
            // Swirl
            interfaceCtx->minigamePerfectLetterDirection[i] = D_801BFC50[i] + 0xA000;
            interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_SWIRL_IN;
        } else if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_SWIRL_IN) {
            interfaceCtx->minigamePerfectLetterDirection[i] -= 0x800;
            if (interfaceCtx->minigamePerfectLetterDirection[i] == D_801BFC50[i]) {
                interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_STATIONARY;
            }
        } else if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_SWIRL_OUT) {
            interfaceCtx->minigamePerfectLetterDirection[i] -= 0x800;
            if (interfaceCtx->minigamePerfectLetterDirection[i] == (u16)(D_801BFC50[i] - 0x8000)) {
                interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_OFF;
            }
        }
    }

    // Initialize the next letter in the list
    // TODO: if `minigamePerfectLetterCount == 8`, then isn't minigamePerfectState[] accessed OoB?
    if ((interfaceCtx->minigamePerfectState[interfaceCtx->minigamePerfectLetterCount] == MINIGAME_PERFECT_STATE_OFF) &&
        (interfaceCtx->minigamePerfectLetterCount < 8)) {
        interfaceCtx->minigamePerfectState[interfaceCtx->minigamePerfectLetterCount] = MINIGAME_PERFECT_STATE_INIT;
        interfaceCtx->minigamePerfectLetterPosX[interfaceCtx->minigamePerfectLetterCount] = 140.0f;
        interfaceCtx->minigamePerfectLetterPosY[interfaceCtx->minigamePerfectLetterCount] = 100.0f;
        interfaceCtx->minigamePerfectLetterDirection[interfaceCtx->minigamePerfectLetterCount] =
            D_801BFC50[interfaceCtx->minigamePerfectLetterCount] + 0xA000;
        interfaceCtx->minigamePerfectLetterCount++;
    }

    if ((interfaceCtx->minigamePerfectLetterCount == 8) &&
        (interfaceCtx->minigamePerfectState[7] == MINIGAME_PERFECT_STATE_STATIONARY)) {

        colorStepR = ABS_ALT(interfaceCtx->minigamePerfectPrimColor[0] -
                             sMinigamePerfectType1PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][0]) /
                     interfaceCtx->minigamePerfectColorTimer;
        colorStepG = ABS_ALT(interfaceCtx->minigamePerfectPrimColor[1] -
                             sMinigamePerfectType1PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][1]) /
                     interfaceCtx->minigamePerfectColorTimer;
        colorStepB = ABS_ALT(interfaceCtx->minigamePerfectPrimColor[2] -
                             sMinigamePerfectType1PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][2]) /
                     interfaceCtx->minigamePerfectColorTimer;

        if (interfaceCtx->minigamePerfectPrimColor[0] >=
            sMinigamePerfectType1PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][0]) {
            interfaceCtx->minigamePerfectPrimColor[0] -= colorStepR;
        } else {
            interfaceCtx->minigamePerfectPrimColor[0] += colorStepR;
        }

        if (interfaceCtx->minigamePerfectPrimColor[1] >=
            sMinigamePerfectType1PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][1]) {
            interfaceCtx->minigamePerfectPrimColor[1] -= colorStepG;
        } else {
            interfaceCtx->minigamePerfectPrimColor[1] += colorStepG;
        }

        if (interfaceCtx->minigamePerfectPrimColor[2] >=
            sMinigamePerfectType1PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][2]) {
            interfaceCtx->minigamePerfectPrimColor[2] -= colorStepB;
        } else {
            interfaceCtx->minigamePerfectPrimColor[2] += colorStepB;
        }

        interfaceCtx->minigamePerfectColorTimer--;

        if (interfaceCtx->minigamePerfectColorTimer == 0) {
            interfaceCtx->minigamePerfectColorTimer = 20;
            interfaceCtx->minigamePerfectColorTargetIndex ^= 1;
            interfaceCtx->minigamePerfectTimer++;

            if (interfaceCtx->minigamePerfectTimer == 6) {
                for (i = 0; i < 8; i++) {
                    interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_SWIRL_OUT;
                }
            }
        }
    }

    for (count = 0, i = 0; i < 8; i++) {
        if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_OFF) {
            count++;
        }
    }

    if (count == 8) {
        interfaceCtx->isMinigamePerfect = false;
    }
}

s16 D_801BFC6C[] = {
    78, 54, 29, 5, 0xFFEE, 0xFFD6, 0xFFBD, 0xFFAB,
};
s16 D_801BFC7C[] = {
    180, 180, 180, 180, 0xFF4C, 0xFF4C, 0xFF4C, 0xFF4C,
};
s16 sMinigamePerfectType2PrimColorTargets[2][3] = {
    { 255, 255, 255 },
    { 255, 165, 55 },
};

void Interface_UpdateMinigamePerfectType2(PlayState* play) {
    s16 i;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 stepVar1;
    s16 stepVar2;
    s16 stepVar3;
    s16 new_var;
    s16 j = 0; // unused incrementer

    for (i = 0; i < interfaceCtx->minigamePerfectLetterCount; i++, j += 4) {
        if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_INIT) {
            interfaceCtx->minigamePerfectLetterDirection[i] = i << 0xD;
            interfaceCtx->minigamePerfectLetterPosX[i] = 200.0f;
            interfaceCtx->minigamePerfectLetterPosY[i] = 200.0f;
            interfaceCtx->minigamePerfectVtxOffset[i] = 0;
            interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_SWIRL_IN;
        } else if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_SWIRL_IN) {
            interfaceCtx->minigamePerfectLetterDirection[i] -= 0x800;
            interfaceCtx->minigamePerfectLetterPosX[i] -= 8.0f;
            interfaceCtx->minigamePerfectLetterPosY[i] -= 8.0f;
            if (interfaceCtx->minigamePerfectLetterPosX[i] <= 0.0f) {
                interfaceCtx->minigamePerfectLetterPosY[i] = 0.0f;
                interfaceCtx->minigamePerfectLetterPosX[i] = 0.0f;
                interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_STATIONARY;
                if (i == 7) {
                    interfaceCtx->minigamePerfectColorTimer = 5;
                    interfaceCtx->minigamePerfectState[7] = MINIGAME_PERFECT_STATE_SWIRL_OUT;
                    interfaceCtx->minigamePerfectState[6] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[5] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[4] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[3] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[2] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[1] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[0] = interfaceCtx->minigamePerfectState[7];
                }
            }
        } else if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_SWIRL_OUT) {

            stepVar1 = ABS_ALT(interfaceCtx->minigamePerfectVtxOffset[i] - D_801BFC6C[i]) /
                       interfaceCtx->minigamePerfectColorTimer;
            if (interfaceCtx->minigamePerfectVtxOffset[i] >= D_801BFC6C[i]) {
                interfaceCtx->minigamePerfectVtxOffset[i] -= stepVar1;
            } else {
                interfaceCtx->minigamePerfectVtxOffset[i] += stepVar1;
            }
        } else if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_6) {

            stepVar1 = ABS_ALT(interfaceCtx->minigamePerfectVtxOffset[i] - D_801BFC7C[i]) /
                       interfaceCtx->minigamePerfectColorTimer;
            if (interfaceCtx->minigamePerfectVtxOffset[i] >= D_801BFC7C[i]) {
                interfaceCtx->minigamePerfectVtxOffset[i] -= stepVar1;
            } else {
                interfaceCtx->minigamePerfectVtxOffset[i] += stepVar1;
            }
        }
    }

    if ((interfaceCtx->minigamePerfectState[0] == MINIGAME_PERFECT_STATE_SWIRL_OUT) ||
        (interfaceCtx->minigamePerfectState[0] == MINIGAME_PERFECT_STATE_6)) {
        if (interfaceCtx->minigamePerfectState[0] == MINIGAME_PERFECT_STATE_6) {
            new_var = interfaceCtx->minigamePerfectPrimColor[3] / interfaceCtx->minigamePerfectColorTimer;
            interfaceCtx->minigamePerfectPrimColor[3] -= new_var;
        }
        interfaceCtx->minigamePerfectColorTimer--;
        if (interfaceCtx->minigamePerfectColorTimer == 0) {

            if (interfaceCtx->minigamePerfectState[0] == MINIGAME_PERFECT_STATE_SWIRL_OUT) {
                for (i = 0; i < 8; i++) {
                    interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_5;
                }
                interfaceCtx->minigamePerfectColorTimer = 20;
            } else {
                for (i = 0; i < 8; i++) {
                    interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_OFF;
                }
                interfaceCtx->isMinigamePerfect = false;
            }
        }
    }

    if (interfaceCtx->minigamePerfectState[interfaceCtx->minigamePerfectLetterCount] == MINIGAME_PERFECT_STATE_OFF) {
        if (interfaceCtx->minigamePerfectLetterCount < 8) {
            interfaceCtx->minigamePerfectState[interfaceCtx->minigamePerfectLetterCount] = MINIGAME_PERFECT_STATE_INIT;
            interfaceCtx->minigamePerfectLetterCount++;
        }
    }
    if (interfaceCtx->minigamePerfectLetterCount == 8) {
        if (interfaceCtx->minigamePerfectState[7] == MINIGAME_PERFECT_STATE_5) {

            stepVar1 =
                ABS_ALT(interfaceCtx->minigamePerfectPrimColor[0] -
                        sMinigamePerfectType2PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][0]) /
                interfaceCtx->minigamePerfectColorTimer;
            stepVar2 =
                ABS_ALT(interfaceCtx->minigamePerfectPrimColor[1] -
                        sMinigamePerfectType2PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][1]) /
                interfaceCtx->minigamePerfectColorTimer;
            stepVar3 =
                ABS_ALT(interfaceCtx->minigamePerfectPrimColor[2] -
                        sMinigamePerfectType2PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][2]) /
                interfaceCtx->minigamePerfectColorTimer;

            if (interfaceCtx->minigamePerfectPrimColor[0] >=
                sMinigamePerfectType2PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][0]) {
                interfaceCtx->minigamePerfectPrimColor[0] -= stepVar1;
            } else {
                interfaceCtx->minigamePerfectPrimColor[0] += stepVar1;
            }

            if (interfaceCtx->minigamePerfectPrimColor[1] >=
                sMinigamePerfectType2PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][1]) {
                interfaceCtx->minigamePerfectPrimColor[1] -= stepVar2;
            } else {
                interfaceCtx->minigamePerfectPrimColor[1] += stepVar2;
            }

            if (interfaceCtx->minigamePerfectPrimColor[2] >=
                sMinigamePerfectType2PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][2]) {
                interfaceCtx->minigamePerfectPrimColor[2] -= stepVar3;
            } else {
                interfaceCtx->minigamePerfectPrimColor[2] += stepVar3;
            }

            interfaceCtx->minigamePerfectColorTimer--;
            if (interfaceCtx->minigamePerfectColorTimer == 0) {
                interfaceCtx->minigamePerfectColorTimer = 20;
                interfaceCtx->minigamePerfectColorTargetIndex ^= 1;
                interfaceCtx->minigamePerfectTimer++;
                if (interfaceCtx->minigamePerfectTimer == 6) {
                    for (i = 0; i < 8; i++) {
                        interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_6;
                    }
                    interfaceCtx->minigamePerfectColorTimer = 5;
                }
            }
        }
    }
}

s16 D_801BFC98[] = {
    78, 54, 29, 5, 0xFFEE, 0xFFD6, 0xFFBD, 0xFFAB,
};
u16 D_801BFCA8[] = {
    0xC000, 0xE000, 0x0000, 0x2000, 0xA000, 0x8000, 0x6000, 0x4000,
};
s16 sMinigamePerfectType3PrimColorTargets[2][3] = {
    { 255, 255, 255 },
    { 255, 165, 55 },
};

void Interface_UpdateMinigamePerfectType3(PlayState* play) {
    s16 i;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 stepVar1;
    s16 stepVar2;
    s16 stepVar3;
    s16 stepVar4;
    s16 j = 0;

    for (i = 0; i < interfaceCtx->minigamePerfectLetterCount; i++, j += 4) {
        if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_INIT) {
            interfaceCtx->minigamePerfectLetterDirection[i] = i << 0xD;
            interfaceCtx->minigamePerfectLetterPosX[i] = 200.0f;
            interfaceCtx->minigamePerfectLetterPosY[i] = 200.0f;
            interfaceCtx->minigamePerfectVtxOffset[i] = 0;
            interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_SWIRL_IN;
        } else if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_SWIRL_IN) {
            interfaceCtx->minigamePerfectLetterDirection[i] -= 0x800;
            interfaceCtx->minigamePerfectLetterPosX[i] -= 8.0f;
            interfaceCtx->minigamePerfectLetterPosY[i] -= 8.0f;
            if (interfaceCtx->minigamePerfectLetterPosX[i] <= 0.0f) {
                interfaceCtx->minigamePerfectLetterPosY[i] = 0.0f;
                interfaceCtx->minigamePerfectLetterPosX[i] = 0.0f;
                interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_STATIONARY;
                if (i == 7) {
                    interfaceCtx->minigamePerfectState[7] = MINIGAME_PERFECT_STATE_SWIRL_OUT;
                    interfaceCtx->minigamePerfectColorTimer = 5;
                    interfaceCtx->minigamePerfectState[6] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[5] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[4] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[3] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[2] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[1] = interfaceCtx->minigamePerfectState[7];
                    interfaceCtx->minigamePerfectState[0] = interfaceCtx->minigamePerfectState[7];
                }
            }
        } else if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_SWIRL_OUT) {
            stepVar1 = ABS_ALT(interfaceCtx->minigamePerfectVtxOffset[i] - D_801BFC98[i]) /
                       interfaceCtx->minigamePerfectColorTimer;
            if (interfaceCtx->minigamePerfectVtxOffset[i] >= D_801BFC98[i]) {
                interfaceCtx->minigamePerfectVtxOffset[i] -= stepVar1;
            } else {
                interfaceCtx->minigamePerfectVtxOffset[i] += stepVar1;
            }
        } else if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_6) {
            interfaceCtx->minigamePerfectLetterDirection[i] -= 0x800;
            if (interfaceCtx->minigamePerfectLetterDirection[i] == (u16)(D_801BFCA8[i] - 0x8000)) {
                interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_OFF;
            }
        }
    }

    if (interfaceCtx->minigamePerfectState[0] == MINIGAME_PERFECT_STATE_SWIRL_OUT) {
        interfaceCtx->minigamePerfectColorTimer--;
        if (interfaceCtx->minigamePerfectColorTimer == 0) {
            for (i = 0; i < 8; i++) {
                interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_5;
            }
            interfaceCtx->minigamePerfectColorTimer = 20;
        }
    }

    if ((interfaceCtx->minigamePerfectState[interfaceCtx->minigamePerfectLetterCount] == MINIGAME_PERFECT_STATE_OFF) &&
        (interfaceCtx->minigamePerfectLetterCount < 8)) {
        interfaceCtx->minigamePerfectState[interfaceCtx->minigamePerfectLetterCount] = MINIGAME_PERFECT_STATE_INIT;
        interfaceCtx->minigamePerfectLetterCount++;
    }

    if ((interfaceCtx->minigamePerfectLetterCount == 8) &&
        (interfaceCtx->minigamePerfectState[7] == MINIGAME_PERFECT_STATE_5)) {

        stepVar1 = ABS_ALT(interfaceCtx->minigamePerfectPrimColor[0] -
                           sMinigamePerfectType3PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][0]) /
                   interfaceCtx->minigamePerfectColorTimer;
        stepVar2 = ABS_ALT(interfaceCtx->minigamePerfectPrimColor[1] -
                           sMinigamePerfectType3PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][1]) /
                   interfaceCtx->minigamePerfectColorTimer;
        stepVar3 = ABS_ALT(interfaceCtx->minigamePerfectPrimColor[2] -
                           sMinigamePerfectType3PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][2]) /
                   interfaceCtx->minigamePerfectColorTimer;

        if (interfaceCtx->minigamePerfectPrimColor[0] >=
            sMinigamePerfectType3PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][0]) {
            interfaceCtx->minigamePerfectPrimColor[0] -= stepVar1;
        } else {
            interfaceCtx->minigamePerfectPrimColor[0] += stepVar1;
        }

        if (interfaceCtx->minigamePerfectPrimColor[1] >=
            sMinigamePerfectType3PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][1]) {
            interfaceCtx->minigamePerfectPrimColor[1] -= stepVar2;
        } else {
            interfaceCtx->minigamePerfectPrimColor[1] += stepVar2;
        }

        if (interfaceCtx->minigamePerfectPrimColor[2] >=
            sMinigamePerfectType3PrimColorTargets[interfaceCtx->minigamePerfectColorTargetIndex][2]) {
            interfaceCtx->minigamePerfectPrimColor[2] -= stepVar3;
        } else {
            interfaceCtx->minigamePerfectPrimColor[2] += stepVar3;
        }

        interfaceCtx->minigamePerfectColorTimer--;
        if (interfaceCtx->minigamePerfectColorTimer == 0) {
            interfaceCtx->minigamePerfectColorTimer = 20;
            interfaceCtx->minigamePerfectColorTargetIndex ^= 1;
            interfaceCtx->minigamePerfectTimer++;
            if (interfaceCtx->minigamePerfectTimer == 6) {
                for (i = 0; i < 8; i++) {
                    interfaceCtx->minigamePerfectLetterPosX[i] = 140.0f;
                    interfaceCtx->minigamePerfectLetterPosY[i] = 100.0f;
                    interfaceCtx->minigamePerfectLetterDirection[i] = D_801BFCA8[i];
                    interfaceCtx->minigamePerfectState[i] = MINIGAME_PERFECT_STATE_6;
                }
                interfaceCtx->minigamePerfectColorTimer = 5;
            }
        }
    }

    j = 0;
    for (i = 0; i < 8; i++) {
        if (interfaceCtx->minigamePerfectState[i] == MINIGAME_PERFECT_STATE_OFF) {
            j++;
        }
    }

    if (j == 8) {
        interfaceCtx->isMinigamePerfect = false;
    }
}

void Interface_DrawMinigamePerfect(PlayState* play) {
    static TexturePtr sMinigamePerfectTextures[] = {
        gMinigameLetterPTex, gMinigameLetterETex, gMinigameLetterRTex, gMinigameLetterFTex,
        gMinigameLetterETex, gMinigameLetterCTex, gMinigameLetterTTex, gMinigameExclamationTex,
    };
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    f32 letterX;
    f32 letterY;
    s16 i;
    s16 vtxOffset;

    OPEN_DISPS(play->state.gfxCtx);

    func_8012C8D4(play->state.gfxCtx);

    gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                      PRIMITIVE, 0);

    for (vtxOffset = 0, i = 0; i < 8; vtxOffset += 4, i++) {
        if (interfaceCtx->minigamePerfectState[i] != MINIGAME_PERFECT_STATE_OFF) {
            letterX =
                Math_SinS(interfaceCtx->minigamePerfectLetterDirection[i]) * interfaceCtx->minigamePerfectLetterPosX[i];
            letterY =
                Math_CosS(interfaceCtx->minigamePerfectLetterDirection[i]) * interfaceCtx->minigamePerfectLetterPosY[i];

            // Draw Minigame Perfect Shadows
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->minigamePerfectPrimColor[3]);

            Matrix_Translate(letterX, letterY, 0.0f, MTXMODE_NEW);
            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[44 + vtxOffset], 4, 0);

            OVERLAY_DISP = Gfx_DrawTexQuad4b(OVERLAY_DISP, sMinigamePerfectTextures[i], 4, 32, 33, 0);

            // Draw Minigame Perfect Colored Letters
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, interfaceCtx->minigamePerfectPrimColor[0],
                            interfaceCtx->minigamePerfectPrimColor[1], interfaceCtx->minigamePerfectPrimColor[2],
                            interfaceCtx->minigamePerfectPrimColor[3]);

            Matrix_Translate(letterX, letterY, 0.0f, MTXMODE_NEW);
            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[76 + vtxOffset], 4, 0);

            OVERLAY_DISP = Gfx_DrawTexQuad4b(OVERLAY_DISP, sMinigamePerfectTextures[i], 4, 32, 33, 0);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Interface_StartMoonCrash(PlayState* play) {
    if (play->actorCtx.flags & ACTORCTX_FLAG_1) {
        Audio_QueueSeqCmd(0xE0000100);
    }

    gSaveContext.save.day = 4;
    gSaveContext.save.daysElapsed = 4;
    gSaveContext.save.time = CLOCK_TIME(6, 0) + 10;
    play->nextEntrance = ENTRANCE(TERMINA_FIELD, 12);
    gSaveContext.nextCutsceneIndex = 0;
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_03;
}

void Interface_GetTimerDigits(u64 timer, s16* timerArr) {
    u64 time = timer;

    // 6 minutes
    timerArr[0] = time / SECONDS_TO_TIMER(360);
    time -= timerArr[0] * SECONDS_TO_TIMER(360);

    // minutes
    timerArr[1] = time / SECONDS_TO_TIMER(60);
    time -= timerArr[1] * SECONDS_TO_TIMER(60);

    // 10 seconds
    timerArr[3] = time / SECONDS_TO_TIMER(10);
    time -= timerArr[3] * SECONDS_TO_TIMER(10);

    // seconds
    timerArr[4] = time / SECONDS_TO_TIMER(1);
    time -= timerArr[4] * SECONDS_TO_TIMER(1);

    // 100 milliseconds
    timerArr[6] = time / SECONDS_TO_TIMER_PRECISE(0, 10);
    time -= timerArr[6] * SECONDS_TO_TIMER_PRECISE(0, 10);

    // 10 milliseconds
    timerArr[7] = time;
}

#define IS_POSTMAN_TIMER_DRAWN                                                        \
    (((sTimerId == TIMER_ID_POSTMAN) &&                                               \
      (gSaveContext.timerStates[TIMER_ID_POSTMAN] == TIMER_STATE_POSTMAN_COUNTING) && \
      (sPostmanBunnyHoodState == POSTMAN_MINIGAME_BUNNY_HOOD_OFF) &&                  \
      (gSaveContext.timerCurTimes[TIMER_ID_POSTMAN] < SECONDS_TO_TIMER(3))) ||        \
     (sPostmanBunnyHoodState == POSTMAN_MINIGAME_BUNNY_HOOD_ON))

/**
 * Update and draw the timers
 */
void Interface_DrawTimers(PlayState* play) {
    static s16 sTimerStateTimer = 0;
    static s16 sTimerDigits[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static s16 sTimerBeepSfxSeconds = 99;
    static s16 sTimerDigitsOffsetX[] = {
        16, 25, 34, 42, 51, 60, 68, 77,
    };
    static s16 sTimerDigitsWidth[] = {
        9, 9, 8, 9, 9, 8, 9, 9,
    };
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    MessageContext* msgCtx = &play->msgCtx;
    Player* player = GET_PLAYER(play);
    OSTime osTime;
    OSTime postmanTimerStopOsTime;
    s16 j;
    s16 i;

    OPEN_DISPS(play->state.gfxCtx);

    // Not satisfying any of these conditions will pause the timer
    if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
        (play->gameOverCtx.state == GAMEOVER_INACTIVE) &&
        ((msgCtx->msgMode == 0) ||
         ((msgCtx->msgMode != 0) && (msgCtx->currentTextId >= 0x1BB2) && (msgCtx->currentTextId <= 0x1BB6))) &&
        !(player->stateFlags1 & PLAYER_STATE1_200) && (play->transitionTrigger == TRANS_TRIGGER_OFF) &&
        (play->transitionMode == TRANS_MODE_OFF) && !Play_InCsMode(play)) {

        // Account for osTime when the timer is paused
        if (sIsTimerPaused) {
            osTime = osGetTime();

            for (j = 0; j < TIMER_ID_MAX; j++) {
                if (gSaveContext.timerStates[j] == TIMER_STATE_COUNTING) {
                    gSaveContext.timerPausedOsTimes[j] =
                        gSaveContext.timerPausedOsTimes[j] + (osTime - sTimerPausedOsTime);
                }
            }
            sIsTimerPaused = false;
        }

        sTimerId = TIMER_ID_NONE;

        // Process all active timers
        for (i = 0; i < TIMER_ID_MAX; i++) {
            if (gSaveContext.timerStates[i] == TIMER_STATE_OFF) {
                continue;
            }

            sTimerId = i;

            // Process the timer for the postman counting minigame
            if (sTimerId == TIMER_ID_POSTMAN) {
                switch (gSaveContext.timerStates[TIMER_ID_POSTMAN]) {
                    case TIMER_STATE_POSTMAN_START:
                        if (gSaveContext.timerDirections[TIMER_ID_POSTMAN] != TIMER_COUNT_DOWN) {
                            gSaveContext.timerStartOsTimes[TIMER_ID_POSTMAN] = osGetTime();
                        }
                        gSaveContext.timerStates[TIMER_ID_POSTMAN] = TIMER_STATE_POSTMAN_COUNTING;
                        sPostmanTimerInputBtnAPressed = true;
                        func_80174F7C(Interface_PostmanTimerCallback, NULL);
                        break;

                    case TIMER_STATE_POSTMAN_STOP:
                        postmanTimerStopOsTime = gSaveContext.postmanTimerStopOsTime;
                        gSaveContext.timerCurTimes[TIMER_ID_POSTMAN] = OSTIME_TO_TIMER(
                            postmanTimerStopOsTime - ((void)0, gSaveContext.timerStartOsTimes[TIMER_ID_POSTMAN]) -
                            ((void)0, gSaveContext.timerPausedOsTimes[TIMER_ID_POSTMAN]));
                        gSaveContext.timerStates[TIMER_ID_POSTMAN] = TIMER_STATE_POSTMAN_END;
                        func_80174F9C(Interface_PostmanTimerCallback, NULL);
                        break;

                    case TIMER_STATE_POSTMAN_COUNTING:
                    case TIMER_STATE_POSTMAN_END:
                        break;
                }
                break;
            }

            // process the remaining timers
            switch (gSaveContext.timerStates[sTimerId]) {
                case TIMER_STATE_START:
                case TIMER_STATE_ALT_START:
                    sTimerStateTimer = 20;
                    if (interfaceCtx->minigameState != MINIGAME_STATE_NONE) {
                        gSaveContext.timerX[sTimerId] = 26;

                        if (interfaceCtx->magicAlpha != 255) {
                            gSaveContext.timerY[sTimerId] = 22;
                        } else if (gSaveContext.save.playerData.healthCapacity > 0xA0) {
                            gSaveContext.timerY[sTimerId] = 54; // two rows of hearts
                        } else {
                            gSaveContext.timerY[sTimerId] = 46; // one row of hearts
                        }

                        if ((interfaceCtx->minigameState == MINIGAME_STATE_COUNTDOWN_GO) ||
                            (interfaceCtx->minigameState == MINIGAME_STATE_PLAYING)) {
                            if (gSaveContext.timerStates[sTimerId] == TIMER_STATE_START) {
                                gSaveContext.timerStates[sTimerId] = TIMER_STATE_COUNTING;
                            } else {
                                gSaveContext.timerStates[sTimerId] = TIMER_STATE_ALT_COUNTING;
                                D_801BF8F8[sTimerId] = osGetTime();
                                D_801BF930[sTimerId] = 0;
                            }

                            gSaveContext.timerStartOsTimes[sTimerId] = osGetTime();
                            gSaveContext.timerStopTimes[sTimerId] = 0;
                            gSaveContext.timerPausedOsTimes[sTimerId] = 0;
                        }
                    } else {
                        gSaveContext.timerStates[sTimerId] = TIMER_STATE_HOLD_TIMER;
                    }
                    break;

                case TIMER_STATE_HOLD_TIMER:
                    sTimerStateTimer--;
                    if (sTimerStateTimer == 0) {
                        sTimerStateTimer = 20;
                        gSaveContext.timerStates[sTimerId] = TIMER_STATE_MOVING_TIMER;
                    }
                    break;

                case TIMER_STATE_MOVING_TIMER:
                    if (sTimerId == TIMER_ID_MOON_CRASH) {
                        j = ((((void)0, gSaveContext.timerX[sTimerId]) - R_MOON_CRASH_TIMER_X) / sTimerStateTimer);
                        gSaveContext.timerX[sTimerId] = ((void)0, gSaveContext.timerX[sTimerId]) - j;
                        j = ((((void)0, gSaveContext.timerY[sTimerId]) - R_MOON_CRASH_TIMER_Y) / sTimerStateTimer);
                        gSaveContext.timerY[sTimerId] = ((void)0, gSaveContext.timerY[sTimerId]) - j;
                    } else {
                        j = ((((void)0, gSaveContext.timerX[sTimerId]) - 26) / sTimerStateTimer);
                        gSaveContext.timerX[sTimerId] = ((void)0, gSaveContext.timerX[sTimerId]) - j;

                        j = (gSaveContext.save.playerData.healthCapacity > 0xA0)
                                ? ((((void)0, gSaveContext.timerY[sTimerId]) - 54) / sTimerStateTimer)
                                : ((((void)0, gSaveContext.timerY[sTimerId]) - 46) / sTimerStateTimer);
                        gSaveContext.timerY[sTimerId] = ((void)0, gSaveContext.timerY[sTimerId]) - j;
                    }

                    sTimerStateTimer--;
                    if (sTimerStateTimer == 0) {
                        sTimerStateTimer = 20;

                        if (sTimerId == TIMER_ID_MOON_CRASH) {
                            gSaveContext.timerY[sTimerId] = R_MOON_CRASH_TIMER_Y;
                        } else {
                            gSaveContext.timerX[sTimerId] = 26;
                            if (gSaveContext.save.playerData.healthCapacity > 0xA0) {
                                gSaveContext.timerY[sTimerId] = 54; // two rows of hearts
                            } else {
                                gSaveContext.timerY[sTimerId] = 46; // one row of hearts
                            }
                        }

                        gSaveContext.timerStates[sTimerId] = TIMER_STATE_COUNTING;
                        gSaveContext.timerStartOsTimes[sTimerId] = osGetTime();
                        gSaveContext.timerStopTimes[sTimerId] = 0;
                        gSaveContext.timerPausedOsTimes[sTimerId] = 0;
                    }
                    // fallthrough
                case TIMER_STATE_COUNTING:
                    if ((gSaveContext.timerStates[sTimerId] == TIMER_STATE_COUNTING) &&
                        (sTimerId == TIMER_ID_MOON_CRASH)) {
                        gSaveContext.timerX[TIMER_ID_MOON_CRASH] = R_MOON_CRASH_TIMER_X;
                        gSaveContext.timerY[TIMER_ID_MOON_CRASH] = R_MOON_CRASH_TIMER_Y;
                    }
                    break;

                case TIMER_STATE_10:
                    D_801BF8F8[sTimerId] = osGetTime();
                    D_801BF930[sTimerId] = 0;
                    gSaveContext.timerStates[sTimerId] = TIMER_STATE_ALT_COUNTING;
                    // fallthrough
                case TIMER_STATE_ALT_COUNTING:
                    D_801BF930[sTimerId] = osGetTime() - D_801BF8F8[sTimerId];
                    break;

                case TIMER_STATE_12:
                    osTime = osGetTime();

                    gSaveContext.timerPausedOsTimes[sTimerId] =
                        gSaveContext.timerPausedOsTimes[sTimerId] + osTime - D_801BF8F8[sTimerId];
                    D_801BF930[sTimerId] = 0;
                    gSaveContext.timerStates[sTimerId] = TIMER_STATE_COUNTING;
                    break;

                case TIMER_STATE_ENV_START:
                    gSaveContext.timerCurTimes[sTimerId] = SECONDS_TO_TIMER(gSaveContext.save.playerData.health >> 1);
                    gSaveContext.timerDirections[sTimerId] = TIMER_COUNT_DOWN;
                    gSaveContext.timerTimeLimits[sTimerId] = gSaveContext.timerCurTimes[sTimerId];
                    sTimerStateTimer = 20;
                    gSaveContext.timerStates[sTimerId] = TIMER_STATE_MOVING_TIMER;
                    break;

                case TIMER_STATE_STOP:
                    osTime = osGetTime();

                    gSaveContext.timerStopTimes[sTimerId] =
                        OSTIME_TO_TIMER(osTime - ((void)0, gSaveContext.timerStartOsTimes[sTimerId]) -
                                        ((void)0, gSaveContext.timerPausedOsTimes[sTimerId]));

                    gSaveContext.timerStates[sTimerId] = TIMER_STATE_OFF;

                    if (sTimerId == TIMER_ID_MOON_CRASH) {
                        gSaveContext.save.day = 4;
                        if ((play->sceneId == SCENE_OKUJOU) && (gSaveContext.sceneLayer == 3)) {
                            play->nextEntrance = ENTRANCE(TERMINA_FIELD, 1);
                            gSaveContext.nextCutsceneIndex = 0xFFF0;
                            play->transitionTrigger = TRANS_TRIGGER_START;
                        } else {
                            Interface_StartMoonCrash(play);
                        }
                    } else if (gSaveContext.timerStates[TIMER_ID_GORON_RACE_UNUSED] != TIMER_STATE_OFF) {
                        gSaveContext.timerX[TIMER_ID_GORON_RACE_UNUSED] = 115;
                        gSaveContext.timerY[TIMER_ID_GORON_RACE_UNUSED] = 80;
                        if (gSaveContext.timerStates[TIMER_ID_GORON_RACE_UNUSED] <= TIMER_STATE_10) {
                            gSaveContext.timerStates[TIMER_ID_GORON_RACE_UNUSED] = TIMER_STATE_MOVING_TIMER;
                        }
                    }
                    break;

                case TIMER_STATE_6:
                    osTime = osGetTime();

                    gSaveContext.timerStopTimes[sTimerId] =
                        OSTIME_TO_TIMER(osTime - ((void)0, gSaveContext.timerStartOsTimes[sTimerId]) -
                                        ((void)0, gSaveContext.timerPausedOsTimes[sTimerId]));

                    if ((gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) &&
                        (gSaveContext.save.entrance == ENTRANCE(ROMANI_RANCH, 0))) {
                        if (gSaveContext.timerStopTimes[sTimerId] >= SECONDS_TO_TIMER(120)) {
                            gSaveContext.timerStopTimes[sTimerId] = SECONDS_TO_TIMER(120);
                            gSaveContext.timerCurTimes[sTimerId] = SECONDS_TO_TIMER(120);
                        }
                    } else if ((gSaveContext.eventInf[3] & 0x10) && (play->sceneId == SCENE_DEKUTES) &&
                               (gSaveContext.timerStopTimes[sTimerId] >= SECONDS_TO_TIMER(120))) {
                        gSaveContext.timerCurTimes[sTimerId] = SECONDS_TO_TIMER(120);
                    }
                    gSaveContext.timerStates[sTimerId] = TIMER_STATE_7;

                    if (gSaveContext.timerStates[TIMER_ID_GORON_RACE_UNUSED] != TIMER_STATE_OFF) {
                        gSaveContext.timerX[TIMER_ID_GORON_RACE_UNUSED] = 115;
                        gSaveContext.timerY[TIMER_ID_GORON_RACE_UNUSED] = 80;
                        if (gSaveContext.timerStates[TIMER_ID_GORON_RACE_UNUSED] <= TIMER_STATE_10) {
                            gSaveContext.timerStates[TIMER_ID_GORON_RACE_UNUSED] = TIMER_STATE_MOVING_TIMER;
                        }
                        gSaveContext.timerStates[sTimerId] = TIMER_STATE_OFF;
                    }
                    break;
            }
            break;
        }

        if ((sTimerId != TIMER_ID_NONE) && gSaveContext.timerStates[sTimerId]) { // != TIMER_STATE_OFF
            if (gSaveContext.timerDirections[sTimerId] == TIMER_COUNT_DOWN) {
                sTimerDigits[0] = sTimerDigits[1] = sTimerDigits[3] = sTimerDigits[4] = sTimerDigits[6] = 0;

                // used to index the counter colon
                sTimerDigits[2] = sTimerDigits[5] = 10;

                if ((gSaveContext.timerStates[sTimerId] == TIMER_STATE_COUNTING) ||
                    (gSaveContext.timerStates[sTimerId] == TIMER_STATE_10) ||
                    (gSaveContext.timerStates[sTimerId] == TIMER_STATE_ALT_COUNTING) ||
                    (gSaveContext.timerStates[sTimerId] == TIMER_STATE_POSTMAN_COUNTING)) {
                    osTime = osGetTime();
                    osTime =
                        OSTIME_TO_TIMER(osTime - ((void)0, gSaveContext.timerPausedOsTimes[sTimerId]) -
                                        D_801BF930[sTimerId] - ((void)0, gSaveContext.timerStartOsTimes[sTimerId]));
                } else if (gSaveContext.timerStates[sTimerId] == TIMER_STATE_7) {
                    osTime = gSaveContext.timerStopTimes[sTimerId];
                } else {
                    osTime = 0;
                }

                if (osTime == 0) {
                    gSaveContext.timerCurTimes[sTimerId] = gSaveContext.timerTimeLimits[sTimerId] - osTime;
                } else if (osTime <= gSaveContext.timerTimeLimits[sTimerId]) {
                    if (osTime >= gSaveContext.timerTimeLimits[sTimerId]) {
                        gSaveContext.timerCurTimes[sTimerId] = SECONDS_TO_TIMER(0);
                    } else {
                        gSaveContext.timerCurTimes[sTimerId] = gSaveContext.timerTimeLimits[sTimerId] - osTime;
                    }
                } else {
                    gSaveContext.timerCurTimes[sTimerId] = SECONDS_TO_TIMER(0);
                    gSaveContext.timerStates[sTimerId] = TIMER_STATE_STOP;
                    if (sEnvTimerActive) {
                        gSaveContext.save.playerData.health = 0;
                        play->damagePlayer(play, -(((void)0, gSaveContext.save.playerData.health) + 2));
                    }
                    sEnvTimerActive = false;
                }

                Interface_GetTimerDigits(((void)0, gSaveContext.timerCurTimes[sTimerId]), sTimerDigits);

                // Use seconds to determine when to beep
                if (gSaveContext.timerCurTimes[sTimerId] > SECONDS_TO_TIMER(60)) {
                    if ((sTimerBeepSfxSeconds != sTimerDigits[4]) && (sTimerDigits[4] == 1)) {
                        play_sound(NA_SE_SY_MESSAGE_WOMAN);
                        sTimerBeepSfxSeconds = sTimerDigits[4];
                    }
                } else if (gSaveContext.timerCurTimes[sTimerId] > SECONDS_TO_TIMER(10)) {
                    if ((sTimerBeepSfxSeconds != sTimerDigits[4]) && ((sTimerDigits[4] % 2) != 0)) {
                        play_sound(NA_SE_SY_WARNING_COUNT_N);
                        sTimerBeepSfxSeconds = sTimerDigits[4];
                    }
                } else if (sTimerBeepSfxSeconds != sTimerDigits[4]) {
                    play_sound(NA_SE_SY_WARNING_COUNT_E);
                    sTimerBeepSfxSeconds = sTimerDigits[4];
                }
            } else { // TIMER_COUNT_UP
                sTimerDigits[0] = sTimerDigits[1] = sTimerDigits[3] = sTimerDigits[4] = sTimerDigits[6] = 0;

                // used to index the counter colon
                sTimerDigits[2] = sTimerDigits[5] = 10;

                if ((gSaveContext.timerStates[sTimerId] == TIMER_STATE_COUNTING) ||
                    (gSaveContext.timerStates[sTimerId] == TIMER_STATE_POSTMAN_COUNTING)) {
                    osTime = osGetTime();
                    osTime =
                        OSTIME_TO_TIMER(osTime - ((void)0, gSaveContext.timerStartOsTimes[sTimerId]) -
                                        ((void)0, gSaveContext.timerPausedOsTimes[sTimerId]) - D_801BF930[sTimerId]);
                } else if (gSaveContext.timerStates[sTimerId] == TIMER_STATE_7) {
                    osTime = gSaveContext.timerStopTimes[sTimerId];
                } else if (sTimerId == TIMER_ID_POSTMAN) {
                    osTime = gSaveContext.timerCurTimes[sTimerId];
                } else {
                    osTime = SECONDS_TO_TIMER(0);
                }

                if ((gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) &&
                    (gSaveContext.save.entrance == ENTRANCE(ROMANI_RANCH, 0))) {
                    if (osTime >= SECONDS_TO_TIMER(120)) {
                        osTime = SECONDS_TO_TIMER(120);
                    }
                } else if ((gSaveContext.eventInf[3] & 0x10) && (play->sceneId == SCENE_DEKUTES) &&
                           (osTime >= SECONDS_TO_TIMER(120))) {
                    osTime = SECONDS_TO_TIMER(120);
                }

                gSaveContext.timerCurTimes[sTimerId] = osTime;

                Interface_GetTimerDigits(osTime, sTimerDigits);

                // Use seconds to determine when to beep
                if ((gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) &&
                    (gSaveContext.save.entrance == ENTRANCE(ROMANI_RANCH, 0))) {
                    if ((gSaveContext.timerCurTimes[sTimerId] > SECONDS_TO_TIMER(110)) &&
                        (sTimerBeepSfxSeconds != sTimerDigits[4])) {
                        play_sound(NA_SE_SY_WARNING_COUNT_E);
                        sTimerBeepSfxSeconds = sTimerDigits[4];
                    }
                } else if ((gSaveContext.eventInf[3] & 0x10) && (play->sceneId == SCENE_DEKUTES)) {
                    if ((((void)0, gSaveContext.timerCurTimes[sTimerId]) >
                         (gSaveContext.save.dekuPlaygroundHighScores[CURRENT_DAY - 1] - SECONDS_TO_TIMER(9))) &&
                        (sTimerBeepSfxSeconds != sTimerDigits[4])) {
                        play_sound(NA_SE_SY_WARNING_COUNT_E);
                        sTimerBeepSfxSeconds = sTimerDigits[4];
                    }
                }
            }

            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
            gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);
            OVERLAY_DISP = Gfx_DrawTexRectIA8(
                OVERLAY_DISP, gTimerClockIconTex, 0x10, 0x10, ((void)0, gSaveContext.timerX[sTimerId]),
                ((void)0, gSaveContext.timerY[sTimerId]) + 2, 0x10, 0x10, 1 << 10, 1 << 10);
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0,
                              0, PRIMITIVE, 0);

            // Set the timer color
            if (IS_POSTMAN_TIMER_DRAWN || (gSaveContext.timerStates[sTimerId] <= TIMER_STATE_12)) {
                if (gSaveContext.timerStates[sTimerId]) { // != TIMER_STATE_OFF
                    if (sTimerId == TIMER_ID_2) {
                        if ((gSaveContext.timerCurTimes[sTimerId] == SECONDS_TO_TIMER(0)) ||
                            (gSaveContext.timerStates[sTimerId] == TIMER_STATE_COUNTING)) {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 50, 0, 255);
                        } else {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
                        }
                    } else if ((gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) &&
                               (gSaveContext.save.entrance == ENTRANCE(ROMANI_RANCH, 0))) {
                        if (gSaveContext.timerCurTimes[sTimerId] >= SECONDS_TO_TIMER(110)) {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 50, 0, 255);
                        } else {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
                        }
                    } else if ((gSaveContext.eventInf[3] & 0x10) && (play->sceneId == SCENE_DEKUTES)) {
                        if (((void)0, gSaveContext.timerCurTimes[sTimerId]) >=
                            gSaveContext.save.dekuPlaygroundHighScores[CURRENT_DAY - 1]) {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 50, 0, 255);
                        } else if (((void)0, gSaveContext.timerCurTimes[sTimerId]) >=
                                   (gSaveContext.save.dekuPlaygroundHighScores[CURRENT_DAY - 1] -
                                    SECONDS_TO_TIMER(9))) {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 0, 255);
                        } else {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
                        }
                    } else if ((gSaveContext.timerCurTimes[sTimerId] < SECONDS_TO_TIMER(10)) &&
                               (gSaveContext.timerDirections[sTimerId] == TIMER_COUNT_DOWN) &&
                               (gSaveContext.timerStates[sTimerId] != TIMER_STATE_ALT_COUNTING)) {
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 50, 0, 255);
                    } else {
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);
                    }
                }

                // Draw the timer
                if (sTimerId == TIMER_ID_POSTMAN) {
                    if (sPostmanBunnyHoodState == POSTMAN_MINIGAME_BUNNY_HOOD_ON) {
                        // draw sTimerDigits[3] (10s of seconds) to sTimerDigits[6] (100s of milliseconds)
                        for (j = 0; j < 4; j++) {
                            OVERLAY_DISP = Gfx_DrawTexRectI8(
                                OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * sTimerDigits[j + 3])), 8, 0x10,
                                ((void)0, gSaveContext.timerX[sTimerId]) + sTimerDigitsOffsetX[j],
                                ((void)0, gSaveContext.timerY[sTimerId]), sTimerDigitsWidth[j], 0xFA, 0x370, 0x370);
                        }
                    } else {
                        // draw sTimerDigits[3] (10s of seconds) to sTimerDigits[7] (10s of milliseconds)
                        for (j = 0; j < 5; j++) {
                            OVERLAY_DISP = Gfx_DrawTexRectI8(
                                OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * sTimerDigits[j + 3])), 8, 0x10,
                                ((void)0, gSaveContext.timerX[sTimerId]) + sTimerDigitsOffsetX[j],
                                ((void)0, gSaveContext.timerY[sTimerId]), sTimerDigitsWidth[j], 0xFA, 0x370, 0x370);
                        }
                    }
                } else {
                    // draw sTimerDigits[3] (6s of minutes) to sTimerDigits[7] (10s of milliseconds)
                    for (j = 0; j < 8; j++) {
                        OVERLAY_DISP = Gfx_DrawTexRectI8(
                            OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * sTimerDigits[j])), 8, 0x10,
                            ((void)0, gSaveContext.timerX[sTimerId]) + sTimerDigitsOffsetX[j],
                            ((void)0, gSaveContext.timerY[sTimerId]), sTimerDigitsWidth[j], 0xFA, 0x370, 0x370);
                    }
                }
            }
        }

    } else if (!sIsTimerPaused) {
        sTimerPausedOsTime = osGetTime();
        sIsTimerPaused = true;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Interface_UpdateBottleTimers(PlayState* play) {
    MessageContext* msgCtx = &play->msgCtx;
    s16 i;
    s16 j;
    u64 osTime;
    s32 pad[2];

    // Not satisfying any of these conditions will pause the bottle timer
    if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE) &&
        (play->gameOverCtx.state == GAMEOVER_INACTIVE) &&
        ((msgCtx->msgMode == 0) || ((msgCtx->currentTextId >= 0x100) && (msgCtx->currentTextId <= 0x200)) ||
         ((msgCtx->currentTextId >= 0x1BB2) && (msgCtx->currentTextId <= 0x1BB6))) &&
        (play->transitionTrigger == TRANS_TRIGGER_OFF) && (play->transitionMode == TRANS_MODE_OFF) &&
        !Play_InCsMode(play)) {

        // Account for osTime when the timer is paused
        if (sIsBottleTimerPaused) {
            osTime = osGetTime();

            for (j = BOTTLE_FIRST; j < BOTTLE_MAX; j++) {
                if (gSaveContext.bottleTimerStates[j] == BOTTLE_TIMER_STATE_COUNTING) {
                    gSaveContext.bottleTimerPausedOsTimes[j] += osTime - sBottleTimerPausedOsTime;
                }
            }

            sIsBottleTimerPaused = false;
        }

        sTimerId = TIMER_ID_NONE;

        for (i = BOTTLE_FIRST; i < BOTTLE_MAX; i++) {
            if (gSaveContext.bottleTimerStates[i] == BOTTLE_TIMER_STATE_COUNTING) {
                osTime = osGetTime();
                osTime = OSTIME_TO_TIMER_ALT(osTime - ((void)0, gSaveContext.bottleTimerPausedOsTimes[i]) -
                                             ((void)0, gSaveContext.bottleTimerStartOsTimes[i]));
                if (osTime == 0) {
                    gSaveContext.bottleTimerCurTimes[i] = gSaveContext.bottleTimerTimeLimits[i] - osTime;
                } else if (osTime <= gSaveContext.bottleTimerTimeLimits[i]) {
                    if (osTime >= gSaveContext.bottleTimerTimeLimits[i]) {
                        gSaveContext.bottleTimerCurTimes[i] = 0;
                    } else {
                        gSaveContext.bottleTimerCurTimes[i] = gSaveContext.bottleTimerTimeLimits[i] - osTime;
                    }
                } else {
                    gSaveContext.bottleTimerCurTimes[i] = 0;

                    if (gSaveContext.save.inventory.items[i + SLOT_BOTTLE_1] == ITEM_HOT_SPRING_WATER) {
                        Inventory_UpdateItem(play, i + SLOT_BOTTLE_1, ITEM_SPRING_WATER);
                        Message_StartTextbox(play, 0xFA, NULL);
                    }
                    gSaveContext.bottleTimerStates[i] = BOTTLE_TIMER_STATE_OFF;
                }
            }
        }
    } else if (!sIsBottleTimerPaused) {
        sBottleTimerPausedOsTime = osGetTime();
        sIsBottleTimerPaused = true;
    }
}

void Interface_DrawMinigameIcons(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s16 i;
    s16 numDigitsDrawn;
    s16 rectX;
    s16 rectY;
    s16 width;
    s16 height;

    OPEN_DISPS(play->state.gfxCtx);

    func_8012C654(play->state.gfxCtx);

    if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {
        // Carrots rendering if the action corresponds to riding a horse
        if (interfaceCtx->aButtonHorseDoAction == DO_ACTION_FASTER) {
            // Load Carrot Icon
            gDPLoadTextureBlock(OVERLAY_DISP++, gCarrotIconTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 16, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);

            rectX = 110;
            rectY = (interfaceCtx->minigameState != MINIGAME_STATE_NONE) ? 200 : 56;

            // Draw 6 carrots
            for (i = 1; i < 7; i++, rectX += 16) {
                // Carrot Color (based on availability)
                if ((interfaceCtx->numHorseBoosts == 0) || (interfaceCtx->numHorseBoosts < i)) {
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 150, 255, interfaceCtx->aAlpha);
                } else {
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->aAlpha);
                }

                gSPTextureRectangle(OVERLAY_DISP++, rectX << 2, rectY << 2, (rectX + 16) << 2, (rectY + 16) << 2,
                                    G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            }
        }

        if (gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) {
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
            width = 24;
            height = 16;
            rectX = 20;
            if (gSaveContext.save.playerData.healthCapacity > 0xA0) {
                rectY = 75; // two rows of hearts
            } else {
                rectY = 67; // one row of hearts
            }

            if (gSaveContext.save.entrance == ENTRANCE(WATERFALL_RAPIDS, 1)) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
                gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
                gDPLoadTextureBlock(OVERLAY_DISP++, gBeaverRingIconTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 24, 16, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
            } else if (play->sceneId == SCENE_DOUJOU) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 140, 50, interfaceCtx->bAlpha);
                gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
                gDPLoadTextureBlock(OVERLAY_DISP++, gSwordTrainingLogIconTex, G_IM_FMT_IA, G_IM_SIZ_8b, 24, 16, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
            } else if (play->sceneId == SCENE_30GYOSON) {
                width = 16;
                height = 30;
                rectX = 24;
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 100, 75, interfaceCtx->bAlpha);
                gDPSetEnvColor(OVERLAY_DISP++, 55, 55, 0, 255);
                gDPLoadTextureBlock(OVERLAY_DISP++, gFishermanMinigameTorchIconTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 30, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
            } else {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
                gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
                gDPLoadTextureBlock(OVERLAY_DISP++, gArcheryScoreIconTex, G_IM_FMT_RGBA, G_IM_SIZ_16b, 24, 16, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
            }

            gSPTextureRectangle(OVERLAY_DISP++, (rectX << 2), (rectY << 2), ((rectX + width) << 2),
                                ((rectY + height) << 2), G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->bAlpha);
            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0,
                              0, PRIMITIVE, 0);

            if (play->sceneId == SCENE_30GYOSON) {
                rectX += 20;
                if (gSaveContext.save.playerData.healthCapacity > 0xA0) {
                    rectY = 87; // two rows of hearts
                } else {
                    rectY = 79; // one row of hearts
                }
            } else {
                rectX += 26;
            }

            for (i = 0, numDigitsDrawn = 0; i < 4; i++) {
                if ((sMinigameScoreDigits[i] != 0) || (numDigitsDrawn != 0) || (i >= 3)) {
                    OVERLAY_DISP =
                        Gfx_DrawTexRectI8(OVERLAY_DISP, ((u8*)gCounterDigit0Tex + (8 * 16 * sMinigameScoreDigits[i])),
                                          8, 0x10, rectX, rectY - 2, 9, 0xFA, 0x370, 0x370);
                    rectX += 9;
                    numDigitsDrawn++;
                }
            }

            gDPPipeSync(OVERLAY_DISP++);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

s16 sRupeeDigitsFirst[] = { 1, 0, 0, 0 };
s16 sRupeeDigitsCount[] = { 2, 3, 3, 0 };
Color_RGB16 sRupeeCounterIconPrimColors[] = {
    { 200, 255, 100 },
    { 170, 170, 255 },
    { 255, 105, 105 },
};
Color_RGB16 sRupeeCounterIconEnvColors[] = {
    { 0, 80, 0 },
    { 10, 10, 80 },
    { 40, 10, 0 },
};
TexturePtr sMinigameCountdownTextures[] = {
    gMinigameCountdown3Tex,
    gMinigameCountdown2Tex,
    gMinigameCountdown1Tex,
    gMinigameCountdownGoTex,
};
s16 sMinigameCountdownTexWidths[] = { 24, 24, 24, 40 };
Color_RGB16 sMinigameCountdownPrimColors[] = {
    { 100, 255, 100 },
    { 255, 255, 60 },
    { 255, 100, 0 },
    { 120, 170, 255 },
};
TexturePtr gStoryTextures[] = {
    gStoryMaskFestivalTex,
    gStoryGiantsLeavingTex,
};
TexturePtr gStoryTLUTs[] = {
    gStoryMaskFestivalTLUT,
    gStoryGiantsLeavingTLUT,
};

// TODO: ucode.h? (see OoT)
#define SP_UCODE_DATA_SIZE 0x800

void Interface_Draw(PlayState* play) {
    s32 pad;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    Player* player = GET_PLAYER(play);
    Gfx* gfx;
    s16 sp2CE;
    s16 sp2CC;
    s16 sp2CA;
    s16 sp2C8;
    PauseContext* pauseCtx = &play->pauseCtx;
    f32 sp2C0;
    s16 counterDigits[4]; // sp2B8
    s16 sp4C;

    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(OVERLAY_DISP++, 0x02, interfaceCtx->parameterSegment);
    gSPSegment(OVERLAY_DISP++, 0x09, interfaceCtx->doActionSegment);
    gSPSegment(OVERLAY_DISP++, 0x08, interfaceCtx->iconItemSegment);
    gSPSegment(OVERLAY_DISP++, 0x0B, interfaceCtx->mapSegment);

    if (pauseCtx->debugEditor == DEBUG_EDITOR_NONE) {
        Interface_SetVertices(play);
        Interface_SetOrthoView(interfaceCtx);

        // Draw Grandma's Story
        if (interfaceCtx->storyDmaStatus == STORY_DMA_DONE) {
            gSPSegment(OVERLAY_DISP++, 0x07, interfaceCtx->storySegment);
            func_8012C628(play->state.gfxCtx);

            gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_POINT);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

            // Load in Grandma's Story
            gSPLoadUcodeL(OVERLAY_DISP++, gspS2DEX2_fifo);
            gfx = OVERLAY_DISP;
            func_80172758(&gfx, gStoryTextures[interfaceCtx->storyType], gStoryTLUTs[interfaceCtx->storyType],
                          SCREEN_WIDTH, SCREEN_HEIGHT, 2, 1, 0x8000, 0x100, 0.0f, 0.0f, 1.0f, 1.0f, 0);
            OVERLAY_DISP = gfx;
            gSPLoadUcode(OVERLAY_DISP++, SysUcode_GetUCode(), SysUcode_GetUCodeData());

            gDPPipeSync(OVERLAY_DISP++);

            // Fill the screen with a black rectangle
            gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, R_STORY_FILL_SCREEN_ALPHA);
            gDPFillRectangle(OVERLAY_DISP++, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        }

        LifeMeter_Draw(play);

        func_8012C654(play->state.gfxCtx);

        // Draw Rupee Icon
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sRupeeCounterIconPrimColors[CUR_UPG_VALUE(4)].r,
                        sRupeeCounterIconPrimColors[CUR_UPG_VALUE(4)].g,
                        sRupeeCounterIconPrimColors[CUR_UPG_VALUE(4)].b, interfaceCtx->magicAlpha);
        gDPSetEnvColor(OVERLAY_DISP++, sRupeeCounterIconEnvColors[CUR_UPG_VALUE(4)].r,
                       sRupeeCounterIconEnvColors[CUR_UPG_VALUE(4)].g, sRupeeCounterIconEnvColors[CUR_UPG_VALUE(4)].b,
                       255);
        OVERLAY_DISP =
            Gfx_DrawTexRectIA8(OVERLAY_DISP, gRupeeCounterIconTex, 16, 16, 26, 206, 16, 16, 1 << 10, 1 << 10);

        switch (play->sceneId) {
            case SCENE_INISIE_N:
            case SCENE_INISIE_R:
            case SCENE_MITURIN:
            case SCENE_HAKUGIN:
            case SCENE_SEA:
                if (DUNGEON_KEY_COUNT(gSaveContext.mapIndex) >= 0) {
                    // Small Key Icon
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 230, 255, interfaceCtx->magicAlpha);
                    gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 20, 255);
                    OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, gSmallKeyCounterIconTex, 16, 16, 26, 190, 16, 16,
                                                      1 << 10, 1 << 10);

                    // Small Key Counter
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE,
                                      TEXEL0, 0, PRIMITIVE, 0);

                    counterDigits[2] = 0;
                    counterDigits[3] = DUNGEON_KEY_COUNT(gSaveContext.mapIndex);

                    while (counterDigits[3] >= 10) {
                        counterDigits[2]++;
                        counterDigits[3] -= 10;
                    }

                    sp2CA = 42;

                    if (counterDigits[2] != 0) {
                        gDPPipeSync(OVERLAY_DISP++);
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->magicAlpha);

                        OVERLAY_DISP =
                            Gfx_DrawTexRectI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[2]), 8, 16,
                                              43, 191, 8, 16, 1 << 10, 1 << 10);

                        gDPPipeSync(OVERLAY_DISP++);
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
                        gSPTextureRectangle(OVERLAY_DISP++, 168, 760, 200, 824, G_TX_RENDERTILE, 0, 0, 1 << 10,
                                            1 << 10);

                        sp2CA += 8;
                    }

                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->magicAlpha);

                    OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[3]),
                                                     8, 16, sp2CA + 1, 191, 8, 16, 1 << 10, 1 << 10);

                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
                    gSPTextureRectangle(OVERLAY_DISP++, sp2CA * 4, 760, (sp2CA * 4) + 0x20, 824, G_TX_RENDERTILE, 0, 0,
                                        1 << 10, 1 << 10);
                }
                break;

            case SCENE_KINSTA1:
            case SCENE_KINDAN2:
                // Gold Skulltula Icon
                gDPPipeSync(OVERLAY_DISP++);
                gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
                gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
                gDPLoadTextureBlock(OVERLAY_DISP++, gGoldSkulltulaCounterIconTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 24, 24,
                                    0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
                gSPTextureRectangle(OVERLAY_DISP++, 80, 748, 176, 820, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

                // Gold Skulluta Counter
                gDPPipeSync(OVERLAY_DISP++);
                gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE,
                                  TEXEL0, 0, PRIMITIVE, 0);

                counterDigits[2] = 0;
                counterDigits[3] = Inventory_GetSkullTokenCount(play->sceneId);

                while (counterDigits[3] >= 10) {
                    counterDigits[2]++;
                    counterDigits[3] -= 10;
                }

                sp2CA = 42;

                if (counterDigits[2] != 0) {
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->magicAlpha);

                    OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[2]),
                                                     8, 16, 43, 191, 8, 16, 1 << 10, 1 << 10);

                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
                    gSPTextureRectangle(OVERLAY_DISP++, 168, 760, 200, 824, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

                    sp2CA += 8;
                }

                gDPPipeSync(OVERLAY_DISP++);
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->magicAlpha);

                OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[3]), 8,
                                                 16, sp2CA + 1, 191, 8, 16, 1 << 10, 1 << 10);

                gDPPipeSync(OVERLAY_DISP++);
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
                gSPTextureRectangle(OVERLAY_DISP++, sp2CA * 4, 760, (sp2CA * 4) + 0x20, 824, G_TX_RENDERTILE, 0, 0,
                                    1 << 10, 1 << 10);
                break;

            default:
                break;
        }

        // Rupee Counter
        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                          PRIMITIVE, 0);

        counterDigits[0] = counterDigits[1] = 0;
        counterDigits[2] = gSaveContext.save.playerData.rupees;

        if ((counterDigits[2] > 9999) || (counterDigits[2] < 0)) {
            counterDigits[2] &= 0xDDD;
        }

        while (counterDigits[2] >= 100) {
            counterDigits[0]++;
            counterDigits[2] -= 100;
        }

        while (counterDigits[2] >= 10) {
            counterDigits[1]++;
            counterDigits[2] -= 10;
        }

        sp2CC = sRupeeDigitsFirst[CUR_UPG_VALUE(UPG_WALLET)];
        sp2C8 = sRupeeDigitsCount[CUR_UPG_VALUE(UPG_WALLET)];

        sp4C = interfaceCtx->magicAlpha;
        if (sp4C > 180) {
            sp4C = 180;
        }

        for (sp2CE = 0, sp2CA = 42; sp2CE < sp2C8; sp2CE++, sp2CC++, sp2CA += 8) {
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, sp4C);

            OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[sp2CC]), 8,
                                             16, sp2CA + 1, 207, 8, 16, 1 << 10, 1 << 10);

            gDPPipeSync(OVERLAY_DISP++);

            if (gSaveContext.save.playerData.rupees == CUR_CAPACITY(UPG_WALLET)) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 120, 255, 0, interfaceCtx->magicAlpha);
            } else if (gSaveContext.save.playerData.rupees != 0) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
            } else {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 100, 100, interfaceCtx->magicAlpha);
            }

            gSPTextureRectangle(OVERLAY_DISP++, sp2CA * 4, 824, (sp2CA * 4) + 0x20, 888, G_TX_RENDERTILE, 0, 0, 1 << 10,
                                1 << 10);
        }

        Magic_DrawMeter(play);
        Minimap_Draw(play);

        if ((SREG(94) != 2) && (SREG(94) != 3)) {
            Actor_DrawZTarget(&play->actorCtx.targetContext, play);
        }

        func_8012C654(play->state.gfxCtx);

        Interface_DrawItemButtons(play);

        if (player->transformation == ((void)0, gSaveContext.save.playerForm)) {
            Interface_DrawBButtonIcons(play);
        }
        Interface_DrawCButtonIcons(play);

        Interface_DrawAButton(play);

        Interface_DrawPauseMenuEquippingIcons(play);

        // Draw either the minigame countdown or the three-day clock
        if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {
            if ((interfaceCtx->minigameState != MINIGAME_STATE_NONE) &&
                (interfaceCtx->minigameState < MINIGAME_STATE_NO_COUNTDOWN_SETUP)) {
                // Minigame Countdown
                if (((u32)interfaceCtx->minigameState % 2) == 0) {

                    sp2CE = (interfaceCtx->minigameState >> 1) - 1;
                    sp2C0 = interfaceCtx->minigameCountdownScale / 100.0f;

                    if (sp2CE == 3) {
                        interfaceCtx->actionVtx[40 + 0].v.ob[0] = interfaceCtx->actionVtx[40 + 2].v.ob[0] = -20;
                        interfaceCtx->actionVtx[40 + 1].v.ob[0] = interfaceCtx->actionVtx[40 + 3].v.ob[0] =
                            interfaceCtx->actionVtx[40 + 0].v.ob[0] + 40;
                        interfaceCtx->actionVtx[40 + 1].v.tc[0] = interfaceCtx->actionVtx[40 + 3].v.tc[0] = 40 << 5;
                    }

                    interfaceCtx->actionVtx[40 + 2].v.tc[1] = interfaceCtx->actionVtx[40 + 3].v.tc[1] = 32 << 5;

                    func_8012C8D4(play->state.gfxCtx);

                    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                    gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sMinigameCountdownPrimColors[sp2CE].r,
                                    sMinigameCountdownPrimColors[sp2CE].g, sMinigameCountdownPrimColors[sp2CE].b,
                                    interfaceCtx->minigameCountdownAlpha);

                    Matrix_Translate(0.0f, -40.0f, 0.0f, MTXMODE_NEW);
                    Matrix_Scale(sp2C0, sp2C0, 0.0f, MTXMODE_APPLY);

                    gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(play->state.gfxCtx),
                              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[40], 4, 0);

                    OVERLAY_DISP = Gfx_DrawTexQuadIA8(OVERLAY_DISP, sMinigameCountdownTextures[sp2CE],
                                                      sMinigameCountdownTexWidths[sp2CE], 32, 0);
                }
            } else {
                Interface_DrawClock(play);
            }
        }

        // Draw the letters of minigame perfect
        if (interfaceCtx->isMinigamePerfect) {
            Interface_DrawMinigamePerfect(play);
        }

        Interface_DrawMinigameIcons(play);
        Interface_DrawTimers(play);
    }

    // Draw pictograph focus icons
    if (sPictographState == PICTOGRAPH_STATE_LENS) {

        func_8012C654(play->state.gfxCtx);

        gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
        gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, 255);
        gDPLoadTextureBlock_4b(OVERLAY_DISP++, gPictoBoxFocusBorderTex, G_IM_FMT_IA, 16, 16, 0, G_TX_MIRROR | G_TX_WRAP,
                               G_TX_MIRROR | G_TX_WRAP, 4, 4, G_TX_NOLOD, G_TX_NOLOD);

        gSPTextureRectangle(OVERLAY_DISP++, YREG(32) << 2, YREG(33) << 2, (YREG(32) << 2) + (16 << 2),
                            (YREG(33) << 2) + (16 << 2), G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        gSPTextureRectangle(OVERLAY_DISP++, YREG(34) << 2, YREG(35) << 2, (YREG(34) << 2) + (16 << 2),
                            (YREG(35) << 2) + (16 << 2), G_TX_RENDERTILE, 512, 0, 1 << 10, 1 << 10);
        gSPTextureRectangle(OVERLAY_DISP++, YREG(36) << 2, YREG(37) << 2, (YREG(36) << 2) + (16 << 2),
                            (YREG(37) << 2) + (16 << 2), G_TX_RENDERTILE, 0, 512, 1 << 10, 1 << 10);
        gSPTextureRectangle(OVERLAY_DISP++, YREG(38) << 2, YREG(39) << 2, (YREG(38) << 2) + (16 << 2),
                            (YREG(39) << 2) + (16 << 2), G_TX_RENDERTILE, 512, 512, 1 << 10, 1 << 10);

        gDPLoadTextureBlock_4b(OVERLAY_DISP++, gPictoBoxFocusIconTex, G_IM_FMT_I, 32, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSPTextureRectangle(OVERLAY_DISP++, YREG(40) << 2, YREG(41) << 2, (YREG(40) << 2) + 0x80,
                            (YREG(41) << 2) + (16 << 2), G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

        gDPLoadTextureBlock_4b(OVERLAY_DISP++, gPictoBoxFocusTextTex, G_IM_FMT_I, 32, 8, 0, G_TX_NOMIRROR | G_TX_WRAP,
                               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSPTextureRectangle(OVERLAY_DISP++, YREG(42) << 2, YREG(43) << 2, (YREG(42) << 2) + 0x80,
                            (YREG(43) << 2) + 0x20, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
    }

    // Draw pictograph photo
    if (sPictographState >= PICTOGRAPH_STATE_SETUP_PHOTO) {
        if (!(play->actorCtx.flags & ACTORCTX_FLAG_PICTOGRAPH_ON)) {
            Play_CompressI8ToI5((play->pictoPhotoI8 != NULL) ? play->pictoPhotoI8 : D_801FBB90,
                                (u8*)gSaveContext.pictoPhotoI5, PICTO_RESOLUTION_X * PICTO_RESOLUTION_Y);

            interfaceCtx->unk_222 = interfaceCtx->unk_224 = 0;

            sPictographState = PICTOGRAPH_STATE_OFF;
            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
        } else {
            s16 pictoRectTop;

            if (sPictographState == PICTOGRAPH_STATE_SETUP_PHOTO) {
                sPictographState = PICTOGRAPH_STATE_PHOTO;
                Message_StartTextbox(play, 0xF8, NULL);
                Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
                player->stateFlags1 |= PLAYER_STATE1_200;
            }

            gDPPipeSync(OVERLAY_DISP++);
            gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 200, 200, 250);
            gDPFillRectangle(OVERLAY_DISP++, 70, 22, 251, 151);

            func_8012C654(play->state.gfxCtx);

            gDPSetRenderMode(OVERLAY_DISP++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEI_PRIM, G_CC_MODULATEI_PRIM);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 250, 160, 160, 255);

            pictoRectTop = PICTO_POS_Y;
            for (sp2CC = 0; sp2CC < (PICTO_RESOLUTION_Y / 8); sp2CC++, pictoRectTop += 8) {
                gDPLoadTextureBlock(OVERLAY_DISP++,
                                    (u8*)((play->pictoPhotoI8 != NULL) ? play->pictoPhotoI8 : D_801FBB90) +
                                        (0x500 * sp2CC),
                                    G_IM_FMT_I, G_IM_SIZ_8b, PICTO_RESOLUTION_X, 8, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

                gSPTextureRectangle(OVERLAY_DISP++, PICTO_POS_X << 2, pictoRectTop << 2,
                                    (PICTO_POS_X + PICTO_RESOLUTION_X) << 2, (pictoRectTop << 2) + (8 << 2),
                                    G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            }
        }
    }

    // Draw over the entire screen (used in gameover)
    if (interfaceCtx->screenFillAlpha != 0) {
        gDPPipeSync(OVERLAY_DISP++);
        gSPDisplayList(OVERLAY_DISP++, gScreenFillSetupDL);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->screenFillAlpha);
        gSPDisplayList(OVERLAY_DISP++, D_0E0002E0);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void Interface_LoadStory(PlayState* play, s32 block) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    switch (interfaceCtx->storyDmaStatus) {
        case STORY_DMA_IDLE:
            if (interfaceCtx->storySegment == NULL) {
                break;
            }
            osCreateMesgQueue(&interfaceCtx->storyMsgQueue, &interfaceCtx->storyMsgBuf, 1);
            DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184, interfaceCtx->storySegment, interfaceCtx->storyAddr,
                                   interfaceCtx->storySize, 0, &interfaceCtx->storyMsgQueue, NULL);
            interfaceCtx->storyDmaStatus = STORY_DMA_LOADING;
            // fallthrough
        case STORY_DMA_LOADING:
            if (osRecvMesg(&interfaceCtx->storyMsgQueue, NULL, block) == 0) {
                interfaceCtx->storyDmaStatus = STORY_DMA_DONE;
            }
            break;

        default:
            break;
    }
}

void Interface_AllocStory(PlayState* play) {
    InterfaceContext* interfaceCtx = &play->interfaceCtx;

    interfaceCtx->storyAddr = SEGMENT_ROM_START(story_static);
    interfaceCtx->storySize = SEGMENT_ROM_SIZE(story_static);

    if (interfaceCtx->storySegment == NULL) {
        interfaceCtx->storySegment = ZeldaArena_Malloc(interfaceCtx->storySize);
    }
    Interface_LoadStory(play, OS_MESG_NOBLOCK);
}

void Interface_Update(PlayState* play) {
    static u8 sIsSunsPlayedAtDay = false;
    static s16 sPrevTimeSpeed = 0;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    MessageContext* msgCtx = &play->msgCtx;
    Player* player = GET_PLAYER(play);
    s16 dimmingAlpha;
    s16 risingAlpha;
    u16 aButtonDoAction;

    // Update buttons
    if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {
        if (play->gameOverCtx.state == GAMEOVER_INACTIVE) {
            Interface_UpdateButtonsPart1(play);
        }
    }

    // Update hud visibility
    switch (gSaveContext.nextHudVisibility) {
        case HUD_VISIBILITY_NONE:
        case HUD_VISIBILITY_NONE_ALT:
        case HUD_VISIBILITY_B:
            dimmingAlpha = 255 - (gSaveContext.hudVisibilityTimer << 5);
            if (dimmingAlpha < 0) {
                dimmingAlpha = 0;
            }

            Interface_UpdateHudAlphas(play, dimmingAlpha);
            gSaveContext.hudVisibilityTimer += 2;
            if (dimmingAlpha == 0) {
                gSaveContext.nextHudVisibility = HUD_VISIBILITY_IDLE;
            }
            break;

        case HUD_VISIBILITY_HEARTS_WITH_OVERWRITE:
        case HUD_VISIBILITY_A:
        case HUD_VISIBILITY_A_HEARTS_MAGIC_WITH_OVERWRITE:
        case HUD_VISIBILITY_A_HEARTS_MAGIC_MINIMAP_WITH_OVERWRITE:
        case HUD_VISIBILITY_ALL_NO_MINIMAP_W_DISABLED:
        case HUD_VISIBILITY_HEARTS_MAGIC:
        case HUD_VISIBILITY_B_ALT:
        case HUD_VISIBILITY_HEARTS:
        case HUD_VISIBILITY_A_B_MINIMAP:
        case HUD_VISIBILITY_HEARTS_MAGIC_WITH_OVERWRITE:
        case HUD_VISIBILITY_HEARTS_MAGIC_C:
        case HUD_VISIBILITY_ALL_NO_MINIMAP:
        case HUD_VISIBILITY_A_B_C:
        case HUD_VISIBILITY_B_MINIMAP:
        case HUD_VISIBILITY_HEARTS_MAGIC_MINIMAP:
        case HUD_VISIBILITY_A_HEARTS_MAGIC_MINIMAP:
        case HUD_VISIBILITY_B_MAGIC:
        case HUD_VISIBILITY_A_B:
        case HUD_VISIBILITY_A_B_HEARTS_MAGIC_MINIMAP:
            dimmingAlpha = 255 - (gSaveContext.hudVisibilityTimer << 5);
            if (dimmingAlpha < 0) {
                dimmingAlpha = 0;
            }

            Interface_UpdateHudAlphas(play, dimmingAlpha);
            gSaveContext.hudVisibilityTimer++;
            if (dimmingAlpha == 0) {
                gSaveContext.nextHudVisibility = HUD_VISIBILITY_IDLE;
            }
            break;

        case HUD_VISIBILITY_ALL:
            dimmingAlpha = 255 - (gSaveContext.hudVisibilityTimer << 5);
            if (dimmingAlpha < 0) {
                dimmingAlpha = 0;
            }

            risingAlpha = 255 - dimmingAlpha;
            if (risingAlpha >= 255) {
                risingAlpha = 255;
            }

            Interface_UpdateButtonAlphasByStatus(play, risingAlpha);

            if (gSaveContext.buttonStatus[5] == BTN_DISABLED) {
                if (interfaceCtx->startAlpha != 70) {
                    interfaceCtx->startAlpha = 70;
                }
            } else {
                if (interfaceCtx->startAlpha != 255) {
                    interfaceCtx->startAlpha = risingAlpha;
                }
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = risingAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = risingAlpha;
            }

            if (play->sceneId == SCENE_SPOT00) {
                if (interfaceCtx->minimapAlpha < 170) {
                    interfaceCtx->minimapAlpha = risingAlpha;
                } else {
                    interfaceCtx->minimapAlpha = 170;
                }
            } else if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = risingAlpha;
            }

            gSaveContext.hudVisibilityTimer++;

            if (risingAlpha == 255) {
                gSaveContext.nextHudVisibility = HUD_VISIBILITY_IDLE;
            }

            break;

        case HUD_VISIBILITY_NONE_INSTANT:
            // Turn off all Hud immediately
            gSaveContext.nextHudVisibility = HUD_VISIBILITY_NONE;
            Interface_UpdateHudAlphas(play, 0);
            gSaveContext.nextHudVisibility = HUD_VISIBILITY_IDLE;
            // fallthrough
        default:
            break;
    }

    Map_Update(play);

    // Update health
    if (gSaveContext.healthAccumulator != 0) {
        gSaveContext.healthAccumulator -= 4;
        gSaveContext.save.playerData.health += 4;

        if ((gSaveContext.save.playerData.health & 0xF) < 4) {
            play_sound(NA_SE_SY_HP_RECOVER);
        }

        if (((void)0, gSaveContext.save.playerData.health) >= ((void)0, gSaveContext.save.playerData.healthCapacity)) {
            gSaveContext.save.playerData.health = gSaveContext.save.playerData.healthCapacity;
            gSaveContext.healthAccumulator = 0;
        }
    }

    LifeMeter_UpdateSizeAndBeep(play);

    // Update Env Timer Part 1
    sEnvTimerType = Player_GetEnvTimerType(play);
    if (sEnvTimerType == PLAYER_ENV_TIMER_HOTROOM) {
        if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_TUNIC) == EQUIP_VALUE_TUNIC_GORON) {
            sEnvTimerType = PLAYER_ENV_TIMER_NONE;
        }
    } else if ((Player_GetEnvTimerType(play) >= PLAYER_ENV_TIMER_UNDERWATER_FLOOR) &&
               (Player_GetEnvTimerType(play) <= PLAYER_ENV_TIMER_UNDERWATER_FREE)) {
        if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_TUNIC) == EQUIP_VALUE_TUNIC_ZORA) {
            sEnvTimerType = PLAYER_ENV_TIMER_NONE;
        }
    }

    LifeMeter_UpdateColors(play);

    // Update rupees
    if (gSaveContext.rupeeAccumulator != 0) {
        if (gSaveContext.rupeeAccumulator > 0) {
            if (gSaveContext.save.playerData.rupees < CUR_CAPACITY(UPG_WALLET)) {
                gSaveContext.rupeeAccumulator--;
                gSaveContext.save.playerData.rupees++;
                play_sound(NA_SE_SY_RUPY_COUNT);
            } else {
                // Max rupees
                gSaveContext.save.playerData.rupees = CUR_CAPACITY(UPG_WALLET);
                gSaveContext.rupeeAccumulator = 0;
            }
        } else if (gSaveContext.save.playerData.rupees != 0) {
            if (gSaveContext.rupeeAccumulator <= -50) {
                gSaveContext.rupeeAccumulator += 10;
                gSaveContext.save.playerData.rupees -= 10;
                if (gSaveContext.save.playerData.rupees < 0) {
                    gSaveContext.save.playerData.rupees = 0;
                }
                play_sound(NA_SE_SY_RUPY_COUNT);
            } else {
                gSaveContext.rupeeAccumulator++;
                gSaveContext.save.playerData.rupees--;
                play_sound(NA_SE_SY_RUPY_COUNT);
            }
        } else {
            gSaveContext.rupeeAccumulator = 0;
        }
    }

    // Update minigame perfect
    if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {
        if (interfaceCtx->isMinigamePerfect) {
            if (interfaceCtx->minigamePerfectType == MINIGAME_PERFECT_TYPE_1) {
                Interface_UpdateMinigamePerfectType1(play);
            } else if (interfaceCtx->minigamePerfectType == MINIGAME_PERFECT_TYPE_2) {
                Interface_UpdateMinigamePerfectType2(play);
            } else if (interfaceCtx->minigamePerfectType == MINIGAME_PERFECT_TYPE_3) {
                Interface_UpdateMinigamePerfectType3(play);
            }
        }
    }

    // Update minigame State
    if ((play->pauseCtx.state == 0) && (play->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {
        if (interfaceCtx->minigameState) { // != MINIGAME_STATE_NONE
            switch (interfaceCtx->minigameState) {
                case MINIGAME_STATE_COUNTDOWN_SETUP_3:  // minigame countdown 3
                case MINIGAME_STATE_COUNTDOWN_SETUP_2:  // minigame countdown 2
                case MINIGAME_STATE_COUNTDOWN_SETUP_1:  // minigame countdown 1
                case MINIGAME_STATE_COUNTDOWN_SETUP_GO: // minigame countdown Go!
                    interfaceCtx->minigameCountdownAlpha = 255;
                    interfaceCtx->minigameCountdownScale = 100;
                    interfaceCtx->minigameState++;
                    if (interfaceCtx->minigameState == MINIGAME_STATE_COUNTDOWN_GO) {
                        interfaceCtx->minigameCountdownScale = 160;
                        play_sound(NA_SE_SY_START_SHOT);
                    } else {
                        play_sound(NA_SE_SY_WARNING_COUNT_E);
                    }
                    break;

                case MINIGAME_STATE_COUNTDOWN_3:
                case MINIGAME_STATE_COUNTDOWN_2:
                case MINIGAME_STATE_COUNTDOWN_1:
                    interfaceCtx->minigameCountdownAlpha -= 10;
                    interfaceCtx->minigameCountdownScale += 3;
                    if (interfaceCtx->minigameCountdownAlpha < 22) {
                        interfaceCtx->minigameState++;
                    }
                    break;

                case MINIGAME_STATE_COUNTDOWN_GO:
                    interfaceCtx->minigameCountdownAlpha -= 10;
                    if (interfaceCtx->minigameCountdownAlpha <= 150) {
                        interfaceCtx->minigameState = MINIGAME_STATE_PLAYING;
                    }
                    break;

                case MINIGAME_STATE_NO_COUNTDOWN_SETUP:
                    interfaceCtx->minigameCountdownAlpha = 0;
                    interfaceCtx->minigameState++;
                    break;

                case MINIGAME_STATE_NO_COUNTDOWN:
                    interfaceCtx->minigameCountdownAlpha++;
                    if (interfaceCtx->minigameCountdownAlpha >= 18) {
                        interfaceCtx->minigameState = MINIGAME_STATE_PLAYING;
                    }
                    break;
            }
        }
    }

    // Update A Button
    switch (interfaceCtx->aButtonState) {
        case A_BTN_STATE_1:
            interfaceCtx->aButtonRoll += 10466.667f;
            if (interfaceCtx->aButtonRoll >= 15700.0f) {
                interfaceCtx->aButtonRoll = -15700.0f;
                interfaceCtx->aButtonState = A_BTN_STATE_2;

                if ((msgCtx->msgMode != 0) && (msgCtx->unk12006 == 0x26)) {
                    XREG(31) = -14;
                } else {
                    XREG(31) = 0;
                }
            }
            break;

        case A_BTN_STATE_2:
            interfaceCtx->aButtonRoll += 10466.667f;
            if (interfaceCtx->aButtonRoll >= 0.0f) {
                interfaceCtx->aButtonRoll = 0.0f;
                interfaceCtx->aButtonState = A_BTN_STATE_0;
                interfaceCtx->aButtonHorseDoAction = interfaceCtx->aButtonDoAction;
                aButtonDoAction = interfaceCtx->aButtonHorseDoAction;
                if ((aButtonDoAction == DO_ACTION_MAX) || (aButtonDoAction == DO_ACTION_MAX + 1)) {
                    aButtonDoAction = DO_ACTION_NONE;
                }
                Interface_LoadAButtonDoActionLabel(&play->interfaceCtx, aButtonDoAction, 0);
            }
            break;

        case A_BTN_STATE_3:
            interfaceCtx->aButtonRoll += 10466.667f;
            if (interfaceCtx->aButtonRoll >= 15700.0f) {
                interfaceCtx->aButtonRoll = -15700.0f;
                interfaceCtx->aButtonState = A_BTN_STATE_2;
            }
            break;

        case A_BTN_STATE_4:
            interfaceCtx->aButtonRoll += 10466.667f;
            if (interfaceCtx->aButtonRoll >= 0.0f) {
                interfaceCtx->aButtonRoll = 0.0f;
                interfaceCtx->aButtonState = A_BTN_STATE_0;
                interfaceCtx->aButtonHorseDoAction = interfaceCtx->aButtonDoAction;
                aButtonDoAction = interfaceCtx->aButtonHorseDoAction;
                if ((aButtonDoAction == DO_ACTION_MAX) || (aButtonDoAction == DO_ACTION_MAX + 1)) {
                    aButtonDoAction = DO_ACTION_NONE;
                }

                Interface_LoadAButtonDoActionLabel(&play->interfaceCtx, aButtonDoAction, 0);
            }
            break;

        default: // A_BTN_STATE_0
            break;
    }

    // Update Magic
    if (!(player->stateFlags1 & PLAYER_STATE1_200)) {
        if (R_MAGIC_DBG_SET_UPGRADE == MAGIC_DBG_SET_UPGRADE_DOUBLE_METER) {
            // Upgrade to double magic
            if (!gSaveContext.save.playerData.isMagicAcquired) {
                gSaveContext.save.playerData.isMagicAcquired = true;
            }
            gSaveContext.save.playerData.isDoubleMagicAcquired = true;
            gSaveContext.save.playerData.magic = MAGIC_DOUBLE_METER;
            gSaveContext.save.playerData.magicLevel = 0;
            R_MAGIC_DBG_SET_UPGRADE = MAGIC_DBG_SET_UPGRADE_NO_ACTION;
        } else if (R_MAGIC_DBG_SET_UPGRADE == MAGIC_DBG_SET_UPGRADE_NORMAL_METER) {
            // Upgrade to normal magic
            if (!gSaveContext.save.playerData.isMagicAcquired) {
                gSaveContext.save.playerData.isMagicAcquired = true;
            }
            gSaveContext.save.playerData.isDoubleMagicAcquired = false;
            gSaveContext.save.playerData.magic = MAGIC_NORMAL_METER;
            gSaveContext.save.playerData.magicLevel = 0;
            R_MAGIC_DBG_SET_UPGRADE = MAGIC_DBG_SET_UPGRADE_NO_ACTION;
        }

        if ((gSaveContext.save.playerData.isMagicAcquired) && (gSaveContext.save.playerData.magicLevel == 0)) {
            // Prepare to step `magicCapacity` to full capacity
            gSaveContext.save.playerData.magicLevel = gSaveContext.save.playerData.isDoubleMagicAcquired + 1;
            gSaveContext.magicFillTarget = gSaveContext.save.playerData.magic;
            gSaveContext.save.playerData.magic = 0;
            gSaveContext.magicState = MAGIC_STATE_STEP_CAPACITY;
            BUTTON_ITEM_EQUIP(PLAYER_FORM_DEKU, EQUIP_SLOT_B) = ITEM_NUT;
        }

        Magic_Update(play);
        Magic_UpdateAddRequest();
    }

    // Update Env Timer Part 2
    if (gSaveContext.timerStates[TIMER_ID_ENV] == TIMER_STATE_OFF) {
        if ((sEnvTimerType == PLAYER_ENV_TIMER_HOTROOM) || (sEnvTimerType == PLAYER_ENV_TIMER_UNDERWATER_FREE)) {
            if (CUR_FORM != PLAYER_FORM_ZORA) {
                if (play->gameOverCtx.state == GAMEOVER_INACTIVE) {
                    if ((gSaveContext.save.playerData.health >> 1) != 0) {
                        gSaveContext.timerStates[TIMER_ID_ENV] = TIMER_STATE_ENV_START;
                        gSaveContext.timerX[TIMER_ID_ENV] = 115;
                        gSaveContext.timerY[TIMER_ID_ENV] = 80;
                        sEnvTimerActive = true;
                        gSaveContext.timerDirections[TIMER_ID_ENV] = TIMER_COUNT_DOWN;
                    }
                }
            }
        }
    } else if (((sEnvTimerType == PLAYER_ENV_TIMER_NONE) || (sEnvTimerType == PLAYER_ENV_TIMER_SWIMMING)) &&
               (gSaveContext.timerStates[TIMER_ID_ENV] <= TIMER_STATE_COUNTING)) {
        gSaveContext.timerStates[TIMER_ID_ENV] = TIMER_STATE_OFF;
    }

    // Update Minigame
    if (gSaveContext.minigameStatus == MINIGAME_STATUS_ACTIVE) {
        gSaveContext.minigameScore += interfaceCtx->minigamePoints;
        gSaveContext.minigameHiddenScore += interfaceCtx->minigameHiddenPoints;
        interfaceCtx->minigamePoints = 0;
        interfaceCtx->minigameHiddenPoints = 0;

        // Update horseback archery tier, unused remnant of OoT.
        if (sHBAScoreTier == 0) {
            if (gSaveContext.minigameScore >= 1000) {
                sHBAScoreTier++;
            }
        } else if (sHBAScoreTier == 1) {
            if (gSaveContext.minigameScore >= 1500) {
                sHBAScoreTier++;
            }
        }

        // Get minigame digits
        sMinigameScoreDigits[0] = sMinigameScoreDigits[1] = 0;
        sMinigameScoreDigits[2] = 0;
        sMinigameScoreDigits[3] = gSaveContext.minigameScore;

        while (sMinigameScoreDigits[3] >= 1000) {
            sMinigameScoreDigits[0]++;
            sMinigameScoreDigits[3] -= 1000;
        }

        while (sMinigameScoreDigits[3] >= 100) {
            sMinigameScoreDigits[1]++;
            sMinigameScoreDigits[3] -= 100;
        }

        while (sMinigameScoreDigits[3] >= 10) {
            sMinigameScoreDigits[2]++;
            sMinigameScoreDigits[3] -= 10;
        }
    }

    // Update Sun Song
    if (gSaveContext.sunsSongState != SUNSSONG_INACTIVE) {
        // exit out of ocarina mode after suns song finishes playing
        if ((msgCtx->ocarinaAction != 0x39) && (gSaveContext.sunsSongState == SUNSSONG_START)) {
            play->msgCtx.ocarinaMode = 4;
        }

        // handle suns song in areas where time moves
        if (play->envCtx.sceneTimeSpeed != 0) {
            if (gSaveContext.sunsSongState != SUNSSONG_SPEED_TIME) {
                sIsSunsPlayedAtDay = false;
                if ((gSaveContext.save.time >= CLOCK_TIME(6, 0)) && (gSaveContext.save.time <= CLOCK_TIME(18, 0))) {
                    sIsSunsPlayedAtDay = true;
                }

                gSaveContext.sunsSongState = SUNSSONG_SPEED_TIME;
                sPrevTimeSpeed = R_TIME_SPEED;
                R_TIME_SPEED = 400;
            } else if (!sIsSunsPlayedAtDay) {
                // Nighttime
                if ((gSaveContext.save.time >= CLOCK_TIME(6, 0)) && (gSaveContext.save.time <= CLOCK_TIME(18, 0))) {
                    // Daytime has been reached. End suns song effect
                    gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
                    R_TIME_SPEED = sPrevTimeSpeed;
                    play->msgCtx.ocarinaMode = 4;
                }
            } else {
                // Daytime
                if (gSaveContext.save.time > CLOCK_TIME(18, 0)) {
                    // Nighttime has been reached. End suns song effect
                    gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
                    R_TIME_SPEED = sPrevTimeSpeed;
                    play->msgCtx.ocarinaMode = 4;
                }
            }
        } else {
            gSaveContext.sunsSongState = SUNSSONG_SPECIAL;
        }
    }

    Interface_UpdateBottleTimers(play);

    // Update Grandma's Story
    if (interfaceCtx->storyState != STORY_STATE_OFF) {
        if (interfaceCtx->storyState == STORY_STATE_INIT) {
            interfaceCtx->storyDmaStatus = STORY_DMA_IDLE;
            interfaceCtx->storyState = STORY_STATE_IDLE;
            R_STORY_FILL_SCREEN_ALPHA = 0;
        }

        Interface_AllocStory(play);

        if (interfaceCtx->storyState == STORY_STATE_DESTROY) {
            interfaceCtx->storyState = STORY_STATE_OFF;
            interfaceCtx->storyDmaStatus = STORY_DMA_IDLE;
            if (interfaceCtx->storySegment != NULL) {
                ZeldaArena_Free(interfaceCtx->storySegment);
                interfaceCtx->storySegment = NULL;
            }
        } else if (interfaceCtx->storyState == STORY_STATE_SETUP_IDLE) {
            interfaceCtx->storyState = STORY_STATE_IDLE;
        } else if (interfaceCtx->storyState == STORY_STATE_FADE_OUT) {
            R_STORY_FILL_SCREEN_ALPHA += 10;
            if (R_STORY_FILL_SCREEN_ALPHA >= 250) {
                R_STORY_FILL_SCREEN_ALPHA = 255;
                interfaceCtx->storyState = STORY_STATE_IDLE;
            }
        } else if (interfaceCtx->storyState == STORY_STATE_FADE_IN) {
            R_STORY_FILL_SCREEN_ALPHA -= 10;
            if (R_STORY_FILL_SCREEN_ALPHA < 0) {
                R_STORY_FILL_SCREEN_ALPHA = 0;
                interfaceCtx->storyState = STORY_STATE_IDLE;
            }
        }
    }
}

void Interface_Destroy(PlayState* play) {
    Map_Destroy(play);
    func_80174F9C(Interface_PostmanTimerCallback, NULL);
}

void Interface_Init(PlayState* play) {
    s32 pad;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    size_t parameterStaticSize;
    s32 i;

    bzero(interfaceCtx, sizeof(InterfaceContext));

    gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
    gSaveContext.nextHudVisibility = HUD_VISIBILITY_IDLE;
    gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;

    View_Init(&interfaceCtx->view, play->state.gfxCtx);

    interfaceCtx->magicConsumptionTimer = 16;
    interfaceCtx->healthTimer = 200;

    parameterStaticSize = SEGMENT_ROM_SIZE(parameter_static);
    interfaceCtx->parameterSegment = THA_AllocEndAlign16(&play->state.heap, parameterStaticSize);
    DmaMgr_SendRequest0(interfaceCtx->parameterSegment, SEGMENT_ROM_START(parameter_static), parameterStaticSize);

    interfaceCtx->doActionSegment = THA_AllocEndAlign16(&play->state.heap, 0xC90);
    DmaMgr_SendRequest0(interfaceCtx->doActionSegment, SEGMENT_ROM_START(do_action_static), 0x300);
    DmaMgr_SendRequest0(interfaceCtx->doActionSegment + 0x300, SEGMENT_ROM_START(do_action_static) + 0x480, 0x180);

    Interface_NewDay(play, CURRENT_DAY);

    interfaceCtx->iconItemSegment = THA_AllocEndAlign16(&play->state.heap, 0x4000);

    if (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) < 0xF0) {
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
    } else if ((BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_NONE) &&
               (BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) != ITEM_FD)) {
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
    }

    if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) < 0xF0) {
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_LEFT);
    }

    if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) < 0xF0) {
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_DOWN);
    }

    if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) < 0xF0) {
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_RIGHT);
    }

    if (((gSaveContext.timerStates[TIMER_ID_MINIGAME_2] == TIMER_STATE_COUNTING) ||
         (gSaveContext.timerStates[TIMER_ID_GORON_RACE_UNUSED] == TIMER_STATE_COUNTING)) &&
        ((gSaveContext.respawnFlag == -1) || (gSaveContext.respawnFlag == 1)) &&
        (gSaveContext.timerStates[TIMER_ID_MINIGAME_2] == TIMER_STATE_COUNTING)) {
        gSaveContext.timerStates[TIMER_ID_MINIGAME_2] = TIMER_STATE_START;
        gSaveContext.timerX[TIMER_ID_MINIGAME_2] = 115;
        gSaveContext.timerY[TIMER_ID_MINIGAME_2] = 80;
    }

    LifeMeter_Init(play);
    Map_Init(play);

    gSaveContext.minigameStatus = MINIGAME_STATUS_DONE;
    interfaceCtx->minigamePerfectPrimColor[0] = 255;
    interfaceCtx->minigamePerfectPrimColor[1] = 165;
    interfaceCtx->minigamePerfectPrimColor[2] = 55;

    if (gSaveContext.eventInf[3] & 0x10) {
        gSaveContext.eventInf[3] &= (u8)~0x10;
        gSaveContext.timerStates[TIMER_ID_MINIGAME_2] = TIMER_STATE_OFF;
    }

    gSaveContext.timerStates[TIMER_ID_MINIGAME_1] = TIMER_STATE_OFF;
    gSaveContext.timerCurTimes[TIMER_ID_MINIGAME_1] = SECONDS_TO_TIMER(0);
    gSaveContext.timerTimeLimits[TIMER_ID_MINIGAME_1] = SECONDS_TO_TIMER(0);
    gSaveContext.timerStartOsTimes[TIMER_ID_MINIGAME_1] = 0;
    gSaveContext.timerStopTimes[TIMER_ID_MINIGAME_1] = 0;
    gSaveContext.timerPausedOsTimes[TIMER_ID_MINIGAME_1] = 0;

    for (i = 0; i < TIMER_ID_MAX; i++) {
        if (gSaveContext.timerStates[i] == TIMER_STATE_7) {
            gSaveContext.timerStates[i] = TIMER_STATE_OFF;
        }
    }

    sPictographState = PICTOGRAPH_STATE_OFF;
    sPictographPhotoBeingTaken = false;

    if ((play->sceneId != SCENE_MITURIN_BS) && (play->sceneId != SCENE_HAKUGIN_BS) && (play->sceneId != SCENE_SEA_BS) &&
        (play->sceneId != SCENE_INISIE_BS) && (play->sceneId != SCENE_LAST_BS) && (play->sceneId != SCENE_MITURIN) &&
        (play->sceneId != SCENE_HAKUGIN) && (play->sceneId != SCENE_SEA) && (play->sceneId != SCENE_INISIE_N) &&
        (play->sceneId != SCENE_INISIE_R) && (play->sceneId != SCENE_LAST_DEKU) &&
        (play->sceneId != SCENE_LAST_GORON) && (play->sceneId != SCENE_LAST_ZORA) &&
        (play->sceneId != SCENE_LAST_LINK)) {

        gSaveContext.eventInf[5] &= (u8)~0x8;
        gSaveContext.eventInf[5] &= (u8)~0x10;
        gSaveContext.eventInf[5] &= (u8)~0x20;
        gSaveContext.eventInf[5] &= (u8)~0x40;
        gSaveContext.eventInf[5] &= (u8)~0x80;
        gSaveContext.eventInf[6] &= (u8)~0x1;
        gSaveContext.eventInf[6] &= (u8)~0x2;
        gSaveContext.eventInf[6] &= (u8)~0x4;
        gSaveContext.eventInf[6] &= (u8)~0x8;
    }

    sFinalHoursClockDigitsRed = sFinalHoursClockFrameEnvRed = sFinalHoursClockFrameEnvGreen =
        sFinalHoursClockFrameEnvBlue = 0;
    sFinalHoursClockColorTimer = 15;
    sFinalHoursClockColorTargetIndex = 0;
}
