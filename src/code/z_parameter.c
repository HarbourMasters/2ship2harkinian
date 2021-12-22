#include "global.h"

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

s32 D_801BF884[] = {
    0x00000000,
};

s32 D_801BF888[] = {
    0x00000000,
};

s16 D_801BF88C = 0;

s32 D_801BF890[] = {
    0x00000000,
    0x00000000,
};

s32 D_801BF898[] = {
    0x00000000,
};

s32 D_801BF89C[] = {
    0x00000000,
};

s32 D_801BF8A0[] = {
    0x00FF0000,
};

s32 D_801BF8A4[] = {
    0x00FF0000,
};

s32 D_801BF8A8[] = {
    0x00FF0000,
};

s32 D_801BF8AC[] = {
    0x00020000,
};

s32 D_801BF8B0[] = {
    0x00010000, 0x00080008, 0x00090009, 0x00060006, 0x00060006, 0x00010001,
    0x00010007, 0x00070007, 0x00070008, 0x00080009, 0x00090000,
};

s32 D_801BF8DC[] = {
    0x00000000,
};

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

s32 D_801BF974[] = {
    0x00000000,
};

s32 D_801BF978[] = {
    0x000A0000,
};

s32 D_801BF97C[] = {
    0x00FF0000,
};

s32 D_801BF980[] = {
    0x3F800000, // f32: 1.0
    0x00000000,
};

s32 D_801BF988[] = {
    0xE7000000,
    0x00000000,
    0xD9C0F9FA,
    0x00000000,
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

s32 D_801BF9EC[] = {
    0x00000000,
};

s32 D_801BF9F0[] = {
    0x00000000,
};

s32 D_801BF9F4[] = {
    0x00000000,
};

s32 D_801BF9F8[] = {
    0x00000000,
};

s32 D_801BF9FC[] = {
    0x000F0000,
};

s32 D_801BFA00[] = {
    0x00000000,
};

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

s32 D_801BFAD4[] = {
    0x02002AE0, 0x02002AE0, 0x02002BA0, 0x02002C60, 0x02002D20, 0x00820088, 0x00880088, 0x00880000,
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

s32 D_801BFB2C[] = {
    0x00FF0000,
};

s32 D_801BFB30[] = {
    0x00000000,
};

s32 D_801BFB34[] = {
    0x00000000,
};

s32 D_801BFB38[] = {
    0x00000AAA, 0x15552000, 0x2AAA3555, 0x40004AAA, 0x55556000, 0x6AAA7555, 0x80008AAA,
    0x9555A000, 0xAAAAB555, 0xC000CAAA, 0xD555E000, 0xEAAAF555, 0xFFFF0000,
};

s32 D_801BFB6C[] = {
    0x02006440, 0x02006498, 0x020064F0, 0x02006548, 0x020065A0, 0x020065F8, 0x02006650, 0x020066A8,
    0x02006700, 0x02006758, 0x020067B0, 0x02006808, 0x02006440, 0x02006498, 0x020064F0, 0x02006548,
    0x020065A0, 0x020065F8, 0x02006650, 0x020066A8, 0x02006700, 0x02006758, 0x020067B0, 0x02006808,
};

s32 D_801BFBCC[] = {
    0x00000000,
};

s32 D_801BFBD0[] = {
    0x009B0000,
};

s32 D_801BFBD4[] = {
    0x00FF0000,
};

s32 D_801BFBD8[] = {
    0x00000000,
};

s32 D_801BFBDC[] = {
    0x00000000,
};

s32 D_801BFBE0[] = {
    0x00000000,
};

s32 D_801BFBE4[] = {
    0x000F0000,
};

s32 D_801BFBE8[] = {
    0x00000000,
};

s32 D_801BFBEC[] = {
    0x00640000,
};

s32 D_801BFBF0[] = {
    0x00CD009B,
};

s32 D_801BFBF4[] = {
    0x00FF00FF,
};

s32 D_801BFBF8[] = {
    0x001E0000,
};

s32 D_801BFBFC[] = {
    0x001E0000,
};

s32 D_801BFC00[] = {
    0x00640000,
};

s32 D_801BFC04[] = {
    0x00FF0000,
};

s32 D_801BFC08[] = {
    0x00640000,
};

s32 D_801BFC0C[] = {
    0x001E0000,
};

s32 D_801BFC10[] = {
    0x00640000,
};

s32 D_801BFC14[] = {
    0x02006860, 0x020068A0, 0x020068E0, 0x02006920, 0x02006960, 0x020069A0,
    0x020069E0, 0x02006A20, 0x02006A60, 0x02006AA0, 0x02006AE0,
};

s32 D_801BFC40[] = {
    0x007F0088,
    0x00900097,
    0x00A000A8,
    0x00AF00B8,
};

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

s32 D_801BFC6C[] = {
    0x004E0036,
    0x001D0005,
    0xFFEEFFD6,
    0xFFBDFFAB,
};

s32 D_801BFC7C[] = {
    0x00B400B4,
    0x00B400B4,
    0xFF4CFF4C,
    0xFF4CFF4C,
};

s32 D_801BFC8C[] = {
    0x00FF00FF,
    0x00FF00FF,
    0x00A50037,
};

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

s32 D_801BFCC4[] = {
    0x02007B28, 0x02007D38, 0x02007F48, 0x02008158, 0x02007D38, 0x02008368, 0x02008578, 0x02008788,
};

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

s32 D_801BFCFC[] = {
    0x00100019,
    0x0022002A,
    0x0033003C,
    0x0044004D,
};

s32 D_801BFD0C[] = {
    0x00090009,
    0x00080009,
    0x00090008,
    0x00090009,
};

s32 D_801BFD1C[] = {
    0x00010000,
    0x00000000,
};

s32 D_801BFD24[] = {
    0x00020003,
    0x00030000,
};

s32 D_801BFD2C[] = {
    0x00C800FF, 0x006400AA, 0x00AA00FF, 0x00FF0069, 0x00690000,
};

s32 D_801BFD40[] = {
    0x00000050, 0x0000000A, 0x000A0050, 0x0028000A, 0x00000000,
};

s32 D_801BFD54[] = {
    0x02007828,
    0x02007528,
    0x02007228,
    0x02006D28,
};

s32 D_801BFD64[] = {
    0x00180018,
    0x00180028,
};

s32 D_801BFD6C[] = {
    0x006400FF, 0x006400FF, 0x00FF003C, 0x00FF0064, 0x00000078, 0x00AA00FF,
};

s32 D_801BFD84[] = {
    0x07000000,
    0x07012C00,
};

s32 D_801BFD8C[] = {
    0x07025800,
    0x07025A00,
};

s32 D_801BFD94[] = {
    0x00000000,
};

s32 D_801BFD98[] = {
    0x00000000,
    0x00000000,
};

s32 D_801BFDA0[] = {
    0x01DE01FF, 0x025D01DB, 0x01DA01FE, 0x0219024C, 0x0221025E, 0x020001FD,
    0x025C025F, 0x01DC024E, 0x025201DD, 0x01D90214, 0x01E401E1, 0x01E201E3,
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

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8010E028.s")

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

void func_8010E9F0(s16 arg0, s16 arg1) {
    gSaveContext.timerX[arg0] = 115;
    gSaveContext.timerY[arg0] = 80;

    D_801BF8E0 = 0;

    gSaveContext.unk_3DE0[arg0] = arg1 * 100;
    gSaveContext.unk_3E18[arg0] = gSaveContext.unk_3DE0[arg0];

    if (gSaveContext.unk_3DE0[arg0] != 0) {
        gSaveContext.unk_3DD7[arg0] = 0;
    } else {
        gSaveContext.unk_3DD7[arg0] = 1;
    }

    gSaveContext.unk_3DD0[arg0] = 1;
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

    DmaMgr_SendRequest0((u32)globalCtx->interfaceCtx.doActionSegment + 0x780, (u32)_week_staticSegmentRomStart + phi_t0 * 0x510,
                        0x510);

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

void func_801B45D8(u32 romSegment, u8 arg1, void* dst, u32 size);

#define SAVE_GET(field) ((void)0, field)
#define TRANSFORMATION SAVE_GET(gSaveContext.playerForm)
#define TRANSFORMATION_CHECK ((TRANSFORMATION == 4) ? 0 : TRANSFORMATION)

#ifdef NON_EQUIVALENT
void Interface_LoadItemIcon(GlobalContext* globalCtx, u8 arg1) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;
    u8 phi_v0 = (arg1 == 0) ? gSaveContext.equips.buttonItems[TRANSFORMATION_CHECK][arg1] : gSaveContext.equips.buttonItems[0][arg1];

    func_80178E3C(_icon_item_static_testSegmentRomStart, phi_v0, (u32)interfaceCtx->iconItemSegment + (arg1 * 0x1000), 0x1000);
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

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80114B84.s")
/* s32 func_80114B84(s32 arg0, u8 arg1, u8 arg2) {
    u8 phi_a3;
    s32 phi_a0;
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
                        phi_a0 = CUR_FORM;
                        gSaveContext.save.equips.buttonItems[phi_a0][phi_a3_2] = arg2;
                    } else {
                        gSaveContext.save.equips.buttonItems[0][phi_a3_2] = arg2;
                    }
                    Interface_LoadItemIcon(arg0, phi_a3_2 & 0xFF);
                    return 1;
                }
            }
            return 1;
        }
    }
    return 0;
} */

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

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115D5C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80115DB4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80116088.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80116114.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80116348.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80116918.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80116FD8.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_801170B8.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80117100.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80117A20.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80117BD0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80118084.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80118890.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80118BA4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80119030.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80119610.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011B4E0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011B5C0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011B9E0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011BF70.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011C4C4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011C808.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011C898.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011CA64.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011E3B4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_8011E730.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_Draw.s")

#ifdef NON_MATCHING
void func_80120F90(GlobalContext* globalCtx, s32 block) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    switch (interfaceCtx->unk_31C) {
        case 0:
            if (interfaceCtx->unk_33C == NULL) {
                break;
            }
            osCreateMesgQueue(&interfaceCtx->unk_320, interfaceCtx->unk_338, 1);
            DmaMgr_SendRequestImpl(&interfaceCtx->dmaRequest_184, interfaceCtx->unk_33C, interfaceCtx->unk_340,
                                interfaceCtx->unk_344, 0, &interfaceCtx->unk_320, NULL);
            interfaceCtx->unk_31C = 1;
        case 1:
            if (osRecvMesg(&interfaceCtx->unk_320, NULL, block) == 0) {
                interfaceCtx->unk_31C = 2;
            }
            break;
    }
}
#else
#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80120F90.s")
#endif

void func_80121064(GlobalContext* globalCtx) {
    InterfaceContext* interfaceCtx = &globalCtx->interfaceCtx;

    interfaceCtx->unk_340 = (u32)_story_staticSegmentRomStart;
    interfaceCtx->unk_344 = (u32)_story_staticSegmentRomEnd - (u32)_story_staticSegmentRomStart;

    if (interfaceCtx->unk_33C == NULL) {
        interfaceCtx->unk_33C = ZeldaArena_Malloc(interfaceCtx->unk_344);
    }
    func_80120F90(globalCtx, 0);
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/Interface_Update.s")

void func_80121F94(void) {
    func_8010A410();
    func_80174F9C(func_8010E968, NULL);
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_parameter/func_80121FC4.s")
