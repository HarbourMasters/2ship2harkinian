#ifndef Z_EN_ZOB_H
#define Z_EN_ZOB_H

#include "global.h"

struct EnZob;

typedef void (*EnZobActionFunc)(struct EnZob*, PlayState*);

#define ENZOB_GET_F(thisx) ((thisx)->params & 0xF)

typedef enum {
    /* 1 */ ENZOB_F_1 = 1,
    /* 2 */ ENZOB_F_2
} EnZobParam;

typedef struct EnZob {
    /* 0x000 */ Actor actor;
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ Vec3s jointTable[24];
    /* 0x218 */ Vec3s morphTable[24];
    /* 0x2A8 */ ColliderCylinder collider;
    /* 0x2F4 */ u16 unk_2F4;
    /* 0x2F6 */ Vec3s unk_2F6;
    /* 0x2FC */ Vec3s unk_2FC;
    /* 0x302 */ s16 unk_302;
    /* 0x304 */ u16 unk_304;
    /* 0x306 */ s16 csIdList[4];
    /* 0x30E */ s16 csIdIndex;
    /* 0x310 */ s16 cueId;
    /* 0x312 */ s16 unk_312;
    /* 0x314 */ EnZobActionFunc actionFunc;
} EnZob; // size = 0x318

#endif // Z_EN_ZOB_H
