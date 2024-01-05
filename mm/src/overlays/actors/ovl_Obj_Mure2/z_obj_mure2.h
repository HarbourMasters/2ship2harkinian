#ifndef Z_OBJ_MURE2_H
#define Z_OBJ_MURE2_H

#include "global.h"

struct ObjMure2;

typedef void (*ObjMure2ActionFunc)(struct ObjMure2*, PlayState*);

typedef struct ObjMure2 {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ObjMure2ActionFunc actionFunc;
    /* 0x148 */ Actor* actors[12];
    /* 0x178*/  u16 spawnFlags;
    /* 0x17C */ f32 unk_17C;

} ObjMure2; // size = 0x180

#endif // Z_OBJ_MURE2_H
