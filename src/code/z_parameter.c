#include "global.h"
#include "misc/parameter_static/parameter_static.h"

void Map_Update(GlobalContext* globalCtx);
u8 func_800FE4A8();
s32 func_801234D4(GlobalContext* globalCtx);
s16 func_801242DC(GlobalContext*); // s32 func_8008F2F8 (OoT)
void func_801663C4(u8* arg0, u8* arg1, u32 arg2);

extern Gfx D_0E0001C8[]; // Display List
extern Gfx D_0E0002E0[]; // Display List
extern u8 D_801ABAB0[];
extern u8 D_801E3BB0[];

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

s32 D_801BF888[] = {
    0x00000000,
};

// sMinigameScoreTier
s16 D_801BF88C = 0;

// sMinigameScoreDigits
u16 D_801BF890[] = { 0, 0, 0, 0 };

s32 D_801BF898[] = {
    0x00000000,
};

s32 D_801BF89C[] = {
    0x00000000,
};

s16 D_801BF8A0 = 0xFF;

s16 D_801BF8A4 = 0xFF;

s16 D_801BF8A8 = 0xFF;

s32 D_801BF8AC[] = {
    0x00020000,
};

s32 D_801BF8B0[] = {
    0x00010000, 0x00080008, 0x00090009, 0x00060006, 0x00060006, 0x00010001,
    0x00010007, 0x00070007, 0x00070008, 0x00080009, 0x00090000,
};

s16 D_801BF8DC = 0;

s16 D_801BF8E0 = 0;

s16 D_801BF8E4 = 0;

s32 D_801BF8E8[] = {
    0x00000000,
    0x00000000,
};

OSTime D_801BF8F0 = 0;

s32 D_801BF8F8[] = {
    0x00000000,
};

s32 D_801BF8FC[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

s32 D_801BF930[] = {
    0x00000000,
};

s32 D_801BF934[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

s32 D_801BF968[] = {
    0x00000000,
};

s32 D_801BF96C[] = {
    0x00000000,
};

s32 D_801BF970[] = {
    0x00630000,
};

u32 D_801BF974 = 0;

s16 D_801BF978 = 10;

s16 D_801BF97C = 255;

f32 D_801BF980 = 1.0f;

s32 D_801BF984 = 0;

// Display List
Gfx D_801BF988[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
};

s32 D_801BF998[] = {
    0xEF802C30, 0x00504340, 0xFCFFFFFF, 0xFFFDF6FB, 0xDF000000, 0x00000000,
};

s32 D_801BF9B0[] = {
    0x00000000,
};

s32 D_801BF9B4[] = {
    0x42C80000, // f32: 100.0
    0x42DA0000, // f32: 109.0
};

s32 D_801BF9BC[] = {
    0x022602A8,
    0x02A802A8,
};

s32 D_801BF9C4[] = {
    0x009E009B,
};

s32 D_801BF9C8[] = {
    0x00170016,
};

s32 D_801BF9CC[] = {
    0xC3BE0000, // f32: -380.0
    0xC3AF0000, // f32: -350.0
};

s32 D_801BF9D4[] = {
    0x00A700E3,
};

s32 D_801BF9D8[] = {
    0x00F9010F,
};

s32 D_801BF9DC[] = {
    0x00110012,
};

s32 D_801BF9E0[] = {
    0x00220012,
};

s32 D_801BF9E4[] = {
    0x023F026C,
};

s32 D_801BF9E8[] = {
    0x026C026C,
};

s16 D_801BF9EC = 0;

s16 D_801BF9F0 = 0;

s16 D_801BF9F4 = 0;

s16 D_801BF9F8 = 0;

s16 D_801BF9FC = 0xF;

u32 D_801BFA00 = 0;

s32 D_801BFA04[] = {
    0xFFF2FFF2, 0xFFE8FFF8, 0xFFF4FFF4, 0xFFF9FFF8, 0xFFF9FFF8, 0xFFF40000,
};

s32 D_801BFA1C[] = {
    0x001C001C, 0x00300010, 0x00180018, 0x00100010, 0x00100010, 0x00180000,
};

s32 D_801BFA34[] = {
    0x000E000E, 0x00080018, 0xFFAEFFAE, 0x003A003B, 0x003A003B, 0x00200000,
};

s32 D_801BFA4C[] = {
    0x001C001C, 0x00100010, 0x00180018, 0x000B000B, 0x000B000B, 0x00200000,
};

s32 D_801BFA64[] = {
    0xFFC3FFD3,
    0x001D0068,
    0xFF8BFFD6,
    0x00200037,
};

s32 D_801BFA74[] = {
    0x0001FFBA,
    0xFF9DFFBA,
    0x00470065,
    0x00480001,
};

// Possibly?
// s32 D_801BFA84 = 0;
// s16 D_801BFA88 = {...};
s32 D_801BFA84[] = {
    0x00000000, 0x0005000A, 0x0014001E, 0x000A001E, 0x00280032, 0x0014000A,
    0x00010005, 0x00010005, 0x000A0014, 0x00320064, 0x00C80000,
};

s32 sDoActionTextures[] = {
    0x09000000, // gAttackDoActionENGTex
    0x09000180, // gCheckDoActionENGTex
};

s32 D_801BFAB8[] = {
    0x00FF00FF,
    0x00FF0096,
    0x00960096,
};

s32 D_801BFAC4[] = {
    0x00000001,
    0x00010000,
};

s32 D_801BFACC[] = {
    0x00020001,
    0x00020001,
};

TexturePtr D_801BFAD4[] = {
    gTatlCUpENGTex, gTatlCUpENGTex, gTatlCUpGERTex, gTatlCUpFRATex, gTatlCUpESPTex,
};

s32 D_801BFAE8[] = {
    0x00820088,
    0x00880088,
    0x00880000,
};

s32 D_801BFAF4[] = {
    0x001D001B,
};

s32 D_801BFAF8[] = {
    0x001B001B,
};

s32 D_801BFAFC[] = {
    0x001E0018,
    0x00180018,
};

s32 D_801BFB04[] = {
    0x00A200E4,
    0x00FA0110,
};

s32 D_801BFB0C[] = {
    0x00230023,
    0x00330023,
};

s32 D_801BFB14[] = {
    0x00FF0064,
    0x00FF0000,
};

s32 D_801BFB1C[] = {
    0x00000064,
    0x00FF0000,
};

s32 D_801BFB24[] = {
    0x000000FF,
    0x00640000,
};


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
    if ((btnAPressed != D_801BFA84[0]) && btnAPressed) {
        gSaveContext.unk_3DC8 = osGetTime();
        gSaveContext.unk_3DD0[0] = 15;
    }

    D_801BFA84[0] = btnAPressed;
}

void func_8010E9F0(s16 timerId, s16 arg1) {
    gSaveContext.timerX[timerId] = 115;
    gSaveContext.timerY[timerId] = 80;

    D_801BF8E0 = 0;

    gSaveContext.unk_3DE0[timerId] = arg1 * 100;
    gSaveContext.unk_3E18[timerId] = gSaveContext.unk_3DE0[timerId];

    if (gSaveContext.unk_3DE0[timerId] != 0) {
        gSaveContext.unk_3DD7[timerId] = 0;
    } else {
        gSaveContext.unk_3DD7[timerId] = 1;
    }

    gSaveContext.unk_3DD0[timerId] = 1;
}

void func_8010EA9C(s16 arg0, s16 arg1) {
    gSaveContext.timerX[0] = 115;
    gSaveContext.timerY[0] = 80;

    D_801BF8E4 = arg1;

    gSaveContext.unk_3DE0[0] = arg0 * 100;
    gSaveContext.unk_3E18[0] = gSaveContext.unk_3DE0[0];

    if (gSaveContext.unk_3DE0[0] != 0) {
        gSaveContext.unk_3DD7[0] = 0;
    } else {
        gSaveContext.unk_3DD7[0] = 1;
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

void func_8010EBA0(s16 arg0, s16 arg1) {
    gSaveContext.unk_101A[arg1] = 1;
    gSaveContext.unk_1080[arg1] = arg0 * 100;
    gSaveContext.unk_1050[arg1] = gSaveContext.unk_1080[arg1];
    gSaveContext.unk_1020[arg1] = osGetTime();
    gSaveContext.unk_10B0[arg1] = 0;
    D_801BF8F0 = 0;
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8010EC54.s")

void func_8010EE74(GlobalContext* globalCtx, s32 arg1) {
    s32 pad;
    s16 phi_t0 = arg1 - 1;

    if (phi_t0 < 0 || phi_t0 >= 3) {
        phi_t0 = 0;
    }

    DmaMgr_SendRequest0((u32)globalCtx->interfaceCtx.doActionSegment + 0x780,
                        (u32)_week_staticSegmentRomStart + phi_t0 * 0x510, 0x510);

    for (phi_t0 = 0; phi_t0 < 120; phi_t0++) {
        gSaveContext.save.roomInf[phi_t0][0] = gSaveContext.cycleSceneFlags[phi_t0].chest;
        gSaveContext.save.roomInf[phi_t0][1] = gSaveContext.cycleSceneFlags[phi_t0].swch0;
        gSaveContext.save.roomInf[phi_t0][2] = gSaveContext.cycleSceneFlags[phi_t0].swch1;
        gSaveContext.save.roomInf[phi_t0][3] = gSaveContext.cycleSceneFlags[phi_t0].clearedRoom;
        gSaveContext.save.roomInf[phi_t0][4] = gSaveContext.cycleSceneFlags[phi_t0].collectible;
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

void func_801129E4(GlobalContext* globalCtx) {
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

#ifdef NON_MATCHING
void func_80112AFC(GlobalContext* globalCtx) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    gSaveContext.minigameState = 1;
    gSaveContext.minigameScore = 0;
    gSaveContext.unk_3F3C = 0;

    D_801BF88C = 0;

    interfaceCtx->unk_262 = 0;
    interfaceCtx->unk_25E = 0;
    interfaceCtx->unk_25C = 0;
    interfaceCtx->hbaAmmo = 0x14;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80112AFC.s")
#endif

void func_80178E3C(u32 romSegment, u32 arg1, void* dst, u32 size);

#ifdef NON_EQUIVALENT
void Interface_LoadItemIcon(GlobalContext* globalCtx, volatile u8 arg1) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    u8 phi_v0;

    if (arg1 == 0) {
        phi_v0 = gSaveContext.save.equips.buttonItems[CUR_FORM][arg1];
    } else {
        phi_v0 = gSaveContext.save.equips.buttonItems[0][arg1];
    }

    func_80178E3C((u32)_icon_item_static_testSegmentRomStart, phi_v0 & 0xFF,
                  (u32)interfaceCtx->iconItemSegment + (arg1 * 0x1000), 0x1000);
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_LoadItemIcon.s")
#endif

void func_80112BE4(GlobalContext* globalCtx, u8 arg1) {
    Interface_LoadItemIcon(globalCtx, arg1);
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80112C0C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Item_Give.s")

s32 func_801143CC(u8);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_801143CC.s")

s32 func_80114978(u8 arg0) {
    return func_801143CC(arg0);
}

void func_801149A0(s16 arg0, s16 arg1) {
    s16 phi_v0;
    s32 phi_a1;

    gSaveContext.save.inventory.items[arg1] = 0xFF;

    for (phi_v0 = 1; phi_v0 < 4; phi_v0++) {
        if (phi_v0 == 0) {
            phi_a1 = gSaveContext.save.equips.buttonItems[CUR_FORM][phi_v0];
        } else {
            phi_a1 = gSaveContext.save.equips.buttonItems[0][phi_v0];
        }
        if (arg0 == (phi_a1 & 0xFF)) {
            if (phi_v0 == 0) {
                phi_a1 = CUR_FORM;
                gSaveContext.save.equips.buttonItems[phi_a1][phi_v0] = 0xFF;
            } else {
                gSaveContext.save.equips.buttonItems[0][phi_v0] = 0xFF;
            }
            if (phi_v0 == 0) {
                phi_a1 = CUR_FORM;
                gSaveContext.save.equips.cButtonSlots[phi_a1][phi_v0] = 0xFF;
            } else {
                gSaveContext.save.equips.cButtonSlots[0][phi_v0] = 0xFF;
            }
        }
    }
}

void func_80114A9C(s16 arg0) {
    s16 phi_v0;
    s32 phi_a1;

    for (phi_v0 = 1; phi_v0 < 4; phi_v0++) {
        if (phi_v0 == 0) {
            phi_a1 = gSaveContext.save.equips.buttonItems[CUR_FORM][phi_v0];
        } else {
            phi_a1 = gSaveContext.save.equips.buttonItems[0][phi_v0];
        }
        if (arg0 == (phi_a1 & 0xFF)) {
            if (phi_v0 == 0) {
                phi_a1 = CUR_FORM;
                gSaveContext.save.equips.buttonItems[phi_a1][phi_v0] = 0xFF;
            } else {
                gSaveContext.save.equips.buttonItems[0][phi_v0] = 0xFF;
            }
            if (phi_v0 == 0) {
                phi_a1 = CUR_FORM;
                gSaveContext.save.equips.cButtonSlots[phi_a1][phi_v0] = 0xFF;
            } else {
                gSaveContext.save.equips.cButtonSlots[0][phi_v0] = 0xFF;
            }
        }
    }
}

#ifdef NON_MATCHING
s32 func_80114B84(s32 arg0, u8 arg1, u8 arg2) {
    u8 phi_a3;
    u8 phi_a0;
    u8 phi_a3_2;
    for (phi_a3 = 0; phi_a3 < 24; phi_a3++) {
        if (gSaveContext.save.inventory.items[phi_a3] == arg1) {
            gSaveContext.save.inventory.items[phi_a3] = arg2;
            for (phi_a3_2 = 1; phi_a3_2 < 4; phi_a3_2++) {
                if (phi_a3_2 == 0) {
                    phi_a0 = gSaveContext.save.equips.buttonItems[CUR_FORM][phi_a3_2];
                } else {
                    phi_a0 = gSaveContext.save.equips.buttonItems[0][phi_a3_2];
                }

                if (arg1 == (phi_a0 & 0xFF)) {
                    if (phi_a3_2 == 0) {
                        gSaveContext.save.equips.buttonItems[CUR_FORM][phi_a3_2] = arg2;
                    } else {
                        gSaveContext.save.equips.buttonItems[0][phi_a3_2] = arg2;
                    }

                    Interface_LoadItemIcon(arg0, phi_a3_2 & 0xFF);
                    return true;
                }
            }
            return true;
        }
    }
    return false;
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80114B84.s")
#endif

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80114CA0.s")

s32 func_80114E90(void) {
    s32 i;

    for (i = 18; i < 24; i++) {
        if (gSaveContext.save.inventory.items[i] == ITEM_BOTTLE) {
            return true;
        }
    }
    return false;
}

s32 func_80114F2C(u8 arg0) {
    s32 i;

    for (i = 18; i < 24; i++) {
        if (gSaveContext.save.inventory.items[i] == arg0) {
            return true;
        }
    }
    return false;
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80114FD0.s")
/* void func_80114FD0(GlobalContext* globalCtx, u8 arg1, u8 arg2) {
    u8 phi_v1;
    s32 phi_a0;
    if (arg2 == 0) {
        phi_v1 = gSaveContext.save.equips.cButtonSlots[CUR_FORM][arg2];
    } else {
        phi_v1 = gSaveContext.save.equips.cButtonSlots[0][arg2];
    }
    gSaveContext.save.inventory.items[phi_v1] = arg1;
    if (arg2 == 0) {
        phi_a0 = CUR_FORM;
        gSaveContext.save.equips.buttonItems[phi_a0][arg2] = arg1;
    } else {
        gSaveContext.save.equips.buttonItems[0][arg2] = arg1;
    }
    Interface_LoadItemIcon(globalCtx, arg2);
    globalCtx->pauseCtx.cursorItem[0] = arg1;
    gSaveContext.buttonStatus[arg2] = 0;
    if (arg1 == 0x20) {
        if (arg2 == 0) {
            phi_a0 = CUR_FORM;
            phi_v1 = gSaveContext.save.equips.cButtonSlots[phi_a0][arg2];
        } else {
            phi_v1 = gSaveContext.save.equips.cButtonSlots[0][arg2];
        }
        func_8012FB84(0x3C, phi_v1 - 0x12);
    }
} */

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115130.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_801152B8.s")
/* void func_801152B8(GlobalContext* globalCtx, s16 arg1, s16 arg2) {
    s16 phi_s0;
    u8 phi_v1;
    gSaveContext.save.inventory.items[arg1] = arg2;
    for (phi_s0 = 1; phi_s0 < 4; phi_s0++) {
        if (phi_s0 == 0) {
            phi_v1 = gSaveContext.save.equips.cButtonSlots[CUR_FORM][phi_s0];
        } else {
            phi_v1 = gSaveContext.save.equips.cButtonSlots[0][phi_s0];
        }
        if (arg1 == (phi_v1 & 0xFF)) {
            if (phi_s0 == 0) {
                gSaveContext.save.equips.buttonItems[CUR_FORM][phi_s0] = arg2;
            } else {
                gSaveContext.save.equips.buttonItems[0][phi_s0] = arg2;
            }
            Interface_LoadItemIcon(globalCtx, phi_s0);
        }
    }
} */

void func_801153C8(s32* buf, s32 size) {
    s32 i;

    for (i = 0; i != size; i++) {
        buf[i] = 0;
    }
}

void Interface_LoadActionLabel(InterfaceContext* interfaceCtx, u16 action, s16 loadOffset) {
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

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_801155B4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115764.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115844.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115908.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_801159c0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_801159EC.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115A14.s")

void Interface_AddMagic(GlobalContext* globalCtx, s16 arg1) {
    if (gSaveContext.save.playerData.magic) {}

    if (gSaveContext.save.playerData.magic < gSaveContext.unk_3F2E) {
        gSaveContext.unk_3F34 += arg1;
        gSaveContext.unk_3F2A = 1;
    }
}

void func_80115D5C(GameState* gamestate) {
    if ((gSaveContext.unk_3F28 != 8) && (gSaveContext.unk_3F28 != 9)) {
        D_801BF8A0 = D_801BF8A4 = D_801BF8A8 = 0xFF;
        gSaveContext.unk_3F28 = 0;
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115DB4.s")

void func_80116088(void) {
    if (gSaveContext.unk_3F2A != 0) {
        gSaveContext.save.playerData.magic += 4;
        play_sound(NA_SE_SY_GAUGE_UP - SFX_FLAG);
        if (gSaveContext.save.playerData.magic >= gSaveContext.unk_3F2E) {
            if (gSaveContext.save.playerData.magic) {}
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

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80116114.s")

void Interface_UpdateMagicBar(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_UpdateMagicBar.s")

void Interface_DrawMagicBar(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_DrawMagicBar.s")

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

void Interface_DrawItemButtons(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_DrawItemButtons.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80117A20.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80117BD0.s")

void func_80118084(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80118084.s")

void func_80118890(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80118890.s")

void func_80118BA4(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80118BA4.s")

void func_80119030(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80119030.s")

#ifdef NON_EQUIVALENT
void func_80119610(GlobalContext* globalCtx) {
    static s16 D_801BFB2C = 255;
    static s16 D_801BFB30 = 0;
    static s16 D_801BFB34 = 0;
    static u16 D_801BFB38[] = {
        0x0000, 0x0AAA, 0x1555, 0x2000, 0x2AAA, 0x3555, 0x4000, 0x4AAA, 0x5555, 0x6000, 0x6AAA, 0x7555, 0x8000,
        0x8AAA, 0x9555, 0xA000, 0xAAAA, 0xB555, 0xC000, 0xCAAA, 0xD555, 0xE000, 0xEAAA, 0xF555, 0xFFFF,
    };
    static TexturePtr D_801BFB6C[] = {
        gThreeDayTimerHour12Tex, gThreeDayTimerHour1Tex, gThreeDayTimerHour2Tex,  gThreeDayTimerHour3Tex,
        gThreeDayTimerHour4Tex,  gThreeDayTimerHour5Tex, gThreeDayTimerHour6Tex,  gThreeDayTimerHour7Tex,
        gThreeDayTimerHour8Tex,  gThreeDayTimerHour9Tex, gThreeDayTimerHour10Tex, gThreeDayTimerHour11Tex,
        gThreeDayTimerHour12Tex, gThreeDayTimerHour1Tex, gThreeDayTimerHour2Tex,  gThreeDayTimerHour3Tex,
        gThreeDayTimerHour4Tex,  gThreeDayTimerHour5Tex, gThreeDayTimerHour6Tex,  gThreeDayTimerHour7Tex,
        gThreeDayTimerHour8Tex,  gThreeDayTimerHour9Tex, gThreeDayTimerHour10Tex, gThreeDayTimerHour11Tex,
    };
    static s16 D_801BFBCC = 0; // color R
    static s16 D_801BFBD0 = 0; // color G
    static s16 D_801BFBD4 = 255;
    static s16 D_801BFBD8 = 0;
    static s16 D_801BFBDC = 0;
    static s16 D_801BFBE0 = 0;
    static s16 D_801BFBE4 = 0xF;
    static u32 D_801BFBE8 = 0;
    static s16 D_801BFBEC[] = { 100, 0 };
    static s16 D_801BFBF0[] = { 205, 155 };
    static s16 D_801BFBF4[] = { 255, 255 };
    static s16 D_801BFBF8[] = { 30, 0 };
    static s16 D_801BFBFC[] = { 30, 0 };
    static s16 D_801BFC00[] = { 100, 0 };
    static s16 D_801BFC04[] = { 255, 0 };
    static s16 D_801BFC08[] = { 100, 0 };
    static s16 D_801BFC0C[] = { 30, 0 };
    static s16 D_801BFC10[] = { 100, 0 };
    static TexturePtr D_801BFC14[] = {
        gFinalHoursTimerDigit0Tex, gFinalHoursTimerDigit1Tex, gFinalHoursTimerDigit2Tex, gFinalHoursTimerDigit3Tex,
        gFinalHoursTimerDigit4Tex, gFinalHoursTimerDigit5Tex, gFinalHoursTimerDigit6Tex, gFinalHoursTimerDigit7Tex,
        gFinalHoursTimerDigit8Tex, gFinalHoursTimerDigit9Tex, gFinalHoursTimerColonTex,
    };
    static s16 D_801BFC40[] = {
        0x007F, 0x0088,
        0x0090, 0x0097,
        0x00A0, 0x00A8,
        0x00AF, 0x00B8,
    };
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    MessageContext* msgCtx = &globalCtx->msgCtx;
    s16 sp1E6;
    f32 sp1D8;
    f32 sp1D0;
    f32 sp1CC;
    s16 sp1C6;
    s16 sp1AC[8];

    f32 temp_f0;
    f32 temp_f14;
    u32 temp_a1_7;
    s32 phi_v1_2;
    s16 phi_t0_3;

    s16 frac;
    s16 color1;
    s16 color2;
    s16 color3;
    s16 color4;
    f32 new_var1;
    f32 new_var2;

    s32 pad[5]; // available temps to use

    OPEN_DISPS(globalCtx->state.gfxCtx);

    if (gGameInfo->data[0xF] != 0) {
        if ((msgCtx->unk11F22 == 0) || ((globalCtx->actorCtx.unk5 & 2) && (func_801690CC(globalCtx) == 0)) || (msgCtx->unk11F22 == 0) || ((msgCtx->unk11F04 >= 0x100) && (msgCtx->unk11F04 < 0x201)) || (gSaveContext.gameMode == 3)) {
            if (FrameAdvance_IsEnabled(globalCtx) == 0) {
                if (func_800FE4A8() == 0) {
                    if ((0, gSaveContext.save.day) < 4) {
                        if ((0, gSaveContext.unk_3F22) == 0x32) {
                            if (func_801234D4(globalCtx) != 0) {
                                D_801BFB2C = 80;
                                D_801BFB34 = 20;
                                D_801BFB30 = 5;
                            } else if (D_801BFB34 != 0) {
                                D_801BFB34--;
                            } else if (D_801BFB30 != 0) {
                                D_801BFB2C += (s16)(ABS_ALT(D_801BFB2C - 255) / D_801BFB30);
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

                            gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                            gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 130, 130, 130, D_801BFB2C);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

                            OVERLAY_DISP = func_8010D9F4(OVERLAY_DISP, gThreeDayTimerHourLinesTex, 4, 64, 35, 96, 180, 128, 35, 1, 6, 0, 1 << 10, 1 << 10);

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, D_801BFB2C);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                            
                            OVERLAY_DISP = func_8010D9F4(OVERLAY_DISP, gThreeDayTimerDialBoarderTex, 4, 64, 50, 96, 168, 128, 50, 1, 6, 0, 1 << 10, 1 << 10);

                            if (((CURRENT_DAY >= 4) || ((CURRENT_DAY == 3) && ((0, gSaveContext.save.time) >= 5) && ((0, gSaveContext.save.time) < 0x4000)))) {
                                func_8012C8D4(globalCtx->state.gfxCtx);
                                gSPMatrix(OVERLAY_DISP++, &D_801D1DE0, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            } else {
                                gDPPipeSync(OVERLAY_DISP++);

                                if (gSaveContext.save.daySpeed == -2) {
                                    color1 = D_801BFBEC[D_801BFBE8];
                                    phi_v1_2 = ABS_ALT(D_801BFBCC - color1);
                                    frac = phi_v1_2 / D_801BFBE4;
                                    if (D_801BFBCC >= color1) {
                                        D_801BFBCC -= frac;
                                    } else {
                                        D_801BFBCC += frac;
                                    }

                                    color2 = D_801BFBF0[D_801BFBE8];
                                    phi_v1_2 = ABS_ALT(D_801BFBD0 - color2);
                                    frac = phi_v1_2 / D_801BFBE4;
                                    if (D_801BFBD0 >= color2) {
                                        D_801BFBD0 -= frac;
                                    } else {
                                        D_801BFBD0 += frac;
                                    }

                                    phi_v1_2 = ABS_ALT(D_801BFBD4 - D_801BFBF4[D_801BFBE8]);
                                    frac = phi_v1_2 / D_801BFBE4;
                                    if (D_801BFBD4 >= D_801BFBF4[D_801BFBE8]) {
                                        D_801BFBD4 -= frac;
                                    } else {
                                        D_801BFBD4 += frac;
                                    }

                                    phi_v1_2 = ABS_ALT(D_801BFBD8 - D_801BFBF8[D_801BFBE8]);
                                    frac = phi_v1_2 / D_801BFBE4;
                                    if (D_801BFBD8 >= D_801BFBF8[D_801BFBE8]) {
                                        D_801BFBD8 -= frac;
                                    } else {
                                        D_801BFBD8 += frac;
                                    }

                                    color3 = D_801BFBFC[D_801BFBE8];
                                    phi_v1_2 = ABS_ALT(D_801BFBDC - color3);
                                    frac = phi_v1_2 / D_801BFBE4;
                                    if (D_801BFBDC >= color3) {
                                        D_801BFBDC -= frac;
                                    } else {
                                        D_801BFBDC += frac;
                                    }

                                    color4 = D_801BFC00[D_801BFBE8];
                                    phi_v1_2 = ABS_ALT(D_801BFBE0 - color4);
                                    frac = phi_v1_2 / D_801BFBE4;
                                    if (D_801BFBE0 >= color4) {
                                        D_801BFBE0 -= frac;
                                    } else {
                                        D_801BFBE0 += frac;
                                    }

                                    D_801BFBE4--;

                                    if (D_801BFBE4 == 0) {
                                        D_801BFBCC = color1;
                                        D_801BFBE4 = 0xF;
                                        D_801BFBE8 ^= 1;
                                        D_801BFBD8 = color2;
                                        D_801BFBDC = color3;
                                        D_801BFBE0 = color4;
                                    }

                                    gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, D_801BFBCC, D_801BFBD0, 255, D_801BFB2C);
                                    gDPSetEnvColor(OVERLAY_DISP++, D_801BFBD8, D_801BFBDC, D_801BFBE0, 0);
                                } else {
                                    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 170, 100, D_801BFB2C);
                                }

                                OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, gThreeDayTimerDiamondTex, 40, 32, 140, 190, 40, 32, 1 << 10, 1 << 10);

                                gDPPipeSync(OVERLAY_DISP++);
                                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, D_801BFB2C);

                                OVERLAY_DISP = Gfx_TextureIA8(OVERLAY_DISP, interfaceCtx->doActionSegment + 0x780, 48, 27, 137, 192, 48, 27, 1 << 10, 1 << 10);

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

                                sp1D0 = (s16)((0, gSaveContext.save.time) * 1.3183594f) - ((((0, gSaveContext.save.time) * 1.3183594f) / 3600.0f) * 3600.0f);

                                func_8012C8D4(globalCtx->state.gfxCtx);

                                gSPMatrix(OVERLAY_DISP++, &D_801D1DE0, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

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

                                gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                                gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0xC], 4, 0);
                                gDPLoadTextureBlock_4b(OVERLAY_DISP++, gThreeDayTimerStarMinuteTex, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                                gSP1Quadrangle(OVERLAY_DISP++, 0, 2, 3, 1, 0);
                            }

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetScissorFrac(OVERLAY_DISP++, G_SC_NON_INTERLACE, 400, 620, 880, gGameInfo->data[0x56C] * 4.0f);

                            for (phi_t0_3 = 0; phi_t0_3 < 25; phi_t0_3++) {
                                if (((0, gSaveContext.save.time) & 0xFFFF) >= D_801BFB38[phi_t0_3 + 1]) {
                                    break;
                                }
                            }

                            sp1D8 = Math_SinS(gSaveContext.save.time) * -40.0f;
                            temp_f14 = Math_CosS(gSaveContext.save.time) * -34.0f;

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 100, 110, D_801BFB2C);

                            Matrix_InsertTranslation(sp1D8, temp_f14, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0x10], 4, 0);

                            OVERLAY_DISP = func_8010DC58(OVERLAY_DISP, gThreeDayTimerSunHourTex, 24, 24, 0);

                            sp1D8 = Math_SinS(gSaveContext.save.time) * 40.0f;
                            temp_f14 = Math_CosS(gSaveContext.save.time) * 34.0f;

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 55, D_801BFB2C);

                            Matrix_InsertTranslation(sp1D8, temp_f14, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0x14], 4, 0);

                            OVERLAY_DISP = func_8010DC58(OVERLAY_DISP, gThreeDayTimerMoonHourTex, 24, 24, 0);

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetScissorFrac(OVERLAY_DISP++, G_SC_NON_INTERLACE, 400, 620, 880, gGameInfo->data[0x56D] * 4.0f);

                            sp1CC = (0, gSaveContext.save.time) * 0.000096131f;

                            Matrix_InsertTranslation(0.0f, gGameInfo->data[0x56B] / 10.0f, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            Matrix_InsertZRotation_f(-(sp1CC - 3.15f), MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, D_801BFB2C);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0x18], 8, 0);
                            
                            OVERLAY_DISP = func_8010DE38(OVERLAY_DISP, D_801BFB6C[phi_t0_3], 4, 16, 11, 0);

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, D_801BFB2C);
                            gSP1Quadrangle(OVERLAY_DISP++, 4, 6, 7, 5, 0);

                            Matrix_InsertTranslation(0.0f, gGameInfo->data[0x56B] / 10.0f, 0.0f, MTXMODE_NEW);
                            Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);
                            Matrix_InsertZRotation_f(-sp1CC, MTXMODE_APPLY);
                            gSPMatrix(OVERLAY_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetCombineLERP(OVERLAY_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, D_801BFB2C);
                            gSPVertex(OVERLAY_DISP++, &interfaceCtx->actionVtx[0x20], 8, 0);

                            OVERLAY_DISP = func_8010DE38(OVERLAY_DISP, D_801BFB6C[phi_t0_3], 4, 16, 11, 0);

                            gDPPipeSync(OVERLAY_DISP++);
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 155, D_801BFB2C);
                            gSP1Quadrangle(OVERLAY_DISP++, 4, 6, 7, 5, 0);
                            gSPDisplayList(OVERLAY_DISP++, D_0E0001C8);

                            if ((CURRENT_DAY >= 4) || ((CURRENT_DAY == 3) && ((0, gSaveContext.save.time) >= 5) && ((0, gSaveContext.save.time) < 0x4000))) {
                                if ((0, gSaveContext.save.time) >= 0x3555) {

                                    color1 = D_801BFC04[D_801BFA00];
                                    phi_v1_2 = ABS_ALT(D_801BF9EC - color1);

                                    frac = phi_v1_2 / D_801BF9FC;
                                    if (D_801BF9EC >= color1) {
                                        D_801BF9EC -= frac;
                                    } else {
                                        D_801BF9EC += frac;
                                    }

                                    color2 = D_801BFC08[D_801BFA00];
                                    phi_v1_2 = ABS_ALT(D_801BF9F0 - color2);

                                    frac = phi_v1_2 / D_801BF9FC;
                                    if (D_801BF9F0 >= color2) {
                                        D_801BF9F0 -= frac;
                                    } else {
                                        D_801BF9F0 += frac;
                                    }

                                    color3 = D_801BFC0C[D_801BFA00];
                                    phi_v1_2 = ABS_ALT(D_801BF9F4 - color3);

                                    frac = phi_v1_2 / D_801BF9FC;
                                    if (D_801BF9F4 >= color3) {
                                        D_801BF9F4 -= frac;
                                    } else {
                                        D_801BF9F4 += frac;
                                    }

                                    color4 = D_801BFC10[D_801BFA00];
                                    phi_v1_2 = ABS_ALT(D_801BF9F8 - color4);

                                    frac = phi_v1_2 / D_801BF9FC;
                                    if (D_801BF9F8 >= color4) {
                                        D_801BF9F8 -= frac;
                                    } else {
                                        D_801BF9F8 += frac;
                                    }

                                    D_801BF9FC--;
                                    if (D_801BF9FC == 0) {
                                        D_801BF9EC = color1;
                                        D_801BF9F0 = color2;
                                        D_801BF9F4 = color3;
                                        D_801BF9F8 = color4;
                                        D_801BF9FC = 6;
                                        D_801BFA00 ^= 1;
                                    }
                                }

                                sp1E6 = D_801BFB2C;
                                if (D_801BFB2C != 0) {
                                    sp1E6 = 0xFF;
                                }

                                func_8012C654(globalCtx->state.gfxCtx);

                                gSPMatrix(OVERLAY_DISP++, &D_801D1DE0, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                                gDPSetAlphaCompare(OVERLAY_DISP++, G_AC_THRESHOLD);
                                gDPSetRenderMode(OVERLAY_DISP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
                                gDPSetCombineLERP(OVERLAY_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
                                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 195, sp1E6);
                                gDPSetEnvColor(OVERLAY_DISP++, D_801BF9F0, D_801BF9F4, D_801BF9F8, 0);

                                OVERLAY_DISP = func_8010D9F4(OVERLAY_DISP, gFinalHoursTimerFrameTex, 3, 80, 13, 119, 202, 80, 13, 0, 0, 0, 1 << 10, 1 << 10);
                                
                                temp_a1_7 = (-(CURRENT_DAY << 0x10) - (((0, gSaveContext.save.time) - 0x4000) & 0xFFFF)) + 0x40000;
                                temp_f0 = temp_a1_7 * 0.021972656f;
                                
                                sp1AC[0] = 0;
                                sp1AC[1] = temp_f0 / 60.0f;;
                                sp1AC[2] = temp_f0 / 60.0f;;

                                while (sp1AC[1] >= 0xA) {
                                    sp1AC[0]++;
                                    sp1AC[1] -= 10;
                                }


                                sp1AC[3] = 0;
                                sp1AC[4] = (s32)temp_f0 % 60;

                                while (sp1AC[4] >= 10) {
                                    sp1AC[3]++;
                                    sp1AC[4] -= 10;
                                }

                                sp1AC[6] = 0;
                                new_var1 = (sp1AC[4] * 45.511112f);
                                sp1AC[7] = temp_a1_7 - (u32)(new_var1 + (sp1AC[2] * 2730.6667f));

                                while (sp1AC[7] >= 10) {
                                    sp1AC[6]++;
                                    sp1AC[7] -= 10;
                                }

                                sp1AC[2] = 10;
                                sp1AC[5] = 10;

                                gDPPipeSync(OVERLAY_DISP++);
                                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, D_801BF9EC, 0, 0, sp1E6);
                                gDPSetEnvColor(OVERLAY_DISP++, D_801BF9EC, 0, 0, 0);

                                for (sp1C6 = 0; sp1C6 < 8; sp1C6++) {
                                    OVERLAY_DISP = Gfx_TextureI8(OVERLAY_DISP, D_801BFC14[sp1AC[sp1C6]], 8, 8, D_801BFC40[sp1C6], 205, 8, 8, 1 << 10, 1 << 10);
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
    0x0000, 0x0AAA, 0x1555, 0x2000, 0x2AAA, 0x3555, 0x4000, 0x4AAA, 0x5555, 0x6000, 0x6AAA, 0x7555, 0x8000,
    0x8AAA, 0x9555, 0xA000, 0xAAAA, 0xB555, 0xC000, 0xCAAA, 0xD555, 0xE000, 0xEAAA, 0xF555, 0xFFFF,
};
TexturePtr D_801BFB6C[] = {
    gThreeDayTimerHour12Tex, gThreeDayTimerHour1Tex, gThreeDayTimerHour2Tex,  gThreeDayTimerHour3Tex,
    gThreeDayTimerHour4Tex,  gThreeDayTimerHour5Tex, gThreeDayTimerHour6Tex,  gThreeDayTimerHour7Tex,
    gThreeDayTimerHour8Tex,  gThreeDayTimerHour9Tex, gThreeDayTimerHour10Tex, gThreeDayTimerHour11Tex,
    gThreeDayTimerHour12Tex, gThreeDayTimerHour1Tex, gThreeDayTimerHour2Tex,  gThreeDayTimerHour3Tex,
    gThreeDayTimerHour4Tex,  gThreeDayTimerHour5Tex, gThreeDayTimerHour6Tex,  gThreeDayTimerHour7Tex,
    gThreeDayTimerHour8Tex,  gThreeDayTimerHour9Tex, gThreeDayTimerHour10Tex, gThreeDayTimerHour11Tex,
};
s16 D_801BFBCC = 0; // color R
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
    gFinalHoursTimerDigit0Tex, gFinalHoursTimerDigit1Tex, gFinalHoursTimerDigit2Tex, gFinalHoursTimerDigit3Tex,
    gFinalHoursTimerDigit4Tex, gFinalHoursTimerDigit5Tex, gFinalHoursTimerDigit6Tex, gFinalHoursTimerDigit7Tex,
    gFinalHoursTimerDigit8Tex, gFinalHoursTimerDigit9Tex, gFinalHoursTimerColonTex,
};
s16 D_801BFC40[] = {
    0x007F, 0x0088,
    0x0090, 0x0097,
    0x00A0, 0x00A8,
    0x00AF, 0x00B8,
};
void func_80119610(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80119610.s")
#endif



void func_8011B4E0(GlobalContext* globalCtx, s16 arg1) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    s16 i;

    interfaceCtx->unk_286 = 1;
    interfaceCtx->unk_288 = arg1;
    interfaceCtx->unk_2FC[0] = 255;
    interfaceCtx->unk_2FC[1] = 165;
    interfaceCtx->unk_2FC[2] = 55;
    interfaceCtx->unk_2FC[3] = 255;
    interfaceCtx->unk_30A = 0x14;
    interfaceCtx->unk_308 = 0;
    interfaceCtx->unk_304 = 1;
    interfaceCtx->unk_30C = 0;

    for (i = 0; i < 8; i++) {
        interfaceCtx->unk_2AA[i] = 0;
        interfaceCtx->unk_29A[i] = 0;
        interfaceCtx->unk_28A[i] = interfaceCtx->unk_2AA[i];
        if (interfaceCtx->unk_288 == 1) {
            interfaceCtx->unk_2BC[i] = 140.0f;
            interfaceCtx->unk_2DC[i] = 100.0f;
        } else {
            interfaceCtx->unk_2DC[i] = 200.0f;
            interfaceCtx->unk_2BC[i] = 200.0f;
        }
    }

    interfaceCtx->unk_28A[0] = 1;
}


s32 D_801BFC50[] = {
    0xC000E000,
    0x00002000,
    0xA0008000,
    0x60004000,
};

s32 D_801BFC60[] = {
    0x00FF00FF,
};

s32 D_801BFC64[] = {
    0x00FF00FF,
    0x00A50037,
};
void func_8011B5C0(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011B5C0.s")


s16 D_801BFC6C[] = {
    0x004E, 0x0036, 0x001D, 0x0005, 0xFFEE, 0xFFD6, 0xFFBD, 0xFFAB,
};

s16 D_801BFC7C[] = {
    0x00B4, 0x00B4, 0x00B4, 0x00B4, 0xFF4C, 0xFF4C, 0xFF4C, 0xFF4C,
};

s32 D_801BFC8C[] = {
    0x00FF00FF,
    0x00FF00FF,
    0x00A50037,
};

void func_8011B9E0(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011B9E0.s")


s32 D_801BFC98[] = {
    0x004E0036,
    0x001D0005,
    0xFFEEFFD6,
    0xFFBDFFAB,
};

s32 D_801BFCA8[] = {
    0xC000E000,
    0x00002000,
    0xA0008000,
    0x60004000,
};

s32 D_801BFCB8[] = {
    0x00FF00FF,
    0x00FF00FF,
    0x00A50037,
};

void func_8011BF70(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011BF70.s")


TexturePtr D_801BFCC4[] = {
    gMinigameLetterPTex, gMinigameLetterETex, gMinigameLetterRTex, gMinigameLetterFTex,
    gMinigameLetterETex, gMinigameLetterCTex, gMinigameLetterTTex, gMinigameExclamationTex,
};

void func_8011C4C4(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011C4C4.s")

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

void func_8011CA64(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011CA64.s")

void func_8011E3B4(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011E3B4.s")

void func_8011E730(GlobalContext* globalCtx);
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011E730.s")

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
                if (gSaveContext.save.inventory.dungeonKeys[0, gSaveContext.mapIndex] >= 0) {
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
                    counterDigits[3] = gSaveContext.save.inventory.dungeonKeys[0, gSaveContext.mapIndex];

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
                func_80119610(globalCtx);
            }
        }

        if (interfaceCtx->unk_286 != 0) {
            func_8011C4C4(globalCtx);
        }

        func_8011E730(globalCtx);
        func_8011CA64(globalCtx);
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

            if (gSaveContext.buttonStatus[5] == 0xFF) {
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
        if (interfaceCtx->unk_286 != 0) {
            if (interfaceCtx->unk_288 == 1) {
                func_8011B5C0(globalCtx);
            } else if (interfaceCtx->unk_288 == 2) {
                func_8011B9E0(globalCtx);
            } else if (interfaceCtx->unk_288 == 3) {
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

                if ((msgCtx->unk11F22 != 0) && (msgCtx->unk12006 == 0x26)) {
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
                        gSaveContext.unk_3DD7[5] = 0;
                    }
                }
            }
        }
    } else {
        if (((D_801BF8DC == 0) || (D_801BF8DC == 3)) && (gSaveContext.unk_3DD0[5] < 5)) {
            gSaveContext.unk_3DD0[5] = 0;
        }
    }

    // Update Horseback Archery (remnant of OoT)
    if (gSaveContext.minigameState == 1) {
        gSaveContext.minigameScore += interfaceCtx->unk_25C;
        gSaveContext.unk_3F3C += interfaceCtx->unk_25E;
        interfaceCtx->unk_25C = 0;
        interfaceCtx->unk_25E = 0;

        if (D_801BF88C == 0) {
            if (gSaveContext.minigameScore >= 1000) {
                D_801BF88C++;
            }
        } else if (D_801BF88C == 1) {
            if (gSaveContext.minigameScore >= 1500) {
                D_801BF88C++;
            }
        }

        D_801BF890[0] = D_801BF890[1] = 0;
        D_801BF890[2] = 0;
        D_801BF890[3] = gSaveContext.minigameScore;

        while (D_801BF890[3] >= 1000) {
            D_801BF890[0]++;
            D_801BF890[3] -= 1000;
        }

        while (D_801BF890[3] >= 100) {
            D_801BF890[1]++;
            D_801BF890[3] -= 100;
        }

        while (D_801BF890[3] >= 10) {
            D_801BF890[2]++;
            D_801BF890[3] -= 10;
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

    func_8011E3B4(globalCtx);

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

// Belongs to z_player_lib
s16 D_801BFDA0[] = {
    0x1DE, 0x1FF, 0x25D, 0x1DB, 0x1DA, 0x1FE, 0x219, 0x24C, 0x221, 0x25E, 0x200, 0x1FD,
    0x25C, 0x25F, 0x1DC, 0x24E, 0x252, 0x1DD, 0x1D9, 0x214, 0x1E4, 0x1E1, 0x1E2, 0x1E3,
};
