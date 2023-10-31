#ifndef Z_OBJ_MURE2_H
#define Z_OBJ_MURE2_H

#include "global.h"

struct ObjMure2;

typedef void (*ObjMure2ActionFunc)(struct ObjMure2*, PlayState*);

typedef struct ObjMure2 {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ObjMure2ActionFunc actionFunc;
    /* 0x148 */ Actor* unk_148[0xC];                /* inferred */
    /* 0x178 */ u16 unk_178;                        /* inferred */
    /* 0x17A */ char pad_17A[2];
    /* 0x17C */ f32 unk_17C;                        /* inferred */
} ObjMure2;                                         /* size = 0x180 */

#endif // Z_OBJ_MURE2_H
