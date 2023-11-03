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
    /* 0x0288 */ char unk_288[0xC];
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
    /* 0x02E0 */ char unk2E0[0xC54];
    /* 0x0F34 */ ColliderJntSph unkF34;
    /* 0x0F54 */ ColliderJntSphElement unk_F54;
} EnBsb; // size = 0x1158

#endif // Z_EN_BSB_H
