#ifndef Z_EN_JSO2_H
#define Z_EN_JSO2_H

#include "global.h"

struct EnJso2;

typedef void (*EnJso2ActionFunc)(struct EnJso2*, PlayState*);

typedef struct EnJso2 {
    /* 0x0000 */ Actor actor;
    /* 0x0144 */ SkelAnime skelAnime;
    /* 0x0188 */ char unk_188[0xF0];
    /* 0x0278 */ EnJso2ActionFunc actionFunc;
    /* 0x027C */ char unk_27C[0x8];
    /* 0x0284 */ s16 unk284;
    /* 0x0286 */ s16 unk286;
    /* 0x0288 */ s16 unk288;
    /* 0x028A */ s16 unk28A;
    /* 0x028C */ char unk_28C[0x2];
    /* 0x028E */ s16 unk28E;
    /* 0x0290 */ s16 unk290;
    /* 0x0292 */ char unk_292[0x2];
    /* 0x0294 */ f32 unk294;
    /* 0x0298 */ f32 unk298;
    /* 0x029C */ s32 unk29C;
    /* 0x02A0 */ s16 unk2A0;
    /* 0x02A2 */ s16 unk2A2;
    /* 0x02A4 */ f32 unk2A4;
    /* 0x02A8 */ f32 unk2A8;
    /* 0x02AC */ char pad2AC[0x8];
    /* 0x02B4 */ s32 unk2B4;
    /* 0x02B6 */ char unk_2B8[0xC];
    /* 0x02C4 */ Vec3f unk2C4;
    /* 0x02D0 */ Actor* unk2D0;
    /* 0x02D4 */ Vec3f unk2D4[12];
    /* 0x0364 */ s16 unk364;
    /* 0x0366 */ s16 unk366;
    /* 0x0368 */ u8 unk368;
    /* 0x0369 */ char unk_369[0x3];
    /* 0x036C */ s32 unk36C;
    /* 0x0370 */ s8 unk370;
    /* 0x0371 */ u8 unk371;
    /* 0x0372 */ char unk_372[0x2];
    /* 0x0374 */ f32 unk374;
    /* 0x0378 */ f32 unk378;
    /* 0x037C */ char unk_37C[0x4];
    /* 0x0380 */ s32 unk380;
    /* 0x0384 */ s32 unk384;
    /* 0x0388 */ s32 unk_388;
    /* 0x38C */ s16 unk_38C;
    /* 0x38E */ s16 unk_38E;
    /* 0x390 */ Vec3f unk_390[20];
    /* 0x480 */ Vec3s unk_480[20];
    /* 0x4F8 */ Vec3s unk_4F8[20][20];
    /* 0x0E58 */ Vec3f unkE58;
    /* 0x0E64 */ Vec3f unk_E64[6];
    /* 0x0EAC */ Vec3f unk_EAC[6];
    /* 0x0EF4 */ ColliderCylinder unkEF4;
    /* 0x0F40 */ ColliderQuad unkF40;
    /* 0x0FC0 */ ColliderQuad unkFC0;
    /* 0x1040 */ s32 unk1040;
    /* 0x1044 */ s16 unk1044;
    /* 0x1046 */ s16 unk1046;
    /* 0x1048 */ s16 unk1048;
    /* 0x104A */ s16 unk104A;
    /* 0x104C */ f32 unk104C;
    /* 0x1050 */ f32 unk1050;
    /* 0x1054 */ Vec3f unk1054;
    /* 0x1060 */ Vec3f unk1060;
    /* 0x106C */ Vec3f unk106C;
    /* 0x1078 */ Vec3f unk1078;
    /* 0x1084 */ Vec3f unk1084;
} EnJso2; // size = 0x1090

#endif // Z_EN_JSO2_H
