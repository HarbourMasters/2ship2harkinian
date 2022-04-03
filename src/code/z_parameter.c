#include "global.h"
#include "interface/parameter_static/parameter_static.h"

void Map_Update(GlobalContext* globalCtx);
u8 func_800FE4A8();
s32 func_801234D4(GlobalContext* globalCtx);
s16 func_801242DC(GlobalContext*); // s32 func_8008F2F8 (OoT)
void func_801663C4(u8* arg0, u8* arg1, u32 arg2);

extern Gfx D_0E0001C8[]; // Display List
extern Gfx D_0E0002E0[]; // Display List
extern u8 D_801ABAB0[];
extern u8 D_801E3BB0[];

/**
 * Steps `var` towards `target` linearly, so that it will arrive in `timeRemaining` seconds. Can be used from either
 * direction. `timeRemaining` must be decremented separately for this to work properly, and obviously timeRemaining = 0
 * must be handled separately.
 *
 * @param var Variable to step.
 * @param target Target to step towards.
 * @param timeRemaining Number of times this function should be run for `var` to reach `target`
 * @param stepVar Variable to use for the step (required to match).
 *
 * The progression is not quite linear because of truncation in the division, but the variable will always reach
 * `target` at the appropriate time since the last step is always the full difference.
 */
#define TIMED_STEP_TO(var, target, timeRemaining, stepVar) \
    {                                                      \
        stepVar = ABS_ALT(var - target) / timeRemaining;   \
        if (var >= target) {                               \
            var -= stepVar;                                \
        } else {                                           \
            var += stepVar;                                \
        }                                                  \
    }                                                      \
    (void)0

typedef struct {
    /* 0x00 */ u8 scene;
    /* 0x01 */ u8 flags1;
    /* 0x02 */ u8 flags2;
    /* 0x03 */ u8 flags3;
} RestrictionFlags;

// bss
Input D_801F5850[4];

// data
static RestrictionFlags sRestrictionFlags[] = {
    { SCENE_20SICHITAI2, 0x00, 0x00, 0x00 },
    { SCENE_UNSET_1, 0x00, 0x00, 0x00 },
    { SCENE_UNSET_2, 0x00, 0x00, 0x00 },
    { SCENE_UNSET_3, 0x00, 0x00, 0x00 },
    { SCENE_UNSET_4, 0x00, 0x00, 0x00 },
    { SCENE_UNSET_5, 0x00, 0x00, 0x00 },
    { SCENE_UNSET_6, 0x00, 0x00, 0x00 },
    { SCENE_KAKUSIANA, 0x00, 0x00, 0x00 },
    { SCENE_SPOT00, 0x00, 0x00, 0x00 },
    { SCENE_UNSET_9, 0x00, 0x00, 0x00 },
    { SCENE_WITCH_SHOP, 0x10, 0x30, 0xC1 },
    { SCENE_LAST_BS, 0x00, 0x3F, 0x00 },
    { SCENE_HAKASHITA, 0x00, 0x00, 0x00 },
    { SCENE_AYASHIISHOP, 0x10, 0x30, 0xC1 },
    { SCENE_UNSET_E, 0x00, 0x00, 0x00 },
    { SCENE_UNSET_F, 0x00, 0x00, 0x00 },
    { SCENE_OMOYA, 0x00, 0x33, 0x00 },
    { SCENE_BOWLING, 0x10, 0x30, 0xC1 },
    { SCENE_SONCHONOIE, 0x10, 0x30, 0xC1 },
    { SCENE_IKANA, 0x00, 0x00, 0x00 },
    { SCENE_KAIZOKU, 0x00, 0x00, 0x00 },
    { SCENE_MILK_BAR, 0x10, 0x30, 0xC1 },
    { SCENE_INISIE_N, 0x00, 0x00, 0x00 },
    { SCENE_TAKARAYA, 0x10, 0x30, 0xC1 },
    { SCENE_INISIE_R, 0x00, 0x00, 0x00 },
    { SCENE_OKUJOU, 0x00, 0x3F, 0xF0 },
    { SCENE_OPENINGDAN, 0x00, 0x00, 0x00 },
    { SCENE_MITURIN, 0x00, 0x00, 0x00 },
    { SCENE_13HUBUKINOMITI, 0x00, 0x00, 0x00 },
    { SCENE_CASTLE, 0x00, 0x00, 0x00 },
    { SCENE_DEKUTES, 0x10, 0x30, 0x01 },
    { SCENE_MITURIN_BS, 0x00, 0x00, 0x00 },
    { SCENE_SYATEKI_MIZU, 0x10, 0x30, 0xC1 },
    { SCENE_HAKUGIN, 0x00, 0x00, 0x00 },
    { SCENE_ROMANYMAE, 0x00, 0x00, 0x00 },
    { SCENE_PIRATE, 0x00, 0x00, 0x00 },
    { SCENE_SYATEKI_MORI, 0x10, 0x30, 0xC1 },
    { SCENE_SINKAI, 0x00, 0x00, 0x00 },
    { SCENE_YOUSEI_IZUMI, 0x00, 0x00, 0x00 },
    { SCENE_KINSTA1, 0x00, 0x30, 0x00 },
    { SCENE_KINDAN2, 0x00, 0x30, 0x00 },
    { SCENE_TENMON_DAI, 0x00, 0x03, 0xC0 },
    { SCENE_LAST_DEKU, 0x00, 0x3F, 0x00 },
    { SCENE_22DEKUCITY, 0x00, 0x00, 0x00 },
    { SCENE_KAJIYA, 0x00, 0x30, 0xC0 },
    { SCENE_00KEIKOKU, 0x00, 0x00, 0x00 },
    { SCENE_POSTHOUSE, 0x00, 0x30, 0xC1 },
    { SCENE_LABO, 0x00, 0x30, 0xC1 },
    { SCENE_DANPEI2TEST, 0x00, 0x30, 0x00 },
    { SCENE_UNSET_31, 0x00, 0x00, 0x00 },
    { SCENE_16GORON_HOUSE, 0x00, 0x00, 0x00 },
    { SCENE_33ZORACITY, 0x00, 0x00, 0xC0 },
    { SCENE_8ITEMSHOP, 0x00, 0x30, 0xC1 },
    { SCENE_F01, 0x00, 0x00, 0x00 },
    { SCENE_INISIE_BS, 0x00, 0x00, 0x00 },
    { SCENE_30GYOSON, 0x00, 0x00, 0x00 },
    { SCENE_31MISAKI, 0x00, 0x00, 0x00 },
    { SCENE_TAKARAKUJI, 0x00, 0x30, 0xC1 },
    { SCENE_UNSET_3A, 0x00, 0x00, 0x00 },
    { SCENE_TORIDE, 0x00, 0x00, 0x00 },
    { SCENE_FISHERMAN, 0x00, 0x30, 0xC1 },
    { SCENE_GORONSHOP, 0x10, 0x30, 0xC1 },
    { SCENE_DEKU_KING, 0x00, 0x30, 0xC0 },
    { SCENE_LAST_GORON, 0x00, 0x3F, 0x00 },
    { SCENE_24KEMONOMITI, 0x00, 0x00, 0x00 },
    { SCENE_F01_B, 0x00, 0x30, 0x00 },
    { SCENE_F01C, 0x00, 0x30, 0x00 },
    { SCENE_BOTI, 0x00, 0x00, 0x00 },
    { SCENE_HAKUGIN_BS, 0x00, 0x00, 0x00 },
    { SCENE_20SICHITAI, 0x00, 0x00, 0x00 },
    { SCENE_21MITURINMAE, 0x00, 0x00, 0x00 },
    { SCENE_LAST_ZORA, 0x00, 0x3F, 0x00 },
    { SCENE_11GORONNOSATO2, 0x00, 0x00, 0x00 },
    { SCENE_SEA, 0x00, 0x00, 0x00 },
    { SCENE_35TAKI, 0x00, 0x00, 0x00 },
    { SCENE_REDEAD, 0x00, 0x00, 0x00 },
    { SCENE_BANDROOM, 0x00, 0x30, 0xC0 },
    { SCENE_11GORONNOSATO, 0x00, 0x00, 0x00 },
    { SCENE_GORON_HAKA, 0x00, 0x00, 0x00 },
    { SCENE_SECOM, 0x00, 0x3C, 0xC0 },
    { SCENE_10YUKIYAMANOMURA, 0x00, 0x00, 0x00 },
    { SCENE_TOUGITES, 0x00, 0x3F, 0xF0 },
    { SCENE_DANPEI, 0x00, 0x30, 0x00 },
    { SCENE_IKANAMAE, 0x00, 0x00, 0x00 },
    { SCENE_DOUJOU, 0x00, 0x30, 0xC1 },
    { SCENE_MUSICHOUSE, 0x00, 0x30, 0xC0 },
    { SCENE_IKNINSIDE, 0x00, 0x3F, 0xC0 },
    { SCENE_MAP_SHOP, 0x10, 0x30, 0xC1 },
    { SCENE_F40, 0x00, 0x00, 0x00 },
    { SCENE_F41, 0x00, 0x00, 0x00 },
    { SCENE_10YUKIYAMANOMURA2, 0x00, 0x00, 0x00 },
    { SCENE_14YUKIDAMANOMITI, 0x00, 0x00, 0x00 },
    { SCENE_12HAKUGINMAE, 0x00, 0x00, 0x00 },
    { SCENE_17SETUGEN, 0x00, 0x00, 0x00 },
    { SCENE_17SETUGEN2, 0x00, 0x00, 0x00 },
    { SCENE_SEA_BS, 0x00, 0x00, 0x00 },
    { SCENE_RANDOM, 0x00, 0x00, 0x00 },
    { SCENE_YADOYA, 0x10, 0x30, 0xC1 },
    { SCENE_KONPEKI_ENT, 0x00, 0x00, 0x00 },
    { SCENE_INSIDETOWER, 0x00, 0xFF, 0xC0 },
    { SCENE_26SARUNOMORI, 0x00, 0x30, 0x00 },
    { SCENE_LOST_WOODS, 0x00, 0x00, 0x00 },
    { SCENE_LAST_LINK, 0x00, 0x3F, 0x00 },
    { SCENE_SOUGEN, 0x00, 0x3F, 0x00 },
    { SCENE_BOMYA, 0x10, 0x30, 0xC1 },
    { SCENE_KYOJINNOMA, 0x00, 0x00, 0x00 },
    { SCENE_KOEPONARACE, 0x00, 0x30, 0x00 },
    { SCENE_GORONRACE, 0x00, 0x00, 0x00 },
    { SCENE_TOWN, 0x00, 0x00, 0x00 },
    { SCENE_ICHIBA, 0x00, 0x00, 0x00 },
    { SCENE_BACKTOWN, 0x00, 0x00, 0x00 },
    { SCENE_CLOCKTOWER, 0x00, 0x00, 0x00 },
    { SCENE_ALLEY, 0x00, 0x00, 0x00 },
};

s16 D_801BF884 = 0; // pictoBox related

s32 D_801BF888 = 0;

s16 sHBAScoreTier = 0;

u16 sMinigameScoreDigits[] = { 0, 0, 0, 0 };

s16 D_801BF898 = 0;
s16 D_801BF89C = 0;

s16 sMagicBarOutlinePrimRed = 255;
s16 sMagicBarOutlinePrimGreen = 255;
s16 sMagicBarOutlinePrimBlue = 255;

s16 D_801BF8AC = 2; // sMagicBorderRatio

s16 D_801BF8B0 = 1;

s16 sExtraItemBases[] = {
    ITEM_STICK,   ITEM_STICK, ITEM_NUT,   ITEM_NUT, ITEM_BOMB,    ITEM_BOMB,    ITEM_BOMB,
    ITEM_BOMB,    ITEM_BOW,   ITEM_BOW,   ITEM_BOW, ITEM_BOMBCHU, ITEM_BOMBCHU, ITEM_BOMBCHU,
    ITEM_BOMBCHU, ITEM_STICK, ITEM_STICK, ITEM_NUT, ITEM_NUT,
};

s16 D_801BF8DC = 0;

s16 D_801BF8E0 = 0;

s16 D_801BF8E4 = 0;

s32 D_801BF8E8[] = {
    0x00000000,
    0x00000000,
};

OSTime D_801BF8F0 = 0;

s32 D_801BF8F8 = 0;

s32 D_801BF8FC[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

s32 D_801BF930 = 0;

s32 D_801BF934[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

s32 D_801BF968 = 0;

s32 D_801BF96C = 0;

s16 D_801BF970 = 99;

s16 D_801BF974 = 0;

s16 D_801BF978 = 10;

s16 D_801BF97C = 255;

f32 D_801BF980 = 1.0f;

s32 D_801BF984 = 0;

// Display List
Gfx D_801BF988[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetOtherMode(G_AD_DISABLE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_1PRIMITIVE,
                     G_AC_NONE | G_ZS_PIXEL | G_RM_CLD_SURF | G_RM_CLD_SURF2),
    gsDPSetCombineMode(G_CC_PRIMITIVE, G_CC_PRIMITIVE),
    gsSPEndDisplayList(),
};

s32 D_801BF9B0 = 0;

s32 D_801BF9B4[] = {
    0x42C80000, // f32: 100.0
    0x42DA0000, // f32: 109.0
};

s16 D_801BF9BC[] = {
    0x226,
    0x2A8,
    0x2A8,
    0x2A8,
};

s16 D_801BF9C4[] = {
    0x9E,
    0x9B,
};

s16 D_801BF9C8[] = {
    0x17,
    0x16,
};

f32 D_801BF9CC[] = {
    -380.0f,
    -350.0f,
};

s16 D_801BF9D4[] = {
    0xA7,
    0xE3,
};

s16 D_801BF9D8[] = {
    0xF9,
    0x10F,
};

s16 D_801BF9DC[] = {
    0x11,
    0x12,
};

s16 D_801BF9E0[] = {
    0x22,
    0x12,
};

s16 D_801BF9E4[] = {
    0x23F,
    0x26C,
};

s16 D_801BF9E8[] = {
    0x26C,
    0x26C,
};

s16 sFinalHoursClockDigitsRed = 0;
s16 sFinalHoursClockFrameEnvRed = 0;
s16 sFinalHoursClockFrameEnvGreen = 0;
s16 sFinalHoursClockFrameEnvBlue = 0;
s16 sFinalHoursClockColorTimer = 15;
s16 sFinalHoursClockColorTargetIndex = 0;

s16 D_801BFA04[] = {
    0xFFF2, 0xFFF2, 0xFFE8, 0xFFF8, 0xFFF4, 0xFFF4, 0xFFF9, 0xFFF8, 0xFFF9, 0xFFF8, 0xFFF4, 0,
};

s16 D_801BFA1C[] = {
    0x1C, 0x1C, 0x30, 0x10, 0x18, 0x18, 0x10, 0x10, 0x10, 0x10, 0x18, 0,
};

s16 D_801BFA34[] = {
    0xE, 0xE, 0x8, 0x18, 0xFFAE, 0xFFAE, 0x3A, 0x3B, 0x3A, 0x3B, 0x20, 0,
};

s16 D_801BFA4C[] = {
    0x1C, 0x1C, 0x10, 0x10, 0x18, 0x18, 0xB, 0xB, 0xB, 0xB, 0x20, 0,
};

s16 D_801BFA64[] = {
    0xFFC3, 0xFFD3, 0x001D, 0x0068, 0xFF8B, 0xFFD6, 0x0020, 0x0037,
};

s16 D_801BFA74[] = {
    0x0001, 0xFFBA, 0xFF9D, 0xFFBA, 0x0047, 0x0065, 0x0048, 0x0001,
};

s32 D_801BFA84 = 0;

Gfx* Gfx_TextureRGBA16(Gfx* displayListHead, void* texture, s16 textureWidth, s16 textureHeight, s16 rectLeft,
                       s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(displayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(displayListHead++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4,
                        (rectTop + rectHeight) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return displayListHead;
}

Gfx* Gfx_TextureIA8(Gfx* displayListHead, void* texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                    s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(displayListHead++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(displayListHead++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4,
                        (rectTop + rectHeight) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return displayListHead;
}

Gfx* func_8010CFBC(Gfx* displayListHead, void* texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                   s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy, s16 r, s16 g, s16 b, s16 a) {
    s16 temp_var = a;

    if (a > 100) {
        temp_var = 100;
    }

    gDPPipeSync(displayListHead++);
    gDPSetPrimColor(displayListHead++, 0, 0, 0, 0, 0, temp_var);

    gDPLoadTextureBlock(displayListHead++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(displayListHead++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    gDPPipeSync(displayListHead++);
    gDPSetPrimColor(displayListHead++, 0, 0, r, g, b, a);

    gSPTextureRectangle(displayListHead++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4,
                        (rectTop + rectHeight) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return displayListHead;
}

Gfx* func_8010D2D4(Gfx* displayListHead, s16 rectLeft, s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy,
                   s16 r, s16 g, s16 b, s16 a) {
    s16 temp_var = a;

    if (a > 100) {
        temp_var = 100;
    }

    gDPPipeSync(displayListHead++);
    gDPSetPrimColor(displayListHead++, 0, 0, 0, 0, 0, temp_var);
    gSPTextureRectangle(displayListHead++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    gDPPipeSync(displayListHead++);
    gDPSetPrimColor(displayListHead++, 0, 0, r, g, b, a);

    gSPTextureRectangle(displayListHead++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4,
                        (rectTop + rectHeight) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return displayListHead;
}

Gfx* func_8010D480(Gfx* displayListHead, void* texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                   s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy, s16 r, s16 g, s16 b, s16 a, s32 argE, s32 argF) {
    s16 temp_var = a;

    if (a > 100) {
        temp_var = 100;
    }

    gDPPipeSync(displayListHead++);
    gDPSetPrimColor(displayListHead++, 0, 0, 0, 0, 0, temp_var);

    gDPLoadTextureBlock(displayListHead++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_MIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, argE, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(displayListHead++, (rectLeft + 2) * 4, (rectTop + 2) * 4, (rectLeft + rectWidth + 2) * 4,
                        (rectTop + rectHeight + 2) * 4, G_TX_RENDERTILE, argF, 0, dsdx, dtdy);

    gDPPipeSync(displayListHead++);
    gDPSetPrimColor(displayListHead++, 0, 0, r, g, b, a);

    gSPTextureRectangle(displayListHead++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4,
                        (rectTop + rectHeight) * 4, G_TX_RENDERTILE, argF, 0, dsdx, dtdy);

    return displayListHead;
}

Gfx* Gfx_TextureI8(Gfx* displayListHead, void* texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                   s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock(displayListHead++, texture, G_IM_FMT_I, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSPTextureRectangle(displayListHead++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4,
                        (rectTop + rectHeight) * 4, G_TX_RENDERTILE, 0, 0, dsdx, dtdy);

    return displayListHead;
}

Gfx* func_8010D9F4(Gfx* displayListHead, void* texture, s32 fmt, s16 textureWidth, s16 textureHeight, s16 rectLeft,
                   s16 rectTop, s16 rectWidth, s16 rectHeight, s32 cms, s32 masks, s32 s, u16 dsdx, u16 dtdy) {
    gDPLoadTextureBlock_4b(displayListHead++, texture, fmt, textureWidth, textureHeight, 0, cms,
                           G_TX_NOMIRROR | G_TX_WRAP, masks, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(displayListHead++, rectLeft * 4, rectTop * 4, (rectLeft + rectWidth) * 4,
                        (rectTop + rectHeight) * 4, G_TX_RENDERTILE, s, 0, dsdx, dtdy);

    return displayListHead;
}

Gfx* func_8010DC58(Gfx* displayListHead, void* texture, s16 textureWidth, s16 textureHeight, u16 i) {
    gDPLoadTextureBlock(displayListHead++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, textureWidth, textureHeight, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    gSP1Quadrangle(displayListHead++, i, i + 2, i + 3, i + 1, 0);

    return displayListHead;
}

Gfx* func_8010DE38(Gfx* displayListHead, void* texture, s32 fmt, s16 textureWidth, s16 textureHeight, u16 i) {
    gDPLoadTextureBlock_4b(displayListHead++, texture, fmt, textureWidth, textureHeight, 0, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSP1Quadrangle(displayListHead++, i, i + 2, i + 3, i + 1, 0);

    return displayListHead;
}

void Interface_InitVertices(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_InitVertices.s")

void func_8010E968(s32 arg0) {
    s32 btnAPressed;

    func_80175E68(&D_801F5850[0], 0);
    btnAPressed = CHECK_BTN_ALL(D_801F5850[0].cur.button, BTN_A);
    if ((btnAPressed != D_801BFA84) && btnAPressed) {
        gSaveContext.unk_3DC8 = osGetTime();
        gSaveContext.unk_3DD0[0] = 15;
    }

    D_801BFA84 = btnAPressed;
}

void func_8010E9F0(s16 timerId, s16 timeLimit) {
    gSaveContext.timerX[timerId] = 115;
    gSaveContext.timerY[timerId] = 80;

    D_801BF8E0 = 0;

    gSaveContext.unk_3DE0[timerId] = timeLimit * 100;
    gSaveContext.unk_3E18[timerId] = gSaveContext.unk_3DE0[timerId];

    if (gSaveContext.unk_3DE0[timerId] != 0) {
        gSaveContext.timersNoTimeLimit[timerId] = false;
    } else {
        gSaveContext.timersNoTimeLimit[timerId] = true;
    }

    gSaveContext.unk_3DD0[timerId] = 1;
}

void func_8010EA9C(s16 timeLimit, s16 arg1) {
    gSaveContext.timerX[0] = 115;
    gSaveContext.timerY[0] = 80;

    D_801BF8E4 = arg1;

    gSaveContext.unk_3DE0[0] = timeLimit * 100;
    gSaveContext.unk_3E18[0] = gSaveContext.unk_3DE0[0];

    if (gSaveContext.unk_3DE0[0] != 0) {
        gSaveContext.timersNoTimeLimit[0] = false;
    } else {
        gSaveContext.timersNoTimeLimit[0] = true;
    }
    gSaveContext.unk_3DD0[0] = 13;

    gSaveContext.unk_3E88[0] = 0;
    gSaveContext.unk_3EC0[0] = 0;
}

void func_8010EB50(s32 arg0) {
    if (gSaveContext.unk_3DD0[6] != 0) {
        // Goron race started
        if (gSaveContext.eventInf[1] & 1) {
            gSaveContext.unk_3DE0[6] = 239;
        } else {
            gSaveContext.unk_3DE0[6] = 1;
        }
    }
}

void func_8010EBA0(s16 timer, s16 timerId) {
    gSaveContext.unk_101A[timerId] = 1;
    gSaveContext.unk_1080[timerId] = timer * 100;
    gSaveContext.unk_1050[timerId] = gSaveContext.unk_1080[timerId];
    gSaveContext.unk_1020[timerId] = osGetTime();
    gSaveContext.unk_10B0[timerId] = 0;
    D_801BF8F0 = 0;
}

// Crazy u64 math
#ifdef NON_EQUIVALENT
s32 func_8010EC54(s16 timerId) {
    u64 time;
    s16 sp22;
    s16 sp20;
    s16 sp1E;
    s16 sp1C;
    s16 temp_a0;
    s16 final;

    time = gSaveContext.unk_3DE0[timerId];

    // hours
    time -= (sp1C = time / 36000) * 36000;
    if (1) {}

    // minutes
    time -= (sp1E = time / 6000) * 6000;

    // seconds
    time -= (sp20 = time / 1000) * 1000;

    // 100 milliseconds
    time -= (sp22 = time / 100) * 100;

    // 10 miliseconds
    time -= (temp_a0 = time / 10) * 10;

    final = (sp1C << 0x14);

    return time | final | (sp1E << 0x10) | (sp20 << 0xC) | (sp22 << 8) | (temp_a0 << 4);
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8010EC54.s")
#endif

void Interface_NewDay(GlobalContext* globalCtx, s32 day) {
    s32 pad;
    s16 i = day - 1;

    // i is used to store dayMinusOne
    if (i < 0 || i >= 3) {
        i = 0;
    }

    // Loads day number from week_static for the three-day clock
    DmaMgr_SendRequest0((u32)globalCtx->interfaceCtx.doActionSegment + 0x780,
                        (u32)_week_staticSegmentRomStart + i * 0x510, 0x510);

    // i is used to store sceneId
    for (i = 0; i < 120; i++) {
        gSaveContext.save.permanentSceneFlags[i].chest = gSaveContext.cycleSceneFlags[i].chest;
        gSaveContext.save.permanentSceneFlags[i].swch0 = gSaveContext.cycleSceneFlags[i].swch0;
        gSaveContext.save.permanentSceneFlags[i].swch1 = gSaveContext.cycleSceneFlags[i].swch1;
        gSaveContext.save.permanentSceneFlags[i].clearedRoom = gSaveContext.cycleSceneFlags[i].clearedRoom;
        gSaveContext.save.permanentSceneFlags[i].collectible = gSaveContext.cycleSceneFlags[i].collectible;
    }
}

void Interface_ChangeAlpha(u16 alphaType) {
    if (gSaveContext.unk_3F22 != alphaType) {
        gSaveContext.unk_3F22 = alphaType;
        gSaveContext.unk_3F20 = alphaType;
        gSaveContext.unk_3F24 = 1;
    }
}

/**
 * Nearly identical to func_80082644 from OoT
 */
void func_8010EF9C(GlobalContext* globalCtx, s16 alpha) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    if ((gSaveContext.buttonStatus[0] == BTN_DISABLED) || (gSaveContext.unk_1015 == 0xFF)) {
        if (interfaceCtx->bAlpha != 70) {
            interfaceCtx->bAlpha = 70;
        }
    } else {
        if (interfaceCtx->bAlpha != 255) {
            interfaceCtx->bAlpha = alpha;
        }
    }

    if (gSaveContext.buttonStatus[1] == BTN_DISABLED) {
        if (interfaceCtx->cLeftAlpha != 70) {
            interfaceCtx->cLeftAlpha = 70;
        }
    } else {
        if (interfaceCtx->cLeftAlpha != 255) {
            interfaceCtx->cLeftAlpha = alpha;
        }
    }

    if (gSaveContext.buttonStatus[2] == BTN_DISABLED) {
        if (interfaceCtx->cDownAlpha != 70) {
            interfaceCtx->cDownAlpha = 70;
        }
    } else {
        if (interfaceCtx->cDownAlpha != 255) {
            interfaceCtx->cDownAlpha = alpha;
        }
    }

    if (gSaveContext.buttonStatus[3] == BTN_DISABLED) {
        if (interfaceCtx->cRightAlpha != 70) {
            interfaceCtx->cRightAlpha = 70;
        }
    } else {
        if (interfaceCtx->cRightAlpha != 255) {
            interfaceCtx->cRightAlpha = alpha;
        }
    }

    if (gSaveContext.buttonStatus[4] == BTN_DISABLED) {
        if (interfaceCtx->aAlpha != 70) {
            interfaceCtx->aAlpha = 70;
        }
    } else {
        if (interfaceCtx->aAlpha != 255) {
            interfaceCtx->aAlpha = alpha;
        }
    }
}

/**
 * Identical to func_8008277C from OoT
 */
void func_8010F0D4(GlobalContext* globalCtx, s16 maxAlpha, s16 alpha) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    if (gSaveContext.unk_3F1E != 0) {
        func_8010EF9C(globalCtx, alpha);
        return;
    }

    if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > maxAlpha)) {
        interfaceCtx->bAlpha = maxAlpha;
    }

    if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
        interfaceCtx->aAlpha = maxAlpha;
    }

    if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
        interfaceCtx->cLeftAlpha = maxAlpha;
    }

    if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
        interfaceCtx->cDownAlpha = maxAlpha;
    }

    if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
        interfaceCtx->cRightAlpha = maxAlpha;
    }
}

void func_8010F1A8(GlobalContext* globalCtx, s16 maxAlpha) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 alpha = 255 - maxAlpha;

    switch (gSaveContext.unk_3F20) {
        case 1:
        case 2:
        case 8:
            if (gSaveContext.unk_3F20 == 8) {
                if (interfaceCtx->bAlpha != 255) {
                    interfaceCtx->bAlpha = alpha;
                }
            } else {
                if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > maxAlpha)) {
                    interfaceCtx->bAlpha = maxAlpha;
                }
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > maxAlpha)) {
                interfaceCtx->healthAlpha = maxAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            break;
        case 3:
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            func_8010F0D4(globalCtx, maxAlpha, alpha + 0);

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            break;
        case 4:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > maxAlpha)) {
                interfaceCtx->bAlpha = maxAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > maxAlpha)) {
                interfaceCtx->healthAlpha = maxAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            break;
        case 5:
            func_8010F0D4(globalCtx, maxAlpha, alpha);

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            break;
        case 6:
            func_8010F0D4(globalCtx, maxAlpha, alpha);

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            if (globalCtx->sceneNum == SCENE_SPOT00) {
                if (interfaceCtx->minimapAlpha < 170) {
                    interfaceCtx->minimapAlpha = alpha;
                } else {
                    interfaceCtx->minimapAlpha = 170;
                }
            } else if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = alpha;
            }

            break;
        case 7:
            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            func_8010EF9C(globalCtx, alpha);

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            break;
        case 9:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > maxAlpha)) {
                interfaceCtx->bAlpha = maxAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            break;
        case 10:
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > maxAlpha)) {
                interfaceCtx->healthAlpha = maxAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = alpha;
            }

            break;
        case 11:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > maxAlpha)) {
                interfaceCtx->bAlpha = maxAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            break;
        case 12:
            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            if ((gSaveContext.buttonStatus[0] == BTN_DISABLED) || (gSaveContext.unk_1015 == 0xFF)) {
                if (interfaceCtx->bAlpha != 70) {
                    interfaceCtx->bAlpha = 70;
                }
            } else {
                if (interfaceCtx->bAlpha != 255) {
                    interfaceCtx->bAlpha = alpha;
                }
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = alpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > maxAlpha)) {
                interfaceCtx->healthAlpha = maxAlpha;
            }

            break;
        case 13:
            func_8010F0D4(globalCtx, maxAlpha, alpha);

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            break;
        case 14:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > maxAlpha)) {
                interfaceCtx->bAlpha = maxAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if (interfaceCtx->cLeftAlpha != 255) {
                interfaceCtx->cLeftAlpha = alpha;
            }

            if (interfaceCtx->cDownAlpha != 255) {
                interfaceCtx->cDownAlpha = alpha;
            }

            if (interfaceCtx->cRightAlpha != 255) {
                interfaceCtx->cRightAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            break;
        case 15:
            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = alpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            if (interfaceCtx->cLeftAlpha != 255) {
                interfaceCtx->cLeftAlpha = alpha;
            }

            if (interfaceCtx->cDownAlpha != 255) {
                interfaceCtx->cDownAlpha = alpha;
            }

            if (interfaceCtx->cRightAlpha != 255) {
                interfaceCtx->cRightAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            break;

        case 16:
            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > maxAlpha)) {
                interfaceCtx->healthAlpha = maxAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = alpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            if (interfaceCtx->cLeftAlpha != 255) {
                interfaceCtx->cLeftAlpha = alpha;
            }

            if (interfaceCtx->cDownAlpha != 255) {
                interfaceCtx->cDownAlpha = alpha;
            }

            if (interfaceCtx->cRightAlpha != 255) {
                interfaceCtx->cRightAlpha = alpha;
            }

            break;
        case 17:
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > maxAlpha)) {
                interfaceCtx->healthAlpha = maxAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = alpha;
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = alpha;
            }

            break;
        case 18:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > maxAlpha)) {
                interfaceCtx->bAlpha = maxAlpha;
            }

            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = alpha;
            }

            break;
        case 19:
            if ((interfaceCtx->bAlpha != 0) && (interfaceCtx->bAlpha > maxAlpha)) {
                interfaceCtx->bAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            break;
        case 20:
            if ((interfaceCtx->aAlpha != 0) && (interfaceCtx->aAlpha > maxAlpha)) {
                interfaceCtx->aAlpha = maxAlpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > maxAlpha)) {
                interfaceCtx->healthAlpha = maxAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            break;
        case 21:
            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = alpha;
            }

            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if ((interfaceCtx->minimapAlpha != 0) && (interfaceCtx->minimapAlpha > maxAlpha)) {
                interfaceCtx->minimapAlpha = maxAlpha;
            }

            if ((interfaceCtx->magicAlpha != 0) && (interfaceCtx->magicAlpha > maxAlpha)) {
                interfaceCtx->magicAlpha = maxAlpha;
            }

            if ((interfaceCtx->healthAlpha != 0) && (interfaceCtx->healthAlpha > maxAlpha)) {
                interfaceCtx->healthAlpha = maxAlpha;
            }

            break;
        case 22:
            if ((interfaceCtx->cLeftAlpha != 0) && (interfaceCtx->cLeftAlpha > maxAlpha)) {
                interfaceCtx->cLeftAlpha = maxAlpha;
            }

            if ((interfaceCtx->cDownAlpha != 0) && (interfaceCtx->cDownAlpha > maxAlpha)) {
                interfaceCtx->cDownAlpha = maxAlpha;
            }

            if ((interfaceCtx->cRightAlpha != 0) && (interfaceCtx->cRightAlpha > maxAlpha)) {
                interfaceCtx->cRightAlpha = maxAlpha;
            }

            if (interfaceCtx->bAlpha != 255) {
                interfaceCtx->bAlpha = alpha;
            }

            if (interfaceCtx->aAlpha != 255) {
                interfaceCtx->aAlpha = alpha;
            }

            if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = alpha;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha;
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha;
            }

            break;
    }

    if ((globalCtx->roomCtx.currRoom.unk3 == 1) && (interfaceCtx->minimapAlpha >= 255)) {
        interfaceCtx->minimapAlpha = 255;
    }
}

// oot func_80083108
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80110038.s")

void func_80111CB4(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80111CB4.s")

void Interface_SetSceneRestrictions(GlobalContext* globalCtx) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 i = 0;
    u8 currentScene;

    do {
        currentScene = (u8)globalCtx->sceneNum;
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
            return;
        }
        i++;
    } while (sRestrictionFlags[i].scene != 0xFF);
}

void func_80112AF4(void) {
}

void Interface_InitMinigame(GlobalContext* globalCtx) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    gSaveContext.minigameState = 1;
    gSaveContext.minigameScore = 0;
    gSaveContext.unk_3F3C = 0;

    sHBAScoreTier = 0;
    interfaceCtx->unk_25C = interfaceCtx->unk_25E = interfaceCtx->unk_262 = 0;

    interfaceCtx->hbaAmmo = 20;
}

void Interface_LoadItemIconImpl(GlobalContext* globalCtx, u8 btn) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    func_80178E3C(_icon_item_static_testSegmentRomStart, GET_CUR_FORM_BTN_ITEM(btn),
                  &interfaceCtx->iconItemSegment[(u32)btn * 0x1000], 0x1000);
}

void Interface_LoadItemIcon(GlobalContext* globalCtx, u8 btn) {
    Interface_LoadItemIconImpl(globalCtx, btn);
}

void func_80112C0C(GlobalContext* globalCtx, u16 flag) {
    if (flag) {
        if ((gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_BOW) ||
            (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_BOMB) ||
            (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_BOMBCHU) ||
            (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_FISHING_POLE) ||
            (gSaveContext.buttonStatus[0] == BTN_DISABLED)) {
            if ((gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_BOW) ||
                (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_BOMB) ||
                (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_BOMBCHU) ||
                (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_FISHING_POLE)) {
                gSaveContext.save.equips.buttonItems[CUR_FORM][0] = gSaveContext.buttonStatus[0];
                Interface_LoadItemIconImpl(globalCtx, 0);
            }
        } else if (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_NONE) {
            if (gSaveContext.save.equips.buttonItems[CUR_FORM][0] != ITEM_NONE) {
                gSaveContext.save.equips.buttonItems[CUR_FORM][0] = gSaveContext.buttonStatus[0];
                Interface_LoadItemIconImpl(globalCtx, 0);
            }
        }

        gSaveContext.buttonStatus[3] = gSaveContext.buttonStatus[2] = gSaveContext.buttonStatus[1] =
            gSaveContext.buttonStatus[0] = BTN_ENABLED;
        Interface_ChangeAlpha(7);
    } else {
        gSaveContext.buttonStatus[3] = gSaveContext.buttonStatus[2] = gSaveContext.buttonStatus[1] =
            gSaveContext.buttonStatus[0] = BTN_ENABLED;
        func_80111CB4(globalCtx);
    }
}

#ifdef NON_EQUIVALENT
u8 Item_Give(GlobalContext* globalCtx, u8 item) {
    static s16 sAmmoRefillCounts[] = {
        5, 10, 20, 30, 10, 30, 40, 50, 20, 10, 1, 5, 1, 5, 10, 20, 50, 100, 200, 0,
    };
    Player* player = GET_PLAYER(globalCtx);
    u8 i;
    u8 slot;
    u16 value;
    u8 phi_a2_2;

    slot = SLOT(item);
    if (item >= ITEM_STICKS_5) {
        slot = SLOT(sExtraItemBases[item - ITEM_STICKS_5]);
    }

    if (item == ITEM_SKULL_TOKEN) {
        gSaveContext.save.inventory.questItems |= gBitFlags[item - ITEM_SKULL_TOKEN + QUEST_SKULL_TOKEN];
        func_8012F1BC(globalCtx->sceneNum);
        return ITEM_NONE;

    } else if (item == ITEM_TINGLE_MAP) {
        return ITEM_NONE;

    } else if (item == ITEM_BOMBERS_NOTEBOOK) {
        gSaveContext.save.inventory.questItems |= gBitFlags[QUEST_BOMBERS_NOTEBOOK];
        return ITEM_NONE;

    } else if ((item == ITEM_HEART_PIECE_2) || (item == ITEM_HEART_PIECE)) {
        gSaveContext.save.inventory.questItems += (1 << QUEST_HEART_PIECE);
        if ((gSaveContext.save.inventory.questItems & 0xF0000000) == (1 << QUEST_HEART_CONTAINER)) {
            gSaveContext.save.inventory.questItems ^= (1 << QUEST_HEART_CONTAINER);
            gSaveContext.save.playerData.healthCapacity += 0x10;
            gSaveContext.save.playerData.health += 0x10;
        }
        return ITEM_NONE;

    } else if (item == ITEM_HEART_CONTAINER) {
        gSaveContext.save.playerData.healthCapacity += 0x10;
        gSaveContext.save.playerData.health += 0x10;
        return ITEM_NONE;

    } else if ((item >= ITEM_SONG_SONATA) && (item <= ITEM_SONG_LULLABY_INTRO)) {
        gSaveContext.save.inventory.questItems |= gBitFlags[item - ITEM_SONG_SONATA + QUEST_SONG_SONATA];
        return ITEM_NONE;

    } else if ((item >= ITEM_SWORD_KOKIRI) && (item <= ITEM_SWORD_GILDED)) {
        value = item - ITEM_SWORD_KOKIRI - 1;
        SET_EQUIP_VALUE(EQUIP_SWORD, value);
        gSaveContext.save.equips.buttonItems[CUR_FORM][0] = item;
        Interface_LoadItemIconImpl(globalCtx, 0);
        if (item == ITEM_SWORD_RAZOR) {
            gSaveContext.save.playerData.swordHealth = 100;
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_SHIELD_HERO) && (item <= ITEM_SHIELD_MIRROR)) {
        value = item - ITEM_SHIELD_HERO - 1;
        if (value != CUR_EQUIP_VALUE(EQUIP_SHIELD)) {
            SET_EQUIP_VALUE(EQUIP_SHIELD, value);
            func_80123C90(globalCtx, player);
            return ITEM_NONE;
        }
        return item;

    } else if ((item == ITEM_KEY_BOSS) || (item == ITEM_COMPASS) || (item == ITEM_DUNGEON_MAP)) {
        gSaveContext.save.inventory.dungeonItems[gSaveContext.mapIndex] |= gBitFlags[item - ITEM_KEY_BOSS];
        return ITEM_NONE;

    } else if (item == ITEM_KEY_SMALL) {
        if (gSaveContext.save.inventory.dungeonKeys[gSaveContext.mapIndex] < 0) {
            gSaveContext.save.inventory.dungeonKeys[gSaveContext.mapIndex] = 1;
            return ITEM_NONE;
        } else {
            gSaveContext.save.inventory.dungeonKeys[gSaveContext.mapIndex]++;
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
            if (AMMO(ITEM_BOW) > CUR_CAPACITY(UPG_QUIVER)) {
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
            slot = gItemSlots[item];
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
        slot = gItemSlots[8];

    } else if (item == ITEM_NUT) {
        if (INV_CONTENT(ITEM_NUT) != ITEM_NUT) {
            Inventory_ChangeUpgrade(UPG_NUTS, 1);
            AMMO(ITEM_NUT) = 1;
            slot = gItemSlots[item];
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
        slot = gItemSlots[9];

    } else if (item == ITEM_POWDER_KEG) {
        if (INV_CONTENT(ITEM_POWDER_KEG) != ITEM_POWDER_KEG) {
            INV_CONTENT(ITEM_POWDER_KEG) = ITEM_POWDER_KEG;
        }

        AMMO(ITEM_POWDER_KEG) = 1;
        return ITEM_NONE;

    } else if (item == ITEM_BOMB) {
        AMMO(ITEM_BOMB)++;
        if (AMMO(ITEM_BOMB) > CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMB) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_BOMBS_5) && (item <= ITEM_BOMBS_30)) {
        if (gSaveContext.save.inventory.items[6] != ITEM_BOMB) {
            INV_CONTENT(ITEM_BOMB) = ITEM_BOMB;
            AMMO(ITEM_BOMB) += sAmmoRefillCounts[item - ITEM_BOMBS_5];
            return ITEM_NONE;
        }

        AMMO(ITEM_BOMB) += sAmmoRefillCounts[item - ITEM_BOMBS_5];

        if (AMMO(ITEM_BOMB) > CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMB) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
        return ITEM_NONE;

    } else if (item == ITEM_BOMBCHU) {
        if (INV_CONTENT(ITEM_BOMBCHU) != ITEM_BOMBCHU) {
            INV_CONTENT(ITEM_BOMBCHU) = ITEM_BOMBCHU;
            AMMO(ITEM_BOMBCHU) = 10;
            return ITEM_NONE;
        }

        AMMO(ITEM_BOMBCHU) += 10;
        if (AMMO(ITEM_BOMBCHU) > CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_BOMBCHUS_20) && (item <= ITEM_BOMBCHUS_5)) {
        if (gSaveContext.save.inventory.items[7] != ITEM_BOMBCHU) {
            INV_CONTENT(ITEM_BOMBCHU) = ITEM_BOMBCHU;
            AMMO(ITEM_BOMBCHU) += sAmmoRefillCounts[item - ITEM_BOMBCHUS_20];

            if (AMMO(ITEM_BOMBCHU) > CUR_CAPACITY(UPG_BOMB_BAG)) {
                AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
            }
            return ITEM_NONE;
        }

        AMMO(ITEM_BOMBCHU) += sAmmoRefillCounts[item - ITEM_BOMBCHUS_20];
        if (AMMO(ITEM_BOMBCHU) > CUR_CAPACITY(UPG_BOMB_BAG)) {
            AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
        return ITEM_NONE;

    } else if ((item >= ITEM_ARROWS_10) && (item <= ITEM_ARROWS_50)) {
        AMMO(ITEM_BOW) += sAmmoRefillCounts[item - ITEM_ARROWS_10 + 4];

        if ((AMMO(ITEM_BOW) >= CUR_CAPACITY(UPG_QUIVER)) || (AMMO(ITEM_BOW) < 0)) {
            AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        }
        return ITEM_BOW;

    } else if (item == ITEM_OCARINA) {
        INV_CONTENT(ITEM_OCARINA) = 0;
        return ITEM_NONE;

    } else if (item == ITEM_BEAN) {
        if (INV_CONTENT(ITEM_BEAN) == ITEM_NONE) {
            INV_CONTENT(ITEM_BEAN) = item;
            AMMO(ITEM_BEAN) = 1;
            return ITEM_NONE;
        }

        if (AMMO(ITEM_BEAN) < 20) {
            AMMO(ITEM_BEAN)++;
            return ITEM_NONE;
        }

        AMMO(ITEM_BEAN) = 20;
        return ITEM_NONE;

    } else if ((item >= ITEM_REMAINS_ODOLWA) && (item <= ITEM_REMAINS_TWINMOLD)) {
        gSaveContext.save.inventory.questItems |= gBitFlags[item - ITEM_REMAINS_ODOLWA + QUEST_REMAINS_ODOWLA];
        return ITEM_NONE;

    } else if (item == ITEM_HEART) {
        Health_ChangeBy(globalCtx, 0x10);
        return item;

    } else if (item == ITEM_MAGIC_SMALL) {
        Interface_AddMagic(globalCtx, 0x18);
        if (!(gSaveContext.save.weekEventReg[0xC] & 0x80)) {
            gSaveContext.save.weekEventReg[0xC] |= 0x80;
            return ITEM_NONE;
        }
        return item;

    } else if (item == ITEM_MAGIC_LARGE) {
        Interface_AddMagic(globalCtx, 0x30);
        if (!(gSaveContext.save.weekEventReg[0xC] & 0x80)) {
            gSaveContext.save.weekEventReg[0xC] |= 0x80;
            return ITEM_NONE;
        }
        return item;

    } else if ((item >= ITEM_RUPEE_GREEN) && (item <= ITEM_RUPEE_HUGE)) {
        Rupees_ChangeBy(sAmmoRefillCounts[item - ITEM_RUPEE_GREEN + 10]);
        return ITEM_NONE;

    } else if (item == ITEM_LONGSHOT) {
        for (i = 0; i < 6; i++) {
            if (gSaveContext.save.inventory.items[i] == ITEM_NONE) {
                gSaveContext.save.inventory.items[i] = ITEM_POTION_RED;
            }
        }
        return item;

    } else if ((item == 0x18) || (item == 0x1D) || (item == 0x22) || (item == 0x25) || (item == 0x26)) {
        for (i = 0; i < 6; i++) {
            if (gSaveContext.save.inventory.items[i] == ITEM_NONE) {
                gSaveContext.save.inventory.items[i] = item;
                return ITEM_NONE;
            }
        }
        return item;

    } else if (item == 0x12) {
        for (i = 0; i < 6; i++) {
            if (gSaveContext.save.inventory.items[i] == ITEM_NONE) {
                gSaveContext.save.inventory.items[i] = item;
                return ITEM_NONE;
            }
        }
        return item;

    } else if (((item >= 0x13) && (item < 0x28)) || (item == 0x9F) || (item == 0xA0) || (item == 0xA1) ||
               (item == 0xA2) || (item == 0xA3)) {

        slot = gItemSlots[item];
        if ((item != 0x18) && (item != 0x19)) {
            // i = 0;
            if (item == 0x9F) {
                slot = gItemSlots[0x25];
                item = 0x25;

            } else if (item == 0xA0) {
                slot = gItemSlots[0x18];
                item = 0x18;

            } else if (item == 0xA1) {
                slot = gItemSlots[0x22];
                item = 0x22;

            } else if (item == 0xA2) {
                slot = gItemSlots[0x26];
                item = 0x26;

            } else if (item == 0xA3) {
                slot = gItemSlots[0x24];
                item = 0x24;
            }

            for (i = 0; i < 6; i++) {
                if (gSaveContext.save.inventory.items[slot + i] == 0x12) {
                    if (item == 0x20) {
                        func_8010EBA0(60, i);
                    }

                    if ((slot + i) == gSaveContext.save.equips.cButtonSlots[0][1]) {
                        gSaveContext.save.equips.buttonItems[0][1] = item;
                        Interface_LoadItemIconImpl(globalCtx, 1);
                        gSaveContext.buttonStatus[1] = BTN_ENABLED;
                    } else if ((slot + i) == gSaveContext.save.equips.cButtonSlots[0][2]) {
                        gSaveContext.save.equips.buttonItems[0][2] = item;
                        Interface_LoadItemIconImpl(globalCtx, 2);
                        gSaveContext.buttonStatus[2] = BTN_ENABLED;
                    } else if ((slot + i) == gSaveContext.save.equips.cButtonSlots[0][3]) {
                        gSaveContext.save.equips.buttonItems[0][3] = item;
                        Interface_LoadItemIconImpl(globalCtx, 3);
                        gSaveContext.buttonStatus[3] = BTN_ENABLED;
                    }

                    gSaveContext.save.inventory.items[slot] = item;
                    return ITEM_NONE;
                }
            }
        } else {
            for (i = 0; i < 6; i++) {
                if (gSaveContext.save.inventory.items[i] == ITEM_NONE) {
                    gSaveContext.save.inventory.items[i] = item;
                    return ITEM_NONE;
                }
            }
        }

    } else if ((item >= ITEM_MOON_TEAR) && (item <= ITEM_MASK_GIANT)) {
        INV_CONTENT(item);
        gSaveContext.save.inventory.items[gItemSlots[item]] = item;
        if ((item >= ITEM_MOON_TEAR) && (item <= ITEM_PENDANT_MEMORIES) && (INV_CONTENT(item) != ITEM_NONE)) {
            for (i = 1; i < 4; i++) {
                if (i == 0) {
                    phi_a2_2 = gSaveContext.save.equips.buttonItems[CUR_FORM][i];
                } else {
                    phi_a2_2 = gSaveContext.save.equips.buttonItems[0][i];
                }

                if (INV_CONTENT(item) == phi_a2_2) {
                    if (i == 0) {
                        gSaveContext.save.equips.buttonItems[CUR_FORM][i] = item;
                    } else {
                        gSaveContext.save.equips.buttonItems[0][i] = item;
                    }
                    Interface_LoadItemIconImpl(globalCtx, i);
                    return ITEM_NONE;
                }
            }
        }
        return ITEM_NONE;
    }

    gSaveContext.save.inventory.items[slot] = item;
    return gSaveContext.save.inventory.items[slot];
}
#else
s16 sAmmoRefillCounts[] = {
    5, 10, 20, 30, 10, 30, 40, 50, 20, 10, 1, 5, 1, 5, 10, 20, 50, 100, 200, 0,
};
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Item_Give.s")
#endif

s32 Item_CheckObtainabilityImpl(u8 item);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Item_CheckObtainabilityImpl.s")

s32 Item_CheckObtainability(u8 item) {
    return Item_CheckObtainabilityImpl(item);
}

void Inventory_DeleteItem(s16 item, s16 invSlot) {
    s16 btn;

    gSaveContext.save.inventory.items[invSlot] = ITEM_NONE;

    for (btn = 1; btn < 4; btn++) {
        if (GET_CUR_FORM_BTN_ITEM(btn) == item) {
            SET_CUR_FORM_BTN_ITEM(btn, ITEM_NONE);
            SET_CUR_FORM_BTN_SLOT(btn, SLOT_NONE);
        }
    }
}

void Inventory_UnequipItem(s16 item) {
    s16 btn;

    for (btn = 1; btn < 4; btn++) {
        if (GET_CUR_FORM_BTN_ITEM(btn) == item) {
            SET_CUR_FORM_BTN_ITEM(btn, ITEM_NONE);
            SET_CUR_FORM_BTN_SLOT(btn, SLOT_NONE);
        }
    }
}

s32 Inventory_ReplaceItem(GlobalContext* globalCtx, u8 oldItem, u8 newItem) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(gSaveContext.save.inventory.items); i++) {
        if (gSaveContext.save.inventory.items[i] == oldItem) {
            gSaveContext.save.inventory.items[i] = newItem;

            for (i = 1; i < 4; i++) {
                if (GET_CUR_FORM_BTN_ITEM(i) == oldItem) {
                    SET_CUR_FORM_BTN_ITEM(i, newItem);
                    Interface_LoadItemIconImpl(globalCtx, i);
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80114CA0.s")

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

void Inventory_UpdateBottleItem(GlobalContext* globalCtx, u8 item, u8 btn) {
    gSaveContext.save.inventory.items[GET_CUR_FORM_BTN_SLOT(btn)] = item;
    SET_CUR_FORM_BTN_ITEM(btn, item);

    Interface_LoadItemIconImpl(globalCtx, btn);

    globalCtx->pauseCtx.cursorItem[0] = item;
    gSaveContext.buttonStatus[btn] = BTN_ENABLED;

    if (item == ITEM_HOT_SPRING_WATER) {
        func_8010EBA0(60, GET_CUR_FORM_BTN_SLOT(btn) - 18);
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115130.s")

/**
 * Only used to equip spring water when hot sprint water timer runs out
 */
void Inventory_UpdateItem(GlobalContext* globalCtx, s16 slot, s16 item) {
    s16 btn;

    gSaveContext.save.inventory.items[slot] = item;

    for (btn = 1; btn < 4; btn++) {
        if (GET_CUR_FORM_BTN_SLOT(btn) == slot) {
            SET_CUR_FORM_BTN_ITEM(btn, item);
            Interface_LoadItemIconImpl(globalCtx, btn);
        }
    }
}

void func_801153C8(s32* buf, s32 size) {
    s32 i;

    for (i = 0; i != size; i++) {
        buf[i] = 0;
    }
}

void Interface_LoadActionLabel(InterfaceContext* interfaceCtx, u16 action, s16 loadOffset) {
    static TexturePtr sDoActionTextures[] = {
        0x09000000, // gAttackDoActionENGTex
        0x09000180, // gCheckDoActionENGTex
    };

    if (action >= DO_ACTION_MAX) {
        action = DO_ACTION_NONE;
    }

    if (action != DO_ACTION_NONE) {
        osCreateMesgQueue(&interfaceCtx->loadQueue, &interfaceCtx->loadMsg, 1);
        DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184, (u32)interfaceCtx->doActionSegment + (loadOffset * 0x180),
                               (u32)_do_action_staticSegmentRomStart + (action * 0x180), 0x180, 0,
                               &interfaceCtx->loadQueue, 0);
        osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);
        return;
    }

    gSegments[0x09] = PHYSICAL_TO_VIRTUAL(interfaceCtx->doActionSegment);
    func_801153C8(Lib_SegmentedToVirtual(sDoActionTextures[loadOffset]), 0x60);
}

void Interface_SetDoAction(GlobalContext* globalCtx, u16 action) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    PauseContext* pauseCtx = &globalCtx->pauseCtx;

    if (interfaceCtx->unk_214 != action) {
        interfaceCtx->unk_214 = action;
        interfaceCtx->unk_210 = 1;
        interfaceCtx->unk_218 = 0.0f;
        Interface_LoadActionLabel(interfaceCtx, action, 1);
        if (pauseCtx->state != 0) {
            interfaceCtx->unk_210 = 3;
        }
    }
}

void func_801155B4(GlobalContext* globalCtx, s16 arg1) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    if (((gSaveContext.save.equips.buttonItems[CUR_FORM][0] >= ITEM_SWORD_KOKIRI) &&
         (gSaveContext.save.equips.buttonItems[CUR_FORM][0] <= ITEM_SWORD_GILDED)) ||
        (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_NONE) ||
        (gSaveContext.save.equips.buttonItems[CUR_FORM][0] == ITEM_NUT)) {
        if ((CUR_FORM == PLAYER_FORM_DEKU) && (gSaveContext.save.playerData.magicAcquired == 0)) {
            interfaceCtx->unk_21E = 0xFD;
        } else {
            interfaceCtx->unk_21E = arg1;
            if (interfaceCtx->unk_21E != 0xA) {
                osCreateMesgQueue(&interfaceCtx->loadQueue, &interfaceCtx->loadMsg, 1);
                DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184, interfaceCtx->doActionSegment + 0x600,
                                       (arg1 * 0x180) + SEGMENT_ROM_START(do_action_static), 0x180, 0,
                                       &interfaceCtx->loadQueue, NULL);
                osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);
            }

            interfaceCtx->unk_21C = 1;
        }
    } else {
        interfaceCtx->unk_21C = 0;
        interfaceCtx->unk_21E = 0;
    }
}

void Interface_SetNaviCall(GlobalContext* globalCtx, u16 naviCallState) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    if (((naviCallState == 0x2A) || (naviCallState == 0x2B)) && (interfaceCtx->unk_220 == 0) &&
        (globalCtx->csCtx.state == 0)) {
        if (naviCallState == 0x2B) {
            play_sound(NA_SE_VO_NAVY_CALL);
        }
        if (naviCallState == 0x2A) {
            func_8019FDC8(&D_801DB4A4, NA_SE_VO_NA_HELLO_2, 0x20);
        }
        interfaceCtx->unk_220 = 1;
        D_801BF898 = 0;
        D_801BF89C = 10;
    } else if (naviCallState == 0x2C) {
        if (interfaceCtx->unk_220 != 0) {
            interfaceCtx->unk_220 = 0;
        }
    }
}

// Interface_LoadActionLabelB?
void func_80115844(GlobalContext* globalCtx, s16 arg1) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    interfaceCtx->unk_224 = arg1;

    osCreateMesgQueue(&globalCtx->interfaceCtx.loadQueue, &globalCtx->interfaceCtx.loadMsg, 1);
    DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184, interfaceCtx->doActionSegment + 0x480, (arg1 * 0x180) + SEGMENT_ROM_START(do_action_static), 0x180, 0, &interfaceCtx->loadQueue, NULL);
    osRecvMesg(&interfaceCtx->loadQueue, NULL, OS_MESG_BLOCK);

    interfaceCtx->unk_222 = 1;
}

s32 Health_ChangeBy(GlobalContext* globalCtx, s16 healthChange) {

    if (healthChange > 0) {
        play_sound(NA_SE_SY_HP_RECOVER);
    } else if ((gSaveContext.save.playerData.doubleDefense != 0) && (healthChange < 0)) {
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

    } else if (item == ITEM_BEAN) {
        AMMO(ITEM_BEAN) += ammoChange;

    } else if (item == ITEM_POWDER_KEG) {
        AMMO(ITEM_POWDER_KEG) += ammoChange;
        if (AMMO(ITEM_POWDER_KEG) >= 1) {
            AMMO(ITEM_POWDER_KEG) = 1;
        } else if (AMMO(ITEM_POWDER_KEG) < 0) {
            AMMO(ITEM_POWDER_KEG) = 0;
        }
    }
}

void Interface_AddMagic(GlobalContext* globalCtx, s16 arg1) {
    if (((void)0, gSaveContext.save.playerData.magic) < ((void)0, gSaveContext.unk_3F2E)) {
        gSaveContext.unk_3F34 += arg1;
        gSaveContext.unk_3F2A = 1;
    }
}

void func_80115D5C(GameState* gamestate) {
    if ((gSaveContext.unk_3F28 != 8) && (gSaveContext.unk_3F28 != 9)) {
        sMagicBarOutlinePrimRed = sMagicBarOutlinePrimGreen = sMagicBarOutlinePrimBlue = 255;
        gSaveContext.unk_3F28 = 0;
    }
}

s32 func_80115DB4(GlobalContext* globalCtx, s16 arg1, s16 arg2) {
    if (gSaveContext.save.playerData.magicAcquired == 0) {
        return 0;
    }

    if ((gSaveContext.save.playerData.magic - arg1) < 0) {
        if (gSaveContext.unk_3F2E != 0) {
            play_sound(NA_SE_SY_ERROR);
        }
        return 0;
    }

    switch (arg2) {
        case 0:
        case 2:
            if ((gSaveContext.unk_3F28 == 0) || (gSaveContext.unk_3F28 == 7)) {
                if (gSaveContext.unk_3F28 == 7) {
                    globalCtx->actorCtx.unk3 = 0;
                }
                if (gSaveContext.save.weekEventReg[0xE] & 8) {
                    arg1 = 0;
                }
                gSaveContext.unk_3F32 = arg1;
                gSaveContext.unk_3F28 = 1;
                return 1;
            }

            play_sound(NA_SE_SY_ERROR);
            return 0;
        case 1:
            if ((gSaveContext.unk_3F28 == 0) || (gSaveContext.unk_3F28 == 7)) {
                if (gSaveContext.unk_3F28 == 7) {
                    globalCtx->actorCtx.unk3 = 0;
                }
                if (gSaveContext.save.weekEventReg[0xE] & 8) {
                    arg1 = 0;
                }
                gSaveContext.unk_3F32 = arg1;
                gSaveContext.unk_3F28 = 6;
                return 1;
            }

            play_sound(NA_SE_SY_ERROR);
            return 0;
        case 3:
            if (gSaveContext.unk_3F28 == 0) {
                if (gSaveContext.save.playerData.magic != 0) {
                    globalCtx->interfaceCtx.unk_258 = 0x50;
                    gSaveContext.unk_3F28 = 7;
                    return 1;
                }
                return 0;
            }
            if (gSaveContext.unk_3F28 == 7) {
                return 1;
            }
            return 0;
        case 4:
            if ((gSaveContext.unk_3F28 == 0) || (gSaveContext.unk_3F28 == 7)) {
                if (gSaveContext.unk_3F28 == 7) {
                    globalCtx->actorCtx.unk3 = 0;
                }
                gSaveContext.unk_3F32 = arg1;
                gSaveContext.unk_3F28 = 4;
                return 1;
            }
            play_sound(NA_SE_SY_ERROR);
            return 0;
        case 5:
            if (gSaveContext.save.playerData.magic != 0) {
                globalCtx->interfaceCtx.unk_258 = 0xA;
                gSaveContext.unk_3F28 = 0xA;
                return 1;
            }
            return 0;
        case 6:
            if (gSaveContext.unk_3F28 == 0) {
                if (gSaveContext.save.playerData.magic != 0) {
                    globalCtx->interfaceCtx.unk_258 = gGameInfo->data[0x569];
                    gSaveContext.unk_3F28 = 0xC;
                    return 1;
                }
                return 0;
            }
            if (gSaveContext.unk_3F28 == 0xC) {
                return 1;
            }
            return 0;
        case 7:
            if ((gSaveContext.unk_3F28 == 0) || (gSaveContext.unk_3F28 == 7)) {
                if (gSaveContext.unk_3F28 == 7) {
                    globalCtx->actorCtx.unk3 = 0;
                }
                if (gSaveContext.save.weekEventReg[0xE] & 8) {
                    arg1 = 0;
                }
                gSaveContext.save.playerData.magic -= arg1;
                return 1;
            }
            play_sound(NA_SE_SY_ERROR);
            return 0;
        default:
            return 0;
    }
}

void func_80116088(void) {
    if (gSaveContext.unk_3F2A != 0) {
        gSaveContext.save.playerData.magic += 4;
        play_sound(NA_SE_SY_GAUGE_UP - SFX_FLAG);
        if (((void)0, gSaveContext.save.playerData.magic) >= ((void)0, gSaveContext.unk_3F2E)) {
            gSaveContext.save.playerData.magic = gSaveContext.unk_3F2E;
            gSaveContext.unk_3F34 = 0;
            gSaveContext.unk_3F2A = 0;
        } else {
            gSaveContext.unk_3F34 -= 4;
            if (gSaveContext.unk_3F34 <= 0) {
                gSaveContext.unk_3F34 = 0;
                gSaveContext.unk_3F2A = 0;
            }
        }
    }
}

// Part of Interface_UpdateMagicBar in OoT, but that function was broken into parts 
void func_80116114(void) {
    static s16 magicBorderColors[][3] = {
        { 255, 255, 255 },
        { 150, 150, 150 },
    };
    static s16 magicBorderIndices[] = { 0, 1, 1, 0 };
    static s16 magicBorderColorTimerIndex[] = { 2, 1, 2, 1 };
    s16 colorStep1;
    s16 colorStep2;
    s16 colorStep3;
    s16 index;

    index = magicBorderIndices[D_801BF8B0];
    colorStep1 = ABS_ALT(sMagicBarOutlinePrimRed - magicBorderColors[index][0]) / D_801BF8AC;
    colorStep2 = ABS_ALT(sMagicBarOutlinePrimGreen - magicBorderColors[index][1]) / D_801BF8AC;
    colorStep3 = ABS_ALT(sMagicBarOutlinePrimBlue - magicBorderColors[index][2]) / D_801BF8AC;

    if (sMagicBarOutlinePrimRed >= magicBorderColors[index][0]) {
        sMagicBarOutlinePrimRed -= colorStep1;
    } else {
        sMagicBarOutlinePrimRed += colorStep1;
    }

    if (sMagicBarOutlinePrimGreen >= magicBorderColors[index][1]) {
        sMagicBarOutlinePrimGreen -= colorStep2;
    } else {
        sMagicBarOutlinePrimGreen += colorStep2;
    }

    if (sMagicBarOutlinePrimBlue >= magicBorderColors[index][2]) {
        sMagicBarOutlinePrimBlue -= colorStep3;
    } else {
        sMagicBarOutlinePrimBlue += colorStep3;
    }

    D_801BF8AC--;

    if (D_801BF8AC == 0) {
        sMagicBarOutlinePrimRed = magicBorderColors[index][0];
        sMagicBarOutlinePrimGreen = magicBorderColors[index][1];
        sMagicBarOutlinePrimBlue = magicBorderColors[index][2];
        D_801BF8AC = magicBorderColorTimerIndex[D_801BF8B0];
        D_801BF8B0++;
        if (D_801BF8B0 >= 4) {
            D_801BF8B0 = 0;
        }
    }
}

void Interface_UpdateMagicBar(GlobalContext* globalCtx) {
    MessageContext* msgCtx = &globalCtx->msgCtx;
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 temp;

    if (gSaveContext.save.weekEventReg[0xE] & 8) {
        func_80116114();
    }

    switch (gSaveContext.unk_3F28) {
        case 8:
            temp = gSaveContext.save.playerData.magicLevel * 0x30;
            if (gSaveContext.unk_3F2E != temp) {
                if (gSaveContext.unk_3F2E < temp) {
                    gSaveContext.unk_3F2E += 0x10;
                    if (gSaveContext.unk_3F2E > temp) {
                        gSaveContext.unk_3F2E = temp;
                    }
                } else {
                    gSaveContext.unk_3F2E -= 0x10;
                    if (gSaveContext.unk_3F2E <= temp) {
                        gSaveContext.unk_3F2E = temp;
                    }
                }
            } else {
                gSaveContext.unk_3F28 = 9;
            }
            break;

        case 9:
            gSaveContext.save.playerData.magic += 0x10;
            
            if ((gSaveContext.gameMode == 0) && (gSaveContext.sceneSetupIndex < 4)) {
                play_sound(NA_SE_SY_GAUGE_UP - SFX_FLAG);
            }

            if (((void)0, gSaveContext.save.playerData.magic) >= ((void)0, gSaveContext.unk_3F30)) {
                gSaveContext.save.playerData.magic = gSaveContext.unk_3F30;
                gSaveContext.unk_3F28 = 0;
            }
            break;

        case 1:
            D_801BF8AC = 2;
            gSaveContext.unk_3F28 = 2;
            break;

        case 2:
            if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                gSaveContext.save.playerData.magic = ((void)0, gSaveContext.save.playerData.magic) - ((void)0, gSaveContext.unk_3F32);
                if (gSaveContext.save.playerData.magic <= 0) {
                    gSaveContext.save.playerData.magic = 0;
                }
                gSaveContext.unk_3F28 = 3;
                sMagicBarOutlinePrimBlue = 255;
                sMagicBarOutlinePrimGreen = sMagicBarOutlinePrimBlue;
                sMagicBarOutlinePrimRed = sMagicBarOutlinePrimGreen;
            }
            // fallthrough

        case 3:
        case 4:
        case 6:
            if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                func_80116114();
            }
            break;

        case 5:
            sMagicBarOutlinePrimRed = sMagicBarOutlinePrimGreen = sMagicBarOutlinePrimBlue = 255;
            gSaveContext.unk_3F28 = 0;
            break;

        case 7:
            if ((globalCtx->pauseCtx.state == 0) && (globalCtx->pauseCtx.debugState == 0) &&
                (msgCtx->msgMode == 0) && (globalCtx->gameOverCtx.state == 0) &&
                (globalCtx->sceneLoadFlag == 0) && (globalCtx->unk_18B4A == 0)) {
                if (!func_801690CC(globalCtx)) {
                    if ((gSaveContext.save.playerData.magic == 0) ||
                        ((func_801242DC(globalCtx) >= 2) && (func_801242DC(globalCtx) < 5)) ||
                        (((gSaveContext.save.equips.buttonItems[0][1]) != ITEM_LENS) &&
                         (gSaveContext.save.equips.buttonItems[0][2] != ITEM_LENS) &&
                         (gSaveContext.save.equips.buttonItems[0][3] != ITEM_LENS)) ||
                        (globalCtx->actorCtx.unk3 == 0)) {
                        globalCtx->actorCtx.unk3 = false;
                        play_sound(NA_SE_SY_GLASSMODE_OFF);
                        gSaveContext.unk_3F28 = 0;
                        sMagicBarOutlinePrimRed = sMagicBarOutlinePrimGreen = sMagicBarOutlinePrimBlue = 255;
                        break;
                    }

                    interfaceCtx->unk_258--;
                    if (interfaceCtx->unk_258 == 0) {
                        if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                            gSaveContext.save.playerData.magic--;
                        }
                        interfaceCtx->unk_258 = 80;
                    }
                }
            }
            if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                func_80116114();
            }
            break;

        case 10:
            if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                gSaveContext.save.playerData.magic -= 2;
            }
            if (gSaveContext.save.playerData.magic <= 0) {
                gSaveContext.save.playerData.magic = 0;
            }
            gSaveContext.unk_3F28 = 11;
            // fallthrough

        case 11:
            if ((globalCtx->pauseCtx.state == 0) && (globalCtx->pauseCtx.debugState == 0) &&
                (msgCtx->msgMode == 0) && (globalCtx->gameOverCtx.state == 0) &&
                (globalCtx->sceneLoadFlag == 0) && (globalCtx->unk_18B4A == 0)) {
                if (!func_801690CC(globalCtx)) {
                    interfaceCtx->unk_258--;
                    if (interfaceCtx->unk_258 == 0) {
                        if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                            gSaveContext.save.playerData.magic--;
                        }
                        if (gSaveContext.save.playerData.magic <= 0) {
                            gSaveContext.save.playerData.magic = 0;
                        }
                        interfaceCtx->unk_258 = 10;
                    }
                }
            }
            if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                func_80116114();
            }
            break;

        case 12:
            if ((globalCtx->pauseCtx.state == 0) && (globalCtx->pauseCtx.debugState == 0) &&
                (msgCtx->msgMode == 0) && (globalCtx->gameOverCtx.state == 0) &&
                (globalCtx->sceneLoadFlag == 0) && (globalCtx->unk_18B4A == 0)) {
                if (!func_801690CC(globalCtx)) {
                    interfaceCtx->unk_258--;
                    if (interfaceCtx->unk_258 == 0) {
                        if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                            gSaveContext.save.playerData.magic--;
                        }
                        if (gSaveContext.save.playerData.magic <= 0) {
                            gSaveContext.save.playerData.magic = 0;
                        }
                        interfaceCtx->unk_258 = gGameInfo->data[0x569];
                    }
                }
            }
            if (!(gSaveContext.save.weekEventReg[0xE] & 8)) {
                func_80116114();
            }
            break;

        default:
            gSaveContext.unk_3F28 = 0;
            break;
    }
}

void Interface_DrawMagicBar(GlobalContext* globalCtx) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 magicBarY;

    OPEN_DISPS(globalCtx->state.gfxCtx);

    if (gSaveContext.save.playerData.magicLevel != 0) {
        if (gSaveContext.save.playerData.healthCapacity > 0xA0) {
            magicBarY = 42;
        } else {
            magicBarY = 34;
        }

        func_8012C654(globalCtx->state.gfxCtx);

        gDPSetEnvColor(OVERLAY_DISP++, 100, 50, 50, 255);

        OVERLAY_DISP = func_8010CFBC(OVERLAY_DISP, gMagicBarEndTex, 8, 16, 18, magicBarY, 8, 16, 1 << 10, 1 << 10,
                                     sMagicBarOutlinePrimRed, sMagicBarOutlinePrimGreen, sMagicBarOutlinePrimBlue,
                                     interfaceCtx->magicAlpha);
        OVERLAY_DISP = func_8010CFBC(OVERLAY_DISP, gMagicBarMidTex, 24, 16, 26, magicBarY,
                                     ((void)0, gSaveContext.unk_3F2E), 16, 1 << 10, 1 << 10, sMagicBarOutlinePrimRed,
                                     sMagicBarOutlinePrimGreen, sMagicBarOutlinePrimBlue, interfaceCtx->magicAlpha);
        OVERLAY_DISP =
            func_8010D480(OVERLAY_DISP, gMagicBarEndTex, 8, 16, ((void)0, gSaveContext.unk_3F2E) + 26, magicBarY, 8, 16,
                          1 << 10, 1 << 10, sMagicBarOutlinePrimRed, sMagicBarOutlinePrimGreen,
                          sMagicBarOutlinePrimBlue, interfaceCtx->magicAlpha, 3, 0x100);

        gDPPipeSync(OVERLAY_DISP++);
        gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, PRIMITIVE, PRIMITIVE,
                          ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, PRIMITIVE);
        gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);

        if (gSaveContext.unk_3F28 == 4) {
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 250, 250, 0, interfaceCtx->magicAlpha);
            gDPLoadTextureBlock_4b(OVERLAY_DISP++, gMagicBarFillTex, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(OVERLAY_DISP++, 104, (magicBarY + 3) << 2,
                                (((void)0, gSaveContext.save.playerData.magic) + 26) << 2, (magicBarY + 10) << 2,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

            gDPPipeSync(OVERLAY_DISP++);

            if (gSaveContext.save.weekEventReg[0xE] & 8) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 200, interfaceCtx->magicAlpha);
            } else {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 200, 0, interfaceCtx->magicAlpha);
            }

            gSPTextureRectangle(
                OVERLAY_DISP++, 104, (magicBarY + 3) << 2,
                ((((void)0, gSaveContext.save.playerData.magic) - ((void)0, gSaveContext.unk_3F32)) + 26) << 2,
                (magicBarY + 10) << 2, G_TX_RENDERTILE, 0, 0, 0x0400, 0x0400);
        } else {
            if (gSaveContext.save.weekEventReg[0xE] & 8) {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 200, interfaceCtx->magicAlpha);
            } else {
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 200, 0, interfaceCtx->magicAlpha);
            }

            gDPLoadTextureBlock_4b(OVERLAY_DISP++, gMagicBarFillTex, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(OVERLAY_DISP++, 104, (magicBarY + 3) << 2,
                                (((void)0, gSaveContext.save.playerData.magic) + 26) << 2, (magicBarY + 10) << 2,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        }
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx);
}

void func_80116FD8(GlobalContext* globalCtx, s32 topY, s32 bottomY, s32 leftX, s32 rightX) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    Vec3f eye;
    Vec3f lookAt;
    Vec3f up;

    eye.x = eye.y = eye.z = 0.0f;
    lookAt.x = lookAt.y = 0.0f;
    lookAt.z = -1.0f;
    up.x = up.z = 0.0f;
    up.y = 1.0f;

    View_SetViewOrientation(&interfaceCtx->view, &eye, &lookAt, &up);

    interfaceCtx->viewport.topY = topY;
    interfaceCtx->viewport.bottomY = bottomY;
    interfaceCtx->viewport.leftX = leftX;
    interfaceCtx->viewport.rightX = rightX;
    View_SetViewport(&interfaceCtx->view, &interfaceCtx->viewport);

    func_8013F0D0(&interfaceCtx->view, 60.0f, 10.0f, 60.0f);
    func_8013FD74(&interfaceCtx->view);
}

void func_801170B8(InterfaceContext* interfaceCtx) {
    SET_FULLSCREEN_VIEWPORT(&interfaceCtx->view);
    func_8013FBC8(&interfaceCtx->view);
}

TexturePtr D_801BFAD4[] = {
    gTatlCUpENGTex, gTatlCUpENGTex, gTatlCUpGERTex, gTatlCUpFRATex, gTatlCUpESPTex,
};
s16 D_801BFAE8[] = {
    130, 136, 136, 136, 136,
};
s16 D_801BFAF4[] = {
    0x1D,
    0x1B,
};
s16 D_801BFAF8[] = {
    0x1B,
    0x1B,
};
void Interface_DrawItemButtons(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_DrawItemButtons.s")

void Interface_DrawItemIconTexture(GlobalContext* globalCtx, void* texture, s16 button) {
    static s16 D_801BFAFC[] = { 30, 24, 24, 24 };

    OPEN_DISPS(globalCtx->state.gfxCtx);

    gDPLoadTextureBlock(OVERLAY_DISP++, texture, G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(OVERLAY_DISP++, D_801BF9D4[button] << 2, D_801BF9DC[button] << 2,
                        (D_801BF9D4[button] + D_801BFAFC[button]) << 2, (D_801BF9DC[button] + D_801BFAFC[button]) << 2,
                        G_TX_RENDERTILE, 0, 0, D_801BF9BC[button] << 1, D_801BF9BC[button] << 1);

    CLOSE_DISPS(globalCtx->state.gfxCtx);
}

s16 D_801BFB04[] = { 0xA2, 0xE4, 0xFA, 0x110 };
s16 D_801BFB0C[] = { 0x23, 0x23, 0x33, 0x23 };
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_DrawAmmoCount.s")

void func_80118084(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80118084.s")

void func_80118890(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80118890.s")

void func_80118BA4(GlobalContext* globalCtx) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 aAlpha;

    OPEN_DISPS(globalCtx->state.gfxCtx);

    aAlpha = interfaceCtx->aAlpha;

    if (aAlpha > 100) {
        aAlpha = 100;
    }

    func_8012C8D4(globalCtx->state.gfxCtx);

    func_80116FD8(globalCtx, gGameInfo->data[0x55F] + 0x19, gGameInfo->data[0x55F] + 0x46, 192, 237);

    gSPClearGeometryMode(OVERLAY_DISP++, G_CULL_BOTH);
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);

    Matrix_InsertTranslation(0.0f, 0.0f, -38.0f, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
    Matrix_RotateStateAroundXAxis(interfaceCtx->unk_218 / 10000.0f);

    gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gDPPipeSync(OVERLAY_DISP++);
    gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[4], 4, 0);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, aAlpha);

    OVERLAY_DISP = func_8010DC58(OVERLAY_DISP, gButtonBackgroundTex, 32, 32, 0);

    gDPPipeSync(OVERLAY_DISP++);
    func_80116FD8(globalCtx, gGameInfo->data[0x55F] + 0x17, gGameInfo->data[0x55F] + 0x44, 190, 235);
    gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0], 4, 0);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 100, 200, 255, interfaceCtx->aAlpha);
    gSP1Quadrangle(OVERLAY_DISP++, 0, 2, 3, 1, 0);

    gDPPipeSync(OVERLAY_DISP++);
    func_80116FD8(globalCtx, gGameInfo->data[0x55F] + 0x17, gGameInfo->data[0x55F] + 0x44, 190, 235);
    gSPSetGeometryMode(OVERLAY_DISP++, G_CULL_BACK);
    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->aAlpha);
    gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 0);

    Matrix_InsertTranslation(0.0f, 0.0f, D_801BF9CC[gSaveContext.options.language] / 10.0f, MTXMODE_NEW);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
    Matrix_RotateStateAroundXAxis(interfaceCtx->unk_218 / 10000.0f);
    gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[8], 4, 0);

    if (((interfaceCtx->unk_210 < 2) || (interfaceCtx->unk_210 == 3))) {
        OVERLAY_DISP = func_8010DE38(OVERLAY_DISP, interfaceCtx->doActionSegment, 3, 0x30, 0x10, 0);
    } else {
        OVERLAY_DISP = func_8010DE38(OVERLAY_DISP, interfaceCtx->doActionSegment + 0x180, 3, 0x30, 0x10, 0);
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx);
}

s16 D_801BFB14[] = { 255, 100, 255, 0 };
s16 D_801BFB1C[] = { 0, 100, 255, 0 };
s16 D_801BFB24[] = { 0, 255, 100, 0 };
void func_80119030(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80119030.s")

/**
 * Draws either the analog three-day clock or the digital final-hours clock
 */
#ifdef NON_EQUIVALENT
void Interface_DrawClock(GlobalContext* globalCtx) {
    static s16 D_801BFB2C = 255; // clockAlpha
    static s16 D_801BFB30 = 0;   // clockAlphaTimer1
    static s16 D_801BFB34 = 0;   // clockAlphaTimer2
    // clockHours
    static u16 D_801BFB38[] = {
        CLOCK_TIME(0, 0),  CLOCK_TIME(1, 0),  CLOCK_TIME(2, 0),  CLOCK_TIME(3, 0),  CLOCK_TIME(4, 0),
        CLOCK_TIME(5, 0),  CLOCK_TIME(6, 0),  CLOCK_TIME(7, 0),  CLOCK_TIME(8, 0),  CLOCK_TIME(9, 0),
        CLOCK_TIME(10, 0), CLOCK_TIME(11, 0), CLOCK_TIME(12, 0), CLOCK_TIME(13, 0), CLOCK_TIME(14, 0),
        CLOCK_TIME(15, 0), CLOCK_TIME(16, 0), CLOCK_TIME(17, 0), CLOCK_TIME(18, 0), CLOCK_TIME(19, 0),
        CLOCK_TIME(20, 0), CLOCK_TIME(21, 0), CLOCK_TIME(22, 0), CLOCK_TIME(23, 0), CLOCK_TIME(24, 0) - 1,
    };
    // threeDayTimerHourTexs
    static TexturePtr D_801BFB6C[] = {
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
    // finalHoursCounterDigitTex
    static TexturePtr D_801BFC14[] = {
        gFinalHoursClockDigit0Tex, gFinalHoursClockDigit1Tex, gFinalHoursClockDigit2Tex, gFinalHoursClockDigit3Tex,
        gFinalHoursClockDigit4Tex, gFinalHoursClockDigit5Tex, gFinalHoursClockDigit6Tex, gFinalHoursClockDigit7Tex,
        gFinalHoursClockDigit8Tex, gFinalHoursClockDigit9Tex, gFinalHoursClockColonTex,
    };
    // finalHoursDigitSlotPosXOffset
    static s16 D_801BFC40[] = {
        127, 136, 144, 151, 160, 168, 175, 184,
    };
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    MessageContext* msgCtx = &globalCtx->msgCtx;
    s16 sp1E6;
    f32 sp1D8;
    f32 sp1D0;
    f32 sp1CC;
    s16 sp1C6;
    s16 finalHoursClockSlots[8]; // sp1AC

    f32 temp_f0;
    f32 temp_f14;
    u32 temp_a1_7;
    s32 phi_v1_2; // Unused
    s16 currentHour;
    u16 time;
    f32 new_var1;
    s16 colorStep;

    OPEN_DISPS(globalCtx->state.gfxCtx);

    if (REG(15) != 0) {
        if ((msgCtx->msgMode == 0) || ((globalCtx->actorCtx.unk5 & 2) && !func_801690CC(globalCtx)) ||
            (msgCtx->msgMode == 0) || ((msgCtx->unk11F04 >= 0x100) && (msgCtx->unk11F04 <= 0x200)) ||
            (gSaveContext.gameMode == 3)) {
            if (FrameAdvance_IsEnabled(globalCtx) == 0) {
                if (func_800FE4A8() == 0) {
                    if ((0, gSaveContext.save.day) < 4) {
                        /**
                         * Changes Clock's transparancy depending if Player is moving or not and possibly other things
                         */
                        if ((0, gSaveContext.unk_3F22) == 50) {
                            if (func_801234D4(globalCtx) != 0) {
                                D_801BFB2C = 80;
                                D_801BFB30 = 5;
                                D_801BFB34 = 20;
                            } else if (D_801BFB34 != 0) {
                                D_801BFB34--;
                            } else if (D_801BFB30 != 0) {
                                colorStep = ABS_ALT(D_801BFB2C - 255) / D_801BFB30;
                                D_801BFB2C += colorStep;

                                if (D_801BFB2C >= 255) {
                                    D_801BFB2C = 255;
                                    D_801BFB30 = 0;
                                }
                            } else {
                                if (globalCtx->actorCtx.unk5 & 2) {
                                    D_801BFB2C = 255;
                                } else {
                                    D_801BFB2C = interfaceCtx->bAlpha;
                                }
                                D_801BFB34 = 0;
                                D_801BFB30 = 0;
                            }
                        } else {
                            if (globalCtx->actorCtx.unk5 & 2) {
                                D_801BFB2C = 255;
                            } else {
                                D_801BFB2C = interfaceCtx->bAlpha;
                            }
                            D_801BFB34 = 0;
                            D_801BFB30 = 0;
                        }

                        if ((globalCtx->pauseCtx.state == 0) && (globalCtx->pauseCtx.debugState == 0)) {

                            func_8012C654(globalCtx->state.gfxCtx);

                            /**
                             * Draw Clock's Hour Lines
                             */
                            gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                            gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 130, 130, 130, D_801BFB2C);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                                              PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

                            OVERLAY_DISP = func_8010D9F4(OVERLAY_DISP, gThreeDayClockHourLinesTex, 4, 64, 35, 96, 180,
                                                         128, 35, 1, 6, 0, 1 << 10, 1 << 10);

                            /**
                             * Draw Clock's Border
                             */
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, D_801BFB2C);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                                              PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

                            //! @bug A texture height of 50 is given below. The texture is only 48 units height
                            //!      resulting in this reading into the next texture. This results in a white
                            //!      dot in the bottom center of the clock. For the three-day clock, this is
                            //!      covered by the diamond. However, it can be seen by the final-hours clock.
                            OVERLAY_DISP = func_8010D9F4(OVERLAY_DISP, gThreeDayClockDialBorderTex, 4, 64, 50, 96, 168,
                                                         128, 50, 1, 6, 0, 1 << 10, 1 << 10);

                            if (((CURRENT_DAY >= 4) || ((CURRENT_DAY == 3) && ((0, gSaveContext.save.time) >= 5) &&
                                                        ((0, gSaveContext.save.time) < CLOCK_TIME(6, 0))))) {
                                func_8012C8D4(globalCtx->state.gfxCtx);
                                gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            } else {
                                /**
                                 * Draw Three-Day Clock's Diamond
                                 */
                                gDPPipeSync(OVERLAY_DISP++);

                                // Time is slowed down to half speed with inverted song of time
                                if (gSaveContext.save.daySpeed == -2) {
                                    // Clock diamond is blue and flashes white
                                    TIMED_STEP_TO(D_801BFBCC, D_801BFBEC[D_801BFBE8], D_801BFBE4, colorStep);
                                    TIMED_STEP_TO(D_801BFBD0, D_801BFBF0[D_801BFBE8], D_801BFBE4, colorStep);
                                    TIMED_STEP_TO(D_801BFBD4, D_801BFBF4[D_801BFBE8], D_801BFBE4, colorStep);
                                    TIMED_STEP_TO(D_801BFBD8, D_801BFBF8[D_801BFBE8], D_801BFBE4, colorStep);
                                    TIMED_STEP_TO(D_801BFBDC, D_801BFBFC[D_801BFBE8], D_801BFBE4, colorStep);
                                    TIMED_STEP_TO(D_801BFBE0, D_801BFC00[D_801BFBE8], D_801BFBE4, colorStep);

                                    D_801BFBE4--;

                                    if (D_801BFBE4 == 0) {
                                        D_801BFBCC = D_801BFBEC[D_801BFBE8];
                                        D_801BFBD0 = D_801BFBF0[D_801BFBE8];
                                        D_801BFBD4 = D_801BFBF4[D_801BFBE8];
                                        D_801BFBD8 = D_801BFBF0[D_801BFBE8];
                                        D_801BFBDC = D_801BFBFC[D_801BFBE8];
                                        D_801BFBE0 = D_801BFC00[D_801BFBE8];
                                        D_801BFBE4 = 15;
                                        D_801BFBE8 ^= 1;
                                    }

                                    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT,
                                                      TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE, ENVIRONMENT, TEXEL0,
                                                      ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, D_801BFBCC, D_801BFBD0, 255, D_801BFB2C);
                                    gDPSetEnvColor(OVERLAY_DISP++, D_801BFBD8, D_801BFBDC, D_801BFBE0, 0);
                                } else {
                                    // Clock diamond is green for regular daySpeed
                                    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 170, 100, D_801BFB2C);
                                }

                                OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, gThreeDayClockDiamondTex, 40, 32, 140, 190,
                                                              40, 32, 1 << 10, 1 << 10);

                                /**
                                 * Draw Three-Day Clock's Day-Number over Diamond
                                 */
                                gDPPipeSync(OVERLAY_DISP++);
                                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, D_801BFB2C);

                                OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, interfaceCtx->doActionSegment + 0x780, 48,
                                                              27, 137, 192, 48, 27, 1 << 10, 1 << 10);

                                /**
                                 * Draw Three-Day Clock's Star for the Minute Tracker
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

                                sp1D0 = ((((0, gSaveContext.save.time)) * 1.3183594f) / 3600.0f) * 3600.0f;
                                sp1D0 = (s16)((0, gSaveContext.save.time) * 1.3183594f) - sp1D0;

                                func_8012C8D4(globalCtx->state.gfxCtx);

                                gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                                if (D_801BFB2C != 255) {
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 110, D_801BFB2C);
                                } else {
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 110, D_801BF97C);
                                }

                                gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                                gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                                gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

                                Matrix_InsertTranslation(0.0f, -86.0f, 0.0f, MTXMODE_NEW);
                                Matrix_Scale(1.0f, 1.0f, D_801BF980, MTXMODE_APPLY);
                                Matrix_InsertZRotation_f(-(sp1D0 * 0.0175f) / 10.0f, MTXMODE_APPLY);

                                gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx),
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
                            for (currentHour = 0; currentHour <= 24; currentHour++) {
                                if ((0, gSaveContext.save.time) < D_801BFB38[currentHour + 1]) {
                                    break;
                                }
                            }

                            /**
                             * Draw Three-Day Clock's Sun for the Day-Time Hours Tracker
                             */
                            time = gSaveContext.save.time;
                            sp1D8 = Math_SinS(time) * -40.0f;
                            temp_f14 = Math_CosS(time) * -34.0f;

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 100, 110, D_801BFB2C);

                            Matrix_InsertTranslation(sp1D8, temp_f14, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx),
                                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[16], 4, 0);

                            OVERLAY_DISP = func_8010DC58(OVERLAY_DISP, gThreeDayClockSunHourTex, 24, 24, 0);

                            /**
                             * Draw Three-Day Clock's Moon for the Night-Time Hours Tracker
                             */
                            sp1D8 = Math_SinS(time) * 40.0f;
                            temp_f14 = Math_CosS(time) * 34.0f;

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 55, D_801BFB2C);

                            Matrix_InsertTranslation(sp1D8, temp_f14, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx),
                                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[20], 4, 0);

                            OVERLAY_DISP = func_8010DC58(OVERLAY_DISP, gThreeDayClockMoonHourTex, 24, 24, 0);

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
                            Matrix_InsertTranslation(0.0f, XREG(43) / 10.0f, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            Matrix_InsertZRotation_f(-(sp1CC - 3.15f), MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx),
                                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                            // Draws Three-Day Clock's Hour Digit Above the Sun
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                                              PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, D_801BFB2C);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[24], 8, 0);

                            OVERLAY_DISP = func_8010DE38(OVERLAY_DISP, D_801BFB6C[currentHour], 4, 16, 11, 0);

                            // Colours the Three-Day Clocks's Hour Digit Above the Sun
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, D_801BFB2C);
                            gSP1Quadrangle(OVERLAY_DISP++, 4, 6, 7, 5, 0);

                            /**
                             * Draws Three-Day Clock's Hour Digit Above the Moon
                             */
                            // Rotates Three-Day Clock's Hour Digit To Above the Moon
                            Matrix_InsertTranslation(0.0f, XREG(43) / 10.0f, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            Matrix_InsertZRotation_f(-sp1CC, MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx),
                                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                            // Draws Three-Day Clock's Hour Digit Above the Moon
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0,
                                              PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, D_801BFB2C);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[32], 8, 0);

                            OVERLAY_DISP = func_8010DE38(OVERLAY_DISP, D_801BFB6C[currentHour], 4, 16, 11, 0);

                            // Colours the Three-Day Clocks's Hour Digit Above the Moon
                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, D_801BFB2C);
                            gSP1Quadrangle(OVERLAY_DISP++, 4, 6, 7, 5, 0);

                            // Unknown
                            gSPDisplayList(OVERLAY_DISP++, D_0E0001C8);

                            // Final Hours
                            if ((CURRENT_DAY >= 4) || ((CURRENT_DAY == 3) && ((0, gSaveContext.save.time) >= 5) &&
                                                       ((0, gSaveContext.save.time) < CLOCK_TIME(6, 0)))) {
                                sp1C6 = 0;
                                if ((0, gSaveContext.save.time) >= CLOCK_TIME(5, 0)) {
                                    // The Final Hours clock will flash red

                                    TIMED_STEP_TO(sFinalHoursClockDigitsRed,
                                                  D_801BFC04[sFinalHoursClockColorTargetIndex],
                                                  sFinalHoursClockColorTimer, colorStep);
                                    TIMED_STEP_TO(sFinalHoursClockFrameEnvRed,
                                                  D_801BFC08[sFinalHoursClockColorTargetIndex],
                                                  sFinalHoursClockColorTimer, colorStep);
                                    TIMED_STEP_TO(sFinalHoursClockFrameEnvGreen,
                                                  D_801BFC0C[sFinalHoursClockColorTargetIndex],
                                                  sFinalHoursClockColorTimer, colorStep);
                                    TIMED_STEP_TO(sFinalHoursClockFrameEnvBlue,
                                                  D_801BFC10[sFinalHoursClockColorTargetIndex],
                                                  sFinalHoursClockColorTimer, colorStep);

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

                                sp1E6 = D_801BFB2C;
                                if (sp1E6 != 0) {
                                    sp1E6 = 255;
                                }

                                func_8012C654(globalCtx->state.gfxCtx);

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

                                OVERLAY_DISP = func_8010D9F4(OVERLAY_DISP, gFinalHoursClockFrameTex, 3, 80, 13, 119,
                                                             202, 80, 13, 0, 0, 0, 1 << 10, 1 << 10);

                                temp_a1_7 =
                                    (-(CURRENT_DAY << 0x10) - (u16)((0, gSaveContext.save.time) - 0x4000)) + 0x40000;

                                temp_f0 = temp_a1_7 * 0.021972656f;

                                finalHoursClockSlots[0] = 0;
                                finalHoursClockSlots[1] = temp_f0 / 60.0f;
                                finalHoursClockSlots[2] = temp_f0 / 60.0f;

                                // digits for hours
                                while (finalHoursClockSlots[1] >= 10) {
                                    finalHoursClockSlots[0]++;
                                    finalHoursClockSlots[1] -= 10;
                                }

                                finalHoursClockSlots[3] = 0;
                                finalHoursClockSlots[4] = (s16)temp_f0 % 60;

                                // digits for minutes
                                while (finalHoursClockSlots[4] >= 10) {
                                    finalHoursClockSlots[3]++;
                                    finalHoursClockSlots[4] -= 10;
                                }

                                finalHoursClockSlots[6] = 0;
                                new_var1 = (finalHoursClockSlots[4] * 45.511112f);
                                finalHoursClockSlots[7] =
                                    temp_a1_7 - (u32)(new_var1 + (finalHoursClockSlots[2] * 2730.6667f));

                                // digits for seconds
                                while (finalHoursClockSlots[7] >= 10) {
                                    finalHoursClockSlots[6]++;
                                    finalHoursClockSlots[7] -= 10;
                                }

                                // Colon separating hours from minutes
                                finalHoursClockSlots[2] = 10;
                                // Colon separating minutes from seconds
                                finalHoursClockSlots[5] = 10;

                                /**
                                 * Draws Final-Hours Clock's Digits
                                 */
                                gDPPipeSync(OVERLAY_DISP++);
                                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, sFinalHoursClockDigitsRed, 0, 0, sp1E6);
                                gDPSetEnvColor(OVERLAY_DISP++, sFinalHoursClockDigitsRed, 0, 0, 0);

                                for (; sp1C6 < 8; sp1C6++) {
                                    OVERLAY_DISP = Gfx_TextureI8(OVERLAY_DISP, D_801BFC14[finalHoursClockSlots[sp1C6]],
                                                                 8, 8, D_801BFC40[sp1C6], 205, 8, 8, 1 << 10, 1 << 10);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    CLOSE_DISPS(globalCtx->state.gfxCtx);
}
#else
s16 D_801BFB2C = 255;
s16 D_801BFB30 = 0;
s16 D_801BFB34 = 0;
u16 D_801BFB38[] = {
    CLOCK_TIME(0, 0),  CLOCK_TIME(1, 0),  CLOCK_TIME(2, 0),  CLOCK_TIME(3, 0),  CLOCK_TIME(4, 0),
    CLOCK_TIME(5, 0),  CLOCK_TIME(6, 0),  CLOCK_TIME(7, 0),  CLOCK_TIME(8, 0),  CLOCK_TIME(9, 0),
    CLOCK_TIME(10, 0), CLOCK_TIME(11, 0), CLOCK_TIME(12, 0), CLOCK_TIME(13, 0), CLOCK_TIME(14, 0),
    CLOCK_TIME(15, 0), CLOCK_TIME(16, 0), CLOCK_TIME(17, 0), CLOCK_TIME(18, 0), CLOCK_TIME(19, 0),
    CLOCK_TIME(20, 0), CLOCK_TIME(21, 0), CLOCK_TIME(22, 0), CLOCK_TIME(23, 0), CLOCK_TIME(24, 0) - 1,
};
TexturePtr D_801BFB6C[] = {
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
TexturePtr D_801BFC14[] = {
    gFinalHoursClockDigit0Tex, gFinalHoursClockDigit1Tex, gFinalHoursClockDigit2Tex, gFinalHoursClockDigit3Tex,
    gFinalHoursClockDigit4Tex, gFinalHoursClockDigit5Tex, gFinalHoursClockDigit6Tex, gFinalHoursClockDigit7Tex,
    gFinalHoursClockDigit8Tex, gFinalHoursClockDigit9Tex, gFinalHoursClockColonTex,
};
s16 D_801BFC40[] = {
    0x007F, 0x0088, 0x0090, 0x0097, 0x00A0, 0x00A8, 0x00AF, 0x00B8,
};
void Interface_DrawClock(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_DrawClock.s")
#endif

void Interface_SetPerfectMinigame(GlobalContext* globalCtx, s16 rewardType) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 i;

    interfaceCtx->isMinigamePerfect = true;
    interfaceCtx->minigameRewardType = rewardType;
    interfaceCtx->unk_2FC[0] = 255;
    interfaceCtx->unk_2FC[1] = 165;
    interfaceCtx->unk_2FC[2] = 55;
    interfaceCtx->unk_2FC[3] = 255;
    interfaceCtx->unk_30A = 20;
    interfaceCtx->unk_308 = 0;
    interfaceCtx->unk_304 = 1;
    interfaceCtx->unk_30C = 0;

    for (i = 0; i < 8; i++) {
        interfaceCtx->unk_2AA[i] = 0;
        interfaceCtx->unk_29A[i] = 0;
        interfaceCtx->unk_28A[i] = interfaceCtx->unk_2AA[i];
        if (interfaceCtx->minigameRewardType == 1) {
            interfaceCtx->unk_2BC[i] = 140.0f;
            interfaceCtx->unk_2DC[i] = 100.0f;
        } else {
            interfaceCtx->unk_2DC[i] = 200.0f;
            interfaceCtx->unk_2BC[i] = 200.0f;
        }
    }
    interfaceCtx->unk_28A[0] = 1;
}

void func_8011B5C0(GlobalContext* globalCtx) {
    static u16 D_801BFC50[] = {
        0xC000, 0xE000, 0x0000, 0x2000, 0xA000, 0x8000, 0x6000, 0x4000,
    };
    static s16 D_801BFC60[][3] = {
        255, 255, 255, 255, 165, 55,
    };
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 i;
    s16 count;
    s16 colorStepR;
    s16 colorStepG;
    s16 colorStepB;

    for (count = 0, i = 0; i < interfaceCtx->unk_304; i++, count += 4) {
        if (interfaceCtx->unk_28A[i] == 1) {
            interfaceCtx->unk_29A[i] = D_801BFC50[i] + 0xA000;
            interfaceCtx->unk_28A[i] = 2;
        } else if (interfaceCtx->unk_28A[i] == 2) {
            interfaceCtx->unk_29A[i] -= 0x800;
            if (interfaceCtx->unk_29A[i] == D_801BFC50[i]) {
                interfaceCtx->unk_28A[i] = 3;
            }
        } else if (interfaceCtx->unk_28A[i] == 4) {
            interfaceCtx->unk_29A[i] -= 0x800;
            if (interfaceCtx->unk_29A[i] == (u16)(D_801BFC50[i] - 0x8000)) {
                interfaceCtx->unk_28A[i] = 0;
            }
        }
    }

    if ((interfaceCtx->unk_28A[interfaceCtx->unk_304] == 0) && (interfaceCtx->unk_304 < 8)) {
        interfaceCtx->unk_28A[interfaceCtx->unk_304] = 1;
        interfaceCtx->unk_2BC[interfaceCtx->unk_304] = 140.0f;
        interfaceCtx->unk_2DC[interfaceCtx->unk_304] = 100.0f;
        interfaceCtx->unk_29A[interfaceCtx->unk_304] = D_801BFC50[interfaceCtx->unk_304] + 0xA000;
        interfaceCtx->unk_304++;
    }

    if ((interfaceCtx->unk_304 == 8) && (interfaceCtx->unk_28A[7] == 3)) {

        colorStepR = ABS_ALT(interfaceCtx->unk_2FC[0] - D_801BFC60[interfaceCtx->unk_308][0]) / interfaceCtx->unk_30A;
        colorStepG = ABS_ALT(interfaceCtx->unk_2FC[1] - D_801BFC60[interfaceCtx->unk_308][1]) / interfaceCtx->unk_30A;
        colorStepB = ABS_ALT(interfaceCtx->unk_2FC[2] - D_801BFC60[interfaceCtx->unk_308][2]) / interfaceCtx->unk_30A;

        if (interfaceCtx->unk_2FC[0] >= D_801BFC60[interfaceCtx->unk_308][0]) {
            interfaceCtx->unk_2FC[0] -= colorStepR;
        } else {
            interfaceCtx->unk_2FC[0] += colorStepR;
        }

        if (interfaceCtx->unk_2FC[1] >= D_801BFC60[interfaceCtx->unk_308][1]) {
            interfaceCtx->unk_2FC[1] -= colorStepG;
        } else {
            interfaceCtx->unk_2FC[1] += colorStepG;
        }

        if (interfaceCtx->unk_2FC[2] >= D_801BFC60[interfaceCtx->unk_308][2]) {
            interfaceCtx->unk_2FC[2] -= colorStepB;
        } else {
            interfaceCtx->unk_2FC[2] += colorStepB;
        }

        interfaceCtx->unk_30A--;

        if (interfaceCtx->unk_30A == 0) {
            interfaceCtx->unk_30A = 20;
            interfaceCtx->unk_308 ^= 1;
            interfaceCtx->unk_30C++;

            if (interfaceCtx->unk_30C == 6) {
                for (i = 0; i < 8; i++) {
                    interfaceCtx->unk_28A[i] = 4;
                }
            }
        }
    }

    for (count = 0, i = 0; i < 8; i++) {
        if (interfaceCtx->unk_28A[i] == 0) {
            count++;
        }
    }

    if (count == 8) {
        interfaceCtx->isMinigamePerfect = false;
    }
}

s16 D_801BFC6C[] = {
    0x004E, 0x0036, 0x001D, 0x0005, 0xFFEE, 0xFFD6, 0xFFBD, 0xFFAB,
};
s16 D_801BFC7C[] = {
    0x00B4, 0x00B4, 0x00B4, 0x00B4, 0xFF4C, 0xFF4C, 0xFF4C, 0xFF4C,
};
s16 D_801BFC8C[2][3] = {
    { 0xFF, 0xFF, 0xFF }, 
    { 0xFF, 0xA5, 0x37 },
};
void func_8011B9E0(GlobalContext* globalCtx) {
    s16 i;
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 stepVar1;
    s16 stepVar2;
    s16 stepVar3;
    s16 new_var;
    s16 j = 0; // unused incrementer

    for (i = 0; i < interfaceCtx->unk_304; i++, j += 4) {
        if (interfaceCtx->unk_28A[i] == 1) {
            interfaceCtx->unk_29A[i] = i << 0xD;
            interfaceCtx->unk_2BC[i] = 200.0f;
            interfaceCtx->unk_2DC[i] = 200.0f;
            interfaceCtx->unk_2AA[i] = 0;
            interfaceCtx->unk_28A[i] = 2;
        } else if (interfaceCtx->unk_28A[i] == 2) {
            interfaceCtx->unk_29A[i] -= 0x800;
            interfaceCtx->unk_2BC[i] -= 8.0f;
            interfaceCtx->unk_2DC[i] -= 8.0f;
            if (interfaceCtx->unk_2BC[i] <= 0.0f) {
                interfaceCtx->unk_2DC[i] = 0.0f;
                interfaceCtx->unk_2BC[i] = 0.0f;
                interfaceCtx->unk_28A[i] = 3;
                if (i == 7) {
                    interfaceCtx->unk_30A = 5;
                    interfaceCtx->unk_28A[7] = 4;
                    interfaceCtx->unk_28A[6] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[5] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[4] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[3] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[2] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[1] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[0] = interfaceCtx->unk_28A[7];
                }
            }
        } else if (interfaceCtx->unk_28A[i] == 4) {

            stepVar1 = ABS_ALT(interfaceCtx->unk_2AA[i] - D_801BFC6C[i]) / interfaceCtx->unk_30A;
            if (interfaceCtx->unk_2AA[i] >= D_801BFC6C[i]) {
                interfaceCtx->unk_2AA[i] -= stepVar1;
            } else {
                interfaceCtx->unk_2AA[i] += stepVar1;
            }
        } else if (interfaceCtx->unk_28A[i] == 6) {

            stepVar1 = ABS_ALT(interfaceCtx->unk_2AA[i] - D_801BFC7C[i]) / interfaceCtx->unk_30A;
            if (interfaceCtx->unk_2AA[i] >= D_801BFC7C[i]) {
                interfaceCtx->unk_2AA[i] -= stepVar1;
            } else {
                interfaceCtx->unk_2AA[i] += stepVar1;
            }
        }
    
    }

    if ((interfaceCtx->unk_28A[0] == 4) || (interfaceCtx->unk_28A[0] == 6)) {
        if (interfaceCtx->unk_28A[0] == 6) {
            new_var = interfaceCtx->unk_2FC[3] / interfaceCtx->unk_30A;
            interfaceCtx->unk_2FC[3] -= new_var;
        }
        interfaceCtx->unk_30A--;
        if (interfaceCtx->unk_30A == 0) {

            if (interfaceCtx->unk_28A[0] == 4) {
                for (i = 0; i < 8; i++) {
                    interfaceCtx->unk_28A[i] = 5;
                }
                interfaceCtx->unk_30A = 20;
            } else {
                for (i = 0; i < 8; i++) {
                    interfaceCtx->unk_28A[i] = 0;
                }
                interfaceCtx->isMinigamePerfect = 0;
            }
        }
    }

    if (interfaceCtx->unk_28A[interfaceCtx->unk_304] == 0) {
        if (interfaceCtx->unk_304 < 8) {
            interfaceCtx->unk_28A[interfaceCtx->unk_304] = 1;
            interfaceCtx->unk_304++;
        }
    }
    if (interfaceCtx->unk_304 == 8) {
        if (interfaceCtx->unk_28A[7] == 5) {

            stepVar1 = ABS_ALT(interfaceCtx->unk_2FC[0] - D_801BFC8C[interfaceCtx->unk_308][0]) / interfaceCtx->unk_30A;
            stepVar2 = ABS_ALT(interfaceCtx->unk_2FC[1] - D_801BFC8C[interfaceCtx->unk_308][1]) / interfaceCtx->unk_30A;
            stepVar3 = ABS_ALT(interfaceCtx->unk_2FC[2] - D_801BFC8C[interfaceCtx->unk_308][2]) / interfaceCtx->unk_30A;

            if (interfaceCtx->unk_2FC[0] >= D_801BFC8C[interfaceCtx->unk_308][0]) {
                interfaceCtx->unk_2FC[0] -= stepVar1;
            } else {
                interfaceCtx->unk_2FC[0] += stepVar1;
            }

            if (interfaceCtx->unk_2FC[1] >= D_801BFC8C[interfaceCtx->unk_308][1]) {
                interfaceCtx->unk_2FC[1] -= stepVar2;
            } else {
                interfaceCtx->unk_2FC[1] += stepVar2;
            }

            if (interfaceCtx->unk_2FC[2] >= D_801BFC8C[interfaceCtx->unk_308][2]) {
                interfaceCtx->unk_2FC[2] -= stepVar3;
            } else {
                interfaceCtx->unk_2FC[2] += stepVar3;
            }

            interfaceCtx->unk_30A--;
            if (interfaceCtx->unk_30A == 0) {
                interfaceCtx->unk_30A = 20;
                interfaceCtx->unk_308 ^= 1;
                interfaceCtx->unk_30C++;
                if (interfaceCtx->unk_30C == 6) {
                    for (i = 0; i < 8; i++) {
                        interfaceCtx->unk_28A[i] = 6;
                    }
                    interfaceCtx->unk_30A = 5;
                }
            }
        }
    }
}

s16 D_801BFC98[] = {
    0x004E, 0x0036,
    0x001D, 0x0005,
    0xFFEE, 0xFFD6,
    0xFFBD, 0xFFAB,
};
u16 D_801BFCA8[] = {
    0xC000, 0xE000,
    0x0000, 0x2000,
    0xA000, 0x8000,
    0x6000, 0x4000,
};
s16 D_801BFCB8[2][3] = {
    { 0xFF, 0xFF, 0xFF }, 
    { 0xFF, 0xA5, 0x37 },
};
void func_8011BF70(GlobalContext* globalCtx) {
    s16 i;
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 stepVar1;
    s16 stepVar2;
    s16 stepVar3;
    s16 stepVar4;
    s16 j = 0;

    for (i = 0; i < interfaceCtx->unk_304; i++, j += 4) {
        if (interfaceCtx->unk_28A[i] == 1) {
            interfaceCtx->unk_29A[i] = i << 0xD;
            interfaceCtx->unk_2BC[i] = 200.0f;
            interfaceCtx->unk_2DC[i] = 200.0f;
            interfaceCtx->unk_2AA[i] = 0;
            interfaceCtx->unk_28A[i] = 2;
        } else if (interfaceCtx->unk_28A[i] == 2) {
            interfaceCtx->unk_29A[i] += -0x800;
            interfaceCtx->unk_2BC[i] -= 8.0f;
            interfaceCtx->unk_2DC[i] -= 8.0f;
            if (interfaceCtx->unk_2BC[i] <= 0.0f) {
                interfaceCtx->unk_2DC[i] = 0.0f;
                interfaceCtx->unk_2BC[i] = 0.0f;
                interfaceCtx->unk_28A[i] = 3;
                if (i == 7) {
                    interfaceCtx->unk_28A[7] = 4;
                    interfaceCtx->unk_30A = 5;
                    interfaceCtx->unk_28A[6] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[5] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[4] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[3] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[2] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[1] = interfaceCtx->unk_28A[7];
                    interfaceCtx->unk_28A[0] = interfaceCtx->unk_28A[7];
                }
            }
        } else if (interfaceCtx->unk_28A[i] == 4) {
            stepVar1 = ABS_ALT(interfaceCtx->unk_2AA[i] - D_801BFC98[i]) / interfaceCtx->unk_30A;
            if (interfaceCtx->unk_2AA[i] >= D_801BFC98[i]) {
                interfaceCtx->unk_2AA[i] -= stepVar1;
            } else {
                interfaceCtx->unk_2AA[i] += stepVar1;
            }
        } else if (interfaceCtx->unk_28A[i] == 6) {
            interfaceCtx->unk_29A[i] -= 0x800;
            if (interfaceCtx->unk_29A[i] == (u16)(D_801BFCA8[i] - 0x8000)) {
                interfaceCtx->unk_28A[i] = 0;
            }
        }
    }

    if (interfaceCtx->unk_28A[0] == 4) {
        interfaceCtx->unk_30A--;
        if (interfaceCtx->unk_30A == 0) {
            for (i = 0; i < 8; i++) {
                interfaceCtx->unk_28A[i] = 5;
            }
            interfaceCtx->unk_30A = 20;
        }
    }

    if ((interfaceCtx->unk_28A[interfaceCtx->unk_304] == 0) && (interfaceCtx->unk_304 < 8)) {
        interfaceCtx->unk_28A[interfaceCtx->unk_304] = 1;
        interfaceCtx->unk_304++;
    }

    if ((interfaceCtx->unk_304 == 8) && (interfaceCtx->unk_28A[7] == 5)) {

        stepVar1 = ABS_ALT(interfaceCtx->unk_2FC[0] - D_801BFCB8[interfaceCtx->unk_308][0]) / interfaceCtx->unk_30A;
        stepVar2 = ABS_ALT(interfaceCtx->unk_2FC[1] - D_801BFCB8[interfaceCtx->unk_308][1]) / interfaceCtx->unk_30A;
        stepVar3 = ABS_ALT(interfaceCtx->unk_2FC[2] - D_801BFCB8[interfaceCtx->unk_308][2]) / interfaceCtx->unk_30A;

        if (interfaceCtx->unk_2FC[0] >= D_801BFCB8[interfaceCtx->unk_308][0]) {
            interfaceCtx->unk_2FC[0] -= stepVar1;
        } else {
            interfaceCtx->unk_2FC[0] += stepVar1;
        }

        if (interfaceCtx->unk_2FC[1] >= D_801BFCB8[interfaceCtx->unk_308][1]) {
            interfaceCtx->unk_2FC[1] -= stepVar2;
        } else {
            interfaceCtx->unk_2FC[1] += stepVar2;
        }

        if (interfaceCtx->unk_2FC[2] >= D_801BFCB8[interfaceCtx->unk_308][2]) {
            interfaceCtx->unk_2FC[2] -= stepVar3;
        } else {
            interfaceCtx->unk_2FC[2] += stepVar3;
        }

        interfaceCtx->unk_30A--;
        if (interfaceCtx->unk_30A == 0) {
            interfaceCtx->unk_30A = 20;
            interfaceCtx->unk_308 ^= 1;
            interfaceCtx->unk_30C++;
            if (interfaceCtx->unk_30C == 6) {
                for (i = 0; i < 8; i++) {
                    interfaceCtx->unk_2BC[i] = 140.0f;
                    interfaceCtx->unk_2DC[i] = 100.0f;
                    interfaceCtx->unk_29A[i] = D_801BFCA8[i];
                    interfaceCtx->unk_28A[i] = 6;
                }
                interfaceCtx->unk_30A = 5;
            }
        }
    }

    j = 0;
    for (i = 0; i < 8; i++) {
        if (interfaceCtx->unk_28A[i] == 0) {
            j++;
        }
    }

    if (j == 8) {
        interfaceCtx->isMinigamePerfect = 0;
    }
}



void Interface_DrawMinigamePerfect(GlobalContext* globalCtx) {
    static TexturePtr D_801BFCC4[] = {
        gMinigameLetterPTex, gMinigameLetterETex, gMinigameLetterRTex, gMinigameLetterFTex,
        gMinigameLetterETex, gMinigameLetterCTex, gMinigameLetterTTex, gMinigameExclamationTex,
    };
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    f32 temp_f22;
    f32 temp_f24;
    s16 i;
    s16 phi_s7;

    OPEN_DISPS(globalCtx->state.gfxCtx);

    func_8012C8D4(globalCtx->state.gfxCtx);

    gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                      PRIMITIVE, 0);

    for (phi_s7 = 0, i = 0; i < 8; phi_s7 += 4, i++) {
        if (interfaceCtx->unk_28A[i] != 0) {
            temp_f24 = Math_SinS(interfaceCtx->unk_29A[i]) * interfaceCtx->unk_2BC[i];
            temp_f22 = Math_CosS(interfaceCtx->unk_29A[i]) * interfaceCtx->unk_2DC[i];

            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->unk_2FC[3]);

            Matrix_InsertTranslation(temp_f24, temp_f22, 0.0f, MTXMODE_NEW);
            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx),
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[phi_s7] + 0x2C, 4, 0);

            OVERLAY_DISP = func_8010DE38(OVERLAY_DISP, D_801BFCC4[i], 4, 32, 33, 0);

            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, interfaceCtx->unk_2FC[0], interfaceCtx->unk_2FC[1],
                            interfaceCtx->unk_2FC[2], interfaceCtx->unk_2FC[3]);

            Matrix_InsertTranslation(temp_f24, temp_f22, 0.0f, MTXMODE_NEW);
            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx),
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[phi_s7] + 0x4C, 4, 0);

            OVERLAY_DISP = func_8010DE38(OVERLAY_DISP, D_801BFCC4[i], 4, 32, 33, 0);
        }
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx);
}

#ifdef NON_MATCHING
void func_8011C808(GlobalContext* globalCtx) {
    if (globalCtx->actorCtx.unk5 & 2) {
        Audio_QueueSeqCmd(0xE0000100);
    }

    gSaveContext.save.day = 4;
    gSaveContext.save.daysElapsed = gSaveContext.save.day;
    gSaveContext.save.time = 0x400A;
    globalCtx->nextEntranceIndex = 0x54C0;
    gSaveContext.nextCutsceneIndex = 0;
    globalCtx->sceneLoadFlag = 0x14;
    globalCtx->unk_1887F = 3;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011C808.s")
#endif

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011C898.s")

s32 D_801BFCE4[] = {
    0x00000000,
};
s32 D_801BFCE8[] = {
    0x00000000,
};
s32 D_801BFCEC[] = {
    0x00000000,
};
s32 D_801BFCF0[] = {
    0x00000000,
};
s32 D_801BFCF4[] = {
    0x00000000,
};
s32 D_801BFCF8[] = {
    0x00630000,
};
s16 D_801BFCFC[] = {
    0x10, 0x19, 0x22, 0x2A, 0x33, 0x3C, 0x44, 0x4D,
};
s16 D_801BFD0C[] = {
    9, 9, 8, 9, 9, 8, 9, 9,
};
void Interface_DrawTimers(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_DrawTimers.s")

void Interface_UpdateTimers(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_UpdateTimers.s")

void Interface_DrawMinigameIcons(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_DrawMinigameIcons.s")

// rupeeDigitsFirst
s16 D_801BFD1C[] = { 1, 0, 0, 0 };
// rupeeDigitsCount
s16 D_801BFD24[] = { 2, 3, 3, 0 };
// rupeeIconPrimColor
Color_RGB16 D_801BFD2C[] = {
    { 200, 255, 100 },
    { 170, 170, 255 },
    { 255, 105, 105 },
};
// rupeeIconEnvColor
Color_RGB16 D_801BFD40[] = {
    { 0, 80, 0 },
    { 10, 10, 80 },
    { 40, 10, 0 },
};
// minigameCountdownTexs
TexturePtr D_801BFD54[] = {
    gMinigameCountdown3Tex,
    gMinigameCountdown2Tex,
    gMinigameCountdown1Tex,
    gMinigameCountdownGoTex,
};
// minigameCountdownTexHeights
s16 D_801BFD64[] = { 24, 24, 24, 40 };
// minigameCountdownPrimColor
Color_RGB16 D_801BFD6C[] = {
    { 100, 255, 100 },
    { 255, 255, 60 },
    { 255, 100, 0 },
    { 120, 170, 255 },
};
// grandma's story pictures
s32 D_801BFD84[] = {
    0x07000000,
    0x07012C00,
};
// grandma's story TLUT
u32 D_801BFD8C[] = {
    0x07025800,
    0x07025A00,
};
#ifdef NON_MATCHING
void Interface_Draw(GlobalContext* globalCtx) {
    s32 pad;
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    Player* player = GET_PLAYER(globalCtx);
    Gfx* gfx;
    s16 sp2CE;
    s16 sp2CC;
    s16 sp2CA;
    s16 sp2C8;
    PauseContext* pauseCtx = &globalCtx->pauseCtx;
    f32 sp2C0;
    s16 counterDigits[4]; // sp2B8
    s16 sp4C;

    OPEN_DISPS(globalCtx->state.gfxCtx);

    gSPSegment(OVERLAY_DISP++, 0x02, interfaceCtx->parameterSegment);
    gSPSegment(OVERLAY_DISP++, 0x09, interfaceCtx->doActionSegment);
    gSPSegment(OVERLAY_DISP++, 0x08, interfaceCtx->iconItemSegment);
    gSPSegment(OVERLAY_DISP++, 0x0B, interfaceCtx->mapSegment);

    if (pauseCtx->debugState == 0) {
        Interface_InitVertices(globalCtx);
        func_801170B8(interfaceCtx);

        if (interfaceCtx->storyState == 2) {
            gSPSegment(OVERLAY_DISP++, 0x07, interfaceCtx->storySegment);
            func_8012C628(globalCtx->state.gfxCtx);

            gDPSetTextureFilter(POLY_OPA_DISP++, G_TF_POINT);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
            gSPLoadUcodeEx(OVERLAY_DISP++, OS_K0_TO_PHYSICAL(D_801ABAB0), OS_K0_TO_PHYSICAL(D_801E3BB0), 0x800);

            gfx = OVERLAY_DISP;
            func_80172758(&gfx, D_801BFD84[interfaceCtx->storyType], D_801BFD8C[interfaceCtx->storyType], 0x140, 0xF0,
                          2, 1, 0x8000, 0x100, 0.0f, 0.0f, 1.0f, 1.0f, 0);
            OVERLAY_DISP = gfx;

            gSPLoadUcodeEx(OVERLAY_DISP++, SysUcode_GetUCode(), SysUcode_GetUCodeData(), 0x800);
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, gGameInfo->data[0x59B]);
            gDPFillRectangle(OVERLAY_DISP++, 0, 0, 320, 240);
        }

        LifeMeter_Draw(globalCtx);

        func_8012C654(globalCtx->state.gfxCtx);

        // Rupee Icon
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, D_801BFD2C[CUR_UPG_VALUE(4)].r, D_801BFD2C[CUR_UPG_VALUE(4)].g,
                        D_801BFD2C[CUR_UPG_VALUE(4)].b, interfaceCtx->magicAlpha);
        gDPSetEnvColor(OVERLAY_DISP++, D_801BFD40[CUR_UPG_VALUE(4)].r, D_801BFD40[CUR_UPG_VALUE(4)].g,
                       D_801BFD40[CUR_UPG_VALUE(4)].b, 255);
        OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, gRupeeCounterIconTex, 16, 16, 26, 206, 16, 16, 1 << 10, 1 << 10);

        switch (globalCtx->sceneNum) {
            case SCENE_INISIE_N:
            case SCENE_INISIE_R:
            case SCENE_MITURIN:
            case SCENE_HAKUGIN:
            case SCENE_SEA:
                if (gSaveContext.save.inventory.dungeonKeys[(void)0, gSaveContext.mapIndex] >= 0) {
                    // Small Key Icon
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 230, 255, interfaceCtx->magicAlpha);
                    gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 20, 255);
                    OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, gSmallKeyCounterIconTex, 16, 16, 26, 190, 16, 16,
                                                  1 << 10, 1 << 10);

                    // Small Key Counter
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE,
                                      TEXEL0, 0, PRIMITIVE, 0);

                    counterDigits[2] = 0;
                    counterDigits[3] = gSaveContext.save.inventory.dungeonKeys[(void)0, gSaveContext.mapIndex];

                    while (counterDigits[3] >= 10) {
                        counterDigits[2]++;
                        counterDigits[3] -= 10;
                    }

                    sp2CA = 42;

                    if (counterDigits[2] != 0) {
                        gDPPipeSync(OVERLAY_DISP++);
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->magicAlpha);

                        OVERLAY_DISP = Gfx_TextureI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[2]),
                                                     8, 16, 43, 191, 8, 16, 1 << 10, 1 << 10);

                        gDPPipeSync(OVERLAY_DISP++);
                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
                        gSPTextureRectangle(OVERLAY_DISP++, 168, 760, 200, 824, G_TX_RENDERTILE, 0, 0, 1 << 10,
                                            1 << 10);

                        sp2CA += 8;
                    }

                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->magicAlpha);

                    OVERLAY_DISP = Gfx_TextureI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[3]), 8,
                                                 16, sp2CA + 1, 191, 8, 16, 1 << 10, 1 << 10);

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
                counterDigits[3] = func_8012F22C(globalCtx->sceneNum);

                while (counterDigits[3] >= 10) {
                    counterDigits[2]++;
                    counterDigits[3] -= 10;
                }

                sp2CA = 42;

                if (counterDigits[2] != 0) {
                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->magicAlpha);

                    OVERLAY_DISP = Gfx_TextureI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[2]), 8,
                                                 16, 43, 191, 8, 16, 1 << 10, 1 << 10);

                    gDPPipeSync(OVERLAY_DISP++);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, interfaceCtx->magicAlpha);
                    gSPTextureRectangle(OVERLAY_DISP++, 168, 760, 200, 824, G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

                    sp2CA += 8;
                }

                gDPPipeSync(OVERLAY_DISP++);
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->magicAlpha);

                OVERLAY_DISP = Gfx_TextureI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[3]), 8, 16,
                                             sp2CA + 1, 191, 8, 16, 1 << 10, 1 << 10);

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

        sp2CC = D_801BFD1C[CUR_UPG_VALUE(UPG_WALLET)];
        sp2C8 = D_801BFD24[CUR_UPG_VALUE(UPG_WALLET)];

        sp4C = interfaceCtx->magicAlpha;
        if (sp4C > 180) {
            sp4C = 180;
        }

        for (sp2CE = 0, sp2CA = 42; sp2CE < sp2C8; sp2CE++, sp2CC++, sp2CA += 8) {
            gDPPipeSync(OVERLAY_DISP++);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, sp4C);

            OVERLAY_DISP = Gfx_TextureI8(OVERLAY_DISP, (u8*)gCounterDigit0Tex + (8 * 16 * counterDigits[sp2CC]), 8, 16,
                                         sp2CA + 1, 207, 8, 16, 1 << 10, 1 << 10);

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

        Interface_DrawMagicBar(globalCtx);
        func_8010A54C(globalCtx);

        if ((gGameInfo->data[0xBE] != 2) && (gGameInfo->data[0xBE] != 3)) {
            func_800B5208(&globalCtx->actorCtx.targetContext, globalCtx); // Draw Z-Target
        }

        func_8012C654(globalCtx->state.gfxCtx);

        Interface_DrawItemButtons(globalCtx);

        if (player->transformation == ((void)0, gSaveContext.save.playerForm)) {
            func_80118084(globalCtx);
        }

        func_80118890(globalCtx);
        func_80118BA4(globalCtx);
        func_80119030(globalCtx);

        if ((globalCtx->pauseCtx.state == 0) && (globalCtx->pauseCtx.debugState == 0)) {
            if ((interfaceCtx->unk_280 != 0) && (interfaceCtx->unk_280 < 20)) {
                // Minigame Countdown
                if (((u32)interfaceCtx->unk_280 % 2) == 0) {

                    sp2CE = (interfaceCtx->unk_280 >> 1) - 1;
                    sp2C0 = interfaceCtx->unk_284 / 100.0f;

                    if (sp2CE == 3) {
                        interfaceCtx->actionVtx[0x28].v.ob[0] = interfaceCtx->actionVtx[0x2A].v.ob[0] = -0x14;
                        interfaceCtx->actionVtx[0x29].v.ob[0] = interfaceCtx->actionVtx[0x2B].v.ob[0] =
                            interfaceCtx->actionVtx[0x28].v.ob[0] + 0x28;
                        interfaceCtx->actionVtx[0x29].v.tc[0] = interfaceCtx->actionVtx[0x2B].v.tc[0] = 0x500;
                    }

                    interfaceCtx->actionVtx[0x2A].v.tc[1] = interfaceCtx->actionVtx[0x2B].v.tc[1] = 0x400;

                    func_8012C8D4(globalCtx->state.gfxCtx);

                    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                    gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, D_801BFD6C[sp2CE].r, D_801BFD6C[sp2CE].g, D_801BFD6C[sp2CE].b,
                                    interfaceCtx->unk_282);

                    Matrix_InsertTranslation(0.0f, -40.0f, 0.0f, MTXMODE_NEW);
                    Matrix_Scale(sp2C0, sp2C0, 0.0f, MTXMODE_APPLY);

                    gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx),
                              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                    gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0x28], 4, 0);

                    OVERLAY_DISP = func_8010DC58(OVERLAY_DISP, D_801BFD54[sp2CE], D_801BFD64[sp2CE], 32, 0);
                }
            } else {
                Interface_DrawClock(globalCtx);
            }
        }

        if (interfaceCtx->isMinigamePerfect != 0) {
            Interface_DrawMinigamePerfect(globalCtx);
        }

        Interface_DrawMinigameIcons(globalCtx);
        Interface_DrawTimers(globalCtx);
    }

    // PictoBox
    if (D_801BF884 == 1) {

        func_8012C654(globalCtx->state.gfxCtx);

        gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
        gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, 255);
        gDPLoadTextureBlock_4b(OVERLAY_DISP++, gPictoBoxFocusBorderTex, G_IM_FMT_IA, 16, 16, 0, G_TX_MIRROR | G_TX_WRAP,
                               G_TX_MIRROR | G_TX_WRAP, 4, 4, G_TX_NOLOD, G_TX_NOLOD);

        gSPTextureRectangle(OVERLAY_DISP++, YREG(32) * 4, YREG(33) * 4, (YREG(32) * 4) + 0x40, (YREG(33) * 4) + 0x40,
                            G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        gSPTextureRectangle(OVERLAY_DISP++, YREG(34) * 4, YREG(35) * 4, (YREG(34) * 4) + 0x40, (YREG(35) * 4) + 0x40,
                            G_TX_RENDERTILE, 0x200, 0, 1 << 10, 1 << 10);
        gSPTextureRectangle(OVERLAY_DISP++, YREG(36) * 4, YREG(37) * 4, (YREG(36) * 4) + 0x40, (YREG(37) * 4) + 0x40,
                            G_TX_RENDERTILE, 0, 0x200, 1 << 10, 1 << 10);
        gSPTextureRectangle(OVERLAY_DISP++, YREG(38) * 4, YREG(39) * 4, (YREG(38) * 4) + 0x40, (YREG(39) * 4) + 0x40,
                            G_TX_RENDERTILE, 0x200, 0x200, 1 << 10, 1 << 10);

        gDPLoadTextureBlock_4b(OVERLAY_DISP++, gPictoBoxFocusIconTex, G_IM_FMT_I, 32, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSPTextureRectangle(OVERLAY_DISP++, YREG(40) * 4, YREG(41) * 4, (YREG(40) * 4) + 0x80, (YREG(41) * 4) + 0x40,
                            G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);

        gDPLoadTextureBlock_4b(OVERLAY_DISP++, gPictoBoxFocusTextTex, G_IM_FMT_I, 32, 8, 0, G_TX_NOMIRROR | G_TX_WRAP,
                               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSPTextureRectangle(OVERLAY_DISP++, YREG(42) * 4, YREG(43) * 4, (YREG(42) * 4) + 0x80, (YREG(43) * 4) + 0x20,
                            G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
    }

    if (D_801BF884 >= 2) {
        if ((globalCtx->actorCtx.unk5 & 4) == 0) {
            func_801663C4((u8*)((globalCtx->unk_18E5C != NULL) ? globalCtx->unk_18E5C : D_801FBB90),
                          gSaveContext.pictoPhoto, 0x4600);

            interfaceCtx->unk_224 = 0;
            interfaceCtx->unk_222 = interfaceCtx->unk_224;

            D_801BF884 = 0;
            gSaveContext.unk_3F22 = 0;
            Interface_ChangeAlpha(50);

        } else {
            s16 phi_a2;
            s16 phi_t0_2;

            if (D_801BF884 == 2) {
                D_801BF884 = 3;
                func_801518B0(globalCtx, 0xF8, NULL);
                Interface_ChangeAlpha(1);
                player->stateFlags1 |= 0x200;
            }

            if (1) {}

            gDPPipeSync(OVERLAY_DISP++);
            gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 200, 200, 250);
            gDPFillRectangle(OVERLAY_DISP++, 70, 22, 251, 151);

            func_8012C654(globalCtx->state.gfxCtx);

            gDPSetRenderMode(OVERLAY_DISP++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEI_PRIM, G_CC_MODULATEI_PRIM);
            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 250, 160, 160, 255);

            phi_a2 = 0x1F;
            for (phi_t0_2 = 0; phi_t0_2 < 14; phi_t0_2++, phi_a2 += 8) {
                gDPLoadTextureBlock(OVERLAY_DISP++,
                                    (u8*)((globalCtx->unk_18E5C != NULL) ? globalCtx->unk_18E5C : D_801FBB90) +
                                        (0x500 * phi_t0_2),
                                    G_IM_FMT_I, G_IM_SIZ_8b, 160, 8, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

                gSPTextureRectangle(OVERLAY_DISP++, ((void)0, 320), phi_a2 * 4, ((void)0, 960), phi_a2 * 4 + 0x20,
                                    G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
            }
        }
    }

    if (interfaceCtx->unk_264 != 0) {
        gDPPipeSync(OVERLAY_DISP++);
        gSPDisplayList(OVERLAY_DISP++, D_801BF988);
        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, interfaceCtx->unk_264);
        gSPDisplayList(OVERLAY_DISP++, D_0E0002E0);
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx);
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_Draw.s")
#endif

#ifdef NON_MATCHING
void Interface_LoadStory(GlobalContext* globalCtx, s32 block) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    switch (interfaceCtx->storyState) {
        case 0:
            if (interfaceCtx->storySegment == NULL) {
                break;
            }
            osCreateMesgQueue(&interfaceCtx->storyMsgQueue, interfaceCtx->storyMsgBuf, 1);
            DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184, interfaceCtx->storySegment, interfaceCtx->storyAddr,
                                   interfaceCtx->storySize, 0, &interfaceCtx->storyMsgQueue, NULL);
            interfaceCtx->storyState = 1;
        case 1:
            if (osRecvMesg(&interfaceCtx->storyMsgQueue, NULL, block) == 0) {
                interfaceCtx->storyState = 2;
            }
            break;
    }
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_LoadStory.s")
#endif

void Interface_AllocStory(GlobalContext* globalCtx) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    interfaceCtx->storyAddr = (u32)_story_staticSegmentRomStart;
    interfaceCtx->storySize = (u32)_story_staticSegmentRomEnd - (u32)_story_staticSegmentRomStart;

    if (interfaceCtx->storySegment == NULL) {
        interfaceCtx->storySegment = ZeldaArena_Malloc(interfaceCtx->storySize);
    }
    Interface_LoadStory(globalCtx, 0);
}

void Interface_Update(GlobalContext* globalCtx) {
    static u8 D_801BFD94 = 0;
    static s16 D_801BFD98 = 0;
    static s16 D_801BFD9C = 0;
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    MessageContext* msgCtx = &globalCtx->msgCtx;
    Player* player = GET_PLAYER(globalCtx);
    s16 alpha;
    s16 alpha1;
    u16 action;
    s16 health;

    if ((globalCtx->pauseCtx.state == 0) && (globalCtx->pauseCtx.debugState == 0)) {
        if (globalCtx->gameOverCtx.state == 0) {
            func_80111CB4(globalCtx);
        }
    }

    switch (gSaveContext.unk_3F20) {
        case 0x1:
        case 0x2:
        case 0x8:
            alpha = 255 - (gSaveContext.unk_3F24 << 5);
            if (alpha < 0) {
                alpha = 0;
            }

            func_8010F1A8(globalCtx, alpha);
            health = 2; // Fake
            gSaveContext.unk_3F24 += health;
            if (alpha == 0) {
                gSaveContext.unk_3F20 = 0;
            }
            break;
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
        case 0x9:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
            alpha = 255 - (gSaveContext.unk_3F24 << 5);
            if (alpha < 0) {
                alpha = 0;
            }

            func_8010F1A8(globalCtx, alpha);
            gSaveContext.unk_3F24++;
            if (alpha == 0) {
                gSaveContext.unk_3F20 = 0;
            }
            break;

        case 0x32:
            alpha = 255 - (gSaveContext.unk_3F24 << 5);
            if (alpha < 0) {
                alpha = 0;
            }

            alpha1 = 255 - alpha;
            if (alpha1 >= 255) {
                alpha1 = 255;
            }

            func_8010EF9C(globalCtx, alpha1);

            if (gSaveContext.buttonStatus[5] == BTN_DISABLED) {
                if (interfaceCtx->startAlpha != 70) {
                    interfaceCtx->startAlpha = 70;
                }
            } else {
                if (interfaceCtx->startAlpha != 255) {
                    interfaceCtx->startAlpha = alpha1;
                }
            }

            if (interfaceCtx->healthAlpha != 255) {
                interfaceCtx->healthAlpha = alpha1;
            }

            if (interfaceCtx->magicAlpha != 255) {
                interfaceCtx->magicAlpha = alpha1;
            }

            if (globalCtx->sceneNum == SCENE_SPOT00) {
                if (interfaceCtx->minimapAlpha < 170) {
                    interfaceCtx->minimapAlpha = alpha1;
                } else {
                    interfaceCtx->minimapAlpha = 170;
                }
            } else if (interfaceCtx->minimapAlpha != 255) {
                interfaceCtx->minimapAlpha = alpha1;
            }

            gSaveContext.unk_3F24++;

            if (alpha1 == 255) {
                gSaveContext.unk_3F20 = 0;
            }
            break;

        case 0x34:
            gSaveContext.unk_3F20 = 1;
            func_8010F1A8(globalCtx, 0);
            gSaveContext.unk_3F20 = 0;
        default:
            break;
    }

    Map_Update(globalCtx);

    if (gSaveContext.healthAccumulator != 0) {
        gSaveContext.healthAccumulator -= 4;
        gSaveContext.save.playerData.health += 4;

        if ((gSaveContext.save.playerData.health & 0xF) < 4) {
            play_sound(NA_SE_SY_HP_RECOVER);
        }

        health = gSaveContext.save.playerData.health;
        if (health >= gSaveContext.save.playerData.healthCapacity) {
            gSaveContext.save.playerData.health = gSaveContext.save.playerData.healthCapacity;
            gSaveContext.healthAccumulator = 0;
        }
    }

    LifeMeter_UpdateSizeAndBeep(globalCtx);
    D_801BF8DC = func_801242DC(globalCtx);

    if (D_801BF8DC == 1) {
        if (CUR_EQUIP_VALUE_VOID(EQUIP_TUNIC) == 2) {
            D_801BF8DC = 0;
        }
    } else if ((func_801242DC(globalCtx) >= 2) && (func_801242DC(globalCtx) < 5)) {
        if (CUR_EQUIP_VALUE_VOID(EQUIP_TUNIC) == 3) {
            D_801BF8DC = 0;
        }
    }

    LifeMeter_UpdateColors(globalCtx);

    // Update Rupees
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

    if ((globalCtx->pauseCtx.state == 0) && (globalCtx->pauseCtx.debugState == 0)) {
        if (interfaceCtx->isMinigamePerfect) {
            if (interfaceCtx->minigameRewardType == 1) {
                func_8011B5C0(globalCtx);
            } else if (interfaceCtx->minigameRewardType == 2) {
                func_8011B9E0(globalCtx);
            } else if (interfaceCtx->minigameRewardType == 3) {
                func_8011BF70(globalCtx);
            }
        }
    }

    // Update Minigame State
    if ((globalCtx->pauseCtx.state == 0) && (globalCtx->pauseCtx.debugState == 0)) {
        if (interfaceCtx->unk_280) {
            switch (interfaceCtx->unk_280) {
                case 1: // minigame countdown 3
                case 3: // minigame countdown 2
                case 5: // minigame countdown 1
                case 7: // minigame countdown Go!
                    interfaceCtx->unk_282 = 255;
                    interfaceCtx->unk_284 = 100;
                    interfaceCtx->unk_280++;
                    if (interfaceCtx->unk_280 == 8) {
                        interfaceCtx->unk_284 = 160;
                        play_sound(NA_SE_SY_START_SHOT);
                    } else {
                        play_sound(NA_SE_SY_WARNING_COUNT_E);
                    }
                    break;
                case 2:
                case 4:
                case 6:
                    interfaceCtx->unk_282 -= 10;
                    interfaceCtx->unk_284 += 3;
                    if (interfaceCtx->unk_282 < 22) {
                        interfaceCtx->unk_280++;
                    }
                    break;
                case 8:
                    interfaceCtx->unk_282 += -10;
                    if (interfaceCtx->unk_282 <= 150) {
                        interfaceCtx->unk_280 = 0x1E;
                    }
                    break;
                case 20:
                    interfaceCtx->unk_282 = 0;
                    interfaceCtx->unk_280++;
                    break;
                case 21:
                    interfaceCtx->unk_282++;
                    if (interfaceCtx->unk_282 >= 18) {
                        interfaceCtx->unk_280 = 30;
                    }
                    break;
            }
        }
    }

    // Update Do-Actions
    switch (interfaceCtx->unk_210) {
        case 1:
            interfaceCtx->unk_218 += 10466.667f;
            if (interfaceCtx->unk_218 >= 15700.0f) {
                interfaceCtx->unk_218 = -15700.0f;
                interfaceCtx->unk_210 = 2;

                if ((msgCtx->msgMode != 0) && (msgCtx->unk12006 == 0x26)) {
                    gGameInfo->data[0x55F] = -14;
                } else {
                    gGameInfo->data[0x55F] = 0;
                }
            }
            break;
        case 2:
            interfaceCtx->unk_218 += 10466.667f;
            if (interfaceCtx->unk_218 >= 0.0f) {
                interfaceCtx->unk_218 = 0.0f;
                interfaceCtx->unk_210 = 0;
                interfaceCtx->unk_212 = interfaceCtx->unk_214;
                action = interfaceCtx->unk_212;
                if ((action == DO_ACTION_MAX) || (action == DO_ACTION_MAX + 1)) {
                    action = DO_ACTION_NONE;
                }
                Interface_LoadActionLabel(&globalCtx->interfaceCtx, action, 0);
            }
            break;
        case 3:
            interfaceCtx->unk_218 += 10466.667f;
            if (interfaceCtx->unk_218 >= 15700.0f) {
                interfaceCtx->unk_218 = -15700.0f;
                interfaceCtx->unk_210 = 2;
            }
            break;
        case 4:
            interfaceCtx->unk_218 += 10466.667f;
            if (interfaceCtx->unk_218 >= 0.0f) {
                interfaceCtx->unk_218 = 0.0f;
                interfaceCtx->unk_210 = 0;
                interfaceCtx->unk_212 = interfaceCtx->unk_214;
                action = interfaceCtx->unk_212;
                if ((action == DO_ACTION_MAX) || (action == DO_ACTION_MAX + 1)) {
                    action = DO_ACTION_NONE;
                }

                Interface_LoadActionLabel(&globalCtx->interfaceCtx, action, 0);
            }
            break;
    }

    // Update Magic
    if (!(player->stateFlags1 & 0x200)) {
        if (gGameInfo->data[0x544] == 1) {
            if (gSaveContext.save.playerData.magicAcquired == 0) {
                gSaveContext.save.playerData.magicAcquired = 1;
            }
            gSaveContext.save.playerData.doubleMagic = 1;
            gSaveContext.save.playerData.magic = 0x60;
            gSaveContext.save.playerData.magicLevel = 0;
            gGameInfo->data[0x544] = 0;
        } else if (gGameInfo->data[0x544] == -1) {
            if (gSaveContext.save.playerData.magicAcquired == 0) {
                gSaveContext.save.playerData.magicAcquired = 1;
            }
            gSaveContext.save.playerData.doubleMagic = 0;
            gSaveContext.save.playerData.magic = 0x30;
            gSaveContext.save.playerData.magicLevel = 0;
            gGameInfo->data[0x544] = 0;
        }

        if (gSaveContext.save.playerData.magicAcquired != 0) {
            if (gSaveContext.save.playerData.magicLevel == 0) {
                gSaveContext.save.playerData.magicLevel = gSaveContext.save.playerData.doubleMagic + 1;
                gSaveContext.unk_3F30 = gSaveContext.save.playerData.magic;
                gSaveContext.save.playerData.magic = 0;
                gSaveContext.unk_3F28 = 8;
                gSaveContext.save.equips.buttonItems[3][0] = ITEM_NUT;
            }
        }
        Interface_UpdateMagicBar(globalCtx);

        func_80116088();
    }

    if (gSaveContext.unk_3DD0[5] == 0) {
        if ((D_801BF8DC == 1) || (D_801BF8DC == 4)) {
            if (CUR_FORM != 2) {
                if (globalCtx->gameOverCtx.state == 0) {
                    if ((gSaveContext.save.playerData.health >> 1) != 0) {
                        gSaveContext.unk_3DD0[5] = 8;
                        gSaveContext.timerX[5] = 115;
                        gSaveContext.timerY[5] = 80;
                        D_801BF8E0 = 1;
                        gSaveContext.timersNoTimeLimit[5] = false;
                    }
                }
            }
        }
    } else {
        if (((D_801BF8DC == 0) || (D_801BF8DC == 3)) && (gSaveContext.unk_3DD0[5] < 5)) {
            gSaveContext.unk_3DD0[5] = 0;
        }
    }

    // Update Minigame
    if (gSaveContext.minigameState == 1) {
        gSaveContext.minigameScore += interfaceCtx->unk_25C;
        gSaveContext.unk_3F3C += interfaceCtx->unk_25E;
        interfaceCtx->unk_25C = 0;
        interfaceCtx->unk_25E = 0;

        if (sHBAScoreTier == 0) {
            if (gSaveContext.minigameScore >= 1000) {
                sHBAScoreTier++;
            }
        } else if (sHBAScoreTier == 1) {
            if (gSaveContext.minigameScore >= 1500) {
                sHBAScoreTier++;
            }
        }

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

    // Update sun song
    if (gSaveContext.sunsSongState != SUNSSONG_INACTIVE) {
        // exit out of ocarina mode after suns song finishes playing
        if ((msgCtx->ocarinaAction != 0x39) && (gSaveContext.sunsSongState == SUNSSONG_START)) {
            globalCtx->msgCtx.ocarinaMode = 4;
        }

        // handle suns song in areas where time moves
        if (globalCtx->envCtx.timeIncrement != 0) {
            if (gSaveContext.sunsSongState != SUNSSONG_SPEED_TIME) {
                D_801BFD94 = false;
                if ((gSaveContext.save.time >= CLOCK_TIME(6, 0)) && (gSaveContext.save.time <= CLOCK_TIME(18, 0))) {
                    D_801BFD94 = true;
                }

                gSaveContext.sunsSongState = SUNSSONG_SPEED_TIME;
                D_801BFD98 = gGameInfo->data[0xF];
                gGameInfo->data[0xF] = 400;
            } else if (!D_801BFD94) {
                if ((gSaveContext.save.time >= CLOCK_TIME(6, 0)) && (gSaveContext.save.time <= CLOCK_TIME(18, 0))) {
                    gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
                    gGameInfo->data[0xF] = D_801BFD98;
                    globalCtx->msgCtx.ocarinaMode = 4;
                }
            } else if (gSaveContext.save.time > CLOCK_TIME(18, 0)) {
                gSaveContext.sunsSongState = SUNSSONG_INACTIVE;
                gGameInfo->data[0xF] = D_801BFD98;
                globalCtx->msgCtx.ocarinaMode = 4;
            }
        } else {
            gSaveContext.sunsSongState = SUNSSONG_SPECIAL;
        }
    }

    Interface_UpdateTimers(globalCtx);

    // Update Grandma's Story
    if (interfaceCtx->unk_31A != 0) {
        if (interfaceCtx->unk_31A == 1) {
            interfaceCtx->storyState = 0;
            interfaceCtx->unk_31A = 2;
            gGameInfo->data[0x59B] = 0;
        }

        Interface_AllocStory(globalCtx);

        if (interfaceCtx->unk_31A == 3) {
            interfaceCtx->unk_31A = 0;
            interfaceCtx->storyState = 0;
            if (interfaceCtx->storySegment != NULL) {
                ZeldaArena_Free(interfaceCtx->storySegment);
                interfaceCtx->storySegment = NULL;
            }
        } else if (interfaceCtx->unk_31A == 4) {
            interfaceCtx->unk_31A = 2;
        } else if (interfaceCtx->unk_31A == 5) {
            gGameInfo->data[0x59B] += 10;
            if (gGameInfo->data[0x59B] >= 250) {
                gGameInfo->data[0x59B] = 255;
                interfaceCtx->unk_31A = 2;
            }
        } else if (interfaceCtx->unk_31A == 6) {
            gGameInfo->data[0x59B] -= 10;
            if (gGameInfo->data[0x59B] < 0) {
                gGameInfo->data[0x59B] = 0;
                interfaceCtx->unk_31A = 2;
            }
        }
    }
}

void func_80121F94(void) {
    func_8010A410();
    func_80174F9C(func_8010E968, NULL);
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80121FC4.s")
