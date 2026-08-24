/**
 * remains_ally_bug.c - Odolwa's friendly "bug" summon ally (ACTOR_REMAINS_ALLY_BUG, 0x2BE).
 *
 * KAMIKAZE, ephemeral: a small billboarded moth-bug that HOPS toward the nearest enemy and,
 * on reaching it, delivers a burst of AT_TYPE_PLAYER damage and dies (Actor_Kill). Odolwa's
 * remains spawns a few of these at once (each call to RemainsAllyBug_Spawn makes ONE bug).
 *
 * ---- Unity include (NOT in CMake/vcxproj) ----------------------------------------------
 * This .c is #included into boss_remains.cpp inside its `extern "C"` block, exactly like
 * spiritual_stones.cpp #includes ../actors/spiritual_stone_statue.c, so RemainsAllyBug_Profile
 * / RemainsAllyBug_Spawn get C linkage and match the DEFINE_ACTOR-generated
 * `extern ActorProfile RemainsAllyBug_Profile`. It is therefore compiled as C++:
 *   - NEVER name a local `this` (reserved word in C++) -> the typed pointer is `self`.
 *   - OTR path strings cast to (Gfx*) explicitly (no implicit ptr->ptr in C++).
 * remains_ally_common.c must be #included BEFORE this file in boss_remains.cpp so the
 * RemainsAlly_* helpers are defined.
 *
 * ---- Art (verified) --------------------------------------------------------------------
 * Odolwa's real bug is ACTOR_EN_TANRON1, a 200-particle moth SWARM with NO skeleton — its
 * AI is NOT reused. Its moth sprite is overlay-embedded and, in 2ship, is reachable by OTR
 * path: the vanilla EnTanron1_Draw itself renders the swarm by handing these very strings
 * to gSPDisplayList (see z_en_tanron1.c func_80BB5AAC + assets/overlays/ovl_En_Tanron1.h),
 * which proves overlay-DL-by-OTR-path is reliable here. We draw the "alive moth" pair:
 *   ovl_En_Tanron1_DL_001888  = setup DL (loads the wing texture at 0x1028 + combiner)
 *   ovl_En_Tanron1_DL_001900  = the moth sprite model
 * The resource manager resolves each DL's internal Vtx/texture refs, so no object bank is
 * bound. Drawn as a camera-facing billboard with an optional sine wing-flap.
 * TODO(art): if a future asset audit shows these overlay DLs failing to resolve, fall back
 * to a tiny scaled gameplay_keep sprite (e.g. a gEffDust or gEffShieldParticle DL) -- the AI
 * below is independent of the art.
 */

#include "remains_ally_common.h"

#include "z64.h"
#include "functions.h"
#include "macros.h"
#include "variables.h"
// The friendly bug IS Odolwa's actual summoned bug — the ACTOR_BOSS_01/ODOLWA_TYPE_BUG creature (its
// own crawling beetle, gOdolwaBugSkel + gOdolwaBugCrawlAnim in object_boss01, drawn as a FLEX skeleton).
// Loaded by OTR path so object_boss01 need not be resident (same route the bombchu ally uses).
#include "objects/object_boss01/object_boss01.h"
// gEffLightningDL — the classic lightning bolt sheet (Goht-thunder projectile visual).
#include "objects/gameplay_keep/gameplay_keep.h"
// gGohtLightningMaterialDL / gGohtLightningModelDL — Goht's REAL lightning bolt model (z_boss_hakugin.c
// BossHakugin_DrawLightningSegments). Used by the thunder mode so the bolt looks exactly like Goht's.
#include "objects/object_boss_hakugin/object_boss_hakugin.h"

#define BUG_ODOLWA_LIMB_MAX 0x15 // ODOLWA_BUG_LIMB_MAX
#define BUG_BEETLE_SCALE 0.025f  // Boss01_Bug uses 0.025

// ============================================================================
// Art paths (OTR) + tuning
// ============================================================================

// Odolwa's moth sprite from ovl_En_Tanron1 (see file header). Same "hand an __OTR__ path
// straight to the gfx pipe" idiom as boss_remains.cpp's kMaskDLPath and Din's Fire.
static const char* const sMothSetupDLPath = "__OTR__overlays/ovl_En_Tanron1/ovl_En_Tanron1_DL_001888";
static const char* const sMothModelDLPath = "__OTR__overlays/ovl_En_Tanron1/ovl_En_Tanron1_DL_001900";

#define BUG_SEARCH_RANGE 600.0f // XZ radius to look for an enemy each frame
#define BUG_HOP_INTERVAL 15     // frames between upward hops
#define BUG_HOP_STRENGTH 4.5f   // velocity.y injected on a hop
#define BUG_CHASE_SPEED 4.2f    // XZ speed while homing on an enemy
#define BUG_IDLE_SPEED 2.5f     // XZ speed while following the player
#define BUG_HIT_DIST 30.0f      // XZ distance at which the kamikaze detonates
#define BUG_FOLLOW_DIST 55.0f   // idle: stop this close to the player
#define BUG_IDLE_LIFETIME 300   // frames the bug survives with nothing to fight
#define BUG_GRAVITY -1.0f       // pulls each hop back down
#define BUG_TERMINAL_VY -8.0f   // clamp on fall speed
#define BUG_DRAW_SCALE 4.5f     // billboard sprite scale — big + obvious (vanilla En_Tanron1 particle ~1.2)
#define BUG_DRAW_Y_LIFT 8.0f    // lift the sprite off its ground anchor
#define BUG_FLAP_RATE 0x1400    // wing-flap phase advance per frame (binang)

// Projectile mode (Odolwa sword-beam moth): fly straight forward like an FD sword beam.
#define BUG_MODE_PROJECTILE 1
#define BUG_PROJECTILE_SPEED 26.0f       // forward flight speed
#define BUG_PROJECTILE_TTL 45            // frames before it fizzles if it hits nothing
#define BUG_PROJECTILE_HOME_RANGE 550.0f // look this far for an enemy to curve toward
#define BUG_PROJECTILE_HOME_CONE 0x3000  // only home if the enemy is within ~66° of the flight path
#define BUG_PROJECTILE_HOME_STEP 0x600   // max yaw turn per frame toward it (gentle self-guiding)

// Thunder mode (Goht's R+A bolt): a lightning bolt that flies straight, PIERCES WALLS (no bgcheck /
// wall kill), homes like the beam, and hits harder. Visual = gameplay_keep's lightning DL, Goht cyan.
#define BUG_MODE_THUNDER 2
#define BUG_THUNDER_SPEED 34.0f
#define BUG_THUNDER_TTL 40
#define BUG_THUNDER_DAMAGE 0x06

// Cloud mode (Odolwa "Nimbus" flight): a moth that hovers UNDER Link in an orbiting ring, forming the
// cloud that carries him. It only exists while Link is flying (BossRemains_IsOdolwaFlying) and never
// attacks. hopTimer is repurposed as the orbit angle.
#define BUG_MODE_CLOUD 3
#define BUG_CLOUD_RADIUS 22.0f // ring radius under Link
#define BUG_CLOUD_BELOW 10.0f  // how far below Link the cloud sits
#define BUG_CLOUD_ORBIT 0x0300 // orbit angular speed (binang/frame)

// Pikmin ball (ground beetles trail Link like Gyorg's fish school): a loose spaced BALL BEHIND Link that
// only breaks formation to attack when Link holds still, converging on the enemy nearest to LINK.
#define BUG_BALL_DIST 50.0f    // base distance the ball trails behind Link
#define BUG_BALL_SPACING 26.0f // extra ring depth so they don't all sit at one radius
#define BUG_BALL_SEP 22.0f     // boids separation radius (keeps the ball from merging to a dot)
#define BUG_LINK_MOVE 2.5f     // Link speedXZ above this = "moving" → re-form (don't peel off to attack)

extern s32 BossRemains_IsOdolwaFlying(void); // defined later in the same TU (boss_remains.cpp)

typedef struct RemainsAllyBug {
    /**/ Actor actor;
    /**/ SkelAnime skelAnime; // ground mode: Odolwa's bug (gOdolwaBugSkel) crawling skeleton
    /**/ Vec3s jointTable[BUG_ODOLWA_LIMB_MAX];
    /**/ Vec3s morphTable[BUG_ODOLWA_LIMB_MAX];
    /**/ ColliderCylinder collider;
    /**/ s16 spawnParams; // 0 = ground beetle, BUG_MODE_PROJECTILE/THUNDER/CLOUD otherwise
    /**/ s16 hopTimer;    // frames until the next hop (CLOUD: orbit angle)
    /**/ s16 lifeTimer;   // hard TTL countdown
    /**/ s16 flapTimer;   // projectile (moth) wing-flap phase
} RemainsAllyBug;

// ============================================================================
// Collider — the crux of "friendly": friend/foe is the AT/AC TYPE bits, NOT category.
// AT_TYPE_PLAYER toucher hits every enemy (their body bumpers are AC_TYPE_PLAYER) and can
// NEVER hit Link (his body bumper is AC_TYPE_ENEMY). Masks copied from Ivan
// (z_en_partner.inc.c) so the hit actually lands: the gate at z_collision_check.c:3002
// needs a shared TYPE bit AND overlapping dmgFlags. AC_NONE => we never call
// CollisionCheck_SetAC, so the bug is invulnerable (the acDmgInfo below is inert but kept
// as Ivan's value for parity). No OC so it never shoves actors around.
// ============================================================================
static ColliderCylinderInit sRemainsAllyBugColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        // Ivan's toucher dmgFlags (deku-stick class hit) so enemy bumpers accept it; damage 4.
        { DMG_DEKU_STICK, 0x00, 0x04 },
        // Ivan's accept-all bumper mask — inert here (AC_NONE), kept for parity.
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    // Radius 24 / height 44 — deliberately generous so the AT toucher OVERLAPS an enemy's AC bumper
    // a few frames BEFORE the proximity kill distance (BUG_HIT_DIST) fires; otherwise a too-small
    // cylinder would let the kamikaze self-destruct before the hit ever lands.
    { 24, 44, 0, { 0, 0, 0 } },
};

// MM DLs can branch into segment 0x0C (scene cull list). The moth sprite almost certainly
// does not, but bind it to a no-op gsSPEndDisplayList defensively — same trick as
// spiritual_stone_statue.c:105 — so any stray 0x0C branch just returns instead of crashing.
static Gfx sBugSegment0xC_Noop[] = {
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
};

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void RemainsAllyBug_Init(Actor* thisx, PlayState* play);
static void RemainsAllyBug_Destroy(Actor* thisx, PlayState* play);
static void RemainsAllyBug_Update(Actor* thisx, PlayState* play);
static void RemainsAllyBug_Draw(Actor* thisx, PlayState* play);

// ============================================================================
// INIT / DESTROY
// ============================================================================

static void RemainsAllyBug_Init(Actor* thisx, PlayState* play) {
    RemainsAllyBug* self = (RemainsAllyBug*)thisx;

    self->spawnParams = thisx->params; // 0 = ground beetle, BUG_MODE_PROJECTILE(1) = flying moth-beam

    if (self->spawnParams != 0) {
        // Projectile modes: 1 = sword-beam MOTH (billboard), 2 = Goht THUNDER bolt. Both fly
        // dead-straight (no gravity), short-lived.
        self->flapTimer = (s16)Rand_ZeroFloat(65535.0f);
        self->actor.shape.shadowDraw = NULL;
        self->actor.shape.shadowScale = 0.0f;
        Actor_SetScale(&self->actor, 0.01f);
        self->actor.gravity = 0.0f;
        self->actor.terminalVelocity = 0.0f;
        self->actor.speed = (self->spawnParams == BUG_MODE_THUNDER) ? BUG_THUNDER_SPEED : BUG_PROJECTILE_SPEED;
        self->actor.world.rot.y = self->actor.shape.rot.y; // Actor_MoveWithGravity flies along this
        self->hopTimer = 0;
        self->lifeTimer = (self->spawnParams == BUG_MODE_THUNDER) ? BUG_THUNDER_TTL : BUG_PROJECTILE_TTL;

        if (self->spawnParams == BUG_MODE_CLOUD) {
            // Not a projectile: it hovers under Link. hopTimer = orbit angle (seeded from spawn rot), no
            // speed of its own, and no TTL — it lives until the flight ends (self-kills in Update).
            self->actor.speed = 0.0f;
            self->hopTimer = self->actor.shape.rot.y;
            self->lifeTimer = 0;
            Actor_SetScale(&self->actor, 0.014f);
        }
    } else {
        // Ground = Odolwa's real BEETLE: FLEX skeleton + crawl anim from object_boss01, loaded by OTR path
        // (the resource manager resolves the limb DLs — same route the bombchu ally uses).
        SkelAnime_InitFlex(play, &self->skelAnime, (FlexSkeletonHeader*)gOdolwaBugSkel,
                           (AnimationHeader*)gOdolwaBugCrawlAnim, self->jointTable, self->morphTable,
                           BUG_ODOLWA_LIMB_MAX);
        Animation_PlayLoop(&self->skelAnime, (AnimationHeader*)gOdolwaBugCrawlAnim);
        Actor_SetScale(&self->actor, BUG_BEETLE_SCALE);
        ActorShape_Init(&self->actor.shape, 0.0f, ActorShadow_DrawCircle, 12.0f);
        self->actor.gravity = BUG_GRAVITY;
        self->actor.terminalVelocity = BUG_TERMINAL_VY;
        self->hopTimer = (s16)Rand_ZeroFloat((f32)BUG_HOP_INTERVAL); // desync a group's hops
        self->lifeTimer = BUG_IDLE_LIFETIME;
    }

    // Collider is initialised HERE (after the actor is linked), per the contract.
    Collider_InitCylinder(play, &self->collider);
    Collider_SetCylinder(play, &self->collider, &self->actor, &sRemainsAllyBugColliderInit);
    if (self->spawnParams == BUG_MODE_THUNDER) {
        self->collider.elem.atDmgInfo.damage = BUG_THUNDER_DAMAGE; // the bolt hits harder
    }
}

static void RemainsAllyBug_Destroy(Actor* thisx, PlayState* play) {
    RemainsAllyBug* self = (RemainsAllyBug*)thisx;

    Collider_DestroyCylinder(play, &self->collider);
}

// Boids separation among GROUND beetles: nudge apart so the Pikmin ball stays spaced instead of merging
// into one dot (mirrors RemainsAllyFish_Separate). Only ground bugs (spawnParams 0) push each other.
static void RemainsAllyBug_Separate(RemainsAllyBug* self, PlayState* play) {
    for (Actor* a = play->actorCtx.actorLists[ACTORCAT_MISC].first; a != NULL; a = a->next) {
        if ((a == &self->actor) || (a->id != ACTOR_REMAINS_ALLY_BUG) || (((RemainsAllyBug*)a)->spawnParams != 0)) {
            continue;
        }
        f32 d = Math_Vec3f_DistXZ(&self->actor.world.pos, &a->world.pos);
        if ((d < BUG_BALL_SEP) && (d > 0.1f)) {
            s16 away = Math_Vec3f_Yaw(&a->world.pos, &self->actor.world.pos); // neighbor -> self
            f32 push = (BUG_BALL_SEP - d) * 0.25f;
            self->actor.world.pos.x += Math_SinS(away) * push;
            self->actor.world.pos.z += Math_CosS(away) * push;
        }
    }
}

// ============================================================================
// UPDATE — Pikmin ball behind Link (like Gyorg's fish), peeling off to swarm the
// nearest enemy to Link only while Link holds still; kamikaze on contact.
// ============================================================================

static void RemainsAllyBug_Update(Actor* thisx, PlayState* play) {
    RemainsAllyBug* self = (RemainsAllyBug*)thisx;
    Actor* target;

    if (self->spawnParams != 0) {
        self->flapTimer += BUG_FLAP_RATE; // moth wing-flap / thunder flicker phase
    } else {
        SkelAnime_Update(&self->skelAnime); // crawl the beetle's legs
    }

    // CLOUD: hover under Link in an orbiting ring for as long as the flight lasts; self-kill when it ends.
    if (self->spawnParams == BUG_MODE_CLOUD) {
        if (!BossRemains_IsOdolwaFlying()) {
            Actor_Kill(&self->actor);
            return;
        }
        Player* pl = GET_PLAYER(play);
        if (pl != NULL) {
            self->hopTimer += BUG_CLOUD_ORBIT; // swirl the ring
            self->actor.world.pos.x = pl->actor.world.pos.x + (Math_SinS(self->hopTimer) * BUG_CLOUD_RADIUS);
            self->actor.world.pos.z = pl->actor.world.pos.z + (Math_CosS(self->hopTimer) * BUG_CLOUD_RADIUS);
            self->actor.world.pos.y = pl->actor.world.pos.y - BUG_CLOUD_BELOW;
            self->actor.shape.rot.y = self->hopTimer;
        }
        return;
    }

    if (self->hopTimer > 0) {
        self->hopTimer--;
    }

    // Hard TTL: every bug expires after BUG_IDLE_LIFETIME frames whether or not it ever reaches a
    // foe — so a target it can't actually get to (behind a wall, flying, unreachable) can't keep the
    // kamikaze alive forever. Kamikaze-on-contact (below) still ends it early on a successful hit.
    if (self->lifeTimer > 0) {
        self->lifeTimer--;
        if (self->lifeTimer == 0) {
            Actor_PlaySfx(&self->actor, NA_SE_EN_MB_MOTH_DEAD);
            Actor_Kill(&self->actor);
            return;
        }
    }

    // Projectile modes: fly dead-straight, damage the first enemy touched. Moth-beam (1) dies on
    // walls; Goht THUNDER (2) PIERCES WALLS (no bg check at all) and just times out.
    if (self->spawnParams != 0) {
        // Light self-guiding: if an enemy is roughly ahead, curve gently toward it (a little homing,
        // not a hard lock). Enemies behind/beside are ignored so the bolt still reads as forward-fired.
        Actor* homeTarget = RemainsAlly_FindNearestEnemy(play, &self->actor.world.pos, BUG_PROJECTILE_HOME_RANGE);
        if (homeTarget != NULL) {
            s16 toTarget = Actor_WorldYawTowardActor(&self->actor, homeTarget);
            s16 diff = toTarget - self->actor.world.rot.y;
            s16 absDiff = (diff < 0) ? -diff : diff;
            if (absDiff < BUG_PROJECTILE_HOME_CONE) {
                Math_SmoothStepToS(&self->actor.world.rot.y, toTarget, 3, BUG_PROJECTILE_HOME_STEP, 0);
                self->actor.shape.rot.y = self->actor.world.rot.y;
            }
        }
        Actor_MoveWithGravity(&self->actor); // gravity 0 → flies along world.rot.y
        if (self->spawnParams == BUG_MODE_PROJECTILE) {
            Actor_UpdateBgCheckInfo(play, &self->actor, 20.0f, 12.0f, 0.0f, UPDBGCHECKINFO_FLAG_1);
        } else {
            // thunder: NO bg check — it flies through walls; crackle with Goht's thunder loop as it travels.
            Actor_PlaySfx(&self->actor, NA_SE_EN_COMMON_THUNDER - SFX_FLAG);
        }
        Collider_UpdateCylinder(&self->actor, &self->collider);
        CollisionCheck_SetAT(play, &play->colChkCtx, &self->collider.base);
        if ((self->collider.base.atFlags & AT_HIT) ||
            ((self->spawnParams == BUG_MODE_PROJECTILE) && (self->actor.bgCheckFlags & BGCHECKFLAG_WALL))) {
            self->collider.base.atFlags &= ~AT_HIT;
            Actor_PlaySfx(&self->actor, NA_SE_EN_MB_MOTH_DEAD);
            Actor_Kill(&self->actor);
        }
        return;
    }

    // Pikmin ball: this bug's spot is BEHIND Link (facing reversed), fanned + ring-depthed by a stable
    // per-bug hash of its pointer, and boids-separated so the ball has volume. Like Gyorg's fish.
    Player* player = GET_PLAYER(play);
    Vec3f anchor = self->actor.world.pos;
    s32 linkMoving = (player != NULL) && (player->speedXZ > BUG_LINK_MOVE);
    if (player != NULL) {
        s16 behindYaw = player->actor.shape.rot.y + 0x8000;
        s16 fan = (s16)((((s32)((uintptr_t)self >> 5) & 3) - 1) * 0x1800);
        f32 ringDist = BUG_BALL_DIST + (f32)(((uintptr_t)self >> 7) & 1) * BUG_BALL_SPACING;
        anchor = player->actor.world.pos;
        anchor.x += Math_SinS(behindYaw + fan) * ringDist;
        anchor.z += Math_CosS(behindYaw + fan) * ringDist;
        RemainsAllyBug_Separate(self, play);
    }

    // Peel off to swarm ONLY while Link holds still, converging on the enemy nearest to LINK (so they
    // gang up "en banco"); while he moves they re-form the ball and keep up.
    target = (linkMoving || (player == NULL))
                 ? NULL
                 : RemainsAlly_FindNearestEnemy(play, &player->actor.world.pos, BUG_SEARCH_RANGE);

    if (target != NULL) {
        // Hop cadence: inject upward velocity only when grounded so it reads as hopping. Done BEFORE
        // the move so Actor_MoveWithGravity (inside HomeTowardPos) applies this frame's hop.
        if ((self->hopTimer <= 0) && (self->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
            self->actor.velocity.y = BUG_HOP_STRENGTH;
            self->hopTimer = BUG_HOP_INTERVAL;
            Actor_PlaySfx_Flagged(&self->actor, NA_SE_EN_MB_MOTH_FLY - SFX_FLAG);
        }

        // Steer yaw toward the enemy and advance (Actor_MoveWithGravity keeps the hop's vy).
        RemainsAlly_HomeTowardPos(play, &self->actor, &target->world.pos, BUG_CHASE_SPEED);

        // Attack is live: position + register the AT collider AT THE NEW spot every frame so any
        // contact lands the AT_TYPE_PLAYER hit on the enemy (never on Link). Order matters — update
        // AFTER the move, otherwise the toucher lags a frame behind the body.
        Collider_UpdateCylinder(&self->actor, &self->collider);
        CollisionCheck_SetAT(play, &play->colChkCtx, &self->collider.base);

        // Kamikaze: die once the hit actually CONNECTED (AT_HIT is set by the previous frame's
        // collision pass → the enemy already took damage), or as a fallback once we are essentially
        // on top of it (covers enemies immune to the deku-stick damage type, so the bug doesn't
        // hover forever). Clear AT_HIT first so a re-used collider slot can't false-trigger.
        s32 hit = (self->collider.base.atFlags & AT_HIT) != 0;
        if (hit) {
            self->collider.base.atFlags &= ~AT_HIT;
        }
        if (hit || (Actor_WorldDistXZToActor(&self->actor, target) < BUG_HIT_DIST)) {
            Actor_PlaySfx(&self->actor, NA_SE_EN_MB_MOTH_DEAD);
            Actor_Kill(&self->actor);
            return;
        }
    } else if (player != NULL) {
        // Form up: hop toward this bug's spot in the ball behind Link. Sprint if far (keep up with him),
        // ease when near, hold when there — same speed ramp the fish school uses.
        if ((self->hopTimer <= 0) && (self->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
            self->actor.velocity.y = BUG_HOP_STRENGTH;
            self->hopTimer = BUG_HOP_INTERVAL;
        }

        f32 distToAnchor = Math_Vec3f_DistXZ(&self->actor.world.pos, &anchor);
        f32 sp = (distToAnchor > BUG_FOLLOW_DIST) ? BUG_CHASE_SPEED : (distToAnchor > 10.0f) ? BUG_IDLE_SPEED : 0.0f;
        RemainsAlly_HomeTowardPos(play, &self->actor, &anchor, sp);
    }
}

// ============================================================================
// DRAW — camera-facing billboard of Odolwa's moth sprite (ovl_En_Tanron1 DLs by OTR path).
// ============================================================================

static void RemainsAllyBug_Draw(Actor* thisx, PlayState* play) {
    RemainsAllyBug* self = (RemainsAllyBug*)thisx;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    // Defensive scene-cull segment bind (Odolwa's DLs can branch into 0x0C).
    gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)sBugSegment0xC_Noop);

    if (self->spawnParams == BUG_MODE_THUNDER) {
        // Goht thunder — the ACTUAL Goht lightning bolt (gGohtLightningModelDL), rendered exactly like
        // BossHakugin_DrawLightningSegments: env = Goht's cyan (sLightningColor 0,255,255), prim = white,
        // and each segment drawn TWICE (second copy rotated 0x4000 on Z) for the cross-shaped bolt volume.
        // A short jagged chain trails the projectile head so it reads as a real forked bolt, not a sprite.
        const s32 kBoltSegments = 5;
        const f32 kSegSpacing = 70.0f;
        s16 yaw = self->actor.shape.rot.y;
        f32 fwdX = Math_SinS(yaw);
        f32 fwdZ = Math_CosS(yaw);
        u8 alpha = (play->gameplayFrames & 1) ? 255 : 160; // rapid lightning flicker

        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gDPSetEnvColor(POLY_XLU_DISP++, 0, 255, 255, 0); // sLightningColor
        gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGohtLightningMaterialDL);

        for (s32 s = 0; s < kBoltSegments; s++) {
            Vec3s rot;
            // Fixed per-segment zig-zag so the chain looks jagged (like the random offsets Goht bakes in).
            rot.x = (s16)(((s & 1) ? -0x0500 : 0x0500));
            rot.y = (s16)(yaw + ((s & 1) ? 0x0900 : -0x0900));
            rot.z = 0;
            Vec3f p;
            p.x = self->actor.world.pos.x - (fwdX * kSegSpacing * s);
            p.y = self->actor.world.pos.y;
            p.z = self->actor.world.pos.z - (fwdZ * kSegSpacing * s);

            Matrix_SetTranslateRotateYXZ(p.x, p.y, p.z, &rot);
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, alpha);
            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGohtLightningModelDL);

            Matrix_RotateZS(0x4000, MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
            gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGohtLightningModelDL);
        }
    } else if ((self->spawnParams == BUG_MODE_PROJECTILE) || (self->spawnParams == BUG_MODE_CLOUD)) {
        // Sword-beam / carrying-cloud = the En_Tanron1 MOTH sprite as a camera-facing billboard, wing-flap.
        f32 flap = 0.75f + (0.30f * Math_SinS(self->flapTimer));
        gSPDisplayList(POLY_OPA_DISP++, (Gfx*)sMothSetupDLPath);
        Matrix_Translate(self->actor.world.pos.x, self->actor.world.pos.y + BUG_DRAW_Y_LIFT, self->actor.world.pos.z,
                         MTXMODE_NEW);
        Matrix_Mult(&play->billboardMtxF, MTXMODE_APPLY);
        Matrix_Scale(BUG_DRAW_SCALE * flap, BUG_DRAW_SCALE, BUG_DRAW_SCALE, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, (Gfx*)sMothModelDLPath);
    } else {
        // Ground = Odolwa's real BEETLE — its FLEX skeleton (gOdolwaBugSkel limbs) at the actor transform.
        SkelAnime_DrawFlexOpa(play, self->skelAnime.skeleton, self->skelAnime.jointTable, self->skelAnime.dListCount,
                              NULL, NULL, NULL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

// ============================================================================
// ACTOR PROFILE + SPAWN
// ============================================================================

// GAMEPLAY_KEEP object so Actor_Spawn never returns NULL for a missing scene object; the moth
// DLs are loaded standalone by OTR path in Draw, not from the segment-6 object bank.
// ACTORCAT_MISC (NOT ACTORCAT_ENEMY) so it never pollutes enemy-count / room-clear / BGM, and
// so it can't be found by RemainsAlly_FindNearestEnemy (which scans ENEMY + BOSS only).
// Culling disabled so the summon keeps updating/drawing off-screen; NO ACTOR_FLAG_HOSTILE.
ActorProfile RemainsAllyBug_Profile = {
    /**/ ACTOR_REMAINS_ALLY_BUG,
    /**/ ACTORCAT_MISC,
    /**/ (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED),
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(RemainsAllyBug),
    /**/ RemainsAllyBug_Init,
    /**/ RemainsAllyBug_Destroy,
    /**/ RemainsAllyBug_Update,
    /**/ RemainsAllyBug_Draw,
};

// One bug per call — Odolwa's remains action calls this a few times to make a swarm.
// rotY seeds the initial facing; params is 0 (reserved sub-mode, read in Init).
Actor* RemainsAllyBug_Spawn(PlayState* play, Vec3f* pos, s16 rotY) {
    if ((play == NULL) || (pos == NULL)) {
        return NULL;
    }

    return Actor_Spawn(&play->actorCtx, play, ACTOR_REMAINS_ALLY_BUG, pos->x, pos->y, pos->z, 0, rotY, 0, 0);
}

// Nimbus cloud moth: hovers under Link (rotY seeds its ring angle). Lives while the flight is active.
Actor* RemainsAllyBug_SpawnCloud(PlayState* play, Vec3f* pos, s16 rotY) {
    if ((play == NULL) || (pos == NULL)) {
        return NULL;
    }
    // Spread the ring: offset each moth's seed angle so 5 of them fan out around the circle.
    s16 spread = rotY + (s16)Rand_CenteredFloat(65535.0f);
    return Actor_Spawn(&play->actorCtx, play, ACTOR_REMAINS_ALLY_BUG, pos->x, pos->y, pos->z, 0, spread, 0,
                       BUG_MODE_CLOUD);
}

// FD-beam moth: spawn one in projectile mode (params = BUG_MODE_PROJECTILE) flying toward rotY.
Actor* RemainsAllyBug_SpawnProjectile(PlayState* play, Vec3f* pos, s16 rotY) {
    if ((play == NULL) || (pos == NULL)) {
        return NULL;
    }

    return Actor_Spawn(&play->actorCtx, play, ACTOR_REMAINS_ALLY_BUG, pos->x, pos->y, pos->z, 0, rotY, 0,
                       BUG_MODE_PROJECTILE);
}

// Goht thunder bolt: flies toward rotY, pierces walls, hits harder (params = BUG_MODE_THUNDER).
Actor* RemainsAllyBug_SpawnThunder(PlayState* play, Vec3f* pos, s16 rotY) {
    if ((play == NULL) || (pos == NULL)) {
        return NULL;
    }

    return Actor_Spawn(&play->actorCtx, play, ACTOR_REMAINS_ALLY_BUG, pos->x, pos->y, pos->z, 0, rotY, 0,
                       BUG_MODE_THUNDER);
}

// Charged Goht thunder: same bolt, but the charge level sets its lifetime (→ how FAR it reaches) and its
// damage. Init already set the thunder defaults; we override them on the freshly-spawned actor.
Actor* RemainsAllyBug_SpawnThunderCharged(PlayState* play, Vec3f* pos, s16 rotY, s16 ttl, s16 damage) {
    if ((play == NULL) || (pos == NULL)) {
        return NULL;
    }
    Actor* a = Actor_Spawn(&play->actorCtx, play, ACTOR_REMAINS_ALLY_BUG, pos->x, pos->y, pos->z, 0, rotY, 0,
                           BUG_MODE_THUNDER);
    if (a != NULL) {
        RemainsAllyBug* self = (RemainsAllyBug*)a;
        self->lifeTimer = ttl; // TTL × speed = reach distance
        self->collider.elem.atDmgInfo.damage = (u8)damage;
    }
    return a;
}
