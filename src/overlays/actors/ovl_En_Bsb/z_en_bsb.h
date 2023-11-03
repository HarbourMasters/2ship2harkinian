#ifndef Z_EN_BSB_H
#define Z_EN_BSB_H

#include "global.h"

struct EnBsb;

typedef void (*EnBsbActionFunc)(struct EnBsb*, PlayState*);

typedef struct EnBsb {
    /* 0x0000 */ Actor actor;
    /* 0x0144 */ SkelAnime skelAnime;
    /* 0x0188 */ Vec3s unk_188[21];
    /* 0x0188 */ Vec3s unk_206[21];
    /* 0x0284 */ EnBsbActionFunc actionFunc;
    /* 0x0288 */ Path *unk_0288;
    /* 0x028C */ s32 unk_028C;
    /* 0x0290 */ u8 unk_0290;
    /* 0x0291 */ char pad_0291[0x3];
    /* 0x0294 */ s16 unk294;
    /* 0x0296 */ char unk_296[0xE];
    /* 0x02A4 */ s32 unk2A4;
    /* 0x02A8 */ s32 unk2A8;
    /* 0x02AC */ char unk_2AC[0x2];
    /* 0x02AE */ u8 unk2AE;
    /* 0x02B0 */ s32 unk2B0;
    /* 0x02B4 */ s16 unk2B4;
    /* 0x02B6 */ s16 unk_2B6;
    /* 0x02B8 */ s16 unk_2B8;
    /* 0x02BA */ s16 unk_2BA;
    /* 0x02BC */ s16 unk_2BC;
    /* 0x02BE */ char pad_2BE[0x2];
    /* 0x02C0 */ f32 unk2C0;
    /* 0x02C4 */ f32 unk2C4;
    /* 0x02C8 */ s16 unk_2C8;
    /* 0x02CA */ s16 unk2CA;
    /* 0x02CC */ s16 unk_2CC[5];
    /* 0x02D8 */ s32 unk2D8;
    /* 0x02DC */ s32 unk2DC;
    /* 0x2E0 */ Vec3f unk_02E0;
    /* 0x2EC */ Vec3f unk_02EC;
    /* 0x2F8 */ Vec3f unk_02F8;
    /* 0x304 */ Vec3f unk_0304;
    /* 0x310 */ Vec3s unk_0310;
    /* 0x316 */ Vec3s unk_0316;
    /* 0x31C */ Vec3s unk_031C;
    /* 0x322 */ s16 unk_0322;
    /* 0x324 */ s16 unk_0324;
    /* 0x326 */ char pad_326[0x2];
    /* 0x328 */ f32 unk_0328;
    /* 0x32C */ f32 unk_032C;
    /* 0x330 */ Vec3f unk_0330[17];
    /* 0x3FC */ s32 unk_03FC[17];
    /* 0x440 */ s16 unk_0440;
    /* 0x444 */ char pad_0444[0xAF0];
    /* 0x0F34 */ ColliderJntSph unk_F34;
    /* 0x0F54 */ ColliderJntSphElement unk_F54[7];
    /* 0x1114 */ u32 unk_1114;
    /* 0x1118 */ s16 unk_1118;
    /* 0x111A */ s16 unk_111A;
    /* 0x111C */ s16 unk_111C;
    /* 0x1120 */ f32 unk_1120;
    /* 0x1124 */ f32 unk_1124;
    /* 0x1128 */ Vec3f unk_1128;
    /* 0x1134 */ Vec3f unk_1134;
    /* 0x1140 */ Vec3f unk_1140;
    /* 0x114C */ Vec3f unk_114C;
} EnBsb; // size = 0x1158

#endif // Z_EN_BSB_H
