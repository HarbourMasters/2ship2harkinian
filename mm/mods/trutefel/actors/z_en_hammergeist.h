#ifndef Z_EN_HAMMERGEIST_H
#define Z_EN_HAMMERGEIST_H

/**
 * Trutefel's Molmauk (formerly Hammergeist) — ice hammer + fire hammer enemy, ported from the
 * modern OoT decomp to 2ship's MM decomp. Compiled assets live in
 * ../assets/object_hammergeist_assets.inc.c (meshes/textures resolve by
 * __OTR__objects/trutefel/object_hammergeist/... path via trutefel-enemies.o2r).
 */

#include "z64.h"
#include "../assets/object_hammergeist_assets.h"

struct EnHammergeist;

typedef void (*EnHammergeistActionFunc)(struct EnHammergeist*, PlayState*);

typedef struct EnHammergeist {
    Actor actor;
    Vec3s firePos[10]; // death-burn flame anchors, one per burning limb
    Vec3s jointTable[GHAMMERGEISTSKEL_NUM_LIMBS];
    Vec3s morphTable[GHAMMERGEISTSKEL_NUM_LIMBS];
    Vec3s headRot;
    Vec3s upperBodyRot;
    SkelAnime skelAnime;
    ColliderCylinder collider;
    ColliderCylinder hammerLeftCollider;
    ColliderCylinder hammerRightCollider;
    ColliderJntSph explosionCollider;
    ColliderJntSphElement explosionColliderItems[1];
    s16 faceIndex;
    s16 fireHammerIndex;
    s16 iceHammerIndex;
    s16 hurtboxCooldown;
    s16 explosionTimer;
    s16 infuseTimer;
    s16 slamTimer;
    s16 heavySlamTimer;
    s16 heavySlamCooldown;
    s16 genericAnimationTimer;
    s16 fireTimer;
    s16 alpha;
    u8 explosionRadiusIncrease;
    u8 leftHammerInfused;  // Ice
    u8 rightHammerInfused; // Fire
    u8 playerHit;
    u8 noHitAgain;
    EnHammergeistActionFunc actionFunc;
} EnHammergeist;

#endif
