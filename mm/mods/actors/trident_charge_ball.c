/**
 * trident_charge_ball.c — Implementation. See header for design rationale.
 *
 * Hijack pattern (same as deku_nut_projectile.c / somaria_cubes.c):
 *   - Actor_Spawn(ACTOR_EN_A_OBJ, ...) gives a trivial real actor with the
 *     right lifetime + categorization; we overwrite actor->update/draw before
 *     returning the pointer, so En_A_Obj's own update never runs.
 *   - sTcbPool[] holds per-actor state keyed by the actor pointer — the struct
 *     can't be extended because Actor_Spawn only allocates the host actor size.
 *
 * Two projectile kinds share the pool:
 *   BALL   — the charged energy ball. Carries the super-damage claim.
 *   HUNTER — the small seekers spawned at the impact point. Ordinary damage.
 *
 * No object is ever loaded for these. Every display list they draw is pulled out
 * of oot.o2r by OTR path (ovl_Boss_Ganon for the big-magic ball and for the
 * hunters' lit streak, object_fhg for the light ball), the same way the held lance
 * is — loading an object here would have to evict the player object.
 *
 * NOTE: text-included from extended_equipment.c. All OOT headers are already in
 * scope from the parent TU. Everything is static except the exported accessors.
 *
 * Skijer's NEI
 */

// ---------------------------------------------------------------------------
// Tunables (frames are 20 Hz logic ticks — R_UPDATE_RATE = 3, NOT 60 fps)
// ---------------------------------------------------------------------------
#define TCB_BALL_MAX 2   // simultaneous charge balls (1 in practice)
#define TCB_HUNTER_MAX 6 // boss-room max charge: six simultaneous light trails
#define TCB_AUX_MAX 12   // leaves room for an older volley/light ball while six spawn
#define TCB_SLOT_MAX (TCB_BALL_MAX + TCB_AUX_MAX)
#define TCB_HUNTER_NORMAL_COUNT 4
#define TCB_HUNTER_BOSS_COUNT 6
#define TCB_BOSS_VOLLEY_DAMAGE 12

#define TCB_GRACE 5     // post-impact super-damage grace, see header
#define TCB_LIFETIME 60 // 3 s before the ball fizzles out
#define TCB_SPEED 22.0f
#define TCB_HOMING 0.25f // per-frame lerp of velocity toward the target
#define TCB_RADIUS 30
#define TCB_HEIGHT 44
// The level-2 boss shot is intentionally harder to evade than the ordinary ball:
// it is the Fierce Deity payload, not one of the four post-burst seekers.
#define TCB_MAX_SPEED 30.0f
#define TCB_MAX_HOMING 0.60f
#define TCB_MAX_RADIUS 55
#define TCB_MAX_HEIGHT 90
// Ranges are now only the FALLBACK for enemies the engine is not drawing; anything
// on screen is targetable however far it is (Tcb_Reachable). Kept generous so a
// culled-but-nearby enemy still counts.
#define TCB_SEEK_RANGE 2500.0f
#define TCB_BOSS_ROOM_RANGE 1000000.0f // detect the encounter even while Goht is far/culled

#define TCB_HUNTER_DAMAGE 8
// Reach has to match the targeting. A seeker only travels SPEED * LIFETIME before it
// expires, so with the old 13 x 40 = 520 units it could be handed a target it had no
// way of reaching once anything on screen became fair game. 22 x 80 = 1760 covers any
// enemy actually being drawn in a room.
#define TCB_HUNTER_LIFETIME 80
#define TCB_HUNTER_SPEED 22.0f
#define TCB_HUNTER_BOSS_SPEED (TCB_HUNTER_SPEED * 2.0f)
#define TCB_HUNTER_FAN 10.0f // initial spread, kept slow so they visibly disperse
#define TCB_HUNTER_HOMING 0.35f
#define TCB_HUNTER_BOSS_HOMING 0.70f
#define TCB_HUNTER_RADIUS 14
#define TCB_HUNTER_HEIGHT 22
#define TCB_HUNTER_RANGE 2500.0f
// The seekers ARE Ganondorf's returning big-magic balls. In the fight those are
// ACTOR_BOSS_GANON spawned with params 0x104+i: you hit the thrown ball with a light
// arrow, it turns around (unk_1C2 == 12), flies back at him trailing a lit streak and
// calls BossGanon_SetupHitByLightBall on arrival. That actor can NOT be reused here —
// its update dereferences actor.parent as a live BossGanon on its very first line and
// it needs OBJECT_GANON loaded — so what is borrowed is its DRAW (func_808E324C):
// 15-sample position/heading history, the last 12 drawn through the tapering streak
// DLs, then the light ball billboarded on top. Same DLs, same colours, same 0.01
// actor scale the thrown balls run at.
// The ball's death flash. When Ganondorf's own light ball is destroyed he does not
// simply remove it: the actor switches to BossGanon_LightBall_Update with unk_1A8 set
// (params 0x12C / 0x190), which swells it hard — Math_ApproachF(scale, 20..30, 0.5,
// 100) — while the alpha drops at 10..30 a frame, and kills it the moment the alpha
// reaches zero. Ours does the same in TCB_BURST_FRAMES: big, then small, then gone,
// with the seekers already on their way out of it.
#define TCB_BURST_FRAMES 6
#define TCB_BURST_PEAK 2.6f // times its normal size at the top of the swell
#define TCB_BURST_RISE 0.3f // fraction of the burst spent growing

#define TCB_TRAIL_LEN 15
#define TCB_TRAIL_DRAWN 12
#define TCB_STREAK_SCALE 0.01f

// Light ball: light-arrow class, the Light Rod's projectile damage (item_rod_light.h).
#define TCB_LIGHT_DAMAGE 4
#define TCB_LIGHT_LIFETIME 70
#define TCB_LIGHT_SPEED 18.0f
#define TCB_LIGHT_HOMING 0.20f
#define TCB_LIGHT_RADIUS 22
#define TCB_LIGHT_HEIGHT 30
#define TCB_LIGHT_RANGE 2500.0f
#define TCB_LIGHT_SCALE 6.0f // EnFhgFire_EnergyBall runs at actor scale 5.25-6.0 (z_en_fhg_fire.c:445)

// The charge ball is Ganondorf's BIG MAGIC ball — the one he summons over his head
// during gGanondorfBigMagicChargeHoldAnim, not the little one in his hand. Drawn by
// TridentBigMagic_Draw below, which the held version calls too. These two sizes must
// stay equal to TRI_BM_CIRCLE_MAX / TRI_BM_BALL_MAX in equip_trident.c: the
// projectile IS the charged ball leaving Link, so it has to appear at the size it
// had in his chest.
#define TCB_BALL_CIRCLE_SCALE 0.16f
#define TCB_BALL_DRAW_SCALE 14.0f

extern void* OotAssets_LoadGfxDirect(const char* path);
extern void Gfx_SetupDL_25Xlu(GraphicsContext* gfxCtx);

// FhgFlash effect selectors. The enum lives in an overlay-local header
// (ovl_Effect_Ss_Fhg_Flash/z_eff_ss_fhg_flash.h) that this TU does not pull in,
// so the values are mirrored here with their names.
#define TCB_FX_LIGHTBALL_PURPLE 5 // FHGFLASH_LIGHTBALL_PURPLE
#define TCB_FX_LIGHTBALL_BLUE 4   // FHGFLASH_LIGHTBALL_BLUE
#define TCB_FX_SHOCK_ANY_ACTOR 3  // FHGFLASH_SHOCK_ANY_ACTOR

extern void EffectSsFhgFlash_SpawnLightBall(PlayState* play, Vec3f* pos, Vec3f* velocity, Vec3f* accel, s16 scale,
                                            u8 param);
extern void EffectSsFhgFlash_SpawnShock(PlayState* play, Actor* actor, Vec3f* pos, s16 scale, u8 param);

typedef enum {
    TCB_KIND_BALL = 0,
    TCB_KIND_HUNTER,
    // Phantom Ganon's light ball (flight B). Lives in the HUNTER slot range on
    // purpose: TridentChargeBall_IsActive only reads the BALL slots, so a light
    // ball can never make a boss treat a hit as a super attack.
    TCB_KIND_LIGHT,
} TcbKind;

typedef struct {
    Actor* owner; // NULL = free slot
    Actor* target;
    u8 kind;
    u8 lifetime;
    u8 colliderInited;
    u8 fixedDamage; // != 0: force this exact FD-beam damage on this projectile
    u8 burstDamage; // ball only: break in place into the six-shot boss volley
    u8 burst;       // ball only: frames left of the death flash. > 0 = already spent
    f32 visScale;   // 0..1 charge level; only scales the visual
    Vec3f velocity;
    // Lit streak (hunters only): a ring buffer of where this thing has BEEN, plus the
    // heading it had there. Exactly Ganondorf's arrangement — 15 samples, of which the
    // draw uses the last 12.
    Vec3f trailPos[TCB_TRAIL_LEN];
    Vec3f trailRot[TCB_TRAIL_LEN]; // radians: .x pitch, .y yaw
    s16 trailIdx;
    ColliderCylinder collider;
} TcbSlot;

static TcbSlot sTcbPool[TCB_SLOT_MAX] = { 0 };

// Post-impact grace. See the header for why this exists — without it the boss
// reads BUMP_HIT one frame after the ball has already cleared itself, and the
// super hit is dropped in silence.
static s16 sTcbGraceTimer = 0;

typedef struct {
    Actor* actor;
    u8 damage;
    u8 timer;
} TcbFierceHitGrace;

static TcbFierceHitGrace sTcbFierceHitGrace[TCB_HUNTER_BOSS_COUNT] = { 0 };

static void Tcb_RememberFierceHit(Actor* actor, u8 damage) {
    s32 i;
    s32 oldest = 0;

    if ((actor == NULL) || (damage == 0)) {
        return;
    }
    for (i = 0; i < TCB_HUNTER_BOSS_COUNT; i++) {
        if ((sTcbFierceHitGrace[i].actor == actor) || (sTcbFierceHitGrace[i].timer == 0)) {
            oldest = i;
            break;
        }
        if (sTcbFierceHitGrace[i].timer < sTcbFierceHitGrace[oldest].timer) {
            oldest = i;
        }
    }
    sTcbFierceHitGrace[oldest].actor = actor;
    sTcbFierceHitGrace[oldest].damage = damage;
    sTcbFierceHitGrace[oldest].timer = TCB_GRACE;
}

// ---------------------------------------------------------------------------
// Pool
// ---------------------------------------------------------------------------
static s8 Tcb_GetSlot(Actor* actor) {
    if (actor == NULL) {
        return -1;
    }
    for (s8 i = 0; i < TCB_SLOT_MAX; i++) {
        if (sTcbPool[i].owner == actor) {
            return i;
        }
    }
    return -1;
}

static s8 Tcb_AllocSlot(Actor* actor, u8 kind) {
    // Balls are capped separately from hunters so a burst of seekers can never
    // starve the next charged shot.
    s8 begin = (kind == TCB_KIND_BALL) ? 0 : TCB_BALL_MAX;
    s8 end = (kind == TCB_KIND_BALL) ? TCB_BALL_MAX : TCB_SLOT_MAX;

    for (s8 i = begin; i < end; i++) {
        if (sTcbPool[i].owner == NULL) {
            sTcbPool[i].owner = actor;
            sTcbPool[i].target = NULL;
            sTcbPool[i].kind = kind;
            sTcbPool[i].colliderInited = 0;
            sTcbPool[i].fixedDamage = 0;
            sTcbPool[i].burstDamage = 0;
            sTcbPool[i].burst = 0;
            sTcbPool[i].visScale = 1.0f;
            sTcbPool[i].lifetime = (kind == TCB_KIND_BALL)    ? TCB_LIFETIME
                                   : (kind == TCB_KIND_LIGHT) ? TCB_LIGHT_LIFETIME
                                                              : TCB_HUNTER_LIFETIME;
            sTcbPool[i].velocity.x = sTcbPool[i].velocity.y = sTcbPool[i].velocity.z = 0.0f;
            sTcbPool[i].trailIdx = 0;
            return i;
        }
    }
    return -1;
}

static void Tcb_FreeSlot(s8 slot) {
    if (slot < 0 || slot >= TCB_SLOT_MAX) {
        return;
    }
    // Same reasoning as deku_nut_projectile.c: the actor is already being killed,
    // and the slot re-initializes its collider on the next alloc.
    sTcbPool[slot].colliderInited = 0;
    sTcbPool[slot].fixedDamage = 0;
    sTcbPool[slot].burstDamage = 0;
    sTcbPool[slot].owner = NULL;
    sTcbPool[slot].target = NULL;
}

// ---------------------------------------------------------------------------
// Colliders
//
// MM has one native sword damage class. The collider's damage byte carries the
// Trident-specific strength while DMG_SWORD keeps normal enemy vulnerability rules.
// ---------------------------------------------------------------------------
static ColliderCylinderInit sTcbBallColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK2,
        { DMG_SWORD, 0x00, 0x01 },
        { 0xFFCFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_NEAREST | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { TCB_RADIUS, TCB_HEIGHT, 0, { 0, 0, 0 } },
};

static ColliderCylinderInit sTcbHunterColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK2,
        { DMG_SWORD, 0x00, TCB_HUNTER_DAMAGE },
        { 0xFFCFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_NEAREST | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { TCB_HUNTER_RADIUS, TCB_HUNTER_HEIGHT, 0, { 0, 0, 0 } },
};

// Light ball: DMG_LIGHT_ARROW so every enemy's own table resolves it as a light
// arrow (that is what makes it the counter it is against Ganon-class foes), with
// the Light Rod's fixed 4 on top so it never rounds down to nothing.
static ColliderCylinderInit sTcbLightColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK2,
        { DMG_LIGHT_ARROW, 0x01, TCB_LIGHT_DAMAGE },
        { 0xFFCFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_NEAREST | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { TCB_LIGHT_RADIUS, TCB_LIGHT_HEIGHT, 0, { 0, 0, 0 } },
};

// ---------------------------------------------------------------------------
// Targeting
// ---------------------------------------------------------------------------
static u8 Tcb_TargetIsUsable(Actor* target) {
    return (target != NULL) && (target->update != NULL) && (target->colChkInfo.health > 0);
}

// Is this enemy worth chasing from `origin`?
//
// ⚠️ An enemy that is BEING DRAWN is fair game at ANY distance ("que siempre le de a
// un enemigo si anda renderizado en scene"). Actor.isDrawn is set by the engine on
// every actor it actually rendered last frame, so it is exactly "on screen / inside
// its cull volume" and nothing else — no distance heuristic can say that. Anything
// not drawn still has to be close, which keeps a seeker from flying off to something
// asleep in another room.
// Is this the boss ITSELF, or one of its furniture?
//
// A multi-actor boss puts every piece of itself in ACTORCAT_BOSS under the same
// actor id, told apart only by params — so a plain "nearest boss" scan happily locks
// onto a tentacle, a stump or a door and the shot is wasted on scenery.
//
// MM split bosses commonly hang vulnerable pieces from another boss-category actor;
// follow that chain so an impact on a rendered weak point can also reach its root.
static Actor* Tcb_GetBossRoot(Actor* actor) {
    Actor* root = actor;

    while ((root != NULL) && (root->parent != NULL) && (root->parent->category == ACTORCAT_BOSS) &&
           (root->parent->update != NULL)) {
        root = root->parent;
    }
    return root;
}

// Return the owning boss for either the boss actor itself or an MM weak-point
// actor parented to it. Keeping the original target lets homing use the precise
// weak-point focus while damage still reaches the encounter owner.
static Actor* Tcb_ResolveBoss(Actor* actor) {
    Actor* root = Tcb_GetBossRoot(actor);

    return ((root != NULL) && (root->category == ACTORCAT_BOSS)) ? root : NULL;
}

static u8 Tcb_Reachable(Actor* actor, Vec3f* origin, f32 range) {
    if (actor->isDrawn) {
        return 1;
    }
    return (Math_Vec3f_DistXYZ(origin, &actor->world.pos) <= range) ? 1 : 0;
}

static Actor* Tcb_NearestUntaken(PlayState* play, Vec3f* origin, Actor** taken, s32 nTaken, f32 range);
static Actor* Tcb_NearestBoss(PlayState* play, Vec3f* origin, f32 range);

// Lock-on first (the player pointed at it deliberately), otherwise the nearest
// reachable boss, otherwise the nearest reachable enemy — same rule the seekers use.
static Actor* Tcb_AcquireTarget(PlayState* play, Actor* from, f32 range) {
    Player* player = GET_PLAYER(play);

    // The lock-on wins, including MM boss weak-point actors. On a max-ball impact
    // Tcb_GetBossRoot also marks the owning boss root as defeated.
    if ((player != NULL) && Tcb_TargetIsUsable(player->focusActor)) {
        return player->focusActor;
    }
    return Tcb_NearestUntaken(play, &from->world.pos, NULL, 0, range);
}

// Steer `velocity` toward the target. Plain vector math on purpose: no engine
// helper is involved, so there is nothing here that can disagree with how the
// actor's position is integrated below.
static void Tcb_Home(TcbSlot* slot, Actor* self, f32 speed, f32 lerp) {
    Vec3f to;
    Vec3f aim;
    f32 len;

    if (!Tcb_TargetIsUsable(slot->target)) {
        return;
    }

    // Actor.focus is the engine's intended lock-on / vulnerable point in WORLD
    // coordinates. Averaging it with world.pos pulled shots below many tall bosses.
    aim = slot->target->focus.pos;
    if (Math_Vec3f_DistXYZ(&aim, &slot->target->world.pos) > 2500.0f) {
        // Defensive fallback for custom actors that leave focus uninitialised.
        aim = slot->target->world.pos;
    }
    to.x = aim.x - self->world.pos.x;
    to.y = aim.y - self->world.pos.y;
    to.z = aim.z - self->world.pos.z;

    len = sqrtf(to.x * to.x + to.y * to.y + to.z * to.z);
    if (len < 1.0f) {
        return;
    }

    to.x = to.x / len * speed;
    to.y = to.y / len * speed;
    to.z = to.z / len * speed;

    slot->velocity.x += (to.x - slot->velocity.x) * lerp;
    slot->velocity.y += (to.y - slot->velocity.y) * lerp;
    slot->velocity.z += (to.z - slot->velocity.z) * lerp;
}

// ---------------------------------------------------------------------------
// Impact
// ---------------------------------------------------------------------------
static void Tcb_SpawnHunters(PlayState* play, Vec3f* origin, u8 fierceDamage);

static void Tcb_BallImpact(PlayState* play, Actor* self, u8 fierceDamage) {
    Vec3f pos = self->world.pos;
    Vec3f zero = { 0.0f, 0.0f, 0.0f };
    Vec3f vel;
    s32 i;

    // Open the grace window BEFORE anything else can kill the actor: the boss
    // reads BUMP_HIT next frame and must still see this projectile as active.
    sTcbGraceTimer = TCB_GRACE;

    // Ganon's own impact signature: a shock burst plus a spray of light balls.
    EffectSsFhgFlash_SpawnShock(play, NULL, &pos, 200, 0);
    for (i = 0; i < 8; i++) {
        vel.x = Rand_CenteredFloat(12.0f);
        vel.y = Rand_ZeroFloat(8.0f) + 2.0f;
        vel.z = Rand_CenteredFloat(12.0f);
        EffectSsFhgFlash_SpawnLightBall(play, &pos, &vel, &zero, (s16)(Rand_ZeroOne() * 80.0f) + 150,
                                        TCB_FX_LIGHTBALL_PURPLE);
    }

    Tcb_SpawnHunters(play, &pos, fierceDamage);
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------
static void Tcb_Update(Actor* thisx, PlayState* play) {
    s8 slot = Tcb_GetSlot(thisx);
    TcbSlot* p;
    Vec3f zero = { 0.0f, 0.0f, 0.0f };
    u8 isBall;
    u8 isLight;

    if (slot < 0) {
        Actor_Kill(thisx);
        return;
    }

    p = &sTcbPool[slot];
    isBall = (p->kind == TCB_KIND_BALL);
    isLight = (p->kind == TCB_KIND_LIGHT);

    // The death flash owns the ball's last frames: no homing, no collider, no
    // lifetime — just the swell, which Tcb_Draw reads straight off this counter.
    if (p->burst > 0) {
        p->burst--;
        if (p->burst == 0) {
            Tcb_FreeSlot(slot);
            Actor_Kill(thisx);
        }
        return;
    }

    // In a boss room the held giant sphere is never launched. At the end of the
    // max-charge animation it breaks exactly where it was held and becomes the
    // six-projectile FD volley before any movement or giant collider can occur.
    if (isBall && (p->burstDamage != 0)) {
        u8 fierceDamage = p->burstDamage;
        p->burstDamage = 0;
        Tcb_BallImpact(play, thisx, fierceDamage);
        p->burst = TCB_BURST_FRAMES;
        return;
    }

    // Lazy collider init — Collider_SetCylinder needs the actor to be live in
    // the actor list, which it is not yet inside Actor_Spawn.
    if (!p->colliderInited) {
        Collider_InitCylinder(play, &p->collider);
        Collider_SetCylinder(play, &p->collider, thisx,
                             isBall    ? &sTcbBallColliderInit
                             : isLight ? &sTcbLightColliderInit
                                       : &sTcbHunterColliderInit);
        // Max charge is a genuine Fierce Deity sword beam. The explicit health-zero
        // path below remains necessary because MM bosses commonly cap one hit.
        if (p->fixedDamage != 0) {
            p->collider.elem.atDmgInfo.dmgFlags = DMG_SWORD_BEAM;
            p->collider.elem.atDmgInfo.damage = p->fixedDamage;
            if (isBall) {
                p->collider.dim.radius = TCB_MAX_RADIUS;
                p->collider.dim.height = TCB_MAX_HEIGHT;
                p->collider.dim.yShift = -(TCB_MAX_HEIGHT / 2);
            }
        }
        p->colliderInited = 1;
    }

    // Re-acquire if the target died mid-flight, so a seeker does not fly off to
    // where a corpse used to be.
    if (!Tcb_TargetIsUsable(p->target)) {
        p->target = (isBall && (p->fixedDamage != 0)) ? Tcb_NearestBoss(play, &thisx->world.pos, TCB_SEEK_RANGE)
                                                      : Tcb_AcquireTarget(play, thisx,
                                                                          isBall    ? TCB_SEEK_RANGE
                                                                          : isLight ? TCB_LIGHT_RANGE
                                                                                    : TCB_HUNTER_RANGE);
    }

    Tcb_Home(p, thisx,
             (isBall && (p->fixedDamage != 0))                ? TCB_MAX_SPEED
             : (!isBall && !isLight && (p->fixedDamage != 0)) ? TCB_HUNTER_BOSS_SPEED
             : isBall                                         ? TCB_SPEED
             : isLight                                        ? TCB_LIGHT_SPEED
                                                              : TCB_HUNTER_SPEED,
             (isBall && (p->fixedDamage != 0))                ? TCB_MAX_HOMING
             : (!isBall && !isLight && (p->fixedDamage != 0)) ? TCB_HUNTER_BOSS_HOMING
             : isBall                                         ? TCB_HOMING
             : isLight                                        ? TCB_LIGHT_HOMING
                                                              : TCB_HUNTER_HOMING);

    thisx->world.pos.x += p->velocity.x;
    thisx->world.pos.y += p->velocity.y;
    thisx->world.pos.z += p->velocity.z;

    // Visual. The charge ball is Ganondorf's yellow-green light ball (Tcb_Draw) with
    // a sparse purple wake; the light ball is Phantom Ganon's own DL with a faint blue
    // trail; the SEEKERS carry his lit streak, recorded here and drawn in Tcb_Draw.
    if (isBall) {
        thisx->shape.rot.z += 0x0C00;
        if ((p->lifetime & 3) == 0) {
            Vec3f fx = thisx->world.pos;
            EffectSsFhgFlash_SpawnLightBall(play, &fx, &zero, &zero, 120, TCB_FX_LIGHTBALL_PURPLE);
        }
    } else if (isLight) {
        Actor_SetScale(thisx, TCB_LIGHT_SCALE);
        thisx->shape.rot.z += 0x1000; // the spin EnFhgFire gives its ball
        if ((p->lifetime & 3) == 0) {
            Vec3f fx = thisx->world.pos;
            EffectSsFhgFlash_SpawnLightBall(play, &fx, &zero, &zero, 120, TCB_FX_LIGHTBALL_BLUE);
        }
    } else {
        // Sample AFTER the move, like func_808E2544 does: position plus the heading it
        // is travelling on, which is what orients each streak segment.
        f32 xz = sqrtf((p->velocity.x * p->velocity.x) + (p->velocity.z * p->velocity.z));
        p->trailIdx++;
        if (p->trailIdx >= TCB_TRAIL_LEN) {
            p->trailIdx = 0;
        }
        p->trailPos[p->trailIdx] = thisx->world.pos;
        p->trailRot[p->trailIdx].y = atan2f(p->velocity.x, p->velocity.z);
        p->trailRot[p->trailIdx].x = atan2f(p->velocity.y, xz);
        p->trailRot[p->trailIdx].z = 0.0f;
        thisx->shape.rot.z += 0x1000; // spins the head, like his
        Actor_SetScale(thisx, TCB_STREAK_SCALE);
    }

    Collider_UpdateCylinder(thisx, &p->collider);
    CollisionCheck_SetAT(play, &play->colChkCtx, &p->collider.base);

    if (p->collider.base.atFlags & AT_HIT) {
        Actor* hit = p->collider.base.at; // the actor OUR toucher landed on
        p->collider.base.atFlags &= ~AT_HIT;
        if (isBall) {
            // Any explicitly FD-tagged ball ends a boss outright. Boss-room max charge
            // now breaks before reaching this path; it transfers the tag to its hunters.
            //
            // Health is zeroed rather than damage being piled on: bosses resolve a hit
            // through their OWN damage table, and most of them either cap what a single
            // hit can take or ignore the number entirely. The collider hit still lands,
            // so the boss enters its normal damaged state and finds itself already at
            // zero — its own death sequence plays instead of the actor being deleted.
            // Our update runs before ACTORCAT_BOSS does, so it reads the zero this frame.
            if ((p->fixedDamage != 0) && (hit != NULL) && (Tcb_ResolveBoss(hit) != NULL)) {
                Actor* root = Tcb_ResolveBoss(hit);
                hit->colChkInfo.health = 0;
                root->colChkInfo.health = 0;
            }
            Tcb_BallImpact(play, thisx, 0);
            p->burst = TCB_BURST_FRAMES; // flash, then die — the seekers are already out
            return;
        }
        if ((p->fixedDamage != 0) && (hit != NULL) && (Tcb_ResolveBoss(hit) != NULL)) {
            Actor* root = Tcb_ResolveBoss(hit);
            Tcb_RememberFierceHit(thisx, p->fixedDamage);
            hit->colChkInfo.health = 0;
            root->colChkInfo.health = 0;
            sTcbGraceTimer = TCB_GRACE;
        }
        Tcb_FreeSlot(slot);
        Actor_Kill(thisx);
        return;
    }

    if (p->lifetime == 0) {
        // A ball that expires without hitting still bursts, so a missed shot
        // reads as a miss rather than as the projectile blinking out.
        if (isBall) {
            Tcb_BallImpact(play, thisx, 0);
            p->burst = TCB_BURST_FRAMES;
            return;
        }
        Tcb_FreeSlot(slot);
        Actor_Kill(thisx);
        return;
    }
    p->lifetime--;
}

// ---------------------------------------------------------------------------
// Ganondorf's BIG MAGIC ball — the one he summons over his head and holds through
// gGanondorfBigMagicChargeHoldAnim (BossGanon_ChargeBigMagic case 2, which drives
// BossGanon_DrawBigMagicCharge — z_boss_ganon.c:3572). Five layers, all off
// ovl_Boss_Ganon: light flecks, magenta background circle, yellow dot, the
// yellow-green light ball itself, and a fan of light rays.
//
// Shared on purpose. The ball Link charges over his head (Trident_Draw) and the
// projectile it turns into (Tcb_Draw, TCB_KIND_BALL) both come through here, so
// the release reads as THAT ball leaving him rather than as a different effect
// spawning in its place.
//
// Two departures from his: the ray angles are fixed per index instead of re-rolled
// every frame from the seed he keeps in unk_1AA (we have no equivalent, and
// re-rolling without it flickers), and the sizes are head-sized rather than
// arena-sized — his targets are circle 0.4 / ball 45.
// Skijer's NEI
// ---------------------------------------------------------------------------
#define TBM_MAT_DL "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightBallMaterialDL"
#define TBM_BALL_DL "__OTR__overlays/ovl_Boss_Ganon/gGanondorfSquareDL"
#define TBM_FLECKS_DL "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightFlecksDL"
#define TBM_CIRCLE_DL "__OTR__overlays/ovl_Boss_Ganon/gGanondorfBigMagicBGCircleDL"
#define TBM_DOT_DL "__OTR__overlays/ovl_Boss_Ganon/gGanondorfDotDL"
#define TBM_RAY_DL "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightRayTriDL"
#define TBM_RAYS_MAX 6

static void TridentBigMagic_Draw(PlayState* play, Vec3f* pos, f32 circleScale, f32 ballScale, f32 alpha, s32 rays,
                                 f32 spinRad) {
    static Gfx* sMat = NULL;
    static Gfx* sBall = NULL;
    static Gfx* sFlecks = NULL;
    static Gfx* sCircle = NULL;
    static Gfx* sDot = NULL;
    static Gfx* sRay = NULL;
    static u8 sTried = 0;
    GraphicsContext* gfxCtx;
    u32 frame;
    u8 a;
    s32 i;

    if ((play == NULL) || (pos == NULL) || (circleScale <= 0.001f)) {
        return;
    }
    if (!sTried) {
        sTried = 1;
        sMat = (Gfx*)OotAssets_LoadGfxDirect(TBM_MAT_DL);
        sBall = (Gfx*)OotAssets_LoadGfxDirect(TBM_BALL_DL);
        sFlecks = (Gfx*)OotAssets_LoadGfxDirect(TBM_FLECKS_DL);
        sCircle = (Gfx*)OotAssets_LoadGfxDirect(TBM_CIRCLE_DL);
        sDot = (Gfx*)OotAssets_LoadGfxDirect(TBM_DOT_DL);
        sRay = (Gfx*)OotAssets_LoadGfxDirect(TBM_RAY_DL);
    }
    if ((sMat == NULL) || (sBall == NULL) || (sFlecks == NULL) || (sCircle == NULL) || (sDot == NULL) ||
        (sRay == NULL)) {
        return;
    }
    gfxCtx = play->state.gfxCtx;
    frame = play->gameplayFrames;
    a = (alpha <= 0.0f) ? 0 : ((alpha >= 255.0f) ? 255 : (u8)alpha);

    OPEN_DISPS(gfxCtx);
    Gfx_SetupDL_25Xlu(gfxCtx);

    // light flecks
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 170, a);
    gDPSetEnvColor(POLY_XLU_DISP++, 200, 255, 0, 128);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(gfxCtx, 0, frame * -2, 0, 0x40, 0x40, 1, 0, frame * 0xA, 0x40, 0x40, -2, 0, 0, 0xA));
    Matrix_Translate(pos->x, pos->y, pos->z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_Scale(circleScale, circleScale, circleScale, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(gfxCtx, (char*)__FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, sFlecks);

    // background circle
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 0, 100, a);
    gSPSegment(POLY_XLU_DISP++, 0x09,
               Gfx_TwoTexScrollEx(gfxCtx, 0, 0, 0, 0x20, 0x20, 1, 0, frame * -4, 0x20, 0x20, 0, 0, 0, -4));
    gSPDisplayList(POLY_XLU_DISP++, sCircle);

    // yellow dot
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 150, 170, 0, a);
    gSPSegment(
        POLY_XLU_DISP++, 0x0A,
        Gfx_TwoTexScrollEx(gfxCtx, 0, 0, 0, 0x20, 0x20, 1, frame * 2, frame * -0x14, 0x40, 0x40, 0, 0, 2, -0x14));
    gSPDisplayList(POLY_XLU_DISP++, sDot);

    // the light ball itself
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 100, 0);
    gSPDisplayList(POLY_XLU_DISP++, sMat);
    Matrix_Translate(pos->x, pos->y, pos->z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_Scale(ballScale, ballScale, ballScale, MTXMODE_APPLY);
    Matrix_RotateZ(spinRad, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(gfxCtx, (char*)__FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, sBall);

    // the ray fan
    if (rays > 0) {
        if (rays > TBM_RAYS_MAX) {
            rays = TBM_RAYS_MAX;
        }
        Matrix_Translate(pos->x, pos->y, pos->z, MTXMODE_NEW);
        Matrix_RotateY((frame * 10.0f) / 1000.0f, MTXMODE_APPLY);
        gDPSetEnvColor(POLY_XLU_DISP++, 200, 255, 0, 0);
        for (i = 0; i < rays; i++) {
            f32 ang = (f32)i * (M_PI * 2.0f / (f32)TBM_RAYS_MAX);
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 200);
            Matrix_Push();
            Matrix_RotateY(ang, MTXMODE_APPLY);
            Matrix_RotateX(0.6f * ((i & 1) ? 1.0f : -1.0f), MTXMODE_APPLY);
            Matrix_RotateZ(ang * 0.5f, MTXMODE_APPLY);
            Matrix_Translate(0.0f, 0.0f, ballScale * 1.6f, MTXMODE_APPLY);
            Matrix_Scale(ballScale * 0.115f, ballScale * 0.115f, ballScale * 0.032f, MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(gfxCtx, (char*)__FILE__, __LINE__),
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, sRay);
            Matrix_Pop();
        }
    }

    CLOSE_DISPS(gfxCtx);
}

// ---------------------------------------------------------------------------
// The seeker's lit streak — func_808E324C (z_boss_ganon.c:4699), the draw of the
// big-magic balls that fly back at Ganondorf after a light arrow turns them around.
// Twelve tapering quads laid along the last twelve samples of the position history,
// each oriented to the heading it was travelling on, then the light ball itself
// billboarded on the head. Segment 0x0D carries the twelve matrices, which is what
// the streak display lists index.
// ---------------------------------------------------------------------------
static const char* sTcbStreakDL[TCB_TRAIL_DRAWN] = {
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak12DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak11DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak10DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak9DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak8DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak7DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak6DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak5DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak4DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak3DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak2DL",
    "__OTR__overlays/ovl_Boss_Ganon/gGanondorfLightStreak1DL",
};

static void Tcb_DrawStreak(TcbSlot* p, Actor* thisx, PlayState* play) {
    static Gfx* sStreakGfx[TCB_TRAIL_DRAWN] = { 0 };
    static Gfx* sHeadMat = NULL;
    static Gfx* sHeadBall = NULL;
    static u8 sTried = 0;
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    // MM has no Graph_Alloc function; its frame allocator is the GRAPH_ALLOC
    // macro over GraphicsContext::polyOpa.d.
    Mtx* mtx = GRAPH_ALLOC(gfxCtx, TCB_TRAIL_DRAWN * sizeof(Mtx));
    u8 alpha;
    s32 i;

    if (!sTried) {
        sTried = 1;
        for (i = 0; i < TCB_TRAIL_DRAWN; i++) {
            sStreakGfx[i] = (Gfx*)OotAssets_LoadGfxDirect(sTcbStreakDL[i]);
        }
        sHeadMat = (Gfx*)OotAssets_LoadGfxDirect(TBM_MAT_DL);
        sHeadBall = (Gfx*)OotAssets_LoadGfxDirect(TBM_BALL_DL);
    }
    if ((mtx == NULL) || (sHeadMat == NULL) || (sHeadBall == NULL)) {
        return;
    }
    for (i = 0; i < TCB_TRAIL_DRAWN; i++) {
        if (sStreakGfx[i] == NULL) {
            return;
        }
    }
    // Fade out over the last handful of frames so a seeker that simply runs out of
    // time does not blink; a seeker that HITS is killed and never reaches this.
    alpha = (p->lifetime >= 8) ? 255 : (u8)((p->lifetime * 255) / 8);

    OPEN_DISPS(gfxCtx);
    Gfx_SetupDL_25Xlu(gfxCtx);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0x80, 255, 255, 255, alpha);
    gDPSetEnvColor(POLY_XLU_DISP++, 150, 255, 0, 128);
    gSPSegment(POLY_XLU_DISP++, 0x0D, mtx);

    for (i = 0; i < TCB_TRAIL_DRAWN; i++) {
        s32 t = ((p->trailIdx - i) + TCB_TRAIL_LEN) % TCB_TRAIL_LEN;
        Matrix_Translate(p->trailPos[t].x, p->trailPos[t].y, p->trailPos[t].z, MTXMODE_NEW);
        Matrix_RotateY(p->trailRot[t].y, MTXMODE_APPLY);
        Matrix_RotateX(-p->trailRot[t].x, MTXMODE_APPLY);
        Matrix_Scale(TCB_STREAK_SCALE, TCB_STREAK_SCALE, TCB_STREAK_SCALE, MTXMODE_APPLY);
        Matrix_RotateY(M_PI / 2.0f, MTXMODE_APPLY);
        MATRIX_TOMTX(mtx);
        gSPMatrix(POLY_XLU_DISP++, mtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(POLY_XLU_DISP++, sStreakGfx[i]);
        mtx++;
    }

    // The head. Ganondorf's is a flat 10; the seekers are smaller than his.
    Matrix_Translate(thisx->world.pos.x, thisx->world.pos.y, thisx->world.pos.z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_Scale(6.0f, 6.0f, 6.0f, MTXMODE_APPLY);
    Matrix_RotateZ((thisx->shape.rot.z / (f32)0x8000) * M_PI, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(gfxCtx, (char*)__FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, sHeadMat);
    gSPDisplayList(POLY_XLU_DISP++, sHeadBall);

    CLOSE_DISPS(gfxCtx);
}

static void Tcb_Draw(Actor* thisx, PlayState* play) {
    // The LIGHT ball is Phantom Ganon's own energy ball, drawn exactly the way
    // EnFhgFire_Draw does it (billboard, XLU, prim white / env cyan-green) — the DL
    // comes straight out of oot.o2r by OTR path, the same way the held lance does
    // (Trident_GetLanceDL); no object gets loaded for it.
    static Gfx* sLightBallDL = NULL;
    static u8 sLightBallTried = 0;
    s8 slot = Tcb_GetSlot(thisx);

    if (slot < 0) {
        return;
    }

    if (sTcbPool[slot].kind == TCB_KIND_BALL) {
        // The big-magic ball, thrown. Identical draw to the one Link charges over his
        // head (Trident_Draw), at the size it had when it left him, so the release
        // reads as THAT ball flying off instead of a new effect appearing.
        f32 v = (sTcbPool[slot].visScale > 0.3f) ? sTcbPool[slot].visScale : 0.3f;
        f32 alpha = 255.0f;
        if (sTcbPool[slot].burst > 0) {
            // Swell and go. t runs 0 -> 1 across the burst; the size rises to
            // TCB_BURST_PEAK over the first TCB_BURST_RISE of it and collapses to
            // nothing over the rest, with the alpha falling the whole way.
            f32 t = 1.0f - ((f32)sTcbPool[slot].burst / (f32)TCB_BURST_FRAMES);
            f32 grow = (t < TCB_BURST_RISE)
                           ? (1.0f + (((TCB_BURST_PEAK - 1.0f) * t) / TCB_BURST_RISE))
                           : (TCB_BURST_PEAK * (1.0f - ((t - TCB_BURST_RISE) / (1.0f - TCB_BURST_RISE))));
            if (grow < 0.0f) {
                grow = 0.0f;
            }
            v *= grow;
            alpha = 255.0f * (1.0f - t);
        }
        TridentBigMagic_Draw(play, &thisx->world.pos, TCB_BALL_CIRCLE_SCALE * v, TCB_BALL_DRAW_SCALE * v, alpha,
                             TBM_RAYS_MAX, (thisx->shape.rot.z / (f32)0x8000) * M_PI);
        return;
    }

    if (sTcbPool[slot].kind == TCB_KIND_HUNTER) {
        Tcb_DrawStreak(&sTcbPool[slot], thisx, play);
        return;
    }

    if (sTcbPool[slot].kind != TCB_KIND_LIGHT) {
        return;
    }
    if (!sLightBallTried) {
        const char* otr = "__OTR__objects/object_fhg/gPhantomEnergyBallDL";
        sLightBallTried = 1;
        sLightBallDL = (Gfx*)OotAssets_LoadGfxDirect(otr);
    }
    if (sLightBallDL == NULL) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);
    Matrix_Translate(thisx->world.pos.x, thisx->world.pos.y, thisx->world.pos.z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_Scale(thisx->scale.x, thisx->scale.y, thisx->scale.z, MTXMODE_APPLY);
    Matrix_RotateZ((thisx->shape.rot.z / (f32)0x8000) * 3.1416f, MTXMODE_APPLY);
    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 255);
    gDPSetEnvColor(POLY_XLU_DISP++, 165, 255, 75, 0);
    gDPPipeSync(POLY_XLU_DISP++);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx, (char*)__FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, sLightBallDL);
    CLOSE_DISPS(play->state.gfxCtx);
}

// ---------------------------------------------------------------------------
// Spawning
// ---------------------------------------------------------------------------
static Actor* Tcb_SpawnInternal(PlayState* play, Vec3f* pos, u8 kind, f32 charge01, Vec3f* initialVel) {
    Actor* actor;
    s8 slot;

    actor = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_A_OBJ, pos->x, pos->y, pos->z, 0, 0, 0, 0);
    if (actor == NULL) {
        return NULL;
    }

    // MM uses En_A_Obj as the harmless host. Unlike SoH's En_Lightbox it does not
    // register a DynaPoly wall, so there is no background actor to tear down here.

    slot = Tcb_AllocSlot(actor, kind);
    if (slot < 0) {
        Actor_Kill(actor);
        return NULL;
    }

    sTcbPool[slot].visScale = charge01;
    if (initialVel != NULL) {
        sTcbPool[slot].velocity = *initialVel;
    }
    // Seed the whole streak history at the spawn point, exactly as BossGanon_Init does
    // for its thrown balls — otherwise the first frames draw a streak reaching back to
    // wherever the slot's previous tenant died.
    {
        s32 t;
        for (t = 0; t < TCB_TRAIL_LEN; t++) {
            sTcbPool[slot].trailPos[t] = *pos;
            sTcbPool[slot].trailRot[t].x = sTcbPool[slot].trailRot[t].y = sTcbPool[slot].trailRot[t].z = 0.0f;
        }
    }
    sTcbPool[slot].target = Tcb_AcquireTarget(play, actor,
                                              (kind == TCB_KIND_BALL)    ? TCB_SEEK_RANGE
                                              : (kind == TCB_KIND_LIGHT) ? TCB_LIGHT_RANGE
                                                                         : TCB_HUNTER_RANGE);

    actor->update = Tcb_Update;
    actor->draw = Tcb_Draw;
    actor->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;
    actor->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;

    return actor;
}

// The nearest reachable enemy to `origin` that is not already in `taken`. Plain
// linear scan of the enemy list rather than Actor_FindNearby, for two reasons:
// Actor_FindNearby has no way to exclude what the previous seeker already claimed
// (and "cada uno el más cercano" only means anything if they pick DIFFERENT ones),
// and it has no way to say "on screen counts however far it is" — see Tcb_Reachable.
// `taken` may be NULL when nothing is claimed yet.
static Actor* Tcb_NearestUntaken(PlayState* play, Vec3f* origin, Actor** taken, s32 nTaken, f32 range) {
    Actor* best = NULL;
    f32 bestDist = 1.0e9f;
    s32 cat;
    s32 i;

    // Bosses first, then ordinary enemies: a boss in the room is what these are for.
    for (cat = 0; cat < 2; cat++) {
        Actor* actor = play->actorCtx.actorLists[(cat == 0) ? ACTORCAT_BOSS : ACTORCAT_ENEMY].first;
        while (actor != NULL) {
            if (Tcb_TargetIsUsable(actor) && Tcb_Reachable(actor, origin, range)) {
                u8 claimed = 0;
                for (i = 0; i < nTaken; i++) {
                    if (taken[i] == actor) {
                        claimed = 1;
                        break;
                    }
                }
                if (!claimed) {
                    f32 d = Math_Vec3f_DistXYZ(origin, &actor->world.pos);
                    if (d < bestDist) {
                        bestDist = d;
                        best = actor;
                    }
                }
            }
            actor = actor->next;
        }
        if (best != NULL) {
            return best;
        }
    }
    return NULL;
}

// Prefer an attention-enabled boss (the engine's actual lock-on candidate) over
// helper actors which MM also stores in ACTORCAT_BOSS. Fall back to any live boss
// only for encounters whose main actor deliberately disables attention.
static Actor* Tcb_NearestBoss(PlayState* play, Vec3f* origin, f32 range) {
    Actor* best = NULL;
    s32 pass;

    for (pass = 0; pass < 2; pass++) {
        Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].first;
        f32 bestDist = 1.0e9f;

        while (actor != NULL) {
            if (Tcb_TargetIsUsable(actor) && Tcb_Reachable(actor, origin, range) &&
                ((pass != 0) || ((actor->flags & ACTOR_FLAG_ATTENTION_ENABLED) != 0))) {
                f32 d = Math_Vec3f_DistXYZ(origin, &actor->focus.pos);
                if (d < bestDist) {
                    bestDist = d;
                    best = actor;
                }
            }
            actor = actor->next;
        }
        if (best != NULL) {
            return best;
        }
    }
    return NULL;
}

// Ordinary impacts keep the four-target spread. A max charge released in a boss
// room instead creates six FD light trails, all committed to the focused/nearest
// boss so fast encounters such as Goht cannot steal shots into scenery.
static void Tcb_SpawnHunters(PlayState* play, Vec3f* origin, u8 fierceDamage) {
    Actor* taken[TCB_HUNTER_MAX];
    Actor* bossTarget = NULL;
    Player* player = GET_PLAYER(play);
    s32 nTaken = 0;
    s32 count = (fierceDamage != 0) ? TCB_HUNTER_BOSS_COUNT : TCB_HUNTER_NORMAL_COUNT;
    Vec3f vel;
    s32 i;

    if (fierceDamage != 0) {
        bossTarget = ((player != NULL) && Tcb_TargetIsUsable(player->focusActor) &&
                      (Tcb_ResolveBoss(player->focusActor) != NULL))
                         ? player->focusActor
                         : Tcb_NearestBoss(play, origin, TCB_BOSS_ROOM_RANGE);
    }

    for (i = 0; i < count; i++) {
        Actor* actor;
        Actor* target =
            (fierceDamage != 0) ? bossTarget : Tcb_NearestUntaken(play, origin, taken, nTaken, TCB_HUNTER_RANGE);
        if ((fierceDamage == 0) && (target == NULL) && (nTaken > 0)) {
            nTaken = 0; // ran out of fresh enemies: go round again
            target = Tcb_NearestUntaken(play, origin, taken, nTaken, TCB_HUNTER_RANGE);
        }

        // Fan them outward so they visibly disperse before homing in.
        f32 ang = (f32)i * (2.0f * 3.14159265f / (f32)count);
        vel.x = cosf(ang) * TCB_HUNTER_FAN;
        vel.y = 4.0f;
        vel.z = sinf(ang) * TCB_HUNTER_FAN;

        actor = Tcb_SpawnInternal(play, origin, TCB_KIND_HUNTER, 1.0f, &vel);
        if (actor == NULL) {
            break; // pool exhausted
        }
        if (fierceDamage != 0) {
            s8 slot = Tcb_GetSlot(actor);
            if (slot >= 0) {
                // The boss-room six are FD projectiles even if the phase actor
                // vanishes between the held sphere and the burst frame.
                sTcbPool[slot].fixedDamage = fierceDamage;
            }
        }
        if (target != NULL) {
            s8 slot = Tcb_GetSlot(actor);
            if (slot >= 0) {
                sTcbPool[slot].target = target; // overrides Tcb_SpawnInternal's own pick
            }
            if (fierceDamage == 0) {
                taken[nTaken++] = target;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Exported accessors
// ---------------------------------------------------------------------------
u8 TridentChargeBall_IsActive(void) {
    s8 i;

    if (sTcbGraceTimer > 0) {
        return 1;
    }
    // Ordinary balls retain the original super-hit window. Boss-room max charge
    // transfers that claim to its six fixed-damage FD hunters.
    for (i = 0; i < TCB_SLOT_MAX; i++) {
        if ((sTcbPool[i].owner != NULL) && ((sTcbPool[i].kind == TCB_KIND_BALL) || (sTcbPool[i].fixedDamage != 0))) {
            return 1;
        }
    }
    return 0;
}

u8 TridentChargeBall_GetFierceDamage(Actor* projectile) {
    s8 slot = Tcb_GetSlot(projectile);
    s32 i;

    if (slot >= 0) {
        return sTcbPool[slot].fixedDamage;
    }
    for (i = 0; i < TCB_HUNTER_BOSS_COUNT; i++) {
        if ((sTcbFierceHitGrace[i].timer != 0) && (sTcbFierceHitGrace[i].actor == projectile)) {
            return sTcbFierceHitGrace[i].damage;
        }
    }
    return 0;
}

void TridentChargeBall_Tick(void) {
    s32 i;

    if (sTcbGraceTimer > 0) {
        sTcbGraceTimer--;
    }
    for (i = 0; i < TCB_HUNTER_BOSS_COUNT; i++) {
        if ((sTcbFierceHitGrace[i].timer != 0) && (--sTcbFierceHitGrace[i].timer == 0)) {
            sTcbFierceHitGrace[i].actor = NULL;
            sTcbFierceHitGrace[i].damage = 0;
        }
    }
}

void TridentChargeBall_Forget(void) {
    memset(sTcbPool, 0, sizeof(sTcbPool));
    memset(sTcbFierceHitGrace, 0, sizeof(sTcbFierceHitGrace));
    sTcbGraceTimer = 0;
}

Actor* TridentChargeBall_Spawn(PlayState* play, Vec3f* pos, f32 charge01) {
    Vec3f vel = { 0.0f, 0.0f, 0.0f };
    Player* player;

    if (play == NULL || pos == NULL) {
        return NULL;
    }

    // Launch along the player's facing so an untargeted shot still travels
    // forward instead of stalling at the muzzle while it looks for a target.
    player = GET_PLAYER(play);
    if (player != NULL) {
        f32 yaw = (f32)player->actor.shape.rot.y * (3.14159265f / 32768.0f);
        vel.x = sinf(yaw) * TCB_SPEED;
        vel.z = cosf(yaw) * TCB_SPEED;
    }

    return Tcb_SpawnInternal(play, pos, TCB_KIND_BALL, charge01, &vel);
}

// The max-charge release never launches the held giant sphere in a boss room.
// It breaks in place on the next update and becomes six 12-damage FD trails aimed
// at that boss. Outside a boss encounter it keeps the ordinary four-shot burst.
Actor* TridentChargeBall_SpawnMax(PlayState* play, Vec3f* pos, s32 damage) {
    Actor* actor;
    Actor* boss;
    Player* player;
    s8 slot;

    if (play == NULL || pos == NULL) {
        return NULL;
    }

    actor = TridentChargeBall_Spawn(play, pos, 1.0f);
    if (actor == NULL) {
        return NULL;
    }
    slot = Tcb_GetSlot(actor);
    if (slot < 0) {
        return actor;
    }

    (void)damage; // Boss-room volley damage is deliberately fixed at 12.

    // Preserve the player's deliberate lock-on, especially when it is a weak-point
    // actor parented to the boss. Only auto-select when no boss-related focus exists.
    player = GET_PLAYER(play);
    boss = ((player != NULL) && Tcb_TargetIsUsable(player->focusActor) && (Tcb_ResolveBoss(player->focusActor) != NULL))
               ? player->focusActor
               : Tcb_NearestBoss(play, &actor->world.pos, TCB_BOSS_ROOM_RANGE);
    if (Tcb_TargetIsUsable(boss)) {
        sTcbPool[slot].target = boss;
        sTcbPool[slot].burstDamage = TCB_BOSS_VOLLEY_DAMAGE;
    } else {
        sTcbPool[slot].lifetime = 1; // breaks next frame; the burst is the payload
    }
    return actor;
}

// One lit mote at `pos`, no collider and no actor — the takeoff streak under Link's
// feet. Lives here so the flight and the projectiles share one visual vocabulary.
void TridentChargeBall_DropSpark(PlayState* play, Vec3f* pos) {
    Vec3f zero = { 0.0f, 0.0f, 0.0f };

    if (play == NULL || pos == NULL) {
        return;
    }
    EffectSsFhgFlash_SpawnLightBall(play, pos, &zero, &zero, 110, TCB_FX_LIGHTBALL_BLUE);
}

Actor* TridentChargeBall_SpawnLight(PlayState* play, Vec3f* pos) {
    Vec3f vel = { 0.0f, 0.0f, 0.0f };
    Player* player;

    if (play == NULL || pos == NULL) {
        return NULL;
    }
    player = GET_PLAYER(play);
    if (player != NULL) {
        f32 yaw = (f32)player->actor.shape.rot.y * (3.14159265f / 32768.0f);
        vel.x = sinf(yaw) * TCB_LIGHT_SPEED;
        vel.z = cosf(yaw) * TCB_LIGHT_SPEED;
    }
    return Tcb_SpawnInternal(play, pos, TCB_KIND_LIGHT, 1.0f, &vel);
}
