/**
 * remains_ally_chu.c - Goht's remains summons a FRIENDLY kamikaze bombchu ally.
 *
 * ACTOR_REMAINS_ALLY_CHU (0x2BF). One of the four boss-remains allies; authored in self
 * actors/ folder and UNITY-#included into boss_remains.cpp inside its extern "C" block
 * (like spiritual_stones.cpp #includes spiritual_stone_statue.c). NOT in CMake/vcxproj.
 * The RemainsAllyChu_Profile symbol therefore gets C linkage and matches the
 * `extern ActorProfile RemainsAllyChu_Profile` the DEFINE_ACTOR row (added by the
 * Integrate step) generates.
 *
 * DESIGN — KAMIKAZE:
 *   A friendly Real Bombchu that crawls toward the nearest live enemy and detonates on
 *   contact. It reuses MM's Real Bombchu (ovl_En_Rat, z_en_rat.c) wholesale:
 *     - the crawl-on-any-surface engine (axis triad + BgCheck line tests, copied verbatim
 *       from z_en_rat.c, renamed; self file stays SELF-CONTAINED and never edits z_en_rat.c),
 *     - the object_rat skeleton + run anim (loaded by OTR path so the profile can stay
 *       GAMEPLAY_KEEP — SkelAnime_InitFlex resolves the "__OTR__objects/object_rat/..."
 *       string through ResourceMgr_LoadSkeletonByName; OBJECT_RAT never has to be bound),
 *     - the detonation: spawn ACTOR_EN_BOM at its position with timer=0 (instant blast),
 *       exactly like EnRat_Explode. EN_BOM's blast collider is AT_TYPE_ALL, so it already
 *       damages every enemy — and Link (MM-authentic; kept on purpose).
 *
 * FRIENDLY / COLLISION:
 *   The chu carries NO attack collider of its own — the bomb it spawns does all the damage,
 *   so there is no AT/AC/OC to make "friendly". It therefore can never be hurt (no AC set →
 *   invulnerable) and never pushes anything (no OC). Friend/foe is handled by WHAT it targets:
 *   it only ever homes at, and only ever detonates on contact with, actors in
 *   ACTORCAT_ENEMY / ACTORCAT_BOSS (via RemainsAlly_FindNearestEnemy). Link is never a target,
 *   so it NEVER detonates from touching Link. When no enemy is in range it idle-follows the
 *   real player, and it self-destructs after a fuse (~250f) whether or not it reached anything.
 *
 * ACTORCAT_MISC (not ENEMY — must not pollute enemy-count/room-clear/BGM). Flags
 * UPDATE_CULLING_DISABLED | DRAW_CULLING_DISABLED, no ACTOR_FLAG_HOSTILE.
 */

#include "remains_ally_common.h"                  // shared helpers + z64.h (structs/prototypes)
#include "objects/object_rat/object_rat.h"        // gRealBombchuSkel / gRealBombchuRunAnim + REAL_BOMBCHU_LIMB_*
#include "objects/gameplay_keep/gameplay_keep.h"  // gBombBodyDL / gBombCapDL + GAMEPLAY_KEEP
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"  // EnBom + BOMB_EXPLOSIVE_TYPE_BOMB / BOMB_TYPE_BODY

// This file is compiled as part of the C++ TU boss_remains.cpp (unity include), so the
// object_rat/gameplay_keep OTR-path symbols are `const char[]` and must be explicitly cast to
// the pointer types the engine wants (an implicit conversion that C allows but C++ does not) —
// same treatment spiritual_stone_statue.c uses for its DL paths.

// ============================================================================
// TUNING
// ============================================================================

#define REMAINS_ALLY_CHU_SCALE       0.010f  // 10/1000 (a touch bigger than the 0.005 tiny chu)
#define REMAINS_ALLY_CHU_CHASE_SPEED 5.0f    // movement speed
#define REMAINS_ALLY_CHU_IDLE_SPEED  5.0f    // wander speed
#define REMAINS_ALLY_CHU_SEEK_RANGE  800.0f  // how far it looks for an enemy to charge
#define REMAINS_ALLY_CHU_HIT_DIST    18.0f   // base XZ contact distance (+ target's cyl radius)
#define REMAINS_ALLY_CHU_FUSE        250     // frames before it self-destructs

// ============================================================================
// STRUCT
// ============================================================================

typedef struct RemainsAllyChu RemainsAllyChu;
typedef void (*RemainsAllyChuActionFunc)(RemainsAllyChu*, PlayState*);

struct RemainsAllyChu {
    /* Actor */ Actor actor;
    /* Anim  */ SkelAnime skelAnime;
    /* AI    */ RemainsAllyChuActionFunc actionFunc;
    /*       */ s16 animLoopCounter;
    /*       */ s16 timer; // fuse countdown; also drives the bomb-body red flash
    /*       */ u8 shouldRotateOntoSurfaces;
    /* Skel  */ Vec3s jointTable[REAL_BOMBCHU_LIMB_MAX];
    /*       */ Vec3s morphTable[REAL_BOMBCHU_LIMB_MAX];
    /* Crawl triad (copied idiom from EnRat) */
    /*       */ Vec3f axisForwards;
    /*       */ Vec3f axisUp;
    /*       */ Vec3f axisLeft;
    /* Goal  */ Actor* target; // nearest enemy self frame, or NULL (idle-follow player)
};

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void RemainsAllyChu_Init(Actor* thisx, PlayState* play);
static void RemainsAllyChu_Destroy(Actor* thisx, PlayState* play);
static void RemainsAllyChu_Update(Actor* thisx, PlayState* play);
static void RemainsAllyChu_Draw(Actor* thisx, PlayState* play);

static void RemainsAllyChu_Active(RemainsAllyChu* self, PlayState* play);
static void RemainsAllyChu_Detonate(RemainsAllyChu* self, PlayState* play);
static void RemainsAllyChu_PostDetonation(RemainsAllyChu* self, PlayState* play);

// The single live instance (Goht allows ONE bombchu at a time). Set in Init, cleared in Destroy.
static RemainsAllyChu* sActiveChu = NULL;

// Read by boss_remains.cpp's TickSummons (same TU via the unity include) to gate the R+B spawn.
s32 RemainsAllyChu_IsAlive(void) {
    return sActiveChu != NULL;
}

// Manual detonation (Goht: press A again). Blows the live chu up RIGHT NOW with a real blast at its own
// position — regardless of whether it had an enemy target — then retires it.
void RemainsAllyChu_DetonateActive(PlayState* play) {
    RemainsAllyChu* self = sActiveChu;
    if ((self == NULL) || (self->actionFunc == RemainsAllyChu_PostDetonation) || (play == NULL)) {
        return;
    }
    EnBom* bomb = (EnBom*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOM, self->actor.world.pos.x,
                                      self->actor.world.pos.y, self->actor.world.pos.z, BOMB_EXPLOSIVE_TYPE_BOMB, 0, 0,
                                      BOMB_TYPE_BODY);
    if (bomb != NULL) {
        bomb->timer = 0; // detonate this frame
    }
    self->actor.speed = 0.0f;
    self->actionFunc = RemainsAllyChu_PostDetonation;
}

// MM DLs may branch into segment 0x0C (the scene cull list) which is not guaranteed to be the
// value we want during our draw. Bind it to a no-op gsSPEndDisplayList so any such branch just
// returns — the exact belt-and-suspenders trick somaria_cubes.c / spiritual_stone_statue.c use.
// (The object_rat skeleton limbs + gameplay_keep bomb DLs En_Rat draws don't actually reference
// 0x0C, so self is insurance, not a requirement.)
static Gfx sRemainsAllyChuSeg0xC_Noop[] = {
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
};

// ============================================================================
// CRAWL ENGINE — copied verbatim from z_en_rat.c (renamed), so the ally moves
// on floors/walls/ceilings/water exactly like the Real Bombchu. `self->actor`
// replaces EnRat's `self->actor`; the axis triad + surface line tests are
// unchanged except that a degenerate surface calls RemainsAllyChu_Detonate.
// ============================================================================

static void RemainsAllyChu_InitializeAxes(RemainsAllyChu* self) {
    Matrix_RotateYS(self->actor.shape.rot.y, MTXMODE_NEW);
    Matrix_RotateXS(self->actor.shape.rot.x, MTXMODE_APPLY);
    Matrix_RotateZS(self->actor.shape.rot.z, MTXMODE_APPLY);

    Matrix_MultVecZ(1.0f, &self->axisForwards);
    Matrix_MultVecY(1.0f, &self->axisUp);
    Matrix_MultVecX(1.0f, &self->axisLeft);
}

static void RemainsAllyChu_UpdateRotation(RemainsAllyChu* self) {
    MtxF mf;

    mf.xx = self->axisLeft.x;
    mf.yx = self->axisLeft.y;
    mf.zx = self->axisLeft.z;

    mf.xy = self->axisUp.x;
    mf.yy = self->axisUp.y;
    mf.zy = self->axisUp.z;

    mf.xz = self->axisForwards.x;
    mf.yz = self->axisForwards.y;
    mf.zz = self->axisForwards.z;

    Matrix_MtxFToYXZRot(&mf, &self->actor.world.rot, false);
    self->actor.world.rot.x = -self->actor.world.rot.x;
}

// Returns true if floorPoly is a valid surface to crawl on. Detonates on a degenerate normal
// (the bombchu blows up when it can't resolve a surface — EnRat behavior).
static s32 RemainsAllyChu_UpdateFloorPoly(RemainsAllyChu* self, CollisionPoly* floorPoly, PlayState* play) {
    Vec3f normal;
    Vec3f vec;
    f32 angle;
    f32 magnitude;
    f32 normDotUp;

    self->actor.floorPoly = floorPoly;

    if (floorPoly != NULL) {
        normal.x = COLPOLY_GET_NORMAL(floorPoly->normal.x);
        normal.y = COLPOLY_GET_NORMAL(floorPoly->normal.y);
        normal.z = COLPOLY_GET_NORMAL(floorPoly->normal.z);
    } else {
        normal.x = 0.0f;
        normal.z = 0.0f;
        normal.y = 1.0f;
    }

    normDotUp = DOTXYZ(normal, self->axisUp);
    if (fabsf(normDotUp) >= 0.999f) {
        return false;
    }

    angle = Math_FAcosF(normDotUp);
    if (angle < 0.001f) {
        return false;
    }

    Math3D_Vec3f_Cross(&self->axisUp, &normal, &vec);

    magnitude = Math3D_Vec3fMagnitude(&vec);
    if (magnitude < 0.001f) {
        RemainsAllyChu_Detonate(self, play);
        return false;
    }

    Math_Vec3f_Scale(&vec, 1.0f / magnitude);
    Matrix_RotateAxisF(angle, &vec, MTXMODE_NEW);
    Matrix_MultVec3f(&self->axisLeft, &vec);
    Math_Vec3f_Copy(&self->axisLeft, &vec);
    Math3D_Vec3f_Cross(&self->axisLeft, &normal, &self->axisForwards);

    magnitude = Math3D_Vec3fMagnitude(&self->axisForwards);
    if (magnitude < 0.001f) {
        RemainsAllyChu_Detonate(self, play);
        return false;
    }

    Math_Vec3f_Scale(&self->axisForwards, 1.0f / magnitude);
    Math_Vec3f_Copy(&self->axisUp, &normal);
    return true;
}

static s32 RemainsAllyChu_IsOnCollisionPoly(PlayState* play, Vec3f* posA, Vec3f* posB, Vec3f* posResult,
                                            CollisionPoly** poly, s32* bgId) {
    WaterBox* waterBox;
    s32 isOnWater;
    f32 waterSurface;

    if (WaterBox_GetSurface1(play, &play->colCtx, posB->x, posB->z, &waterSurface, &waterBox) &&
        (waterSurface <= posA->y) && (posB->y <= waterSurface)) {
        isOnWater = true;
    } else {
        isOnWater = false;
    }

    if (BgCheck_EntityLineTest1(&play->colCtx, posA, posB, posResult, poly, 1, 1, 1, 1, bgId)) {
        if (!(SurfaceType_GetWallFlags(&play->colCtx, *poly, *bgId) & (WALL_FLAG_4 | WALL_FLAG_5)) &&
            (!isOnWater || (waterSurface <= posResult->y))) {
            return true;
        }
    }

    if (isOnWater) {
        posResult->x = posB->x;
        posResult->y = waterSurface;
        posResult->z = posB->z;
        *poly = NULL;
        *bgId = BGCHECK_SCENE;
        return true;
    }

    return false;
}

static s32 RemainsAllyChu_IsTouchingSurface(RemainsAllyChu* self, PlayState* play) {
    CollisionPoly* polySide = NULL;
    CollisionPoly* polyUpDown = NULL;
    s32 bgIdSide;
    s32 bgIdUpDown;
    s32 i;
    f32 lineLength;
    Vec3f posA;
    Vec3f posB;
    Vec3f posSide;
    Vec3f posUpDown;

    bgIdUpDown = bgIdSide = BGCHECK_SCENE;

    lineLength = 2.0f * self->actor.speed;

    posA.x = self->actor.world.pos.x + (self->axisUp.x * 5.0f);
    posA.y = self->actor.world.pos.y + (self->axisUp.y * 5.0f);
    posA.z = self->actor.world.pos.z + (self->axisUp.z * 5.0f);

    posB.x = self->actor.world.pos.x - (self->axisUp.x * 4.0f);
    posB.y = self->actor.world.pos.y - (self->axisUp.y * 4.0f);
    posB.z = self->actor.world.pos.z - (self->axisUp.z * 4.0f);

    if (RemainsAllyChu_IsOnCollisionPoly(play, &posA, &posB, &posUpDown, &polyUpDown, &bgIdUpDown)) {
        posB.x = (self->axisForwards.x * lineLength) + posA.x;
        posB.y = (self->axisForwards.y * lineLength) + posA.y;
        posB.z = (self->axisForwards.z * lineLength) + posA.z;

        if (RemainsAllyChu_IsOnCollisionPoly(play, &posA, &posB, &posSide, &polySide, &bgIdSide)) {
            self->shouldRotateOntoSurfaces |= RemainsAllyChu_UpdateFloorPoly(self, polySide, play);
            Math_Vec3f_Copy(&self->actor.world.pos, &posSide);
            self->actor.floorBgId = bgIdSide;
            self->actor.speed = 0.0f;
        } else {
            if (polyUpDown != self->actor.floorPoly) {
                self->shouldRotateOntoSurfaces |= RemainsAllyChu_UpdateFloorPoly(self, polyUpDown, play);
            }

            Math_Vec3f_Copy(&self->actor.world.pos, &posUpDown);
            self->actor.floorBgId = bgIdUpDown;
        }
    } else {
        self->actor.speed = 0.0f;
        lineLength *= 3.0f;
        Math_Vec3f_Copy(&posA, &posB);

        for (i = 0; i < 3; i++) {
            if (i == 0) {
                // backwards
                posB.x = posA.x - (self->axisForwards.x * lineLength);
                posB.y = posA.y - (self->axisForwards.y * lineLength);
                posB.z = posA.z - (self->axisForwards.z * lineLength);
            } else if (i == 1) {
                // left
                posB.x = posA.x + (self->axisLeft.x * lineLength);
                posB.y = posA.y + (self->axisLeft.y * lineLength);
                posB.z = posA.z + (self->axisLeft.z * lineLength);
            } else {
                // right
                posB.x = posA.x - (self->axisLeft.x * lineLength);
                posB.y = posA.y - (self->axisLeft.y * lineLength);
                posB.z = posA.z - (self->axisLeft.z * lineLength);
            }

            if (RemainsAllyChu_IsOnCollisionPoly(play, &posA, &posB, &posSide, &polySide, &bgIdSide)) {
                self->shouldRotateOntoSurfaces |= RemainsAllyChu_UpdateFloorPoly(self, polySide, play);
                Math_Vec3f_Copy(&self->actor.world.pos, &posSide);
                self->actor.floorBgId = bgIdSide;
                break;
            }
        }

        if (i == 3) {
            // no collision nearby
            return false;
        }
    }

    return true;
}

static void RemainsAllyChu_HandleNonSceneCollision(RemainsAllyChu* self, PlayState* play) {
    s16 yaw = self->actor.shape.rot.y;
    f32 sin;
    f32 cos;
    f32 tempX;

    DynaPolyActor_TransformCarriedActor(&play->colCtx, self->actor.floorBgId, &self->actor);

    if (yaw != self->actor.shape.rot.y) {
        yaw = self->actor.shape.rot.y - yaw;

        sin = Math_SinS(yaw);
        cos = Math_CosS(yaw);

        tempX = self->axisForwards.x;
        self->axisForwards.x = (sin * self->axisForwards.z) + (cos * tempX);
        self->axisForwards.z = (cos * self->axisForwards.z) - (sin * tempX);

        tempX = self->axisUp.x;
        self->axisUp.x = (sin * self->axisUp.z) + (cos * tempX);
        self->axisUp.z = (cos * self->axisUp.z) - (sin * tempX);

        tempX = self->axisLeft.x;
        self->axisLeft.x = (sin * self->axisLeft.z) + (cos * tempX);
        self->axisLeft.z = (cos * self->axisLeft.z) - (sin * tempX);
    }
}

// Steer the crawl heading toward `goal`. This is EnRat_ChooseDirection's "chasing" branch
// generalized to any actor (enemy target, or the player when idle-following): rotate the
// forward axis a little toward the goal's yaw each frame, respecting the up-axis flip so it
// still steers correctly while upside-down on a ceiling.
static void RemainsAllyChu_ChooseDirection(RemainsAllyChu* self, Actor* goal) {
    Vec3f newAxisForwards;
    s16 angle;

    angle = Actor_WorldYawTowardActor(&self->actor, goal) - self->actor.shape.rot.y;
    if (self->axisUp.y < -0.25f) {
        angle -= 0x8000;
    }

    angle = CLAMP(angle, -0x800, 0x800);
    Matrix_RotateAxisF(BINANG_TO_RAD(angle), &self->axisUp, MTXMODE_NEW);
    Matrix_MultVec3f(&self->axisForwards, &newAxisForwards);
    Math_Vec3f_Copy(&self->axisForwards, &newAxisForwards);
    Math3D_Vec3f_Cross(&self->axisUp, &self->axisForwards, &self->axisLeft);
    self->shouldRotateOntoSurfaces = true;
}

// ============================================================================
// DETONATION
// ============================================================================

// Retire the chu, spawning a real blast ONLY when it actually reached an enemy. A friendly summon
// must never blow up on Link: EN_BOM's blast is AT_TYPE_ALL (hurts everyone), so we only spawn it
// when `self->target != NULL` (we detonated ON/at an enemy). When there is no target — fuse ran out
// while idle-following Link, or the chu ran off every surface with nobody to hit — it just fizzles
// harmlessly. (When it DOES detonate on an enemy the blast is MM-authentic, so keep Link clear of
// the target the same as you would when throwing a real bombchu.)
static void RemainsAllyChu_Detonate(RemainsAllyChu* self, PlayState* play) {
    // Guard against a double-spawn: the movement chain can reach Detonate from more than one
    // path in a single frame (degenerate floor, lost surface, wall-damage, contact).
    if (self->actionFunc == RemainsAllyChu_PostDetonation) {
        return;
    }

    if (self->target != NULL) {
        EnBom* bomb = (EnBom*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOM, self->actor.world.pos.x,
                                          self->actor.world.pos.y, self->actor.world.pos.z, BOMB_EXPLOSIVE_TYPE_BOMB,
                                          0, 0, BOMB_TYPE_BODY);
        if (bomb != NULL) {
            bomb->timer = 0; // detonate self frame
        }
    } else {
        // No enemy to hit → fizzle out with a chirp, no player-damaging blast.
        Actor_PlaySfx(&self->actor, NA_SE_EN_BOMCHU_AIM);
    }

    self->actor.speed = 0.0f;
    self->actionFunc = RemainsAllyChu_PostDetonation;
}

static void RemainsAllyChu_PostDetonation(RemainsAllyChu* self, PlayState* play) {
    Actor_Kill(&self->actor);
}

// ============================================================================
// AI — a single "active" behavior: crawl toward the current goal, self-destruct
// on the fuse. Target selection + the crawl/move chain live in Update (mirrors
// EnRat_Update, which runs the movement chain around the action func).
// ============================================================================

static void RemainsAllyChu_Active(RemainsAllyChu* self, PlayState* play) {
    // Fuse: a summoned bombchu has a limited life whether or not it reaches a foe.
    if (self->timer > 0) {
        self->timer--;
    }
    if (self->timer == 0) {
        RemainsAllyChu_Detonate(self, play);
        return;
    }

    // Charge speed when it has a foe, idle wander/follow speed otherwise.
    self->actor.speed = (self->target != NULL) ? REMAINS_ALLY_CHU_CHASE_SPEED : REMAINS_ALLY_CHU_IDLE_SPEED;

    // Footstep + chirp sfx, straight from the Real Bombchu.
    if (Animation_OnFrame(&self->skelAnime, 0.0f)) {
        Actor_PlaySfx(&self->actor, NA_SE_EN_BOMCHU_WALK);
        if (self->animLoopCounter != 0) {
            self->animLoopCounter--;
        }
    }
    if ((self->animLoopCounter == 0) && (Rand_ZeroOne() < 0.05f)) {
        Actor_PlaySfx(&self->actor, NA_SE_EN_BOMCHU_AIM);
        self->animLoopCounter = 5;
    }
    if (self->target != NULL) {
        Actor_PlaySfx_Flagged(&self->actor, NA_SE_EN_BOMCHU_RUN - SFX_FLAG);
    }
}

// ============================================================================
// INIT / DESTROY
// ============================================================================

static void RemainsAllyChu_Init(Actor* thisx, PlayState* play) {
    RemainsAllyChu* self = (RemainsAllyChu*)thisx;

    // params is reserved for a future sub-mode; the chu has none, so it is ignored here.
    (void)self->actor.params;

    Actor_SetScale(&self->actor, REMAINS_ALLY_CHU_SCALE);

    // Load the Real Bombchu skeleton + run anim by OTR path. SkelAnime_InitFlex feeds the
    // "__OTR__objects/object_rat/..." strings through ResourceMgr_LoadSkeletonByName /
    // ResourceMgr_LoadAnimByName, so OBJECT_RAT never needs to be in the object bank and the
    // profile can stay GAMEPLAY_KEEP. Explicit casts because self is a C++ TU.
    SkelAnime_InitFlex(play, &self->skelAnime, (FlexSkeletonHeader*)gRealBombchuSkel,
                       (AnimationHeader*)gRealBombchuRunAnim, self->jointTable, self->morphTable,
                       REAL_BOMBCHU_LIMB_MAX);
    ActorShape_Init(&self->actor.shape, 0.0f, ActorShadow_DrawCircle, 25.0f);

    self->timer = REMAINS_ALLY_CHU_FUSE;
    self->animLoopCounter = 5;
    self->target = NULL;
    self->shouldRotateOntoSurfaces = false;

    RemainsAllyChu_InitializeAxes(self);
    RemainsAllyChu_UpdateRotation(self);

    Animation_PlayLoop(&self->skelAnime, (AnimationHeader*)gRealBombchuRunAnim);
    self->actor.speed = REMAINS_ALLY_CHU_IDLE_SPEED;
    self->actionFunc = RemainsAllyChu_Active;

    sActiveChu = self; // one-at-a-time bookkeeping + manual-detonate handle
}

static void RemainsAllyChu_Destroy(Actor* thisx, PlayState* play) {
    // No collider, no effects, and SkelAnime_InitFlex registered no skeleton (it passes NULL to
    // ResourceMgr_LoadSkeletonByName), so there is nothing to unregister — same as En_Rat.
    (void)play;
    if (sActiveChu == (RemainsAllyChu*)thisx) {
        sActiveChu = NULL;
    }
}

// ============================================================================
// UPDATE
// ============================================================================

static void RemainsAllyChu_Update(Actor* thisx, PlayState* play) {
    RemainsAllyChu* self = (RemainsAllyChu*)thisx;

    self->shouldRotateOntoSurfaces = false;
    SkelAnime_Update(&self->skelAnime);

    // Retarget every frame: nearest live enemy in range, or NULL → it just runs its own Real-Bombchu AI
    // (crawls straight ahead, climbing any wall/ceiling it meets) and NEVER targets Link.
    self->target = RemainsAlly_FindNearestEnemy(play, &self->actor.world.pos, REMAINS_ALLY_CHU_SEEK_RANGE);

    self->actionFunc(self, play);

    // Detonated inside the action func (fuse) — stop here; PostDetonation kills next frame.
    if (self->actionFunc == RemainsAllyChu_PostDetonation) {
        return;
    }

    // Crawl chain (EnRat_Update order): dynapoly carry → steer → resolve surface → rotate → move.
    if (self->actor.floorBgId != BGCHECK_SCENE) {
        RemainsAllyChu_HandleNonSceneCollision(self, play);
    }

    // Steer toward the enemy if we have one; with no foe, don't steer — it wanders straight ahead and the
    // surface-resolver keeps it climbing walls/ceilings, exactly like a live Real Bombchu with no target.
    if (self->target != NULL) {
        RemainsAllyChu_ChooseDirection(self, self->target);
    }

    if (!RemainsAllyChu_IsTouchingSurface(self, play)) {
        RemainsAllyChu_Detonate(self, play); // ran off every surface — blow up, like the Real Bombchu
        return;
    }

    if (self->shouldRotateOntoSurfaces) {
        RemainsAllyChu_UpdateRotation(self);
        self->actor.shape.rot.x = -self->actor.world.rot.x;
        self->actor.shape.rot.y = self->actor.world.rot.y;
        self->actor.shape.rot.z = self->actor.world.rot.z;
    }

    Actor_MoveWithoutGravity(&self->actor);
    self->actor.floorHeight = self->actor.world.pos.y;

    if (SurfaceType_IsWallDamage(&play->colCtx, self->actor.floorPoly, self->actor.floorBgId)) {
        RemainsAllyChu_Detonate(self, play);
        return;
    }

    // Contact detonation — ONLY against the enemy target, never Link (Link is never `target`).
    if (self->target != NULL) {
        f32 contact = REMAINS_ALLY_CHU_HIT_DIST + (f32)self->target->colChkInfo.cylRadius;
        if (Actor_WorldDistXZToActor(&self->actor, self->target) < contact) {
            RemainsAllyChu_Detonate(self, play);
            return;
        }
    }

    Actor_SetFocus(&self->actor, self->actor.shape.yOffset * 0.015f);
}

// ============================================================================
// DRAW — object_rat skeleton with the bomb body drawn on the tail-end limb,
// flashing faster as the fuse runs down (EnRat_PostLimbDraw, minus the electric
// spark particles). Both bomb DLs are gameplay_keep, always resident.
// ============================================================================

static s32 RemainsAllyChu_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                           Actor* thisx) {
    if (limbIndex == REAL_BOMBCHU_LIMB_TAIL_END) {
        *dList = NULL; // the tail-end limb is replaced by the bomb, drawn in the post-limb hook
    }
    return false;
}

static void RemainsAllyChu_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    RemainsAllyChu* self = (RemainsAllyChu*)thisx;
    f32 redModifier;

    if (limbIndex == REAL_BOMBCHU_LIMB_TAIL_END) {
        OPEN_DISPS(play->state.gfxCtx);

        Matrix_ReplaceRotation(&play->billboardMtxF);

        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gBombCapDL);

        // Flash red, accelerating as the fuse shortens (EnRat overworld-fuse cadence).
        if (self->timer >= 120) {
            redModifier = fabsf(Math_CosF((self->timer % 30) * (M_PIf / 30.0f)));
        } else if (self->timer >= 30) {
            redModifier = fabsf(Math_CosF((self->timer % 6) * (M_PIf / 6.0f)));
        } else {
            redModifier = fabsf(Math_CosF((self->timer % 3) * (M_PIf / 3.0f)));
        }

        gDPSetEnvColor(POLY_OPA_DISP++, (s32)((1.0f - redModifier) * 255.0f), 0, 40, 255);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, (s32)((1.0f - redModifier) * 255.0f), 0, 40, 255);
        Matrix_RotateZYX(0x4000, 0, 0, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gBombBodyDL);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}

static void RemainsAllyChu_Draw(Actor* thisx, PlayState* play) {
    RemainsAllyChu* self = (RemainsAllyChu*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)sRemainsAllyChuSeg0xC_Noop);
    CLOSE_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    Gfx_SetupDL60_XluNoCD(play->state.gfxCtx);
    func_800B8050(&self->actor, play, 0);
    SkelAnime_DrawFlexOpa(play, self->skelAnime.skeleton, self->skelAnime.jointTable, self->skelAnime.dListCount,
                          RemainsAllyChu_OverrideLimbDraw, RemainsAllyChu_PostLimbDraw, &self->actor);
}

// ============================================================================
// ACTOR PROFILE + SPAWN
// ============================================================================

// Bound by the ACTOR_REMAINS_ALLY_CHU row in actor_table.h (added by the Integrate step;
// DEFINE_ACTOR name RemainsAllyChu → RemainsAllyChu_Profile). ACTORCAT_MISC so it stays out of
// enemy-count / room-clear / battle-BGM; GAMEPLAY_KEEP so Actor_Spawn never fails on a missing
// scene object (the real model is loaded by OTR path in Init). Trailing `reset` field is left
// zero-initialized, matching En_Rat / spiritual_stone_statue.
ActorProfile RemainsAllyChu_Profile = {
    /**/ ACTOR_REMAINS_ALLY_CHU,
    /**/ ACTORCAT_MISC,
    /**/ (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED),
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(RemainsAllyChu),
    /**/ RemainsAllyChu_Init,
    /**/ RemainsAllyChu_Destroy,
    /**/ RemainsAllyChu_Update,
    /**/ RemainsAllyChu_Draw,
};

// Spawn helper for boss_remains.cpp's Phase-3 summon. params carries nothing for the chu (each
// ally id is already boss-specific); pass 0.
Actor* RemainsAllyChu_Spawn(PlayState* play, Vec3f* pos, s16 rotY) {
    if ((play == NULL) || (pos == NULL)) {
        return NULL;
    }
    return Actor_Spawn(&play->actorCtx, play, ACTOR_REMAINS_ALLY_CHU, pos->x, pos->y, pos->z, 0, rotY, 0, 0);
}
