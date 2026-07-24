#ifndef Z_EN_BOOM_H
#define Z_EN_BOOM_H

#include "global.h"

typedef enum {
    /* 0 */ ZORA_BOOMERANG_LEFT,
    /* 1 */ ZORA_BOOMERANG_RIGHT,
    /* 2 */ OOT_BOOMERANG,  // Skijer's NEI: OoT single boomerang (human Link) — OoT En_Boom behavior 1:1
    /* 3 */ IKAXE_TOMAHAWK  // Skijer's NEI: Iron Knuckle's Axe throw — flies/returns like OOT_BOOMERANG
                            // but draws the axe DL (EnBoom_Draw) + deals Goron-punch damage (EnBoom_Init)
} EnBoomType;

// Both single-projectile variants (OoT boomerang + IK axe) share OoT's straight-fly/return flight
// and item-grab, differing only in model + damage. The Zora fins (LEFT/RIGHT) keep the MM behavior.
#define EN_BOOM_IS_OOT_STYLE(params) (((params) == OOT_BOOMERANG) || ((params) == IKAXE_TOMAHAWK))

struct EnBoom;

typedef void (*EnBoomActionFunc)(struct EnBoom*, PlayState*);

typedef struct EnBoom {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ColliderQuad collider;
    /* 0x1C4 */ Actor* moveTo;
    /* 0x1C8 */ Actor* unk_1C8;
    /* 0x1CC */ u8 unk_1CC;
    /* 0x1CD */ u8 unk_1CD;
    /* 0x1CE */ u8 unk_1CE;
    /* 0x1CF */ s8 unk_1CF;
    /* 0x1D0 */ s32 effectIndex;
    /* 0x1D4 */ WeaponInfo weaponInfo;
    /* 0x1F0 */ EnBoomActionFunc actionFunc;
} EnBoom; // size = 0x1F4

#endif // Z_EN_BOOM_H
