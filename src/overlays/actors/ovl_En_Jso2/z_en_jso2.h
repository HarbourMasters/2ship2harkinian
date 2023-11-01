#ifndef Z_EN_JSO2_H
#define Z_EN_JSO2_H

#include "global.h"

struct EnJso2;

typedef void (*EnJso2ActionFunc)(struct EnJso2*, PlayState*);

typedef struct EnJso2 {
    /* 0x0000 */ Actor actor;
    /* 0x0144 */ char unk_144[0x134];
    /* 0x0278 */ EnJso2ActionFunc actionFunc;
    /* 0x027C */ char unk_27C[0x8];
    /* 0x0284 */ s16 unk284;
    /* 0x0286 */ char unk_286[0xE6];
    /* 0x036C */ s32 unk36C;
} EnJso2; // size = 0x1090

#endif // Z_EN_JSO2_H
