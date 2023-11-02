#ifndef Z_EN_JSO2_H
#define Z_EN_JSO2_H

#include "global.h"

struct EnJso2;

typedef void (*EnJso2ActionFunc)(struct EnJso2*, PlayState*);

typedef struct EnJso2 {
    /* 0x0000 */ Actor actor;
    /* 0x0144 */ SkelAnime skelAnime;
    /* 0x0150 */ char unk_150[0xC];
    /* 0x015C */ f32 unk15C;
    /* 0x0160 */ char unk_160[0x118];
    /* 0x0278 */ EnJso2ActionFunc actionFunc;
    /* 0x027C */ char unk_27C[0x8];
    /* 0x0284 */ s16 unk284;
    /* 0x0286 */ s16 unk286;
    /* 0x0288 */ s16 unk288;
    /* 0x0290 */ char unk_290[0xE2];
    /* 0x036C */ s32 unk36C;
    /* 0x0370 */ s8 unk370;
    /* 0x0374 */ f32 unk374;
    /* 0x0378 */ char unk_378[0xB8D];
    /* 0x0F05 */ u8 unkF05;
    /* 0x0F09 */ char unk_F09[0x13B];
    /* 0x1040 */ s32 unk1040;
} EnJso2; // size = 0x1090

#endif // Z_EN_JSO2_H
