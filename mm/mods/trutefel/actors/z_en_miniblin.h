#ifndef Z_EN_MINIBLIN_H
#define Z_EN_MINIBLIN_H

/**
 * Trutefel's Miniblin (rupee thief), ported from the modern OoT decomp to 2ship's MM decomp.
 * Compiled assets live in ../assets/object_miniblin_assets.inc.c (meshes/textures resolve at
 * draw time by __OTR__objects/trutefel/object_miniblin/... path via trutefel-enemies.o2r).
 */

#include "z64.h"
#include "../assets/object_miniblin_assets.h"

struct EnMiniblin;

typedef void (*EnMiniblinActionFunc)(struct EnMiniblin*, PlayState*);

typedef struct EnMiniblin {
    Actor actor;
    Vec3s jointTable[GMINIBLINSKEL_NUM_LIMBS];
    Vec3s morphTable[GMINIBLINSKEL_NUM_LIMBS];
    SkelAnime skelAnime;
    ColliderCylinder collider;
    ColliderQuad quad;
    EnMiniblinActionFunc actionFunc;
    s16 eyeIndex;
    s16 timer;
    s16 deathTimer;
    s16 damageTimer;
    s16 blinkTimer;
    s16 hurtboxCooldown;
    u8 rupeeStolen;
    u8 aboutToSteal;
} EnMiniblin;

#endif
