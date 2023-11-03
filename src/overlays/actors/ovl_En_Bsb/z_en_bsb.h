#ifndef Z_EN_BSB_H
#define Z_EN_BSB_H

#include "global.h"

struct EnBsb;

typedef void (*EnBsbActionFunc)(struct EnBsb*, PlayState*);

typedef struct EnBsb {
    /* 0x0000 */ Actor actor;
    /* 0x0144 */ char unk_144[0x18];
    /* 0x015C */ f32 unk15C;
    /* 0x0160 */ char unk_160[0x124];
    /* 0x0284 */ EnBsbActionFunc actionFunc;
    /* 0x0288 */ char unk_288[0xC];
    /* 0x0294 */ s16 unk294;
    /* 0x0296 */ char unk_296[0xE];
    /* 0x02A4 */ s32 unk2A4;
    /* 0x02A8 */ s32 unk2A8;
    /* 0x02AC */ char unk_2AC[0x2];
    /* 0x02AE */ s8 unk2AE;
    /* 0x02B0 */ s32 unk2B0;
    /* 0x02B4 */ s16 unk2B4;
    /* 0x02B6 */ char unk_2B6[0xA];
    /* 0x02C4 */ f32 unk2C0;
    /* 0x02C4 */ f32 unk2C4;
    /* 0x02C8 */ char unk_2C8[0x2];
    /* 0x02CA */ s16 unk2CA;
    /* 0x02CD */ char unk_2CD[0x5];
    /* 0x02D2 */ s16 unk2D2;
    /* 0x02D4 */ char unk_2D4[0x4];
    /* 0x02D8 */ s32 unk2D8;
    /* 0x02DC */ s32 unk2DC;
    /* 0x02E0 */ char unk2E0[0xC54];
    /* 0x0F34 */ ColliderJntSph unkF34;
} EnBsb; // size = 0x1158

#endif // Z_EN_BSB_H
