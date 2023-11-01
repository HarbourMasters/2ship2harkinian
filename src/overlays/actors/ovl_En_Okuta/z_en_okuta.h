#ifndef Z_EN_OKUTA_H
#define Z_EN_OKUTA_H

#include "global.h"

struct EnOkuta;

typedef void (*EnOkutaActionFunc)(struct EnOkuta*, PlayState*);

typedef struct EnOkuta {
    /* 0x000 */ Actor actor;
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ EnOkutaActionFunc actionFunc;
    /* 0x18C */ u8 unk18C;
    /* 0x18D */ char pad18D;
    /* 0x18E */ s16 unk18E;
    /* 0x190 */ s16 unk190;
    /* 0x192 */ Vec3s jointTable[16];
    /* 0x1F2 */ Vec3s morphTable[16];
    /* 0x252 */ char pad252[2];
    /* 0x254 */ f32 unk254;
    /* 0x258 */ char pad258[0x18];
    /* 0x270 */ Vec3f bodyPartsPos[0xA];
    /* 0x2E8 */ ColliderCylinder collider;
} EnOkuta;  

#endif // Z_EN_OKUTA_H
