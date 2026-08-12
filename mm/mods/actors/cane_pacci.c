/**
 * cane_pacci.c — Pacci side of the Dual Cane. See cane_pacci.h. Skijer's NEI
 *
 * Flip and Stone work on ANY ACTORCAT_ENEMY actor rather than a curated list, so
 * the mechanic behaves the same on enemies nobody thought to special-case. They
 * do it by taking the actor over: its `update` (and, for Stone, its `draw`) is
 * swapped for ours, which is what "loses all its AI" means literally. Everything
 * we overwrite is saved in the pool entry and put back on release, so an enemy
 * that survives a Flip resumes its own behaviour exactly where it left off.
 */

#include "cane_pacci.h"
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "nei_oot_compat.h" // Matrix_NewMtx -> Matrix_Finalize, for the weld bead
#include "objects/gameplay_field_keep/gameplay_field_keep.h"
#include "objects/gameplay_keep/gameplay_keep.h"
#include "objects/object_dy_obj/object_dy_obj.h"
#include "../items/helpers/target_select_helper.h"
#include <math.h>

extern PlayState* gPlayState;

// ============================================================================
// POOL
// ============================================================================

typedef enum {
    PACCI_FX_NONE = 0,
    PACCI_FX_FLIP,
    PACCI_FX_STONE,
} PacciFxMode;

typedef enum {
    PACCI_FLIP_PHASE_AIRBORNE = 0,
    PACCI_FLIP_PHASE_DOWNED,
    PACCI_FLIP_PHASE_RIGHTING,
} PacciFlipPhase;

typedef enum {
    PACCI_STONE_PHASE_IDLE = 0,
    PACCI_STONE_PHASE_HELD,
    PACCI_STONE_PHASE_THROWN,
} PacciStonePhase;

typedef struct {
    Actor* actor;
    u8 mode;  // PacciFxMode
    u8 phase;
    s16 timer;
    // Saved actor state, restored verbatim on release.
    ActorFunc origUpdate;
    ActorFunc origDraw;
    u32 origFlags;
    f32 origGravity;
    f32 origMinVelocityY;
    f32 origSpeed;
    // FULL rotation cache. The ObjTsubo tumble in the free fall writes shape.rot.x
    // and .y as well as .z, so restoring only .z would hand the enemy back to its
    // own AI sitting on a garbage orientation — its animations then play tilted or
    // inside-out for the rest of its life. Both rotations are snapshotted before
    // the flip and forced back verbatim on recovery.
    Vec3s origShapeRot;
    Vec3s origWorldRot;
    s16 origRoom;
    u8 origMass;
    f32 origYOffset; // shape.yOffset is moved during the flip, so it has to come back
    // ObjTsubo_Thrown's tumble, per-entry (the pot keeps these in file statics, which
    // would make every flipped enemy spin in lockstep).
    s16 tumbleX;
    s16 tumbleY;
    s16 tumbleTargetX;
    s16 tumbleTargetY;
    // AT collider used while a flipped enemy is falling, so it smashes pots and
    // cuts grass on the way down.
    ColliderCylinder collider;
    u8 colliderReady;
} PacciFx;

static PacciFx sPacciPool[PACCI_MAX_AFFECTED] = { { 0 } };

// Per-skill aim colours (user-locked): Flip RED, Stone YELLOW, Ultrahand BLUE.
// The tint says which of Pacci's three things the button is about to do, which
// matters once the cane grows toward a Sheikah-Stone-style multi-tool.
//
// LIMITATION: the colour filter is not a colour — it is a small set of hardcoded
// modes (RED / BLUE / GRAY / white). There is no yellow, and no way to ask for one
// without replacing the engine's filter draw. GRAY is used for Stone as the
// nearest reading: it is the "about to be petrified" wash, and it is unmistakably
// distinct from the other two at a glance.
#define PACCI_TINT_FLIP(actor, dur) \
    Actor_SetColorFilter((actor), COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, (dur))
#define PACCI_TINT_STONE(actor, dur) \
    Actor_SetColorFilter((actor), COLORFILTER_COLORFLAG_GRAY, 255, COLORFILTER_BUFFLAG_OPA, (dur))
#define PACCI_TINT_ULTRAHAND(actor, dur) \
    Actor_SetColorFilter((actor), COLORFILTER_COLORFLAG_BLUE, 255, COLORFILTER_BUFFLAG_OPA, (dur))
// There is deliberately NO Zonai tint macro. Every attempt to make one went through
// Actor_SetColorFilter, which offers exactly three modes - white, red, blue - and picking
// white produced the pale, petrified-looking wash the tint was supposed to avoid. Green on
// the object itself comes from a real point light instead (Pacci_UhLightAt), which tints
// the model's own texture rather than painting over it.
// Flash for a non-lethal hit landed on a helpless enemy.
#define PACCI_TINT_HURT(actor, dur) \
    Actor_SetColorFilter((actor), COLORFILTER_COLORFLAG_GRAY, 255, COLORFILTER_BUFFLAG_OPA, (dur))

// Flip sound cues. MM has no Tektites, so neither NA_SE_EN_TEKU_REVERSE nor
// NA_SE_EN_DODO_M_GND exists there — these are the closest stand-ins in MM's bank.
#define PACCI_SFX_FLIP NA_SE_IT_HAMMER_HIT
#define PACCI_SFX_FLIP_LAND NA_SE_EV_BOMB_BOUND

// While flipped the enemy carries THIS collider instead of its own, and it does
// double duty:
//   AT (player-type) — the thrown-pot behaviour: it smashes pots and cuts grass on
//                      the way down, with the vanilla break particles.
//   AC (player-type) — what makes a flipped enemy hittable at all. The collider is
//                      owned by the target actor, so a hit here lands the damage in
//                      the ENEMY's colChkInfo; Pacci_FlipUpdate then hands the actor
//                      straight back to its own update, which processes that damage
//                      through its normal path (death, drops, animation, all of it).
static ColliderCylinderInit sPacciFallColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0xFFCFFFFF, 0x00, 0x08 },
        { 0xFFCFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_ON,
        OCELEM_NONE,
    },
    { 30, 46, -12, { 0, 0, 0 } },
};

void Pacci_CleanupPool(void); // used by Pacci_TakeEntry below, defined further down

static PacciFx* Pacci_FindEntry(Actor* actor) {
    for (u8 i = 0; i < PACCI_MAX_AFFECTED; i++) {
        if (sPacciPool[i].actor == actor) {
            return &sPacciPool[i];
        }
    }
    return NULL;
}

static PacciFx* Pacci_TakeEntry(void) {
    // Reap first. A flipped enemy that gets killed mid-flip — the intended outcome
    // of the move — leaves an entry whose actor is gone, and since the effect now
    // outlives the cane there is no longer a per-frame cleanup guaranteed to be
    // running. Without this the pool would silently fill up with corpses.
    Pacci_CleanupPool();

    for (u8 i = 0; i < PACCI_MAX_AFFECTED; i++) {
        if (sPacciPool[i].actor == NULL) {
            return &sPacciPool[i];
        }
    }
    return NULL;
}

// Put the actor back exactly as we found it and free the slot.
static void Pacci_Restore(PacciFx* fx) {
    Actor* actor = fx->actor;

    if (actor != NULL && actor->update != NULL) {
        actor->update = fx->origUpdate;
        if (fx->origDraw != NULL) {
            actor->draw = fx->origDraw;
        }
        actor->flags = fx->origFlags;
        actor->gravity = fx->origGravity;
        actor->minVelocityY = fx->origMinVelocityY;
        actor->shape.rot = fx->origShapeRot;
        actor->world.rot = fx->origWorldRot;
        actor->room = (s8)fx->origRoom;
        actor->colChkInfo.mass = fx->origMass;
        actor->shape.yOffset = fx->origYOffset;
        actor->speed = 0.0f;
        actor->velocity.x = actor->velocity.y = actor->velocity.z = 0.0f;
    }

    fx->actor = NULL;
    fx->mode = PACCI_FX_NONE;
    fx->phase = 0;
    fx->timer = 0;
}

void Pacci_CleanupPool(void) {
    // Forget, do not release: by the time this runs the actors are gone, so writing
    // their update/flags back would be a use-after-free.
    Pacci_FuseForget();
    for (u8 i = 0; i < PACCI_MAX_AFFECTED; i++) {
        if (sPacciPool[i].actor != NULL && sPacciPool[i].actor->update == NULL) {
            sPacciPool[i].actor = NULL;
            sPacciPool[i].mode = PACCI_FX_NONE;
        }
    }
}

// ============================================================================
// TARGETING
// ============================================================================

u8 Pacci_IsValidEnemy(Actor* actor) {
    if (actor == NULL || actor->update == NULL) {
        return 0;
    }
    if (actor->category != ACTORCAT_ENEMY) {
        return 0; // user-locked: enemies only
    }
    if (actor->id == ACTOR_PLAYER) {
        return 0;
    }
    // Bosses are excluded (user-locked). ACTORCAT_BOSS covers the real bosses;
    // minibosses live in ACTORCAT_ENEMY, so exclude the heavy ones too — an
    // IMMOVABLE mass is exactly what marks "this thing does not get knocked
    // around" and needs no per-actor id list to stay correct.
    if (actor->category == ACTORCAT_BOSS) {
        return 0;
    }
    if (actor->colChkInfo.mass == MASS_IMMOVABLE) {
        return 0;
    }
    switch (actor->id) {
        case ACTOR_EN_WIZ:      // Wizzrobe (arena miniboss, teleport script)
        case ACTOR_EN_DINOFOS:  // Dinolfos
        case ACTOR_EN_BIGPO:    // Big Poe
        case ACTOR_EN_BIGSLIME: // Gekko + Slime
            return 0;
        default:
            break;
    }
    if (Pacci_FindEntry(actor) != NULL) {
        return 0; // already flipped or petrified
    }
    return 1;
}

static s32 Pacci_EnemyFilter(Actor* actor) {
    return Pacci_IsValidEnemy(actor);
}

static const u8 sPacciEnemyCats[1] = { ACTORCAT_ENEMY };

static Actor* Pacci_ScanEnemy(PlayState* play) {
    return TargetSelect_ScanCats(play, sPacciEnemyCats, 1, Pacci_EnemyFilter, TARGETSEL_DEFAULT_RANGE,
                                 TARGETSEL_DEFAULT_CONE);
}

// Defined down in the LIFT section, but Flip shares it — both moves use one notion
// of "something I can grab", and Flip is written above that section.
static Actor* Pacci_ScanLiftable(PlayState* play);
// Defined down in the ULTRAHAND section, but the highlight above it shares the same
// resolver — that is the whole point, so the two can never mark and grab different
// things.
static Actor* Pacci_ResolveUltrahandTarget(PlayState* play, Player* player);
static void Pacci_BurstSpawn(PlayState* play, Player* player, Vec3f* pos, u8 heavy);

// Live aim feedback for Flip and Stone: every frame the cane is in hand with one
// of them selected, the enemy that would be hit is tinted in that skill's colour.
// Same idea as the Switch Hook's continuous selection (z_arms_hook.c calls
// TargetSelect_Highlight with a short duration and re-applies it each frame).
void Pacci_HighlightEnemyTarget(PlayState* play, u8 stone) {
    // Flip acts on anything liftable, so the highlight has to scan the SAME set.
    // It used to scan enemies only, which is why standing in front of a pot showed
    // no suggestion at all even after Flip itself had been widened to props.
    // Stone is still enemies-only — petrifying a crate means nothing.
    Actor* target = stone ? Pacci_ScanEnemy(play) : Pacci_ScanLiftable(play);

    if (target == NULL) {
        return;
    }
    // Short duration, because it is re-applied every frame; the moment the player
    // looks away it lapses on its own.
    if (stone) {
        PACCI_TINT_STONE(target, 4);
    } else {
        PACCI_TINT_FLIP(target, 4);
    }
}

// ============================================================================
// FLIP
// ============================================================================
//
// Ported from two vanilla behaviours, without touching either actor:
//
//   EnTite_SetupFlipOnBack / EnTite_FlipOnBack / EnTite_FlipUpright  — the flip.
//     A hammered Tektite launches straight up (velocity.y 11, gravity -1), rolls
//     shape.rot.z toward 0x7FFF while airborne, and ramps shape.yOffset up to 2800
//     so the actor's pivot slides from its feet to its back. On landing it puffs a
//     floor dust ring, then lies there for the on-back timer before righting itself
//     with a second hop (velocity.y 13) and rot.z easing back to 0. Every constant
//     below is that actor's.
//
//   ObjTsubo_Thrown — the free fall. A thrown pot keeps a LIVE AT+OC collider the
//     whole way down, which is what lets it smash things (and be smashed), and it
//     tumbles by stepping shape.rot.x/y toward random targets. The flipped enemy
//     falls the same way, so anything it lands on breaks exactly as if a pot had
//     been thrown at it.
//
// The enemy is paralysed throughout for the simple reason that its own `update` is
// not running at all — this function replaced it.

// EnTite_SetupFlipOnBack
#define PACCI_FLIP_LAUNCH_VEL_Y 11.0f
#define PACCI_FLIP_LAUNCH_GRAVITY -1.0f
#define PACCI_FLIP_ROT_STEP 4000 // rot.z -> 0x7FFF while flipping over
#define PACCI_FLIP_YOFFSET_STEP 400.0f
#define PACCI_FLIP_YOFFSET_MAX 2800.0f
// EnTite_SetupFlipUpright
#define PACCI_FLIP_RIGHT_VEL_Y 13.0f
#define PACCI_FLIP_RIGHT_ROT_STEP 0xFA0
// ObjTsubo_SetupThrown
#define PACCI_FLIP_THROWN_MASS 240
#define PACCI_FLIP_TUMBLE_STEP 0x64

// ObjTsubo_SetupThrown's tumble targets, rolled per flip so two enemies never spin
// identically. Kept on the pool entry rather than in file statics — the pot uses
// four file-scope s16s, which would make every flipped enemy share one spin.
static void Pacci_FlipRollTumble(PacciFx* fx) {
    fx->tumbleTargetX = (s16)((Rand_ZeroOne() - 0.7f) * 2800.0f);
    fx->tumbleTargetY = (s16)((Rand_ZeroOne() - 0.5f) * 2000.0f);
    fx->tumbleX = 0;
    fx->tumbleY = 0;
}

// One frame of ObjTsubo_Thrown's motion: gravity, tumble, bg check, and the live
// AT/OC that does the smashing.
static void Pacci_FlipFallStep(PacciFx* fx, Actor* thisx, PlayState* play) {
    thisx->velocity.y += thisx->gravity;
    if (thisx->velocity.y < thisx->minVelocityY) {
        thisx->velocity.y = thisx->minVelocityY;
    }
    Actor_UpdatePos(thisx);

    Math_StepToS(&fx->tumbleX, fx->tumbleTargetX, PACCI_FLIP_TUMBLE_STEP);
    Math_StepToS(&fx->tumbleY, fx->tumbleTargetY, PACCI_FLIP_TUMBLE_STEP);
    thisx->shape.rot.x += fx->tumbleX;
    thisx->shape.rot.y += fx->tumbleY;

    Actor_UpdateBgCheckInfo(play, thisx, 5.0f, 15.0f, 0.0f, 0x85);
}

// Submit the flipped enemy's stand-in colliders for this frame.
//
// AC and OC run for the WHOLE effect: the enemy has to stay hittable while it is
// down, which is the entire reason the move exists. AT only runs while it is in the
// air — a downed enemy resting on a pot should not keep smashing it every frame.
static void Pacci_FlipSubmitColliders(PacciFx* fx, Actor* thisx, PlayState* play, u8 airborne) {
    if (!fx->colliderReady) {
        return;
    }
    Collider_UpdateCylinder(thisx, &fx->collider);
    if (airborne) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &fx->collider.base);
    }
    CollisionCheck_SetAC(play, &play->colChkCtx, &fx->collider.base);
    CollisionCheck_SetOC(play, &play->colChkCtx, &fx->collider.base);
}

static void Pacci_FlipUpdate(Actor* thisx, PlayState* play) {
    PacciFx* fx = Pacci_FindEntry(thisx);

    if (fx == NULL) {
        return; // pool entry vanished — leave the actor frozen rather than crash
    }

    // Hit while it was down. Handing the actor straight back to its own update here
    // was wrong: it woke up on the very frame it was struck, so the flip bought no
    // free hits at all. Instead the damage is applied to its health WITHOUT waking
    // it, and it stays helpless — which is the whole point of knocking it over.
    //
    // Only the killing blow gives the actor back, and it goes back with colChkInfo
    // untouched, so its own update runs its own death: its animation, its drops,
    // its effects. Nothing here needs to know how any particular enemy dies.
    if (fx->colliderReady && (fx->collider.base.acFlags & AC_HIT)) {
        fx->collider.base.acFlags &= ~AC_HIT;

        if (thisx->colChkInfo.damage > 0) {
            if (thisx->colChkInfo.health > thisx->colChkInfo.damage) {
                thisx->colChkInfo.health -= thisx->colChkInfo.damage;
                // Flash white on the hit so a free hit still reads as a hit.
                PACCI_TINT_HURT(thisx, 12);
                Audio_PlayActorSound2(thisx, PACCI_SFX_FLIP_LAND);
                thisx->colChkInfo.damage = 0;
            } else {
                thisx->colChkInfo.health = 0;
                Pacci_Restore(fx); // lethal — let it die its own way
                return;
            }
        }
    }

    switch (fx->phase) {
        case PACCI_FLIP_PHASE_AIRBORNE:
            // Roll onto its back on the way up (EnTite_FlipOnBack).
            Math_SmoothStepToS(&thisx->shape.rot.z, 0x7FFF, 1, PACCI_FLIP_ROT_STEP, 0);
            Pacci_FlipFallStep(fx, thisx, play);
            Pacci_FlipSubmitColliders(fx, thisx, play, true);

            if (thisx->bgCheckFlags & (BGCHECKFLAG_GROUND | BGCHECKFLAG_GROUND_TOUCH)) {
                if (thisx->bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
                    Actor_SpawnFloorDustRing(play, thisx, &thisx->world.pos, 20.0f, 11, 4.0f, 0, 0, false);
                    Audio_PlayActorSound2(thisx, PACCI_SFX_FLIP_LAND);
                }
                // A flipped PROP lands like one Link had picked up and dropped: it
                // takes the impact and breaks. Only enemies stay down and helpless —
                // lying there paralysed is what flipping an enemy is FOR.
                if (thisx->category != ACTORCAT_ENEMY) {
                    Player* impactPlayer = GET_PLAYER(play);
                    Vec3f impact = thisx->world.pos;
                    u8 heavy = (thisx->id == ACTOR_EN_ISHI);

                    Pacci_Restore(fx); // hand it back before the burst resolves
                    if (impactPlayer != NULL) {
                        Pacci_BurstSpawn(play, impactPlayer, &impact, heavy);
                    }
                    return;
                }
                fx->phase = PACCI_FLIP_PHASE_DOWNED;
                fx->timer = PACCI_FLIP_ON_BACK_TIMER;
                thisx->speed = 0.0f;
            } else {
                // Slide the pivot from its feet to its back so it visibly lies on
                // its shell rather than hovering upside down over its own origin.
                if (thisx->shape.yOffset < PACCI_FLIP_YOFFSET_MAX) {
                    thisx->shape.yOffset += PACCI_FLIP_YOFFSET_STEP;
                }
            }
            break;

        case PACCI_FLIP_PHASE_DOWNED:
            // Paralysed on its back. Still held at 0x7FFF so a slope cannot roll it.
            Math_SmoothStepToS(&thisx->shape.rot.z, 0x7FFF, 1, PACCI_FLIP_ROT_STEP, 0);
            Math_StepToF(&thisx->speed, 0.0f, 1.0f);
            Actor_MoveWithGravity(thisx);
            Actor_UpdateBgCheckInfo(play, thisx, 5.0f, 15.0f, 0.0f, 0x85);
            Pacci_FlipSubmitColliders(fx, thisx, play, false);
            if (fx->timer > 0) {
                fx->timer--;
            } else {
                // EnTite_SetupFlipUpright: a hop, and rot.z eases back to normal.
                fx->phase = PACCI_FLIP_PHASE_RIGHTING;
                thisx->velocity.y = PACCI_FLIP_RIGHT_VEL_Y;
                Audio_PlayActorSound2(thisx, PACCI_SFX_FLIP);
            }
            break;

        case PACCI_FLIP_PHASE_RIGHTING:
            Math_SmoothStepToS(&thisx->shape.rot.z, fx->origShapeRot.z, 1, PACCI_FLIP_RIGHT_ROT_STEP, 0);
            Actor_MoveWithGravity(thisx);
            Actor_UpdateBgCheckInfo(play, thisx, 5.0f, 15.0f, 0.0f, 0x85);
            Pacci_FlipSubmitColliders(fx, thisx, play, false);
            if (thisx->bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
                Audio_PlayActorSound2(thisx, PACCI_SFX_FLIP_LAND);
                thisx->shape.yOffset = fx->origYOffset;
                thisx->world.pos.y = thisx->floorHeight;
                Pacci_Restore(fx); // back on its feet; its own AI takes over again
                return;
            }
            break;
    }

    thisx->focus.pos = thisx->world.pos;
}

u8 Pacci_CastFlip(PlayState* play, Player* player) {
    // Flip used to scan ACTORCAT_ENEMY only, which is why it silently refused pots,
    // crates and boulders — the prop list existed but only the lift consulted it.
    // Both moves now share one notion of "something I can grab".
    Actor* target = Pacci_ScanLiftable(play);
    PacciFx* fx;

    if (target == NULL) {
        return 0;
    }
    fx = Pacci_TakeEntry();
    if (fx == NULL) {
        return 0;
    }

    fx->actor = target;
    fx->mode = PACCI_FX_FLIP;
    fx->phase = PACCI_FLIP_PHASE_AIRBORNE;
    fx->timer = 0;
    fx->origUpdate = target->update;
    fx->origDraw = NULL; // Flip keeps the enemy's own look
    fx->origFlags = target->flags;
    fx->origGravity = target->gravity;
    fx->origMinVelocityY = target->minVelocityY;
    fx->origSpeed = target->speed;
    fx->origShapeRot = target->shape.rot;
    fx->origWorldRot = target->world.rot;
    fx->origRoom = target->room;
    fx->origMass = target->colChkInfo.mass;
    fx->origYOffset = target->shape.yOffset;
    Pacci_FlipRollTumble(fx);

    if (!fx->colliderReady) {
        Collider_InitCylinder(play, &fx->collider);
        fx->colliderReady = 1;
    }
    Collider_SetCylinder(play, &fx->collider, target, &sPacciFallColliderInit);

    // EnTite_SetupFlipOnBack: straight up. Plus a thrown pot's mass, so on the way
    // down it shoulders things aside instead of being shoved by them.
    target->update = Pacci_FlipUpdate;
    target->gravity = PACCI_FLIP_LAUNCH_GRAVITY;
    target->minVelocityY = PACCI_FLIP_MIN_VEL_Y;
    target->velocity.y = PACCI_FLIP_LAUNCH_VEL_Y;
    target->speed = 0.0f;
    target->colChkInfo.mass = PACCI_FLIP_THROWN_MASS;
    target->bgCheckFlags &= ~(BGCHECKFLAG_GROUND | BGCHECKFLAG_GROUND_TOUCH);
    // Culling would stop OUR update too, freezing the enemy mid-flip until the
    // player walked back into range. The flags are part of origFlags, so they go
    // back to normal on recovery.
    target->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;

    Audio_PlayActorSound2(target, PACCI_SFX_FLIP);
    PacciFlipVfx_Start(play, player, target); // stub — see pacci_flip_vfx.h
    return 1;
}

// ============================================================================
// LIFT / THROW  (Pacci: hold C)
// ============================================================================
//
// Holding the button telekinetically picks the target up and holds it in the air
// in front of Link; releasing throws it. If Link is Z-targeting something when he
// lets go, the throw is aimed at that target, so a lifted enemy or pot becomes a
// projectile you can hurl into something else.
//
// WHAT CAN BE LIFTED, and why it is deliberately narrow
//   - Props: an EXPLICIT list (sPacciLiftableProps). What each tool can move is the
//     only thing separating this from Magnesis and from Ultrahand; leaving it open
//     collapses all three into one ability.
//   - Enemies: only on their LAST point of health. Anything more generous turns the
//     lift into a delete button, and it is meant to be a finisher.
//   - And only within PACCI_LIFT_RADIUS. This is the deliberate difference from the
//     Switch Hook, which does not care how far its target is: the lift is short
//     ranged on purpose, so each tool owns a distinct band of reach.

// Enemies have to be worn down before Pacci can manhandle them. 1 HP proved far too
// strict — sharing this gate with Flip meant almost nothing in a room qualified and
// the move looked broken. Three is low enough to still be a finisher.
#define PACCI_LIFT_MAX_HP 3
#define PACCI_LIFT_RADIUS 220.0f
#define PACCI_LIFT_DIST 70.0f    // held this far in front of Link
#define PACCI_LIFT_HEIGHT 45.0f  // and this far above him
#define PACCI_LIFT_FOLLOW 0.55f  // weight of the OLD position; snappier than Ultrahand
#define PACCI_LIFT_THROW_SPEED 12.0f
// Nearly flat. The object is ALREADY held PACCI_LIFT_HEIGHT above Link, so adding a
// real upward kick on release lobbed it way over whatever you were aiming at — the
// arc has to start from that raised position, not from Link's feet.
#define PACCI_LIFT_THROW_VEL_Y 0.5f
#define PACCI_LIFT_GRAVITY -1.4f
#define PACCI_LIFT_FLIGHT_FRAMES 90 // give up and drop it after this long in flight

typedef struct {
    Actor* held;
    u8 thrown;
    s16 flightTimer;
    f32 origGravity;
    f32 origMinVelocityY;
    s16 origRoom;
    ActorFunc origUpdate;
    u32 origFlags;
    u8 frozeEnemy;
    PacciFx* fx; // pool entry that owns the collider doing the smashing
} PacciLift;

static PacciLift sLift = { 0 };

// The cane owns movement and collision while an enemy is held or thrown. Keeping
// a live no-op update also prevents the actor list from treating it as destroyed.
static void Pacci_LiftFrozenEnemyUpdate(Actor* actor, PlayState* play) {
}

u8 Pacci_IsLifting(void) {
    return (sLift.held != NULL) && !sLift.thrown;
}

// Enemies Pacci must never touch. Bosses are excluded by category; these are the
// ACTORCAT_ENEMY entries that would break if knocked over — arena minibosses with
// their own scripted state, and anything that is not really a body.
// Same bit the switch hook uses to let a custom actor opt in regardless of its category
// (item_switchhook.h). Declared with a guard rather than by including that header: this file is
// pulled into the cane's translation unit and must not start dragging item headers in with it.
#ifndef PACCI_FLAG_LIFTABLE
#define PACCI_FLAG_LIFTABLE (1 << 28)
#endif

static const s16 sPacciBlacklist[] = {
    ACTOR_EN_WIZ,      // Wizzrobe
    ACTOR_EN_DINOFOS,  // Dinolfos
    ACTOR_EN_BIGPO,    // Big Poe
    ACTOR_EN_BIGSLIME, // Gekko + Slime
    ACTOR_EN_WALLMAS,  // Wallmaster
    ACTOR_EN_FLOORMAS, // Floormaster
    ACTOR_EN_RD,       // Gibdo / Redead
    ACTOR_EN_FZ,       // Freezard
    // Invisible song spots. They have no model and no texture - their whole job is to sit
    // on a patch of ground and notice an ocarina - so grabbing one hauls nothing visible
    // around and, worse, carries the trigger away from the place it is supposed to be.
    ACTOR_EN_OKARINA_TAG,
    ACTOR_EN_OKARINA_EFFECT,
};

// Per-actor behaviour. Everything defaults to THROW; the table only lists actors
// that should do something else, which is what keeps it short and what makes it
// the place to extend when a new special case turns up.
typedef enum {
    PACCI_BEHAV_THROW = 0,  // flies, damages what it hits, breaks on impact
    PACCI_BEHAV_SNAP_FLOOR, // released onto the floor, and onto a switch if one is
                            // underneath — this is what makes a pushable block a
                            // puzzle solver instead of a thing you shove around
    PACCI_BEHAV_PLACE,      // stays exactly where released, no gravity
    PACCI_BEHAV_CARRY_ONLY, // movable, but its own logic keeps running (live bombs)
} PacciBehaviour;

typedef struct {
    s16 actorId;
    u8 behaviour;
} PacciBehaviourRow;

static const PacciBehaviourRow sPacciBehaviours[] = {
    { ACTOR_OBJ_OSHIHIKI, PACCI_BEHAV_SNAP_FLOOR }, // pushable block — reposition, do not hurl
    { ACTOR_OBJ_LIFT, PACCI_BEHAV_SNAP_FLOOR },     // platform slab
    { ACTOR_EN_BOM, PACCI_BEHAV_CARRY_ONLY },       // lit bomb: the fuse keeps burning
    // Uprooting a bomb flower takes the PLANT, so you can replant it where the
    // puzzle actually needs a bomb rather than carrying a lit one there.
    { ACTOR_EN_BOMBF, PACCI_BEHAV_PLACE },
};

u8 Pacci_BehaviourFor(Actor* actor) {
    if (actor != NULL) {
        for (u32 i = 0; i < ARRAY_COUNT(sPacciBehaviours); i++) {
            if (actor->id == sPacciBehaviours[i].actorId) {
                return sPacciBehaviours[i].behaviour;
            }
        }
    }
    return PACCI_BEHAV_THROW;
}

// BLACKLIST, not whitelist (user-locked): anything Pacci can plausibly grab is fair
// game, and only the listed exceptions are refused. The earlier whitelist plus an
// HP gate is what made Flip look broken — almost nothing in a room qualified.
// isDyna: this actor was handed to us by DynaPoly_GetActor, so it OWNS a registered collision
// surface. That is a stronger qualification than any category could be, and it is why the
// category gate below is skipped for it - see the note on the gate.
u8 Pacci_IsLiftableEx(Actor* actor, u8 isDyna) {
    if ((actor == NULL) || (actor->update == NULL)) {
        return 0;
    }
    if ((actor->id == ACTOR_PLAYER) || (actor->category == ACTORCAT_BOSS)) {
        return 0;
    }
    // Modelled on SwitchHook_CanSwap (item_switchhook.h): an ALLOW-list of categories plus an
    // opt-in actor flag, instead of a deny-list of symptoms. A deny-list only ever knows about
    // the junk somebody already ran into - dialogue triggers, song spots, spawn markers - and
    // every scene has more of it.
    //
    // It does NOT apply to dynapoly. Registering a collision surface is not something a talk
    // trigger or a spawn marker does; it is the definition of the thing Ultrahand exists to
    // pick up. And dynapoly is spread across far more categories than these four - elevators,
    // pushblocks, doors and cranes sit in SWITCH, DOOR, ITEMACTION and MISC depending on the
    // scene - so gating it by category broke grabbing scenery outright the moment this list
    // arrived. The list is for the ACTOR SCAN, which has no such qualification to lean on.
    if (!isDyna && !(actor->flags & PACCI_FLAG_LIFTABLE)) {
        if ((actor->category != ACTORCAT_ENEMY) && (actor->category != ACTORCAT_PROP) &&
            (actor->category != ACTORCAT_BG) && (actor->category != ACTORCAT_CHEST)) {
            return 0;
        }
    }
    // MASS_IMMOVABLE only disqualifies ENEMIES, where it marks the heavy minibosses
    // that should not be knocked around. It must NOT be applied to props: pots,
    // crates and pushable blocks all set MASS_IMMOVABLE deliberately (Obj_Oshihiki
    // does it right in its Init), so testing it across every category silently
    // excluded the exact objects the lift exists for.
    if ((actor->category == ACTORCAT_ENEMY) && (actor->colChkInfo.mass == MASS_IMMOVABLE)) {
        return 0;
    }
    for (u32 i = 0; i < ARRAY_COUNT(sPacciBlacklist); i++) {
        if (actor->id == sPacciBlacklist[i]) {
            return 0;
        }
    }
    // No draw function means nothing is rendered: talk triggers, spawn points, cutscene
    // and region markers. Their entire job is to sit invisibly and offer a conversation,
    // so grabbing one moves something the player cannot see and looks like a bug.
    if (actor->draw == NULL) {
        return 0;
    }
    if (Pacci_FindEntry(actor) != NULL) {
        return 0; // already flipped, petrified or held
    }
    // Glued parts stay VISIBLE to the targeting on purpose. Refusing them here used to
    // make a finished structure unclickable anywhere except its root, which is not how
    // you look at a thing you just built. The grab redirects to the root instead — see
    // Pacci_FuseRootOf in Pacci_CastUltrahand — so aiming at any piece takes the whole
    // assembly, models and all.
    return 1;
}

u8 Pacci_IsLiftable(Actor* actor) {
    return Pacci_IsLiftableEx(actor, 0);
}

static s32 Pacci_LiftFilter(Actor* actor) {
    return Pacci_IsLiftable(actor);
}

// Categories the lift scans, and the short leash it scans them with.
static const u8 sPacciLiftCats[3] = { ACTORCAT_ENEMY, ACTORCAT_PROP, ACTORCAT_BG };

static Actor* Pacci_ScanLiftable(PlayState* play) {
    return TargetSelect_ScanCats(play, sPacciLiftCats, 3, Pacci_LiftFilter, PACCI_LIFT_RADIUS,
                                 TARGETSEL_DEFAULT_CONE);
}

// ---------------------------------------------------------------------------
// IMPACT BURST
// ---------------------------------------------------------------------------
// When a thrown object lands, the damage is dealt by a REAL collider instead of by
// writing health directly. Writing colChkInfo.health by hand skipped every reaction
// an actor has to being hurt: no flinch, no death effect, no drop.
//
// The burst is owned by the PLAYER, and that is the whole trick. An actor AT can
// never hit that same actor AC, so a collider owned by the thrown object could
// damage what it landed on but never itself. Owned by Link, it damages both.
// Impact damage flavours. MM has no hammer and no aggregate DMG_ARROW, so both are
// spelled out. HEAVY is the Goron PUNCH rather than the pound: En_Ishi's bumpers in
// MM are 0x01C37FBE (small) and 0x01C37BB6 (boulder), and only the small one accepts
// GORON_POUND (bit 10) — bit 8, the punch, is the one BOTH take.
#define PACCI_DMG_LIGHT (DMG_NORMAL_ARROW | DMG_FIRE_ARROW | DMG_ICE_ARROW | DMG_LIGHT_ARROW)
#define PACCI_DMG_HEAVY DMG_GORON_PUNCH
#define PACCI_BURST_DAMAGE 8
#define PACCI_BURST_FRAMES 3
#define PACCI_BURST_RADIUS 34
#define PACCI_BURST_HEIGHT 40

static ColliderCylinder sPacciBurst;
static u8 sPacciBurstReady = 0;
static s16 sPacciBurstTimer = 0;

static ColliderCylinderInit sPacciBurstInit = {
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
        { 0xFFCFFFFF, 0x00, PACCI_BURST_DAMAGE },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { PACCI_BURST_RADIUS, PACCI_BURST_HEIGHT, -10, { 0, 0, 0 } },
};

// `heavy` picks the damage flavour: rocks take a HAMMER hit (the only thing En_Ishi
// breaks to), everything else takes an ARROW hit.
static void Pacci_BurstSpawn(PlayState* play, Player* player, Vec3f* pos, u8 heavy) {
    if (!sPacciBurstReady) {
        Collider_InitCylinder(play, &sPacciBurst);
        sPacciBurstReady = 1;
    }
    Collider_SetCylinder(play, &sPacciBurst, &player->actor, &sPacciBurstInit);
    sPacciBurst.elem.atDmgInfo.dmgFlags = heavy ? PACCI_DMG_HEAVY : PACCI_DMG_LIGHT;
    sPacciBurst.dim.pos.x = (s16)pos->x;
    sPacciBurst.dim.pos.y = (s16)pos->y;
    sPacciBurst.dim.pos.z = (s16)pos->z;
    sPacciBurstTimer = PACCI_BURST_FRAMES;
}

static void Pacci_BurstUpdate(PlayState* play) {
    if ((sPacciBurstTimer <= 0) || !sPacciBurstReady) {
        return;
    }
    sPacciBurstTimer--;
    CollisionCheck_SetAT(play, &play->colChkCtx, &sPacciBurst.base);
}

void Pacci_HighlightLiftTarget(PlayState* play) {
    Actor* target;

    if (Pacci_IsLifting()) {
        return;
    }
    target = Pacci_ScanLiftable(play);
    if (target != NULL) {
        PACCI_TINT_FLIP(target, 4);
    }
}

// Let go of whatever is held and put its physics back the way we found them.
static void Pacci_LiftLetGo(void) {
    Actor* actor = sLift.held;

    if ((actor != NULL) && (actor->update != NULL)) {
        if (sLift.frozeEnemy) {
            actor->update = sLift.origUpdate;
            actor->flags = sLift.origFlags;
        }
        actor->gravity = sLift.origGravity;
        actor->minVelocityY = sLift.origMinVelocityY;
        actor->room = (s8)sLift.origRoom;
        actor->colorFilterParams = 0;
    }
    if (sLift.fx != NULL) {
        sLift.fx->actor = NULL;
        sLift.fx->mode = PACCI_FX_NONE;
        sLift.fx = NULL;
    }
    sLift.held = NULL;
    sLift.thrown = 0;
    sLift.flightTimer = 0;
    sLift.origUpdate = NULL;
    sLift.origFlags = 0;
    sLift.frozeEnemy = 0;
}

void Pacci_LiftCancel(void) {
    PacciFlipVfx_Release();
    Pacci_LiftLetGo();
}

// Grab whatever Link is aiming at. Returns 1 if something was picked up.
u8 Pacci_LiftTryGrab(PlayState* play, Player* player) {
    Actor* target;
    PacciFx* fx;

    if (sLift.held != NULL) {
        return 0; // already holding
    }
    target = Pacci_ScanLiftable(play);
    if (target == NULL) {
        return 0;
    }

    // A pool entry is taken purely for its collider: while the object is in flight
    // it needs a live AT so it damages whatever it slams into.
    fx = Pacci_TakeEntry();
    if (fx == NULL) {
        return 0;
    }
    if (!fx->colliderReady) {
        Collider_InitCylinder(play, &fx->collider);
        fx->colliderReady = 1;
    }
    Collider_SetCylinder(play, &fx->collider, target, &sPacciFallColliderInit);
    fx->actor = target;
    fx->mode = PACCI_FX_NONE; // not a flip/stone — the lift drives it directly

    sLift.held = target;
    sLift.fx = fx;
    sLift.thrown = 0;
    sLift.flightTimer = 0;
    sLift.origGravity = target->gravity;
    sLift.origMinVelocityY = target->minVelocityY;
    sLift.origRoom = target->room;
    sLift.origUpdate = NULL;
    sLift.origFlags = 0;
    sLift.frozeEnemy = 0;
    if (target->category == ACTORCAT_ENEMY) {
        sLift.origUpdate = target->update;
        sLift.origFlags = target->flags;
        sLift.frozeEnemy = 1;
        target->update = Pacci_LiftFrozenEnemyUpdate;
        target->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;
    }
    target->room = -1; // a held object should survive a room change

    Audio_PlayActorSound2(target, PACCI_SFX_FLIP);
    PacciFlipVfx_StartLift(play, player, target);
    return 1;
}

// Throw it. Aimed at Link's lock-on target when he has one, otherwise straight
// ahead — which is what makes "lift this and hurl it into that" work.
void Pacci_LiftThrow(PlayState* play, Player* player) {
    Actor* actor = sLift.held;
    s16 yaw;

    if ((actor == NULL) || sLift.thrown) {
        return;
    }
    if (actor->update == NULL) {
        Pacci_LiftLetGo();
        return;
    }

    if ((player->focusActor != NULL) && (player->focusActor != actor) && (player->focusActor->update != NULL)) {
        yaw = Math_Vec3f_Yaw(&actor->world.pos, &player->focusActor->world.pos);
    } else {
        yaw = player->actor.shape.rot.y;
    }

    actor->world.rot.y = yaw;
    actor->shape.rot.y = yaw;
    actor->speed = PACCI_LIFT_THROW_SPEED;
    actor->velocity.y = PACCI_LIFT_THROW_VEL_Y;
    actor->gravity = PACCI_LIFT_GRAVITY;
    actor->colorFilterParams = 0;

    sLift.thrown = 1;
    sLift.flightTimer = PACCI_LIFT_FLIGHT_FRAMES;
    PacciFlipVfx_Release();
    Audio_PlayActorSound2(actor, PACCI_SFX_FLIP);
}

// Runs every frame the cane is equipped: holds the object aloft, then flies it.
void Pacci_LiftUpdate(PlayState* play, Player* player) {
    Actor* actor = sLift.held;

    // The impact burst outlives the throw by a few frames, so it ticks before the
    // "nothing held" early-out below.
    Pacci_BurstUpdate(play);

    if (actor == NULL) {
        return;
    }
    if (actor->update == NULL) { // it died in our hands
        Pacci_LiftLetGo();
        return;
    }

    if (!sLift.thrown) {
        // Float it in front of and above Link, following his facing.
        f32 targetX = player->actor.world.pos.x + (Math_SinS(player->actor.shape.rot.y) * PACCI_LIFT_DIST);
        f32 targetZ = player->actor.world.pos.z + (Math_CosS(player->actor.shape.rot.y) * PACCI_LIFT_DIST);
        f32 targetY = player->actor.world.pos.y + PACCI_LIFT_HEIGHT;
        f32 oldW = PACCI_LIFT_FOLLOW;
        f32 newW = 1.0f - oldW;

        actor->world.pos.x = (actor->world.pos.x * oldW) + (targetX * newW);
        actor->world.pos.y = (actor->world.pos.y * oldW) + (targetY * newW);
        actor->world.pos.z = (actor->world.pos.z * oldW) + (targetZ * newW);
        actor->velocity.x = actor->velocity.y = actor->velocity.z = 0.0f;
        actor->speed = 0.0f;
        actor->gravity = 0.0f;
        // Slow spin so a held object reads as "under your control", not stuck.
        actor->shape.rot.y += 0x400;
        PACCI_TINT_FLIP(actor, 8);
        return;
    }

    // In flight: fly it, and keep the AT live so it damages what it reaches.
    Actor_MoveWithGravity(actor);
    Actor_UpdateBgCheckInfo(play, actor, 5.0f, 15.0f, 0.0f, 0x85);

    if (sLift.fx != NULL && sLift.fx->colliderReady) {
        Collider_UpdateCylinder(actor, &sLift.fx->collider);
        CollisionCheck_SetAT(play, &play->colChkCtx, &sLift.fx->collider.base);
    }

    if (sLift.flightTimer > 0) {
        sLift.flightTimer--;
    }

    // Landed, hit a wall, connected with something, or ran out of flight time.
    if ((actor->bgCheckFlags & (BGCHECKFLAG_GROUND | BGCHECKFLAG_WALL)) || (sLift.flightTimer <= 0) ||
        ((sLift.fx != NULL) && (sLift.fx->collider.base.atFlags & AT_HIT))) {
        // Real damage, dealt by a real collider, to the thrown object AND to
        // whatever it slammed into. Let go FIRST, so the object is back under its
        // own update with its own AC live by the time the burst resolves.
        Vec3f impact = actor->world.pos;
        u8 heavy = (actor->id == ACTOR_EN_ISHI); // rocks only break to a hammer

        Audio_PlayActorSound2(actor, PACCI_SFX_FLIP_LAND);
        Pacci_LiftLetGo();
        Pacci_BurstSpawn(play, player, &impact, heavy);
    }
}

// ============================================================================
// STONE
// ============================================================================

// En_Ishi's small-rock shatter, reproduced 1:1 (same effect, gravity, life,
// object and display list) so a petrified enemy breaks like a real rock.
static void Pacci_StoneShatter(Actor* actor, PlayState* play) {
    Vec3f pos;
    Vec3f velocity;
    static const s16 sDebrisScales[] = { 12, 10, 10, 8, 8, 6 };

    for (u8 i = 0; i < ARRAY_COUNT(sDebrisScales); i++) {
        pos.x = ((Rand_ZeroOne() - 0.5f) * 8.0f) + actor->world.pos.x;
        pos.y = (Rand_ZeroOne() * 5.0f) + actor->world.pos.y + 5.0f;
        pos.z = ((Rand_ZeroOne() - 0.5f) * 8.0f) + actor->world.pos.z;

        Math_Vec3f_Copy(&velocity, &actor->velocity);
        if (actor->bgCheckFlags & BGCHECKFLAG_GROUND) {
            velocity.x *= 0.6f;
            velocity.y *= -0.3f;
            velocity.z *= 0.6f;
        } else if (actor->bgCheckFlags & BGCHECKFLAG_WALL) {
            velocity.x *= -0.5f;
            velocity.y *= 0.5f;
            velocity.z *= -0.5f;
        }
        velocity.x += (Rand_ZeroOne() - 0.5f) * 11.0f;
        velocity.y += (Rand_ZeroOne() * 7.0f) + 6.0f;
        velocity.z += (Rand_ZeroOne() - 0.5f) * 11.0f;

        EffectSsKakera_Spawn(play, &pos, &velocity, &pos, -420, ((s32)Rand_Next() > 0) ? 65 : 33, 30, 5, 0,
                             sDebrisScales[i], 3, 10, 40, -1, GAMEPLAY_FIELD_KEEP, gFieldSmallRockOpaDL);
    }

    Math_Vec3f_Copy(&pos, &actor->world.pos);
    func_800BBFB0(play, &pos, 60.0f, 3, 80, 60, true);

    Audio_PlaySoundGeneral(NA_SE_EV_ROCK_BROKEN, &actor->projectedPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    Item_DropCollectibleRandom(play, NULL, &actor->world.pos, 0x30);
}

static void Pacci_StoneUpdate(Actor* thisx, PlayState* play) {
    PacciFx* fx = Pacci_FindEntry(thisx);
    Player* player = GET_PLAYER(play);

    if (fx == NULL || player == NULL) {
        return;
    }

    // Keep it grey — the colour filter is a countdown, so it has to be re-armed.
    Actor_SetColorFilter(thisx, COLORFILTER_COLORFLAG_GRAY, 255, COLORFILTER_BUFFLAG_OPA, 20);

    switch (fx->phase) {
        case PACCI_STONE_PHASE_IDLE:
            if (Actor_HasParent(thisx, play)) {
                fx->phase = PACCI_STONE_PHASE_HELD;
                thisx->room = -1;
                break;
            }
            if (thisx->bgCheckFlags & BGCHECKFLAG_GROUND) {
                Math_StepToF(&thisx->speed, 0.0f, 1.0f);
                Actor_OfferCarry(thisx, play);
            } else {
                Math_StepToF(&thisx->speed, 0.0f, 0.2f);
            }
            Actor_MoveWithGravity(thisx);
            Actor_UpdateBgCheckInfo(play, thisx, 30.0f, 20.0f, 0.0f, 0x1D);
            break;

        case PACCI_STONE_PHASE_HELD:
            if (Actor_HasNoParent(thisx, play)) {
                fx->phase = PACCI_STONE_PHASE_THROWN;
                thisx->velocity.y = PACCI_STONE_THROW_VEL_Y;
                thisx->speed = PACCI_STONE_THROW_SPEED;
                thisx->world.rot.y = player->actor.shape.rot.y;
            }
            break;

        case PACCI_STONE_PHASE_THROWN:
            Actor_MoveWithGravity(thisx);
            Actor_UpdateBgCheckInfo(play, thisx, 30.0f, 20.0f, 0.0f, 0x1D);
            // A thrown rock shatters on the first solid thing it meets.
            if ((thisx->bgCheckFlags & (BGCHECKFLAG_GROUND | BGCHECKFLAG_WALL)) &&
                ((thisx->speed > PACCI_STONE_BREAK_SPEED) || (thisx->velocity.y < -PACCI_STONE_BREAK_SPEED))) {
                Pacci_StoneShatter(thisx, play);
                fx->actor = NULL;
                fx->mode = PACCI_FX_NONE;
                Actor_Kill(thisx);
                return;
            }
            break;
    }

    thisx->focus.pos = thisx->world.pos;
}

// A petrified enemy keeps its own skeleton draw (frozen in its last pose); the
// grey comes from the colour filter, which the engine applies around that draw.
u8 Pacci_CastStone(PlayState* play, Player* player) {
    Actor* target = Pacci_ScanEnemy(play);
    PacciFx* fx;

    if (target == NULL) {
        return 0;
    }
    fx = Pacci_TakeEntry();
    if (fx == NULL) {
        return 0;
    }

    // The shatter debris lives in gameplay_field_keep; ask for it now so it is
    // resident by the time the rock actually breaks.
    if (Object_GetSlot(&play->objectCtx, GAMEPLAY_FIELD_KEEP) < 0) {
        Object_SpawnPersistent(&play->objectCtx, GAMEPLAY_FIELD_KEEP);
    }

    fx->actor = target;
    fx->mode = PACCI_FX_STONE;
    fx->phase = PACCI_STONE_PHASE_IDLE;
    fx->timer = 0;
    fx->origUpdate = target->update;
    fx->origDraw = target->draw;
    fx->origFlags = target->flags;
    fx->origGravity = target->gravity;
    fx->origMinVelocityY = target->minVelocityY;
    fx->origSpeed = target->speed;
    fx->origShapeRot = target->shape.rot;
    fx->origWorldRot = target->world.rot;
    fx->origRoom = target->room;
    fx->origMass = target->colChkInfo.mass;

    target->update = Pacci_StoneUpdate;
    target->gravity = PACCI_STONE_GRAVITY;
    target->minVelocityY = PACCI_STONE_MIN_VEL_Y;
    target->speed = 0.0f;
    target->colChkInfo.mass = MASS_HEAVY;
    // A rock presses switches and can no longer be locked on to as an enemy.
    target->flags |= ACTOR_FLAG_CAN_PRESS_SWITCHES;
    target->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    // Same reason as Flip: our update is the only thing moving it now.
    target->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;

    Actor_SetColorFilter(target, COLORFILTER_COLORFLAG_GRAY, 255, COLORFILTER_BUFFLAG_OPA, 20);
    Actor_PlaySfx(target, NA_SE_EV_STONE_STATUE_OPEN);
    return 1;
}

// Ultrahand mode state. Declared up here because Pacci_UpdateUltrahand — further
// down — reads the height offset the mode drives, while the mode's own section
// sits below it.
typedef struct {
    u8 active;     // in the mode at all
    f32 heightOff; // vertical offset from the aim line, driven by R + D-up/down
    f32 sideOff;   // sideways offset, perpendicular to the aim, from R + D-left/right
    u8 prevDpad;   // our own edge detection; see the note in the cane's L/R cycler
    // Detach is a fast left-right shake of the stick, standing in for TotK's right-stick
    // wiggle. Counting DIRECTION FLIPS rather than raw deflection is what separates a shake
    // from simply running sideways, which the stick is also still doing.
    s8 wiggleDir;
    u8 wiggleFlips;
    s16 wiggleTimer;
    s16 summonHold; // frames the cane's C button has been held, for the recall
} PacciUhMode;

static PacciUhMode sUhMode = { 0, 0.0f, 0 };

// ============================================================================
// ULTRAHAND
// ============================================================================

typedef struct {
    Actor* held;
    u8 dropping;
    s16 dropTimer;
    f32 distance;
    f32 origGravity;
    f32 origMinVelocityY;
    s16 origRoom;
    // The chosen pose stays fixed relative to the object-to-Link radial line.
    //
    // Movement keeps the same face presented to Link; only rotation input changes it.
    Vec3s baseRot;
    Vec3s grabRot; // orientation at the moment of the grab, for the Z reset
    s16 faceOffsetYaw;
    // The yaw the CARRY POINT orbits at, which is NOT Link's aim: it chases it at a capped
    // angular rate. Building the hold position straight from focus.rot.y meant a fast turn
    // moved the anchor to the far side instantly, and the positional lerp then walked the
    // object there along the CHORD — straight through Link. Stepping the angle instead makes
    // it sweep the arc at constant radius, so it goes around him however hard you spin.
    s16 carryYaw;
    // How fast the body was actually travelling while carried, smoothed. Handed over as
    // velocity when you let go, so a release inherits the motion you gave it.
    Vec3f carryVel;
    s16 vfxAge;
    // Held objects are frozen: their own update is what submits their OC collider,
    // and with it live the object shoves Link around while he is carrying it.
    ActorFunc origUpdate;
} PacciUltrahand;

static PacciUltrahand sUltrahand = { 0 };
static Actor* sUhHighlightTarget = NULL;

// z_player_lib.c — re-latch Link's model group. The group is sampled when the item
// ACTION changes and cached in nextModelGroup, so changing what
// ExtPlayer_GetActionModelGroup returns mid-hold has no effect on its own: nothing
// asks again. Grabbing and releasing have to ask for it explicitly.
s32 Player_ActionToModelGroup(Player* this, s32 actionParam);
void Player_SetModels(Player* this, s32 modelGroup);

// -- the tint ------------------------------------------------------------------
// Zonai green, on the actor's own model. NOT Actor_SetColorFilter - that offers white, red
// and blue and nothing else, which is why every attempt to get green out of it produced
// either a petrified-looking wash or the wrong colour entirely.
//
// LUS exposes grayscale as RSP STATE (G_SETGRAYSCALE), separate from the combiner: the
// texture is desaturated and multiplied by a colour. Because it is state and not a combiner
// setting, the actor's own material setup cannot clobber it halfway through its display
// list the way a prim or env colour would - which is the whole reason this works on an
// arbitrary actor whose draw we do not control. It is the same mechanism the ports already
// use to recolour rupees (z_en_item00.c) and to grey out kaleido cells.
//
// Applied by swapping actor->draw for a wrapper, the same trick the carry already plays on
// actor->update. The table is rebuilt from scratch every frame from whatever should be lit
// right now, so anything that stops being a target has its own draw back on the next one.
#define PACCI_UH_TINT_SLOTS 8
// Bright, slightly yellow-green: grayscale x colour keeps the model's own light and shade,
// so the multiplier has to be bright or the object just goes dark green. MIX is the blend
// against the untinted texture - 255 is fully recoloured.
#define PACCI_UH_TINT_R 110
#define PACCI_UH_TINT_G 255
#define PACCI_UH_TINT_B 165
#define PACCI_UH_TINT_MIX 255

// The tether, layered so a flat untextured tube reads as light: a wide soft halo, a mid
// body, and a thin near-white core on top. Same idea as the reference - the bright streaks
// are almost white and it is the haze around them that carries the colour.
#define PACCI_UH_FLOW_HALO_R 30
#define PACCI_UH_FLOW_HALO_G 235
#define PACCI_UH_FLOW_HALO_B 165
#define PACCI_UH_FLOW_MID_R 120
#define PACCI_UH_FLOW_MID_G 255
#define PACCI_UH_FLOW_MID_B 200
#define PACCI_UH_FLOW_CORE_R 225
#define PACCI_UH_FLOW_CORE_G 255
#define PACCI_UH_FLOW_CORE_B 240

static struct {
    Actor* actor;
    ActorFunc origDraw;
} sUhTint[PACCI_UH_TINT_SLOTS];
static u8 sUhTintCount = 0;

static ActorFunc Pacci_UhTintFind(Actor* actor) {
    for (u8 i = 0; i < sUhTintCount; i++) {
        if (sUhTint[i].actor == actor) {
            return sUhTint[i].origDraw;
        }
    }
    return NULL;
}

static void Pacci_UhTintedDraw(Actor* thisx, PlayState* play) {
    ActorFunc orig = Pacci_UhTintFind(thisx);

    if (orig == NULL) {
        return; // dropped from the table between the swap and the draw pass
    }

    OPEN_DISPS(play->state.gfxCtx);
    // Both buffers. Which one an actor draws into is its own business and plenty use both,
    // so setting it on one and not the other tints half a model.
    gDPSetGrayscaleColor(POLY_OPA_DISP++, PACCI_UH_TINT_R, PACCI_UH_TINT_G, PACCI_UH_TINT_B,
                         PACCI_UH_TINT_MIX);
    gSPGrayscale(POLY_OPA_DISP++, true);
    gDPSetGrayscaleColor(POLY_XLU_DISP++, PACCI_UH_TINT_R, PACCI_UH_TINT_G, PACCI_UH_TINT_B,
                         PACCI_UH_TINT_MIX);
    gSPGrayscale(POLY_XLU_DISP++, true);
    CLOSE_DISPS(play->state.gfxCtx);

    orig(thisx, play);

    // Turn it off again, or every actor drawn after this one in the same pass comes out green.
    OPEN_DISPS(play->state.gfxCtx);
    gSPGrayscale(POLY_OPA_DISP++, false);
    gSPGrayscale(POLY_XLU_DISP++, false);
    CLOSE_DISPS(play->state.gfxCtx);
}

// Hand every tinted actor its own draw back.
static void Pacci_UhTintClear(void) {
    for (u8 i = 0; i < sUhTintCount; i++) {
        Actor* actor = sUhTint[i].actor;

        // update == NULL means it died while tinted, and its draw pointer died with it.
        if ((actor != NULL) && (actor->update != NULL) && (actor->draw == Pacci_UhTintedDraw)) {
            actor->draw = sUhTint[i].origDraw;
        }
        sUhTint[i].actor = NULL;
        sUhTint[i].origDraw = NULL;
    }
    sUhTintCount = 0;
}

// Light this actor for THIS frame. Safe to call more than once on the same actor.
static void Pacci_UhTintAdd(Actor* actor) {
    if ((actor == NULL) || (actor->update == NULL) || (actor->draw == NULL) ||
        (actor->draw == Pacci_UhTintedDraw) || (sUhTintCount >= PACCI_UH_TINT_SLOTS)) {
        return;
    }
    sUhTint[sUhTintCount].actor = actor;
    sUhTint[sUhTintCount].origDraw = actor->draw;
    sUhTintCount++;
    actor->draw = Pacci_UhTintedDraw;
}

static void Pacci_RefreshPlayerPose(Player* player) {
    if (player != NULL) {
        Player_SetModels(player, Player_ActionToModelGroup(player, player->itemAction));
    }
}


// A held object does nothing on its own — the cane drives its transform, and its
// colliders must stay out of the collision pass so it cannot push the player.
static void Pacci_UltrahandHeldUpdate(Actor* thisx, PlayState* play) {
}

// Lives with the rest of the VFX, far below; the release path has to be able to put the
// light out and it runs long before that.
static void Pacci_UhLightOff(PlayState* play);


u8 Pacci_IsHoldingUltrahand(void) {
    return (sUltrahand.held != NULL) && !sUltrahand.dropping;
}

static void Pacci_UltrahandLetGo(void) {
    Actor* actor = sUltrahand.held;

    if (actor != NULL && actor->update != NULL) {
        if (sUltrahand.origUpdate != NULL) {
            actor->update = sUltrahand.origUpdate;
        }
        actor->gravity = sUltrahand.origGravity;
        actor->minVelocityY = sUltrahand.origMinVelocityY;
        actor->velocity.y = 0.0f;
        actor->room = (s8)sUltrahand.origRoom;
        actor->colorFilterParams = 0;
    }
    sUltrahand.held = NULL;
    sUltrahand.dropping = 0;
    sUltrahand.dropTimer = 0;
    sUltrahand.origUpdate = NULL;
    sUltrahand.vfxAge = 0;
    sUhHighlightTarget = NULL;
    Pacci_UhLightOff(gPlayState); // the draw hook stops running the moment nothing is held
    Pacci_UhTintClear();
    Pacci_RefreshPlayerPose(GET_PLAYER(gPlayState)); // hookshot hold -> empty-handed
}

// Drive a fall in progress no matter what the player is holding, or whether he is holding
// anything at all. Called every frame from CustomItems_Update, which is the only place in this
// mod that keeps running after the cane leaves Link's hand.
//
// Without it a release was only animated while the cane was still out and the mode still up,
// and every other way of letting go - B out of the mode, drawing the sword, reaching for
// another item - dropped the object where it floated. A structure left hanging in the air with
// live collision is the "rompe colliders" case: its surfaces stay registered at a height
// nothing can reach, and Link walks into them.
void Pacci_UltrahandDropTick(PlayState* play) {
    if ((play == NULL) || (sUltrahand.held == NULL) || !sUltrahand.dropping) {
        return;
    }
    Pacci_UpdateUltrahand(play, GET_PLAYER(play));
}

void Pacci_HighlightUltrahandTarget(PlayState* play) {
    sUhHighlightTarget = NULL;
    // sUltrahand.held, not Pacci_IsHoldingUltrahand(): that one goes false the moment a drop
    // starts, and marking a new candidate while the last one is still falling advertises a
    // grab that Pacci_CastUltrahand now correctly refuses.
    if (sUltrahand.held != NULL) {
        return;
    }
    // Nothing is held, so the ONLY thing that may be lit right now is a candidate. Clearing
    // here rather than at the top of the function is deliberate: this runs from the cane's own
    // per-frame handler as well as from the mode, and clearing before the early return above
    // would wipe the held object's tint on every frame a grab happened outside the mode.
    Pacci_UhTintClear();

    Actor* target = Pacci_ResolveUltrahandTarget(play, GET_PLAYER(play));

    if (target != NULL) {
        // What A would take, marked the same way it will be marked once taken.
        target->colorFilterParams = 0;
        Pacci_UhTintAdd(target);
        sUhHighlightTarget = target;
    }
}

// Line-test along Link's aim and return the DynaPoly actor owning whatever surface
// it lands on, or NULL for plain scenery. Same idea as the remote's projectile,
// minus the projectile: it line-tested during flight and read the bgId out of the
// hit, which is what let it latch onto bg actors.
static Actor* Pacci_UltrahandRaycastDyna(PlayState* play, Player* player) {
    Vec3f from = player->actor.world.pos;
    Vec3f to;
    Vec3f hit;
    CollisionPoly* poly = NULL;
    s32 bgId = BGCHECK_SCENE;
    f32 distXZ = Math_CosS(player->actor.focus.rot.x) * PACCI_UH_DIST_MAX;
    f32 distY = Math_SinS(player->actor.focus.rot.x) * PACCI_UH_DIST_MAX;

    from.y += 40.0f; // from the chest, not the feet
    to.x = from.x + (Math_SinS(player->actor.focus.rot.y) * distXZ);
    to.z = from.z + (Math_CosS(player->actor.focus.rot.y) * distXZ);
    to.y = from.y - distY;

    // ProjectileLineTest, not EntityLineTest1: the entity variant resolves against
    // scene geometry and does not report the bg actor behind a dynapoly surface, so
    // bgId came back as BGCHECK_SCENE and DynaPoly_GetActor always returned NULL —
    // which is why scenery never got grabbed. This is the one the remote used, and
    // its signature is identical.
    // Cast repeatedly instead of once. The FIRST surface in front of Link is very often the
    // thing he is already holding — it hangs on the aim line by construction — or a piece
    // welded to it. One cast therefore answered "the held object", the preview rejected it as
    // target == held, and no weld was ever offered. That is the "I am clearly lined up and it
    // will not stick" case. Each time the ray lands on our own assembly it restarts just past
    // that surface and keeps going; anything else ends the search, so this cannot reach
    // through a wall.
    for (s32 attempt = 0; attempt < 4; attempt++) {
        DynaPolyActor* dyna;
        Actor* found;
        f32 dx;
        f32 dy;
        f32 dz;
        f32 len;

        if (!BgCheck_ProjectileLineTest(&play->colCtx, &from, &to, &hit, &poly, true, true, true, true, &bgId)) {
            break;
        }
        dyna = DynaPoly_GetActor(&play->colCtx, bgId);
        found = ((dyna != NULL) && (dyna->actor.update != NULL)) ? &dyna->actor : NULL;

        if (found == NULL) {
            break; // plain scenery: whatever is behind it is behind a wall
        }
        if ((found != sUltrahand.held) && (Pacci_FuseRootOf(found) != sUltrahand.held)) {
            if (Pacci_IsLiftableEx(found, 1)) { // dynapoly by construction
                return found;
            }
            // "Not liftable" is not the same as "nothing here". Returning NULL let a single
            // unliftable dynapoly surface in front hide every grabbable one behind it - and in
            // a graveyard, or any room built out of bg actors, that is most of what the aim
            // line passes through on the way to the thing you are pointing at. Step past it,
            // exactly as we step past our own assembly.
        }

        // It was our own piece. Resume from just past where the ray hit it.
        dx = to.x - from.x;
        dy = to.y - from.y;
        dz = to.z - from.z;
        len = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
        if (len < 1.0f) {
            break;
        }
        from.x = hit.x + ((dx / len) * 3.0f);
        from.y = hit.y + ((dy / len) * 3.0f);
        from.z = hit.z + ((dz / len) * 3.0f);
    }
    return NULL;
}

// The ONE place that decides what Ultrahand would take. Both the grab and the
// highlight go through it, so what you see marked is always exactly what A gets.
// They used to resolve separately — the grab raycast for dynapoly first and then
// fell back to the actor scan, while the highlight only ever ran the actor scan —
// which is why scenery could be grabbed without ever being marked, and why marked
// objects sometimes were not the one taken.
// The hole that let dialogue triggers, song spots and spawners be picked up. The raycast
// half of the resolve has always run Pacci_IsLiftable on what it finds, but the fallback
// actor scan went straight to TargetSelect_IsCommonTarget, which only asks "is this a
// targetable category" - it knows nothing about draw functions or blacklists. So anything
// invisible in PROP or NPC sailed through the one path that never checked.
static s32 Pacci_UhTargetFilter(Actor* actor) {
    return TargetSelect_IsCommonTarget(actor) && Pacci_IsLiftable(actor);
}

static Actor* Pacci_ResolveUltrahandTarget(PlayState* play, Player* player) {
    Actor* target = Pacci_UltrahandRaycastDyna(play, player);

    if (target == NULL) {
        target = TargetSelect_Scan(play, Pacci_UhTargetFilter);
    }
    return target;
}

// Hand the body back to physics and let it fall. Split out because there are three ways to
// let go - the cast, A inside the mode, and leaving the mode - and only the first of them
// used to run any physics at all. The other two called Pacci_UltrahandLetGo directly, which
// restores the actor's own update on the spot: a crate released mid-air simply stopped there
// if its own update had no gravity of its own.
//
// NOT used by Pacci_DropUltrahand. That one runs on UNEQUIP, and the fall is driven from
// Pacci_UpdateUltrahand, which the cane stops calling the moment it leaves Link's hand -
// so a drop begun there would freeze on its first frame. Unequipping stays an instant let go.
static void Pacci_UltrahandBeginDrop(void) {
    Actor* actor = sUltrahand.held;
    f32 speed;

    if ((actor == NULL) || sUltrahand.dropping) {
        return;
    }
    sUltrahand.dropping = 1;
    sUltrahand.dropTimer = PACCI_UH_DROP_TIMEOUT;
    actor->colorFilterParams = 0;
    actor->gravity = PACCI_UH_DROP_GRAVITY;
    actor->minVelocityY = PACCI_UH_DROP_MIN_VEL_Y;
    actor->velocity = sUltrahand.carryVel;

    speed = sqrtf((actor->velocity.x * actor->velocity.x) + (actor->velocity.y * actor->velocity.y) +
                  (actor->velocity.z * actor->velocity.z));
    if (speed > PACCI_UH_THROW_MAX) {
        f32 scale = PACCI_UH_THROW_MAX / speed;

        actor->velocity.x *= scale;
        actor->velocity.y *= scale;
        actor->velocity.z *= scale;
    }
    Audio_PlaySoundGeneral(NA_SE_SY_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

// Everything a grab sets up, with no opinion about how the target was chosen. The summon
// needs exactly this and has no aim to resolve - it just built the thing itself.
static void Pacci_UltrahandTake(Player* player, Actor* target) {
    f32 dx = target->world.pos.x - player->actor.world.pos.x;
    f32 dy = target->world.pos.y - player->actor.world.pos.y;
    f32 dz = target->world.pos.z - player->actor.world.pos.z;

    sUltrahand.held = target;
    sUltrahand.dropping = 0;
    sUltrahand.dropTimer = 0;
    sUltrahand.origGravity = target->gravity;
    sUltrahand.origMinVelocityY = target->minVelocityY;
    sUltrahand.origRoom = target->room;
    sUltrahand.baseRot = target->shape.rot;
    sUltrahand.grabRot = target->shape.rot;
    sUltrahand.carryYaw = player->actor.focus.rot.y; // start on the aim, no opening swing
    sUltrahand.carryVel.x = 0.0f;
    sUltrahand.carryVel.y = 0.0f;
    sUltrahand.carryVel.z = 0.0f;
    sUltrahand.faceOffsetYaw =
        target->shape.rot.y - Math_Vec3f_Yaw(&target->world.pos, &player->actor.world.pos);
    sUltrahand.vfxAge = 0;
    sUhHighlightTarget = NULL;
    sUltrahand.origUpdate = target->update;
    target->update = Pacci_UltrahandHeldUpdate;
    sUltrahand.distance = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
    sUltrahand.distance = CLAMP(sUltrahand.distance, PACCI_UH_DIST_MIN, PACCI_UH_DIST_MAX);
    target->room = -1; // held objects should survive a room change

    Actor_PlaySfx(target, NA_SE_SY_GET_ITEM);
    Pacci_RefreshPlayerPose(player); // empty-handed -> hookshot hold
}

u8 Pacci_CastUltrahand(PlayState* play, Player* player) {
    // Already holding something -> the cast is the release.
    if (Pacci_IsHoldingUltrahand()) {
        Pacci_UltrahandBeginDrop();
        return 1;
    }

    // A drop in progress still owns sUltrahand.held: the body is falling, but it is ours until
    // it lands. Grabbing something else here overwrote that pointer and left the falling object
    // orphaned with our no-op update on it - frozen in mid-air, permanently. It also kept
    // blocking the aim raycast, which skips whatever sUltrahand.held is, so the next dynapoly
    // behind it stopped being detectable. Wait for the landing instead.
    if (sUltrahand.held != NULL) {
        return 0;
    }

    // DynaPoly FIRST, then loose actors. This is the one thing the remote's version
    // could do that ours could not: it grabs scenery. It finds it by line-testing
    // the world and asking DynaPoly_GetActor which bg actor owns the surface it hit
    // — the shared actor selector can never see those, because it walks the actor
    // categories and TargetSelect_IsCommonTarget covers ENEMY/PROP/CHEST/NPC only.
    // Platforms, moving blocks and structural pieces all live in ACTORCAT_BG with
    // their collision registered as dynapoly, so they were simply invisible to us.
    Actor* target = Pacci_ResolveUltrahandTarget(play, player);

    if (target == NULL) {
        return 0;
    }
    // Pointed at a piece of something already built? Take the structure, not the piece.
    // The parts are frozen and driven from the root, so holding a part directly would
    // move nothing at all — the visible failure was "I grabbed it and it did not come".
    if (Pacci_FuseRootOf(target) != NULL) {
        target = Pacci_FuseRootOf(target);
    }

    Pacci_UltrahandTake(player, target);
    return 1;
}

// Defined down in the FUSION section, because it needs the oriented box. Declared here
// because the fall needs it and the fall is part of the carry.
static u8 Pacci_UhGroundUnder(PlayState* play, Actor* actor, f32* outY);

// Last frame this ran, so it cannot run twice in one. There are now three callers - the mode,
// the cane's own handler, and the global drop tick - and gravity applied twice in a frame is a
// fall at double speed that punches through the floor before the landing probe sees it.
static u32 sUhLastTick = 0xFFFFFFFF;

void Pacci_UpdateUltrahand(PlayState* play, Player* player) {
    Actor* actor = sUltrahand.held;
    Input* input = &play->state.input[0];

    if (actor == NULL) {
        return;
    }
    if (sUhLastTick == play->gameplayFrames) {
        return;
    }
    sUhLastTick = play->gameplayFrames;
    if (actor->update == NULL) { // it died while we held it
        sUltrahand.held = NULL;
        sUltrahand.dropping = 0;
        return;
    }

    if (!sUltrahand.dropping) {
        s16 playerFacingYaw;
        s16 targetFacingYaw;
        Vec3f prevWorld = actor->world.pos; // sampled before the carry moves it, for inertia
        // NO D-pad handling here. This function is the carry physics only; the mode
        // (Pacci_UltrahandModeUpdate) owns every button and calls this at the end of
        // its frame. The port's original D-pad block used to live here — reading the
        // pad WITHOUT the L modifier — so it ran alongside the mode's and overrode
        // it: L + D-left/right looked like push/pull instead of the X rotation the
        // mode had just applied, because this ran second and won.
        //
        // Hold it where Link is looking (pitch included), easing in so it glides.
        f32 distXZ = Math_CosS(player->actor.focus.rot.x) * sUltrahand.distance;
        f32 distY = Math_SinS(player->actor.focus.rot.x) * sUltrahand.distance;
        // sideOff runs perpendicular to the aim (yaw + 90 degrees), so R + D-left/right slides the
        // object across your view instead of along it — the plane D-up/down does not cover.
        f32 sideX;
        f32 sideZ;
        f32 targetX;
        f32 targetZ;

        // Chase Link's aim at a capped rate. s16 subtraction already wraps to the shortest
        // signed difference, so this needs no angle normalisation of its own.
        {
            s16 yawDelta = player->actor.focus.rot.y - sUltrahand.carryYaw;

            if (yawDelta > PACCI_UH_TURN_RATE) {
                yawDelta = PACCI_UH_TURN_RATE;
            } else if (yawDelta < -PACCI_UH_TURN_RATE) {
                yawDelta = -PACCI_UH_TURN_RATE;
            }
            sUltrahand.carryYaw += yawDelta;
        }
        // Everything positional hangs off carryYaw, never off the raw aim. That is the whole
        // arc: the anchor can only ever be one capped step further round the circle.
        sideX = Math_SinS(sUltrahand.carryYaw + 0x4000) * sUhMode.sideOff;
        sideZ = Math_CosS(sUltrahand.carryYaw + 0x4000) * sUhMode.sideOff;
        targetX = (Math_SinS(sUltrahand.carryYaw) * distXZ) + player->actor.world.pos.x + sideX;
        targetZ = (Math_CosS(sUltrahand.carryYaw) * distXZ) + player->actor.world.pos.z + sideZ;
        // The mode's L + D-up/down offset rides on top of the aim line.
        f32 targetY = -distY + player->actor.world.pos.y + sUhMode.heightOff;

        f32 oldW = PACCI_UH_FOLLOW_WEIGHT;
        f32 newW = 1.0f - oldW;

        actor->world.pos.x = (actor->world.pos.x * oldW) + (targetX * newW) - actor->colChkInfo.displacement.x;
        actor->world.pos.y = (actor->world.pos.y * oldW) + (targetY * newW) - actor->colChkInfo.displacement.y;
        actor->world.pos.z = (actor->world.pos.z * oldW) + (targetZ * newW) - actor->colChkInfo.displacement.z;
        actor->velocity.x = actor->velocity.y = actor->velocity.z = 0.0f;

        Actor_UpdateBgCheckInfo(play, actor, 0.0f, 0.0f, 0.0f, 4);
        // CLAMP to the floor, do not nudge upward. The old "+= 1.0f while grounded" was a
        // ratchet: on a slope the object stays flagged as grounded every single frame, so it
        // gained a unit per frame and shot into the sky. Snapping to floorHeight only when it
        // is actually below it does the job the nudge was meant to do — keep it out of the
        // ground — with no way to accumulate.
        if ((actor->bgCheckFlags & BGCHECKFLAG_GROUND) && (actor->world.pos.y < actor->floorHeight)) {
            actor->world.pos.y = actor->floorHeight;
        }
        // OC displacement is what Link's own body pushed into it. It was already subtracted
        // above; leaving it set lets it build up frame after frame while he stands against the
        // thing he is carrying, which is the other half of the launch.
        actor->colChkInfo.displacement.x = 0.0f;
        actor->colChkInfo.displacement.y = 0.0f;
        actor->colChkInfo.displacement.z = 0.0f;
        // Release inertia, measured from how far the body ACTUALLY moved and smoothed. The
        // carry eases toward its anchor, so the raw delta on the single frame you let go is
        // either near zero (you had already stopped) or a spike (you had just whipped round);
        // neither is the throw the player meant.
        sUltrahand.carryVel.x = (sUltrahand.carryVel.x * 0.65f) + ((actor->world.pos.x - prevWorld.x) * 0.35f);
        sUltrahand.carryVel.y = (sUltrahand.carryVel.y * 0.65f) + ((actor->world.pos.y - prevWorld.y) * 0.35f);
        sUltrahand.carryVel.z = (sUltrahand.carryVel.z * 0.65f) + ((actor->world.pos.z - prevWorld.z) * 0.35f);
        playerFacingYaw = Math_Vec3f_Yaw(&actor->world.pos, &player->actor.world.pos);
        targetFacingYaw = playerFacingYaw + sUltrahand.faceOffsetYaw +
                          (sUltrahand.baseRot.y - sUltrahand.grabRot.y);
        actor->shape.rot.x = sUltrahand.baseRot.x;
        Math_SmoothStepToS(&actor->shape.rot.y, targetFacingYaw, 4, 0x1000, 0x20);
        actor->shape.rot.z = sUltrahand.baseRot.z;
        actor->world.rot = actor->shape.rot;

        // THE TINT. Re-registered every frame for as long as the object is held; the table is
        // wiped at the top of the mode's frame, so this is what keeps it lit.
        actor->colorFilterParams = 0; // nothing from the engine filter fights the grayscale
        Pacci_UhTintAdd(actor);
        if (sUltrahand.vfxAge < 0x7FFF) {
            sUltrahand.vfxAge++;
        }
        return;
    }

    // Released: real gravity, and a landing decided by the body's own footprint.
    actor->gravity = PACCI_UH_DROP_GRAVITY;
    actor->velocity.y += actor->gravity;
    if (actor->velocity.y < PACCI_UH_DROP_MIN_VEL_Y) {
        actor->velocity.y = PACCI_UH_DROP_MIN_VEL_Y;
    }
    // Integrated by hand rather than through the engine's move-with-gravity helper. That one
    // drives XZ from
    // speedXZ along world.rot.y, and world.rot here is the ORIENTATION YOU CHOSE for the
    // object - so a released crate flew wherever its front face happened to be pointing
    // instead of where you had been swinging it.
    actor->world.pos.x += actor->velocity.x;
    actor->world.pos.y += actor->velocity.y;
    actor->world.pos.z += actor->velocity.z;
    actor->velocity.x *= PACCI_UH_DROP_DRAG;
    actor->velocity.z *= PACCI_UH_DROP_DRAG;
    Actor_UpdateBgCheckInfo(play, actor, 0.0f, 0.0f, 0.0f, 4);
    // The parts have to be where the root says they are BEFORE the footprint is measured,
    // or the probe reads last frame's shape.
    Pacci_FuseFollow(play);

    {
        f32 restY;
        u8 landed = 0;

        if (Pacci_UhGroundUnder(play, actor, &restY)) {
            if (actor->world.pos.y <= restY) {
                actor->world.pos.y = restY;
                landed = 1;
            }
        } else if (actor->bgCheckFlags & BGCHECKFLAG_GROUND) {
            // No usable box: the single-point check is all there is. Better than nothing,
            // and it is the old behaviour, so nothing that used to land stops landing.
            landed = 1;
        }

        if (sUltrahand.dropTimer > 0) {
            sUltrahand.dropTimer--;
        }
        if (landed) {
            actor->velocity.x = 0.0f;
            actor->velocity.y = 0.0f;
            actor->velocity.z = 0.0f;
            Pacci_UltrahandLetGo();
        } else if ((sUltrahand.dropTimer <= 0) ||
                   ((player->actor.world.pos.y - actor->world.pos.y) >= PACCI_UH_ABANDON_DROP)) {
            Pacci_UltrahandLetGo();
        }
    }
}

// ============================================================================
// FUSION
// ============================================================================
//
// Gluing makes the HELD object the ROOT of an assembly. Every piece stuck to it is
// stored as a position and rotation IN THE ROOT'S LOCAL FRAME, and from then on the
// root is the only thing anybody drives: each frame every part is rebuilt as
// root_pos + rotate(offset, root_rot). Move or spin the root and the structure moves
// as one solid. Grab the root again later and the whole thing comes with it.
//
// WHERE pieces may join is not free-form. Each actor gets an oriented box, and the
// box offers 27 weld points: its 8 corners, the 12 edge midpoints, the 6 face
// centres and the centre itself. A weld happens at the CLOSEST pair of points
// between the two boxes, which is what makes a cube land flush on another cube's
// corner or edge instead of floating at whatever sub-unit offset it happened to be
// at. The box comes from real collision wherever there is any: a dynapoly actor's
// CollisionHeader bounds, otherwise the actor's collision cylinder.
//
// An actor with NO collision of its own has no surface to weld along, so it is
// restricted to CORNERS, and the thing it pins to has to be genuine dynapoly.
//
// The collision is NOT merged, and that is deliberate. There used to be a runtime
// CollisionHeader builder here: it unregistered each part's bg actor and folded its polys
// into one surface owned by the root. It existed for one reason - parts were frozen, so
// their own colliders never repositioned - and once a part runs its own update again that
// reason is gone.
//
// Keeping it was actively harmful. Unregistering a part's dynapoly takes away the thing that
// IDENTIFIES it: DynaPoly_GetActor on any surface of the assembly answered "the root", so
// anything that finds an actor through its collision stopped finding the part. Bonk a tree
// glued to another tree and the game saw the other tree.
//
// So an assembly is N actors, each entirely itself - own update, own collision, own colliders -
// whose TRANSFORMS are driven together. A dynapoly actor re-transforms its own header from its
// actor SRT every frame, so writing world.pos and shape.rot moves its collision with it. The
// cost is that two pieces can leave a seam between them; the benefit is that a piece stuck to
// something is still the piece.
#define PACCI_FUSE_MAX_PARTS 6
#define PACCI_FUSE_PTS 27           // 8 corners + 12 edge mids + 6 face centres + centre
#define PACCI_FUSE_WELD_RANGE 130.0f // the best point pair has to be at least this close
// 80 was too strict once face centres were dropped: with only corners and edge midpoints
// left, two boxes can be visibly touching while their nearest PAIR of those points is
// still most of a box apart.
#define PACCI_FUSE_NOCOL_HALF 12.0f // nominal box for an actor with no collision at all
// The other way to earn a weld offer: the two BODIES are this close, measured surface to
// surface, whatever their nearest pair of weld points happens to be doing. See the two gates
// in Pacci_FuseSolve.
#define PACCI_FUSE_NEAR_GAP 55.0f
#define PACCI_FUSE_DETACH_HOLD 12   // frames of held R before a weld comes apart
// The shake-to-detach knobs live HERE, not with the rest of the Ultrahand-mode tuning: the
// detector is part of the fusion code and sits hundreds of lines above that block.
#define PACCI_UH_WIGGLE_WINDOW 14 // frames a direction flip stays "recent"
#define PACCI_UH_WIGGLE_FLIPS 3   // reversals inside that window before it counts as a shake

// Zonai green. Nintendo never published a hex for the Ultrahand glue, so this is
// eyeballed off the in-game glow: a pale chartreuse core inside a deeper green halo.
#define PACCI_ZONAI_CORE_R 210
#define PACCI_ZONAI_CORE_G 255
#define PACCI_ZONAI_CORE_B 140
#define PACCI_ZONAI_GLOW_R 80
#define PACCI_ZONAI_GLOW_G 200
#define PACCI_ZONAI_GLOW_B 40

typedef struct {
    Actor* actor;
    Vec3f offset;         // position in the root's local frame
    Vec3s rot;            // rotation relative to the root
    Vec3f weldLocal;      // where the two met, root-local, so the bead can be redrawn
    // The part's own update, which our wrapper calls before re-imposing the transform.
    ActorFunc origUpdate;
    u32 origFlags;
    f32 origGravity;
    s16 origRoom;
} PacciFusePart;

typedef struct {
    Actor* root;
    PacciFusePart parts[PACCI_FUSE_MAX_PARTS];
    u8 count;
} PacciAssembly;

static PacciAssembly sFuse = { NULL, { { 0 } }, 0 };

// Defined in the merge section at the bottom: every caller above needs them, and the
// merge needs the part list they operate on.
// What the next A press would do, recomputed every frame while you hold something.
static struct {
    u8 valid;
    Actor* target;
    Vec3f weld;    // world point the two meet at — the Zonai bead sits here
    Vec3f snapPos; // where the held actor's origin lands if you commit
    // The two points the solve actually paired, each still on its own object. Both
    // are drawn, because a single bead at the meeting point cannot tell you WHICH
    // corner of the thing in your hands is about to land on WHICH corner of the
    // target — and that pairing is the whole decision you are making.
    Vec3f heldPt;
    Vec3f targetPt;
} sFusePv = { 0, NULL, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };

static s16 sFuseDetachHold = 0;

// An oriented box standing in for the actor's shape. Only yaw: pitch and roll would
// need the full matrix and no weld point is worth that.
typedef struct {
    Vec3f center; // world
    Vec3f half;   // half-extents, already scaled
    s16 yaw;
    u8 solid; // has real collision of some kind
    u8 dyna;  // specifically a dynapoly bg actor
} PacciFuseBox;

// Put one part back where the assembly says it belongs: the offset is stored in the root's
// LOCAL frame, so it is rotated by the root's yaw and added to the root's position.
static void Pacci_FusePlacePart(PacciFusePart* slot, Actor* root) {
    Actor* part = slot->actor;
    f32 sin = Math_SinS(root->shape.rot.y);
    f32 cos = Math_CosS(root->shape.rot.y);

    part->world.pos.x = root->world.pos.x + ((slot->offset.x * cos) + (slot->offset.z * sin));
    part->world.pos.y = root->world.pos.y + slot->offset.y;
    part->world.pos.z = root->world.pos.z + ((slot->offset.z * cos) - (slot->offset.x * sin));
    part->shape.rot.x = root->shape.rot.x + slot->rot.x;
    part->shape.rot.y = root->shape.rot.y + slot->rot.y;
    part->shape.rot.z = root->shape.rot.z + slot->rot.z;
    part->world.rot = part->shape.rot;
    // Whatever its own logic built up this frame is not motion the assembly agreed to.
    part->velocity.x = 0.0f;
    part->velocity.y = 0.0f;
    part->velocity.z = 0.0f;
}

// A glued part KEEPS ITS OWN AI. This used to be an empty function - the part was frozen
// outright, the same update-replacement the held object gets - and that was simpler, but it
// also meant a glued Deku Baba stopped biting, a glued torch stopped burning and every
// animation on every piece stopped dead. Gluing an ACTOR to something is not worth doing if
// what you get back is a statue.
//
// So its update runs, and then the assembly's transform is re-imposed IN THE SAME CALL.
// Correcting it afterwards from Pacci_FuseFollow is not enough on its own: actors update in
// category order, so a piece whose category runs after that correction would already have
// walked away from the structure by the time anything drew it.
static void Pacci_FusePartUpdate(Actor* thisx, PlayState* play) {
    for (u8 i = 0; i < sFuse.count; i++) {
        PacciFusePart* slot = &sFuse.parts[i];

        if (slot->actor != thisx) {
            continue;
        }
        if (slot->origUpdate != NULL) {
            slot->origUpdate(thisx, play);
        }
        // Its own update is allowed to kill it (a glued pot smashed by something, an enemy
        // that died mid-structure), and a dead actor must not be written to.
        if ((thisx->update != NULL) && (sFuse.root != NULL) && (sFuse.root->update != NULL)) {
            Pacci_FusePlacePart(slot, sFuse.root);
        }
        return;
    }
}

u8 Pacci_FuseCount(void) {
    return sFuse.count;
}

// Grabbing a piece that is already glued would tear the formation apart, so the
// targeting refuses it. The ROOT stays grabbable on purpose — that is how you pick
// the finished structure back up.
u8 Pacci_FuseIsPart(Actor* actor) {
    for (u8 i = 0; i < sFuse.count; i++) {
        if (sFuse.parts[i].actor == actor) {
            return 1;
        }
    }
    return 0;
}

// Which assembly, if any, this actor belongs to. Aiming at a glued piece has to act
// on the ROOT, because the root is the only thing the transform driver moves — taking
// hold of a part directly would drag it out of formation while the rest stayed put.
Actor* Pacci_FuseRootOf(Actor* actor) {
    if ((actor != NULL) && (sFuse.count > 0)) {
        if (actor == sFuse.root) {
            return sFuse.root;
        }
        if (Pacci_FuseIsPart(actor)) {
            return sFuse.root;
        }
    }
    return NULL;
}

// Drop the bookkeeping WITHOUT touching the actors. For scene teardown, where the
// pointers are already dead and writing through them would be a use-after-free.
void Pacci_FuseForget(void) {
    for (u8 i = 0; i < PACCI_FUSE_MAX_PARTS; i++) {
        sFuse.parts[i].actor = NULL;
    }
    sFuse.root = NULL;
    sFuse.count = 0;
    sFusePv.valid = 0;
    sFusePv.target = NULL;
}

static void Pacci_FuseGiveBack(PacciFusePart* part) {
    Actor* actor = part->actor;

    if ((actor != NULL) && (actor->update != NULL)) {
        if (part->origUpdate != NULL) {
            actor->update = part->origUpdate;
        }
        actor->flags = part->origFlags;
        actor->gravity = part->origGravity;
        actor->room = (s8)part->origRoom;
        actor->colorFilterParams = 0;
    }
    part->actor = NULL;
}

// Hand every part back to itself, in place. Un-fuses a live structure.
void Pacci_FuseRelease(void) {
    for (u8 i = 0; i < sFuse.count; i++) {
        Pacci_FuseGiveBack(&sFuse.parts[i]);
    }
    sFuse.root = NULL;
    sFuse.count = 0;
    sFusePv.valid = 0;
    sFusePv.target = NULL;
}

// Pull ONE piece off and leave the rest of the structure standing.
u8 Pacci_FuseDetachPart(Actor* actor) {
    for (u8 i = 0; i < sFuse.count; i++) {
        if (sFuse.parts[i].actor != actor) {
            continue;
        }
        Pacci_FuseGiveBack(&sFuse.parts[i]);
        // Close the gap so the array stays dense; order carries no meaning here.
        for (u8 j = i; j < (u8)(sFuse.count - 1); j++) {
            sFuse.parts[j] = sFuse.parts[j + 1];
        }
        sFuse.count--;
        if (sFuse.count == 0) {
            sFuse.root = NULL;
        }
        return 1;
    }
    return 0;
}

// Fit an oriented box to whatever collision the actor actually has. Dynapoly gives a
// true fit from its CollisionHeader bounds; everything else falls back to the
// collision cylinder, which is the only size field every actor carries.
static u8 Pacci_FuseGetBox(PlayState* play, Actor* actor, PacciFuseBox* box) {
    CollisionHeader* hdr = NULL;

    if ((actor == NULL) || (actor->update == NULL)) {
        return 0;
    }
    box->yaw = actor->shape.rot.y;
    box->solid = 0;
    box->dyna = 0;

    for (s32 i = 0; i < BG_ACTOR_MAX; i++) {
        BgActor* bg = &play->colCtx.dyna.bgActors[i];

        if ((bg->actor == actor) && (bg->colHeader != NULL)) {
            hdr = bg->colHeader;
            break;
        }
    }

    if (hdr != NULL) {
        f32 lx = ((f32)hdr->maxBounds.x + (f32)hdr->minBounds.x) * 0.5f * actor->scale.x;
        f32 ly = ((f32)hdr->maxBounds.y + (f32)hdr->minBounds.y) * 0.5f * actor->scale.y;
        f32 lz = ((f32)hdr->maxBounds.z + (f32)hdr->minBounds.z) * 0.5f * actor->scale.z;
        f32 sin = Math_SinS(box->yaw);
        f32 cos = Math_CosS(box->yaw);

        box->half.x = ((f32)hdr->maxBounds.x - (f32)hdr->minBounds.x) * 0.5f * actor->scale.x;
        box->half.y = ((f32)hdr->maxBounds.y - (f32)hdr->minBounds.y) * 0.5f * actor->scale.y;
        box->half.z = ((f32)hdr->maxBounds.z - (f32)hdr->minBounds.z) * 0.5f * actor->scale.z;
        // The bounds are model-space, so their centre has to be carried out to world
        // through the actor's yaw before it means anything.
        box->center.x = actor->world.pos.x + ((lx * cos) + (lz * sin));
        box->center.y = actor->world.pos.y + ly;
        box->center.z = actor->world.pos.z + ((lz * cos) - (lx * sin));
        box->solid = 1;
        box->dyna = 1;
        return 1;
    }

    if (actor->colChkInfo.cylRadius > 0) {
        f32 r = (f32)actor->colChkInfo.cylRadius;
        f32 h = (actor->colChkInfo.cylHeight > 0) ? (f32)actor->colChkInfo.cylHeight : (r * 2.0f);

        box->half.x = r;
        box->half.z = r;
        box->half.y = h * 0.5f;
        box->center.x = actor->world.pos.x;
        box->center.y = actor->world.pos.y + (f32)actor->colChkInfo.cylYShift + (h * 0.5f);
        box->center.z = actor->world.pos.z;
        box->solid = 1;
        return 1;
    }

    // No collision at all. It still gets a nominal box so it has corners to be
    // pinned by, but solid stays 0 and that is what triggers the corners-only rule.
    box->half.x = PACCI_FUSE_NOCOL_HALF;
    box->half.y = PACCI_FUSE_NOCOL_HALF;
    box->half.z = PACCI_FUSE_NOCOL_HALF;
    box->center = actor->world.pos;
    box->center.y += PACCI_FUSE_NOCOL_HALF;
    return 1;
}

// Emit the box's weld points in world space. Sweeping i/j/k over {-1,0,1} produces
// exactly the set we want: all three non-zero is a corner, one zero an edge midpoint,
// two zeros a face centre, all zero the centre.
static u8 Pacci_FuseBoxPoints(PacciFuseBox* box, u8 cornersOnly, Vec3f* out) {
    f32 sin = Math_SinS(box->yaw);
    f32 cos = Math_CosS(box->yaw);
    u8 n = 0;

    for (s32 i = -1; i <= 1; i++) {
        for (s32 j = -1; j <= 1; j++) {
            for (s32 k = -1; k <= 1; k++) {
                f32 lx;
                f32 ly;
                f32 lz;

                // Face centres and the box centre are dropped. They were winning the
                // closest-pair search whenever two objects overlapped, which welded
                // pieces INTO each other instead of against each other. Corners and edge
                // midpoints are the only places two solids can meet and still be
                // touching, which is what the bead is supposed to mark.
                s32 zeros = ((i == 0) ? 1 : 0) + ((j == 0) ? 1 : 0) + ((k == 0) ? 1 : 0);

                if (zeros > (cornersOnly ? 0 : 1)) {
                    continue;
                }
                lx = (f32)i * box->half.x;
                ly = (f32)j * box->half.y;
                lz = (f32)k * box->half.z;

                out[n].x = box->center.x + ((lx * cos) + (lz * sin));
                out[n].y = box->center.y + ly;
                out[n].z = box->center.z + ((lz * cos) - (lx * sin));
                n++;
            }
        }
    }
    return n;
}

// Find the closest weld-point pair between held and target. `weld` comes back as the
// world point they meet at, `snapPos` as where the held actor's origin has to move
// for the two points to coincide.
// Distance between the two BODIES, not between their nearest weld points. Standard box-to-box
// separation: an axis where they overlap contributes nothing, and what is left is how far apart
// they actually are. Yaw is ignored here on purpose - this is a proximity gate, not the solve,
// and an oriented test would only ever move the threshold by a few units.
static f32 Pacci_FuseBoxGap(PacciFuseBox* a, PacciFuseBox* b) {
    f32 gx = fabsf(b->center.x - a->center.x) - (a->half.x + b->half.x);
    f32 gy = fabsf(b->center.y - a->center.y) - (a->half.y + b->half.y);
    f32 gz = fabsf(b->center.z - a->center.z) - (a->half.z + b->half.z);

    if (gx < 0.0f) {
        gx = 0.0f;
    }
    if (gy < 0.0f) {
        gy = 0.0f;
    }
    if (gz < 0.0f) {
        gz = 0.0f;
    }
    return sqrtf((gx * gx) + (gy * gy) + (gz * gz));
}

static u8 Pacci_FuseSolve(PlayState* play, Actor* held, Actor* target, Vec3f* weld, Vec3f* snapPos,
                          Vec3f* outHeldPt, Vec3f* outTargetPt) {
    PacciFuseBox hb;
    PacciFuseBox tb;
    Vec3f hp[PACCI_FUSE_PTS];
    Vec3f tp[PACCI_FUSE_PTS];
    u8 hn;
    u8 tn;
    u8 cornersOnly;
    // No cap on the search itself any more. The closest pair is always found; whether that pair
    // is CLOSE ENOUGH is a separate question, answered below by two gates instead of one.
    f32 best = 3.0e38f;
    s32 bh = -1;
    s32 bt = -1;

    if (!Pacci_FuseGetBox(play, held, &hb) || !Pacci_FuseGetBox(play, target, &tb)) {
        return 0;
    }

    // Something with no collision has no face or edge to lie along, so it may only be
    // pinned corner-to-corner, and only onto real dynapoly.
    cornersOnly = (!hb.solid || !tb.solid);
    if (cornersOnly && !hb.dyna && !tb.dyna) {
        return 0;
    }

    hn = Pacci_FuseBoxPoints(&hb, cornersOnly, hp);
    tn = Pacci_FuseBoxPoints(&tb, cornersOnly, tp);

    for (u8 i = 0; i < hn; i++) {
        for (u8 j = 0; j < tn; j++) {
            f32 dx = tp[j].x - hp[i].x;
            f32 dy = tp[j].y - hp[i].y;
            f32 dz = tp[j].z - hp[i].z;
            f32 d = (dx * dx) + (dy * dy) + (dz * dz);

            if (d < best) {
                best = d;
                bh = i;
                bt = j;
            }
        }
    }
    if (bh < 0) {
        return 0;
    }
    // TWO gates, and either one is enough.
    //
    // The weld-point test alone is what made the offer feel dead: only corners and edge
    // midpoints are candidates, so two large boxes can be visibly touching - resting against
    // each other, even overlapping - while their nearest CORNER pair is still most of a box
    // apart and nothing is offered. Asking how far apart the bodies are instead answers the
    // question the player is actually asking, which is "are these two things next to each
    // other". The point test still earns its keep for small objects held out at arm's length,
    // where the bodies are far apart but you are lining a corner up precisely.
    if ((best > (PACCI_FUSE_WELD_RANGE * PACCI_FUSE_WELD_RANGE)) &&
        (Pacci_FuseBoxGap(&hb, &tb) > PACCI_FUSE_NEAR_GAP)) {
        return 0;
    }

    *weld = tp[bt];
    snapPos->x = held->world.pos.x + (tp[bt].x - hp[bh].x);
    snapPos->y = held->world.pos.y + (tp[bt].y - hp[bh].y);
    snapPos->z = held->world.pos.z + (tp[bt].z - hp[bh].z);

    // Hand back the pair itself, not just where it ends up, so the preview can mark
    // both objects.
    if (outHeldPt != NULL) {
        *outHeldPt = hp[bh];
    }
    if (outTargetPt != NULL) {
        *outTargetPt = tp[bt];
    }
    return 1;
}

// ── preview ──────────────────────────────────────────────────────────────────

u8 Pacci_FusePreviewValid(void) {
    return sFusePv.valid;
}

// Every frame you are holding something: work out whether a weld is on offer and
// where. Must run after the carry has moved the held object, or the bead lags a
// frame behind the thing it is supposed to be touching.
void Pacci_FuseUpdatePreview(PlayState* play, Player* player) {
    Actor* held = sUltrahand.held;
    Actor* target;

    sFusePv.valid = 0;
    sFusePv.target = NULL;

    if ((held == NULL) || (held->update == NULL)) {
        return;
    }
    if (sFuse.count >= PACCI_FUSE_MAX_PARTS) {
        return;
    }
    // One assembly at a time: if a structure already exists and this is not its root,
    // welding to it would leave the old one with nothing driving it.
    if ((sFuse.count > 0) && (sFuse.root != held)) {
        return;
    }

    target = Pacci_ResolveUltrahandTarget(play, player);
    if ((target == NULL) || (target == held) || (target->update == NULL) || Pacci_FuseIsPart(target)) {
        return;
    }
    if (!Pacci_FuseSolve(play, held, target, &sFusePv.weld, &sFusePv.snapPos, &sFusePv.heldPt,
                         &sFusePv.targetPt)) {
        return;
    }

    sFusePv.valid = 1;
    sFusePv.target = target;
    // BOTH ends light up, so the offer reads as a pair about to join rather than as
    // one more thing you could grab.
    // BOTH ends light up, so the offer reads as a pair about to join rather than as one more
    // thing you could grab.
    Pacci_UhTintAdd(target);
    Pacci_UhTintAdd(held);
}

// Commit the previewed weld. Returns 0 if there was nothing on offer, which is what
// lets A fall through to "drop".
// Register an ALREADY-POSITIONED part on the root at a local offset it is handed, instead of
// one measured from where the solve put the two objects. That is what rebuilding a stored
// structure needs: it knows the shape and has to reproduce it exactly, and there is no aim,
// no preview and no weld point involved. The caller merges the collision once, at the end.
static u8 Pacci_FuseAdopt(Actor* root, Actor* part, Vec3f* localOff, Vec3s* relRot) {
    PacciFusePart* slot;

    if ((root == NULL) || (part == NULL) || (part->update == NULL) ||
        (sFuse.count >= PACCI_FUSE_MAX_PARTS)) {
        return 0;
    }
    slot = &sFuse.parts[sFuse.count];
    sFuse.root = root;
    slot->actor = part;
    slot->offset = *localOff;
    slot->rot = *relRot;
    slot->weldLocal = *localOff; // no solve ran, so the bead marks the piece itself
    slot->origUpdate = part->update;
    slot->origFlags = part->flags;
    slot->origGravity = part->gravity;
    slot->origRoom = part->room;
    sFuse.count++;

    part->update = Pacci_FusePartUpdate;
    part->gravity = 0.0f;
    part->velocity.x = 0.0f;
    part->velocity.y = 0.0f;
    part->velocity.z = 0.0f;
    part->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;
    return 1;
}

u8 Pacci_FuseTryAttach(PlayState* play) {
    Actor* root = sUltrahand.held;
    Actor* part = sFusePv.target;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 sin;
    f32 cos;
    PacciFusePart* slot;

    if (!sFusePv.valid || (root == NULL) || (part == NULL) || (part->update == NULL)) {
        return 0;
    }
    if (sFuse.count >= PACCI_FUSE_MAX_PARTS) {
        return 0;
    }

    // Snap the held piece onto the weld point BEFORE the offset is measured, so what
    // gets recorded is the aligned pose and not where your aim happened to be.
    root->world.pos = sFusePv.snapPos;
    root->prevPos = root->world.pos;

    dx = part->world.pos.x - root->world.pos.x;
    dy = part->world.pos.y - root->world.pos.y;
    dz = part->world.pos.z - root->world.pos.z;

    // World delta -> the root's local frame, so the part keeps its relative place when
    // the root turns. Negative angle because this is the inverse rotation.
    sin = Math_SinS(-root->shape.rot.y);
    cos = Math_CosS(-root->shape.rot.y);

    slot = &sFuse.parts[sFuse.count];
    sFuse.root = root;
    slot->actor = part;
    slot->offset.x = (dx * cos) + (dz * sin);
    slot->offset.y = dy;
    slot->offset.z = (dz * cos) - (dx * sin);
    slot->rot.x = part->shape.rot.x - root->shape.rot.x;
    slot->rot.y = part->shape.rot.y - root->shape.rot.y;
    slot->rot.z = part->shape.rot.z - root->shape.rot.z;
    {
        f32 wx = sFusePv.weld.x - root->world.pos.x;
        f32 wy = sFusePv.weld.y - root->world.pos.y;
        f32 wz = sFusePv.weld.z - root->world.pos.z;

        slot->weldLocal.x = (wx * cos) + (wz * sin);
        slot->weldLocal.y = wy;
        slot->weldLocal.z = (wz * cos) - (wx * sin);
    }
    slot->origUpdate = part->update;
    slot->origFlags = part->flags;
    slot->origGravity = part->gravity;
    slot->origRoom = part->room;
    sFuse.count++;

    part->update = Pacci_FusePartUpdate;
    part->gravity = 0.0f;
    part->velocity.x = 0.0f;
    part->velocity.y = 0.0f;
    part->velocity.z = 0.0f;
    // Culling would stop the part's transform being driven while the root is still on
    // screen, and the piece would be left behind mid-air.
    part->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;

    sFusePv.valid = 0;
    sFusePv.target = NULL;

    Audio_PlaySoundGeneral(NA_SE_SY_GET_ITEM, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    return 1;
}

// Detach by shaking the stick left and right, the closest a controller with one stick gets to
// TotK's right-stick wiggle. Aiming at one piece takes that piece off; aiming at nothing in
// particular dissolves the whole structure.
//
// FLIPS are what is counted, not deflection: the stick is still steering Link, so any threshold
// on "how far left" would fire every time he ran sideways. Three reversals inside the window is
// something you can only do on purpose.
void Pacci_FuseWiggleDetach(PlayState* play, Player* player, Input* input) {
    s8 x = input->cur.stick_x;
    s8 dir = (x > 40) ? 1 : ((x < -40) ? -1 : 0);

    if (sUhMode.wiggleTimer > 0) {
        sUhMode.wiggleTimer--;
    } else {
        sUhMode.wiggleFlips = 0;
        sUhMode.wiggleDir = 0;
    }

    if (dir != 0) {
        if ((sUhMode.wiggleDir != 0) && (dir != sUhMode.wiggleDir)) {
            sUhMode.wiggleFlips++;
        }
        sUhMode.wiggleDir = dir;
        sUhMode.wiggleTimer = PACCI_UH_WIGGLE_WINDOW;
    }

    if (sUhMode.wiggleFlips < PACCI_UH_WIGGLE_FLIPS) {
        return;
    }
    sUhMode.wiggleFlips = 0;
    sUhMode.wiggleDir = 0;
    sUhMode.wiggleTimer = 0;

    if (sFuse.count == 0) {
        return;
    }
    if (!Pacci_FuseDetachPart(Pacci_ResolveUltrahandTarget(play, player))) {
        Pacci_FuseRelease();
    }
    Audio_PlaySoundGeneral(NA_SE_SY_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

// Hold R to come apart. Aiming at one piece takes that piece off; aiming at nothing
// in particular dissolves the whole structure.
void Pacci_FuseHoldDetach(PlayState* play, Player* player, u8 rHeld) {
    if (!rHeld) {
        sFuseDetachHold = 0;
        return;
    }
    sFuseDetachHold++;
    if (sFuseDetachHold != PACCI_FUSE_DETACH_HOLD) {
        return; // fires once per hold, not every frame it stays down
    }
    if (sFuse.count == 0) {
        return;
    }
    if (!Pacci_FuseDetachPart(Pacci_ResolveUltrahandTarget(play, player))) {
        Pacci_FuseRelease();
    }
    Audio_PlaySoundGeneral(NA_SE_SY_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

// Rebuild every part from the root. Must run AFTER the root has been positioned.
void Pacci_FuseFollow(PlayState* play) {
    Actor* root = sFuse.root;
    u8 lit;

    if ((root == NULL) || (sFuse.count == 0)) {
        return;
    }
    if (root->update == NULL) { // root died — the structure is not a structure any more
        // The parts outlived it and nothing drives them any more, so hand them back to
        // themselves rather than leaving them frozen for the rest of the scene.
        Pacci_FuseRelease();
        return;
    }

    // Only while the assembly is in hand. This also runs from CustomItems_Update long after a
    // structure was set down, and a permanently glowing pile of crates is not the effect.
    lit = Pacci_IsHoldingUltrahand() && (root == sUltrahand.held);
    for (u8 i = 0; i < sFuse.count; i++) {
        Actor* part = sFuse.parts[i].actor;

        if ((part == NULL) || (part->update == NULL)) {
            continue;
        }
        // Same helper the part's own update calls, so there is exactly one definition of where
        // a piece belongs. This pass still matters: it catches the pieces whose category
        // updates BEFORE the root moved, which would otherwise sit one frame behind.
        Pacci_FusePlacePart(&sFuse.parts[i], root);
        if (lit) {
            Pacci_UhTintAdd(part);
        }
    }
}

// ── the bead ─────────────────────────────────────────────────────────────────

// An octahedron, not a cube: at bead size the silhouette is all you read, and eight
// faces round off where six would show corners.
static Vtx sFuseBeadVtx[] = {
    VTX(0, 1, 0, 0, 0, 0, 0, 0, 255),  VTX(0, -1, 0, 0, 0, 0, 0, 0, 255),
    VTX(1, 0, 0, 0, 0, 0, 0, 0, 255),  VTX(-1, 0, 0, 0, 0, 0, 0, 0, 255),
    VTX(0, 0, 1, 0, 0, 0, 0, 0, 255),  VTX(0, 0, -1, 0, 0, 0, 0, 0, 255),
};

static Gfx sFuseBeadDL[] = {
    gsSPVertex(sFuseBeadVtx, 6, 0),
    gsSP2Triangles(0, 4, 2, 0, 0, 2, 5, 0),
    gsSP2Triangles(0, 5, 3, 0, 0, 3, 4, 0),
    gsSP2Triangles(1, 2, 4, 0, 1, 5, 2, 0),
    gsSP2Triangles(1, 3, 5, 0, 1, 4, 3, 0),
    gsSPEndDisplayList(),
};

static void Pacci_FuseDrawBead(PlayState* play, Vec3f* pos, f32 scale, u8 pulsing) {
    // Two passes: a soft halo with a brighter core inside it. One flat blob does not
    // read as glowing, it reads as a green rock.
    f32 pulse = pulsing ? (0.85f + (0.15f * Math_SinS((s16)(play->gameplayFrames * 2200)))) : 1.0f;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gDPSetCombineLERP(POLY_XLU_DISP++, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE);
    gSPClearGeometryMode(POLY_XLU_DISP++, G_LIGHTING | G_CULL_BACK);

    // Halo. Components spelled out: MSVC hands a multi-value #define to a
    // function-like macro as ONE argument, so a packed colour would not expand.
    Matrix_Translate(pos->x, pos->y, pos->z, MTXMODE_NEW);
    Matrix_Scale(scale * 1.9f * pulse, scale * 1.9f * pulse, scale * 1.9f * pulse, MTXMODE_APPLY);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, PACCI_ZONAI_GLOW_R, PACCI_ZONAI_GLOW_G, PACCI_ZONAI_GLOW_B, 90);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, sFuseBeadDL);

    // Core.
    Matrix_Translate(pos->x, pos->y, pos->z, MTXMODE_NEW);
    Matrix_Scale(scale * pulse, scale * pulse, scale * pulse, MTXMODE_APPLY);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, PACCI_ZONAI_CORE_R, PACCI_ZONAI_CORE_G, PACCI_ZONAI_CORE_B, 235);
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, sFuseBeadDL);

    CLOSE_DISPS(play->state.gfxCtx);
}

// Zonai marks on a whole object: a small bead on each of its eight box corners.
//
// This exists because the engine's colour filter CANNOT do green — OoT offers white (0x8000),
// red (0x4000) and blue (0), MM adds grey, and that is the whole palette. A Zonai read therefore
// has to be geometry rather than a tint, so the corners of the box the weld solver is already
// using get lit up instead.
static void Pacci_FuseDrawBoxMarks(PlayState* play, Actor* actor) {
    PacciFuseBox box;
    Vec3f pts[PACCI_FUSE_PTS];
    u8 n;

    if ((actor == NULL) || (actor->update == NULL)) {
        return;
    }
    if (!Pacci_FuseGetBox(play, actor, &box)) {
        return;
    }
    n = Pacci_FuseBoxPoints(&box, 1, pts); // corners only: 8 marks, not the full 20
    for (u8 i = 0; i < n; i++) {
        Pacci_FuseDrawBead(play, &pts[i], 3.0f, 0);
    }
}

// One bead per existing joint, plus a pulsing one on the weld being offered.
void Pacci_FuseDrawPreview(PlayState* play) {
    Actor* root = sFuse.root;

    if ((root != NULL) && (root->update != NULL)) {
        f32 sin = Math_SinS(root->shape.rot.y);
        f32 cos = Math_CosS(root->shape.rot.y);

        for (u8 i = 0; i < sFuse.count; i++) {
            Vec3f* w = &sFuse.parts[i].weldLocal;
            Vec3f pos;

            if ((sFuse.parts[i].actor == NULL) || (sFuse.parts[i].actor->update == NULL)) {
                continue;
            }
            pos.x = root->world.pos.x + ((w->x * cos) + (w->z * sin));
            pos.y = root->world.pos.y + w->y;
            pos.z = root->world.pos.z + ((w->z * cos) - (w->x * sin));
            Pacci_FuseDrawBead(play, &pos, 6.0f, 0);
        }
    }

    if (sFusePv.valid) {
        // One bead on EACH object, so the pairing is legible before you commit: the
        // corner of the thing in your hands, the corner of the thing it will stick
        // to, and a dotted run between them for the move about to happen.
        // Outline BOTH bodies in Zonai corners first, under the joint beads: the engine has no
        // green colour filter, so "these two are what is about to join" has to be drawn.
        Pacci_FuseDrawBoxMarks(play, sUltrahand.held);
        Pacci_FuseDrawBoxMarks(play, sFusePv.target);
        Pacci_FuseDrawBead(play, &sFusePv.heldPt, 6.0f, 1);
        Pacci_FuseDrawBead(play, &sFusePv.targetPt, 7.0f, 1);

        {
            Vec3f step;
            f32 t;

            for (t = 0.2f; t < 0.99f; t += 0.2f) {
                step.x = sFusePv.heldPt.x + ((sFusePv.targetPt.x - sFusePv.heldPt.x) * t);
                step.y = sFusePv.heldPt.y + ((sFusePv.targetPt.y - sFusePv.heldPt.y) * t);
                step.z = sFusePv.heldPt.z + ((sFusePv.targetPt.z - sFusePv.heldPt.z) * t);
                Pacci_FuseDrawBead(play, &step, 2.5f, 0);
            }
        }
    }
}

// ============================================================================
// ULTRAHAND VFX + WORLD-SPACE CONTROL GIZMO
// ============================================================================

#define PACCI_UH_VFX_POINTS 13
#define PACCI_UH_GIZMO_SEGMENTS 20
// Thickness of the solid gizmo parts, in world units.
#define PACCI_UH_GIZMO_SHAFT_R 3.6f
#define PACCI_UH_GIZMO_HEAD_R 10.5f
#define PACCI_UH_GIZMO_RING_R 3.0f
// The energy wave: how many pulses ride the tether at once, how many frames one takes
// to travel it end to end, how much of the tether a single pulse covers (0..1), and the
// tube's resting / peak radius.
// Colours lifted from the Blender materials rather than picked by eye.
//   "Selected object outline"  (0.005, 0.92, 0.20)
//   "Contact rings"            (0.01,  0.95, 0.28)
//   the held tint's ramp, dark end (0.015, 0.22, 0.09)
// The point light the held body casts. The radius is generous on purpose: the reference's
// bounce light reaches the floor and the walls, and a tight radius just makes a green dot.
#define PACCI_UH_LIGHT_R 55
#define PACCI_UH_LIGHT_G 255
#define PACCI_UH_LIGHT_B 160
#define PACCI_UH_LIGHT_RADIUS 340

// -- solid gizmo geometry ----------------------------------------------------
// The gizmo is real geometry, not beam sprites. Maya and Blender draw their move
// and rotate handles as opaque shafts, cones and bands, and that reading is the
// whole point of a gizmo: a solid arrow says "this axis moves", a spiral beam
// says "magic is happening". Both primitives are unit-sized at 100 model units
// and run along LOCAL +Y, so a caller scales by (radius/100, length/100,
// radius/100) and shares the beams' own orientation maths.

static Vtx sPacciUhPrismVtx[] = {
    VTX(100, 0, 0, 0, 0, 0, 0, 0, 255),
    VTX(71, 0, 71, 0, 0, 0, 0, 0, 255),
    VTX(0, 0, 100, 0, 0, 0, 0, 0, 255),
    VTX(-71, 0, 71, 0, 0, 0, 0, 0, 255),
    VTX(-100, 0, 0, 0, 0, 0, 0, 0, 255),
    VTX(-71, 0, -71, 0, 0, 0, 0, 0, 255),
    VTX(0, 0, -100, 0, 0, 0, 0, 0, 255),
    VTX(71, 0, -71, 0, 0, 0, 0, 0, 255),
    VTX(100, 100, 0, 0, 0, 0, 0, 0, 255),
    VTX(71, 100, 71, 0, 0, 0, 0, 0, 255),
    VTX(0, 100, 100, 0, 0, 0, 0, 0, 255),
    VTX(-71, 100, 71, 0, 0, 0, 0, 0, 255),
    VTX(-100, 100, 0, 0, 0, 0, 0, 0, 255),
    VTX(-71, 100, -71, 0, 0, 0, 0, 0, 255),
    VTX(0, 100, -100, 0, 0, 0, 0, 0, 255),
    VTX(71, 100, -71, 0, 0, 0, 0, 0, 255),
};

static Gfx sPacciUhPrismDL[] = {
    gsSPVertex(sPacciUhPrismVtx, 16, 0),
    gsSP2Triangles(0, 1, 9, 0, 0, 9, 8, 0),
    gsSP2Triangles(1, 2, 10, 0, 1, 10, 9, 0),
    gsSP2Triangles(2, 3, 11, 0, 2, 11, 10, 0),
    gsSP2Triangles(3, 4, 12, 0, 3, 12, 11, 0),
    gsSP2Triangles(4, 5, 13, 0, 4, 13, 12, 0),
    gsSP2Triangles(5, 6, 14, 0, 5, 14, 13, 0),
    gsSP2Triangles(6, 7, 15, 0, 6, 15, 14, 0),
    gsSP2Triangles(7, 0, 8, 0, 7, 8, 15, 0),
    gsSPEndDisplayList(),
};

static Vtx sPacciUhConeVtx[] = {
    VTX(0, 100, 0, 0, 0, 0, 0, 0, 255),
    VTX(100, 0, 0, 0, 0, 0, 0, 0, 255),
    VTX(71, 0, 71, 0, 0, 0, 0, 0, 255),
    VTX(0, 0, 100, 0, 0, 0, 0, 0, 255),
    VTX(-71, 0, 71, 0, 0, 0, 0, 0, 255),
    VTX(-100, 0, 0, 0, 0, 0, 0, 0, 255),
    VTX(-71, 0, -71, 0, 0, 0, 0, 0, 255),
    VTX(0, 0, -100, 0, 0, 0, 0, 0, 255),
    VTX(71, 0, -71, 0, 0, 0, 0, 0, 255),
};

static Gfx sPacciUhConeDL[] = {
    gsSPVertex(sPacciUhConeVtx, 9, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    gsSP2Triangles(0, 3, 4, 0, 0, 4, 5, 0),
    gsSP2Triangles(0, 5, 6, 0, 0, 6, 7, 0),
    gsSP2Triangles(0, 7, 8, 0, 0, 8, 1, 0),
    gsSP2Triangles(1, 2, 3, 0, 1, 3, 4, 0),
    gsSP2Triangles(1, 4, 5, 0, 1, 5, 6, 0),
    gsSP2Triangles(1, 6, 7, 0, 1, 7, 8, 0),
    gsSPEndDisplayList(),
};
static void Pacci_UhGetHandPos(Player* player, Vec3f* pos) {
    Vec3f forearm = player->bodyPartsPos[PLAYER_BODYPART_R_FOREARM];
    Vec3f hand = player->bodyPartsPos[PLAYER_BODYPART_R_HAND];
    f32 dx = hand.x - forearm.x;
    f32 dy = hand.y - forearm.y;
    f32 dz = hand.z - forearm.z;
    f32 length = sqrtf((dx * dx) + (dy * dy) + (dz * dz));

    *pos = hand;
    if (length > 0.001f) {
        pos->x += (dx / length) * 10.0f;
        pos->y += (dy / length) * 10.0f;
        pos->z += (dz / length) * 10.0f;
    }
}

static void Pacci_UhGetStreamPoint(Vec3f* point, Vec3f* start, Vec3f* end, f32 t, s16 phase, u32 frame) {
    f32 dx = end->x - start->x;
    f32 dz = end->z - start->z;
    f32 xzLength = sqrtf((dx * dx) + (dz * dz));
    f32 envelope = Math_SinS((s16)(t * 0x7FFF));
    f32 sideX = 1.0f;
    f32 sideZ = 0.0f;
    s16 broadWave = (s16)((t * 0x6000) + (frame * 0x0180) + phase);
    s16 fineWave = (s16)((t * 0xE000) - (frame * 0x00C0) + (phase >> 1));
    f32 flutter = ((Math_SinS(broadWave) * 0.65f) + (Math_SinS(fineWave) * 0.35f)) * envelope;

    if (xzLength > 0.001f) {
        sideX = dz / xzLength;
        sideZ = -dx / xzLength;
    }
    point->x = start->x + ((end->x - start->x) * t) + (sideX * flutter * 3.8f);
    point->y = start->y + ((end->y - start->y) * t) + (envelope * 10.0f) +
               (Math_CosS(broadWave) * envelope * 1.8f);
    point->z = start->z + ((end->z - start->z) * t) + (sideZ * flutter * 3.8f);
}

// Flat, untextured, double-sided state for the solid primitives. Hoisted out of the
// per-part draw because a rotation band is twenty-odd segments: re-sending the
// combiner and the geometry mode for each one is the difference between three
// display-list commands per segment and ten.
//
// The combiner reads PRIMITIVE in every slot on purpose. Nothing binds a texture for
// these, so leaving TEXEL0 in the equation samples whatever the previous pass happened
// to leave loaded, which is how a solid arrow ends up striped with the tether's noise.
static void Pacci_UhSolidBegin(Gfx** gfxP) {
    Gfx* gfx = *gfxP;

    gDPPipeSync(gfx++);
    gDPSetCombineLERP(gfx++, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE,
                      0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE);
    gSPClearGeometryMode(gfx++, G_LIGHTING | G_CULL_BACK);
    *gfxP = gfx;
}

static void Pacci_UhSolidEnd(Gfx** gfxP) {
    Gfx* gfx = *gfxP;

    gSPSetGeometryMode(gfx++, G_LIGHTING | G_CULL_BACK);
    *gfxP = gfx;
}

// Draw one solid primitive spanning start -> end. Must sit between a Begin/End pair.
static void Pacci_UhDrawSolid(PlayState* play, Gfx** gfxP, Gfx* dl, Vec3f* start, Vec3f* end,
                              f32 radius, u8 r, u8 g, u8 b, u8 alpha) {
    f32 dx = end->x - start->x;
    f32 dy = end->y - start->y;
    f32 dz = end->z - start->z;
    f32 xzLength = sqrtf((dx * dx) + (dz * dz));
    f32 length = sqrtf((dx * dx) + (dy * dy) + (dz * dz));
    Gfx* gfx = *gfxP;

    if (length < 0.1f) {
        return;
    }
    Matrix_Translate(start->x, start->y, start->z, MTXMODE_NEW);
    Matrix_RotateY(BINANG_TO_RAD(Math_Vec3f_Yaw(start, end)), MTXMODE_APPLY);
    Matrix_RotateX(atan2f(xzLength, dy), MTXMODE_APPLY);
    Matrix_Scale(radius / 100.0f, length / 100.0f, radius / 100.0f, MTXMODE_APPLY);
    gDPSetPrimColor(gfx++, 0, 0, r, g, b, alpha);
    gSPMatrix(gfx++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(gfx++, dl);
    *gfxP = gfx;
}

// -- the light -----------------------------------------------------------------
// "hasta luz": the composition lights the scene from the object, not just draws on top of
// it, and that is most of why it reads as energy. A real point light in the scene's light
// context does the same thing here - Link, the floor and everything the structure passes
// get the green spill for free, with no extra geometry.
static LightInfo sUhLightInfo;
static LightNode* sUhLightNode = NULL;
// Which scene the node was inserted in. A scene change reinitialises the whole light
// context, so the node we are holding is freed underneath us: removing it then would be a
// write into a reused list. Comparing the scene lets us drop the pointer instead.
static s16 sUhLightScene = -1;

static void Pacci_UhLightOff(PlayState* play) {
    if (sUhLightNode != NULL) {
        if ((play != NULL) && (play->sceneId == sUhLightScene)) {
            LightContext_RemoveLight(play, &play->lightCtx, sUhLightNode);
        }
        sUhLightNode = NULL;
        sUhLightScene = -1;
    }
}

static void Pacci_UhLightAt(PlayState* play, Vec3f* pos, f32 strength) {
    // Breathes with the same period as the contact rings, so light and geometry pulse
    // together instead of drifting in and out of phase with each other.
    f32 pulse = strength * (0.86f + (Math_SinS((s16)(play->gameplayFrames * 0x0500)) * 0.14f));

    Lights_PointNoGlowSetInfo(&sUhLightInfo, (s16)pos->x, (s16)pos->y, (s16)pos->z,
                              (u8)(PACCI_UH_LIGHT_R * pulse), (u8)(PACCI_UH_LIGHT_G * pulse),
                              (u8)(PACCI_UH_LIGHT_B * pulse), PACCI_UH_LIGHT_RADIUS);
    if (sUhLightNode == NULL) {
        sUhLightNode = LightContext_InsertLight(play, &play->lightCtx, &sUhLightInfo);
        sUhLightScene = play->sceneId;
    }
}

static void Pacci_UhDrawFlowPass(PlayState* play, Gfx** gfxP, Vec3f* hand, Vec3f* target, f32 reach,
                                 s16 phase, f32 width, u8 r, u8 g, u8 b, u8 alpha) {
    Vec3f points[PACCI_UH_VFX_POINTS];
    u8 i;

    for (i = 0; i < PACCI_UH_VFX_POINTS; i++) {
        f32 t = ((f32)i / (PACCI_UH_VFX_POINTS - 1)) * reach;

        Pacci_UhGetStreamPoint(&points[i], hand, target, t, phase, play->gameplayFrames);
    }
    // One Begin/End for the whole strand: twelve segments at three commands each instead of
    // twelve full state changes.
    Pacci_UhSolidBegin(gfxP);
    for (i = 0; i < PACCI_UH_VFX_POINTS - 1; i++) {
        // Fat in the middle, tapering to nothing at both ends, so the strand is born at the
        // hand and dies into the object rather than being cut off flat.
        f32 edge = Math_SinS((s16)((i * 0x7FFF) / (PACCI_UH_VFX_POINTS - 2)));

        Pacci_UhDrawSolid(play, gfxP, sPacciUhPrismDL, &points[i], &points[i + 1],
                          width * (0.55f + (edge * 0.45f)), r, g, b, alpha);
    }
    Pacci_UhSolidEnd(gfxP);
}

// A Maya / Blender translate handle: solid shaft, solid cone head, flat colour. The
// beam-segment version read as a spell rather than as a control, which is the one
// thing a gizmo must not do.
static void Pacci_UhDrawArrow(PlayState* play, Gfx** gfxP, Vec3f* center, Vec3f* direction, f32 startDist,
                              f32 length, u8 r, u8 g, u8 b) {
    f32 headLength = CLAMP_MIN((length - startDist) * 0.34f, 16.0f);
    f32 shaftEnd = length - headLength;
    Vec3f start;
    Vec3f neck;
    Vec3f tip;

    if (shaftEnd <= (startDist + 1.0f)) {
        shaftEnd = startDist + 1.0f;
    }
    start.x = center->x + (direction->x * startDist);
    start.y = center->y + (direction->y * startDist);
    start.z = center->z + (direction->z * startDist);
    neck.x = center->x + (direction->x * shaftEnd);
    neck.y = center->y + (direction->y * shaftEnd);
    neck.z = center->z + (direction->z * shaftEnd);
    tip.x = center->x + (direction->x * length);
    tip.y = center->y + (direction->y * length);
    tip.z = center->z + (direction->z * length);

    Pacci_UhSolidBegin(gfxP);
    Pacci_UhDrawSolid(play, gfxP, sPacciUhPrismDL, &start, &neck, PACCI_UH_GIZMO_SHAFT_R, r, g, b, 255);
    Pacci_UhDrawSolid(play, gfxP, sPacciUhConeDL, &neck, &tip, PACCI_UH_GIZMO_HEAD_R, r, g, b, 255);
    Pacci_UhSolidEnd(gfxP);
}

// The rotate handle: a solid band around the axis, built from the same prism the
// arrows use. One Begin/End wraps the whole ring, so a twenty-segment band costs
// twenty matrices instead of twenty full state changes.
static void Pacci_UhDrawRotationRing(PlayState* play, Gfx** gfxP, Vec3f* center, f32 radius, s16 yaw,
                                     u8 vertical, u8 r, u8 g, u8 b) {
    Vec3f previous;
    f32 sinYaw = Math_SinS(yaw);
    f32 cosYaw = Math_CosS(yaw);
    u8 i;

    Pacci_UhSolidBegin(gfxP);
    for (i = 0; i <= PACCI_UH_GIZMO_SEGMENTS; i++) {
        s16 angle = (s16)((i * 0x10000) / PACCI_UH_GIZMO_SEGMENTS);
        f32 sin = Math_SinS(angle);
        f32 cos = Math_CosS(angle);
        Vec3f point;

        if (vertical) {
            point.x = center->x + (sinYaw * cos * radius);
            point.y = center->y + (sin * radius);
            point.z = center->z + (cosYaw * cos * radius);
        } else {
            point.x = center->x + (sin * radius);
            point.y = center->y;
            point.z = center->z + (cos * radius);
        }
        if (i != 0) {
            Pacci_UhDrawSolid(play, gfxP, sPacciUhPrismDL, &previous, &point, PACCI_UH_GIZMO_RING_R,
                              r, g, b, 245);
        }
        previous = point;
    }
    Pacci_UhSolidEnd(gfxP);
}

void Pacci_UltrahandDrawVfx(PlayState* play, Player* player) {
    Actor* held;
    PacciFuseBox box;
    Vec3f hand;
    Vec3f forward;
    Vec3f side;
    Vec3f up = { 0.0f, 1.0f, 0.0f };
    Vec3f down = { 0.0f, -1.0f, 0.0f };
    u16 buttons;
    u8 skip = 0;

    if (!sUhMode.active) {
        Pacci_UhLightOff(play);
        return;
    }
    held = sUltrahand.held;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 0x08,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, play->gameplayFrames * 2, 0x20, 0x40, 1,
                                  play->gameplayFrames, play->gameplayFrames * -5, 0x10, 0x10, 2, 0, 1, -8));

    // ONE CLOSE_DISPS per OPEN_DISPS, and both at the same brace level. OPEN_DISPS opens a
    // block; every CLOSE_DISPS closes one. Two of these used to sit inside early returns, so
    // the first bail-out closed the function itself and everything after it was parsed at file
    // scope — which is where the "syntax error: '&'" on the next call came from. Hence the
    // guards set a flag instead of returning.
    if ((held == NULL) || sUltrahand.dropping) {
        // Nothing in hand: the colour filter IS the selection feedback. No geometry is drawn
        // over a candidate at all - that is what the wireframe box was, and a box is the one
        // shape almost nothing in this game actually is.
        Pacci_UhLightOff(play);
        skip = 1;
    } else if (!Pacci_FuseGetBox(play, held, &box)) {
        Pacci_UhLightOff(play);
        skip = 1;
    } else {
        // Held: the object becomes a light source. Ramped in over the same frames the tether
        // takes to reach it, so the room does not snap green on the grab frame.
        Pacci_UhLightAt(play, &box.center, CLAMP_MAX((f32)sUltrahand.vfxAge / 12.0f, 1.0f));
    }

    if (!skip) {

        // The same four layers as the Blender study: a narrow green core, two faint
        // irregular wisps and a translucent shell/ripple treatment on the held body.
        Pacci_UhGetHandPos(player, &hand);
        {
            f32 reach = CLAMP_MAX((f32)sUltrahand.vfxAge / 9.0f, 1.0f);

            // Untextured, like the gizmo arrows: solid octagonal tube, flat colour, no sprite.
            // The Great Fairy beam sprite it used before carried a visible spiral pattern
            // that read as "a vanilla effect stuck on" rather than as Zonai energy.
            //
            // Three strands, drawn widest-first so the thin bright one lands on top: a soft
            // halo, a mid body, and a near-white core. That layering is what makes a flat
            // untextured tube glow - a single pass of one colour is just a green stick. The
            // two outer strands run on different phases of the same wobble, so they cross the
            // core instead of sheathing it.
            Pacci_UhDrawFlowPass(play, &POLY_XLU_DISP, &hand, &box.center, reach, 0x2AAA, 5.0f,
                                 PACCI_UH_FLOW_HALO_R, PACCI_UH_FLOW_HALO_G, PACCI_UH_FLOW_HALO_B, 55);
            Pacci_UhDrawFlowPass(play, &POLY_XLU_DISP, &hand, &box.center, reach, 0x6AAA, 2.4f,
                                 PACCI_UH_FLOW_MID_R, PACCI_UH_FLOW_MID_G, PACCI_UH_FLOW_MID_B, 150);
            Pacci_UhDrawFlowPass(play, &POLY_XLU_DISP, &hand, &box.center, reach, 0, 1.1f,
                                 PACCI_UH_FLOW_CORE_R, PACCI_UH_FLOW_CORE_G, PACCI_UH_FLOW_CORE_B, 255);
        }

        // The handles are only drawn while the control they describe is being used. They used
        // to draw unconditionally, and four solid arrows the size of the object, permanently
        // wrapped around it, read as a box stuck to its front rather than as a gizmo. Maya
        // and Blender do the same: the handle appears when you reach for it.
        buttons = play->state.input[0].cur.button;
        forward.x = Math_SinS(player->actor.focus.rot.y);
        forward.y = 0.0f;
        forward.z = Math_CosS(player->actor.focus.rot.y);
        side.x = Math_SinS(player->actor.focus.rot.y + 0x4000);
        side.y = 0.0f;
        side.z = Math_CosS(player->actor.focus.rot.y + 0x4000);

        if (buttons & BTN_L) {
            f32 yawRadius = fmaxf(box.half.x, box.half.z) + 30.0f;
            f32 pitchRadius = fmaxf(box.half.y, box.half.z) + 30.0f;
            // Blue follows left/right (yaw), red follows up/down (pitch).
            Pacci_UhDrawRotationRing(play, &POLY_XLU_DISP, &box.center, yawRadius, player->actor.focus.rot.y,
                                     0, 40, 135, 255);
            Pacci_UhDrawRotationRing(play, &POLY_XLU_DISP, &box.center, pitchRadius, player->actor.focus.rot.y,
                                     1, 255, 75, 55);
        } else if (buttons & BTN_R) {
            f32 maxHalf = fmaxf(fmaxf(box.half.x, box.half.y), box.half.z);
            f32 start = maxHalf * 0.35f;
            f32 length = maxHalf + 72.0f;
            // R owns the screen plane: red vertical, blue horizontal.
            Pacci_UhDrawArrow(play, &POLY_XLU_DISP, &box.center, &up, start, length, 255, 70, 50);
            Pacci_UhDrawArrow(play, &POLY_XLU_DISP, &box.center, &down, start, length, 255, 70, 50);
            Pacci_UhDrawArrow(play, &POLY_XLU_DISP, &box.center, &side, start, length, 45, 135, 255);
            side.x = -side.x;
            side.z = -side.z;
            Pacci_UhDrawArrow(play, &POLY_XLU_DISP, &box.center, &side, start, length, 45, 135, 255);
        } else if (buttons & (BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT)) {
            f32 maxHalf = fmaxf(fmaxf(box.half.x, box.half.y), box.half.z);
            f32 start = maxHalf * 0.35f;
            f32 length = maxHalf + 72.0f;
            // Ground plane: red forward/back, blue right/left.
            Pacci_UhDrawArrow(play, &POLY_XLU_DISP, &box.center, &forward, start, length, 255, 70, 50);
            forward.x = -forward.x;
            forward.z = -forward.z;
            Pacci_UhDrawArrow(play, &POLY_XLU_DISP, &box.center, &forward, start, length, 255, 70, 50);
            Pacci_UhDrawArrow(play, &POLY_XLU_DISP, &box.center, &side, start, length, 45, 135, 255);
            side.x = -side.x;
            side.z = -side.z;
            Pacci_UhDrawArrow(play, &POLY_XLU_DISP, &box.center, &side, start, length, 45, 135, 255);
        }

    }
    CLOSE_DISPS(play->state.gfxCtx);
}

// Where this body's FOOTPRINT lands, as opposed to where its origin does.
// Actor_UpdateBgCheckInfo raycasts one point at the actor's origin, so a structure whose
// origin sits in its middle sinks half its own height into the floor before anything
// reports contact - and a wide platform dropped across a pit fell straight through, because
// its centre had nothing under it while its corners had plenty of floor. This is the
// "usando sus geometrias" half of the drop.
//
// BgCheck_ProjectileLineTest rather than one of the BgCheck floor helpers: their names and
// signatures diverge between the two games, this one does not, and it sees dynapoly as well
// as scene collision - which is what lets you set one built structure down on another.
//
// The probe starts just BELOW the box instead of above it. Our own merged surface is still
// registered and still moving while we fall, and a ray starting inside it would hit the very
// body it is testing; every one of our polys is inside the box by definition, so starting
// under it cannot self-intersect.
static u8 Pacci_UhGroundUnder(PlayState* play, Actor* actor, f32* outY) {
    static const f32 sProbeX[5] = { -1.0f, 1.0f, -1.0f, 1.0f, 0.0f };
    static const f32 sProbeZ[5] = { -1.0f, -1.0f, 1.0f, 1.0f, 0.0f };
    PacciFuseBox box;
    Vec3f from;
    Vec3f to;
    Vec3f hit;
    CollisionPoly* poly;
    s32 bgId;
    f32 best = 0.0f;
    f32 bottom;
    f32 sin;
    f32 cos;
    u8 found = 0;
    u8 i;

    if (!Pacci_FuseGetBox(play, actor, &box)) {
        return 0;
    }
    bottom = box.center.y - box.half.y;
    sin = Math_SinS(box.yaw);
    cos = Math_CosS(box.yaw);

    // Four bottom corners and the centre. Five probes catch an edge hanging over a drop
    // without turning every falling frame into a raycast storm.
    for (i = 0; i < 5; i++) {
        f32 lx = sProbeX[i] * box.half.x;
        f32 lz = sProbeZ[i] * box.half.z;

        from.x = box.center.x + ((lx * cos) + (lz * sin));
        from.z = box.center.z + ((lz * cos) - (lx * sin));
        from.y = bottom - 1.0f;
        to.x = from.x;
        to.z = from.z;
        to.y = bottom - PACCI_UH_GROUND_PROBE;
        if (BgCheck_ProjectileLineTest(&play->colCtx, &from, &to, &hit, &poly, true, true, true, true,
                                       &bgId)) {
            // HIGHEST hit wins: that is the surface the body comes to rest on first, and it is
            // why a structure straddling a step ends up on the step and not through it.
            if (!found || (hit.y > best)) {
                best = hit.y;
                found = 1;
            }
        }
    }
    if (!found) {
        return 0;
    }
    // Floor-under-the-box -> where the ORIGIN goes, since the origin is not the box bottom.
    *outY = best + (actor->world.pos.y - bottom);
    return 1;
}

// -- stored geometry ---------------------------------------------------------
// One slot, holding a RECIPE rather than geometry: actor id, params, and each piece's place
// in the root's local frame. Copying the merged CollisionHeader instead would be wrong twice
// over - the pools are a single static instance that the next weld overwrites, and we own
// none of the display lists it describes, so what came back would be an invisible shape that
// stopped existing as soon as you glued anything else together.
typedef struct {
    s16 actorId;
    s16 params;
    Vec3f offset; // root-local; index 0 is the root itself and is all zero
    Vec3s rot;    // relative to the root
} PacciBlueprintPiece;

static struct {
    u8 used;
    u8 count;
    // Scene it was taken from. An actor cannot be spawned without its object loaded, and the
    // object bank is per scene: rebuilding a graveyard's gravestones inside a dungeon spawns
    // actors whose object is not resident, which is a crash and not a missing model.
    s16 scene;
    PacciBlueprintPiece piece[PACCI_FUSE_MAX_PARTS + 1];
} sBlueprint = { 0 };

u8 Pacci_BlueprintStored(void) {
    return sBlueprint.used;
}

static void Pacci_BlueprintDeny(void) {
    Audio_PlaySoundGeneral(NA_SE_SY_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

u8 Pacci_BlueprintSave(PlayState* play) {
    Actor* doomed[PACCI_FUSE_MAX_PARTS + 1];
    Actor* root = sUltrahand.held;
    PacciFuseBox box;
    u8 n = 0;
    u8 i;

    if ((root == NULL) || sUltrahand.dropping) {
        Pacci_BlueprintDeny();
        return 0;
    }
    // Dynapoly only, and every piece of it. Anything without a collision surface of its own
    // would come back from the recipe as a plain actor you fall through, which is worse than
    // refusing to store it. Each piece is asked directly now - nothing unregisters their bg
    // actors any more, so their own boxes tell the truth.
    if (!Pacci_FuseGetBox(play, root, &box) || !box.dyna) {
        Pacci_BlueprintDeny();
        return 0;
    }
    if ((sFuse.count > 0) && (sFuse.root != root)) {
        Pacci_BlueprintDeny();
        return 0;
    }
    for (i = 0; i < sFuse.count; i++) {
        Actor* part = sFuse.parts[i].actor;

        if ((part == NULL) || (part->update == NULL) || !Pacci_FuseGetBox(play, part, &box) ||
            !box.dyna) {
            Pacci_BlueprintDeny();
            return 0;
        }
    }
    // The root's box was overwritten by the loop above; nothing below reads it.

    // Validated in full before anything is written or killed: a half-stored structure with
    // half its pieces deleted is not something the player can undo.
    sBlueprint.piece[0].actorId = root->id;
    sBlueprint.piece[0].params = root->params;
    sBlueprint.piece[0].offset.x = 0.0f;
    sBlueprint.piece[0].offset.y = 0.0f;
    sBlueprint.piece[0].offset.z = 0.0f;
    sBlueprint.piece[0].rot.x = 0;
    sBlueprint.piece[0].rot.y = 0;
    sBlueprint.piece[0].rot.z = 0;
    doomed[0] = root;
    n = 1;

    for (i = 0; i < sFuse.count; i++) {
        sBlueprint.piece[n].actorId = sFuse.parts[i].actor->id;
        sBlueprint.piece[n].params = sFuse.parts[i].actor->params;
        sBlueprint.piece[n].offset = sFuse.parts[i].offset;
        sBlueprint.piece[n].rot = sFuse.parts[i].rot;
        doomed[n] = sFuse.parts[i].actor;
        n++;
    }

    // Give every piece its collision and its update back BEFORE killing any of it. Killing an
    // actor whose update we had replaced runs its Destroy against a body we are still holding
    // half-rewired, and a dynapoly piece would take its merged surface to the grave with it.
    Pacci_FuseRelease();
    Pacci_UltrahandLetGo();
    for (i = 0; i < n; i++) {
        if ((doomed[i] != NULL) && (doomed[i]->update != NULL)) {
            Actor_Kill(doomed[i]);
        }
    }

    sBlueprint.used = 1;
    sBlueprint.count = n;
    sBlueprint.scene = play->sceneId;
    Audio_PlaySoundGeneral(NA_SE_SY_GET_ITEM, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    return 1;
}

u8 Pacci_BlueprintSummon(PlayState* play, Player* player) {
    Actor* spawned[PACCI_FUSE_MAX_PARTS + 1];
    Vec3f base;
    s16 yaw = player->actor.focus.rot.y;
    f32 sin = Math_SinS(yaw);
    f32 cos = Math_CosS(yaw);
    u8 built = 0;
    u8 i;

    if (!sBlueprint.used || Pacci_IsHoldingUltrahand() || (sFuse.count > 0)) {
        return 0;
    }
    if (sBlueprint.scene != play->sceneId) {
        Pacci_BlueprintDeny(); // see the note on sBlueprint.scene
        return 0;
    }
    // Magic LAST among the checks and before the first spawn: charging for a summon that
    // then fails to build is the one outcome with no way back.
    if (!Magic_Consume(play, PACCI_UH_SUMMON_COST, MAGIC_CONSUME_NOW)) {
        Pacci_BlueprintDeny();
        return 0;
    }

    base.x = player->actor.world.pos.x + (sin * PACCI_UH_DIST_MIN);
    base.y = player->actor.world.pos.y + PACCI_UH_SUMMON_RISE;
    base.z = player->actor.world.pos.z + (cos * PACCI_UH_DIST_MIN);

    for (i = 0; i < sBlueprint.count; i++) {
        Vec3f off = sBlueprint.piece[i].offset;

        spawned[i] =
            Actor_Spawn(&play->actorCtx, play, sBlueprint.piece[i].actorId,
                        base.x + ((off.x * cos) + (off.z * sin)), base.y + off.y,
                        base.z + ((off.z * cos) - (off.x * sin)), sBlueprint.piece[i].rot.x,
                        sBlueprint.piece[i].rot.y + yaw, sBlueprint.piece[i].rot.z,
                        sBlueprint.piece[i].params);
        if (spawned[i] == NULL) {
            break;
        }
        built++;
    }
    // The actor pool can refuse. Roll the whole thing back rather than leave a half-built
    // structure standing in front of the player with the magic already spent.
    if (built != sBlueprint.count) {
        for (i = 0; i < built; i++) {
            Actor_Kill(spawned[i]);
        }
        Pacci_BlueprintDeny();
        return 0;
    }

    Pacci_UltrahandTake(player, spawned[0]);
    for (i = 1; i < sBlueprint.count; i++) {
        Pacci_FuseAdopt(spawned[0], spawned[i], &sBlueprint.piece[i].offset, &sBlueprint.piece[i].rot);
    }
    // Single use, exactly as asked: the slot empties on recall.
    sBlueprint.used = 0;
    sBlueprint.count = 0;
    return 1;
}

// ============================================================================
// ULTRAHAND MODE
// ============================================================================
//
// C enters the mode and it OWNS the input from then on, which is the whole reason
// it can afford this many controls: nothing here has to share a button with
// rolling, shielding or item swapping.
//
//   A          grab what you are pointing at; it stays in the air
//              (a second A will glue — not built yet)
//   B          drop it and leave the mode
//   D-pad      ground plane: forward/back and left/right
//   L + D-pad  rotate yaw/pitch
//   R + D-pad  vertical plane: up/down and left/right
//
// Roughly TotK's scheme: grab on A, cancel on B, a held modifier to switch the
// D-pad from rotating to moving.
#define PACCI_UH_ROT_SNAP 0x2000  // 45 degrees per press, TotK-style snapping
// Movement is CONTINUOUS while the pad is held, not one step per press: a single
// nudge of 20 was swallowed whole by the carry's easing, which is why raising and
// lowering looked like it did nothing. Rotation stays per-press — snapping to 45
// degrees is the whole point there.
#define PACCI_UH_MOVE_RATE 6.0f  // height units per FRAME while held
#define PACCI_UH_DIST_RATE 8.0f  // push/pull units per FRAME while held
#define PACCI_UH_SIDE_MIN -120.0f
#define PACCI_UH_SIDE_MAX 120.0f
#define PACCI_UH_HEIGHT_MIN -80.0f
#define PACCI_UH_HEIGHT_MAX 160.0f

u8 Pacci_UltrahandModeActive(void) {
    return sUhMode.active;
}

void Pacci_UltrahandModeEnter(PlayState* play, Player* player) {
    sUhMode.active = 1;
    sUhMode.heightOff = 0.0f;
    sUhMode.sideOff = 0.0f;
    sUhMode.prevDpad = 0;
    sUhMode.summonHold = 0;
    sUhHighlightTarget = NULL;
    Audio_PlaySoundGeneral(NA_SE_SY_GET_ITEM, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

void Pacci_UltrahandModeExit(PlayState* play) {
    if (sUltrahand.held != NULL) {
        // Falls on the way out too. Pacci_UpdateUltrahand keeps being called by the cane's own
        // per-frame handler, so the drop finishes after the mode is gone.
        Pacci_UltrahandBeginDrop();
    }
    Pacci_UhTintClear();
    sUhMode.active = 0;
    sUhMode.heightOff = 0.0f;
    sUhMode.sideOff = 0.0f;
    sUhMode.prevDpad = 0;
    sUhMode.summonHold = 0;
    sUhHighlightTarget = NULL;
    Audio_PlaySoundGeneral(NA_SE_SY_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

// One frame of the mode. Returns 1 while it owns the input, so the caller stops.
u8 Pacci_UltrahandModeUpdate(PlayState* play, Player* player) {
    Input* input;
    u16 cur;
    u8 dpad;
    u8 edge;
    u8 withL;
    u8 withR;

    if (!sUhMode.active) {
        return 0;
    }

    input = &play->state.input[0];
    cur = input->cur.button;

    // Wipe the tint table FIRST, before anything in this frame decides what to light. The draw
    // pass runs after the whole update, so re-registering below still lands in time; clearing
    // at the end instead would leave every actor untinted for the frame it mattered.
    Pacci_UhTintClear();

    // Shake the stick left-right to take the structure apart. R used to hold this job and
    // cannot any more — it is the raise/slide modifier now — and a shake is what TotK asks
    // for anyway, just on a stick the N64 does not have.
    Pacci_FuseWiggleDetach(play, player, input);

    // B always leaves, held object or not.
    if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
        Pacci_UltrahandModeExit(play);
        return 1;
    }

    // L + R + A stores the structure. Tested BEFORE the plain A branch below, which would
    // otherwise weld or drop on the same press and there would be nothing left to store.
    if (CHECK_BTN_ALL(input->press.button, BTN_A) && (cur & BTN_L) && (cur & BTN_R)) {
        Pacci_BlueprintSave(play);
        return 1;
    }

    // Keep the cane's C button down to build the stored structure again. The press that
    // opened the mode is the same one that starts this count, so it reads as one gesture:
    // tap C for the mode, keep holding it to get your building back. Any C is accepted
    // because the mode does not know which of the four the cane is on, and it can only have
    // been opened by that one.
    if (cur & (BTN_CUP | BTN_CDOWN | BTN_CLEFT | BTN_CRIGHT)) {
        if (sUhMode.summonHold < PACCI_UH_SUMMON_HOLD) {
            sUhMode.summonHold++;
            if (sUhMode.summonHold == PACCI_UH_SUMMON_HOLD) {
                Pacci_BlueprintSummon(play, player);
            }
        }
    } else {
        sUhMode.summonHold = 0;
    }

    // A does three things, in order of what is possible right now: commit the weld
    // the preview is showing, else drop what you are holding, else grab. Welding has
    // to come first — if A always dropped, you could never stick a second piece on
    // without letting go of the first.
    if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
        if (Pacci_IsHoldingUltrahand()) {
            if (!Pacci_FuseTryAttach(play)) {
                // Nothing on offer: let it FALL, and stay in the mode. This is the only way a
                // release inside the mode ever happens, so with a plain LetGo here the whole
                // gravity path was unreachable - the actor got its own update back in mid-air,
                // and a bg actor's update has no gravity of its own, so it simply hung there.
                // Safe now that Pacci_CastUltrahand refuses to grab while something is falling.
                Pacci_UltrahandBeginDrop();
            }
        } else {
            Pacci_CastUltrahand(play, player);
        }
        return 1;
    }

    // D-pad edges, detected here rather than read from press.button: the player
    // actor consumes those bits for its own item handling before this runs.
    dpad = 0;
    if (cur & BTN_DUP) {
        dpad |= 1;
    }
    if (cur & BTN_DDOWN) {
        dpad |= 2;
    }
    if (cur & BTN_DLEFT) {
        dpad |= 4;
    }
    if (cur & BTN_DRIGHT) {
        dpad |= 8;
    }
    edge = dpad & ~sUhMode.prevDpad;
    sUhMode.prevDpad = dpad;

    // Runs on any pad STATE, not just an edge: continuous moves need every frame.
    if (Pacci_IsHoldingUltrahand() && (dpad != 0)) {
        withL = (cur & BTN_L) ? 1 : 0;
        withR = (cur & BTN_R) ? 1 : 0;

        // Every function gets exactly ONE binding, and a modifier owns a whole PLANE rather
        // than one axis of it — that is what makes the layout guessable:
        //
        //   D-pad alone            push / pull            (continuous)
        //   L + D-left / D-right   rotate Y  (yaw)        (snapped, per press)
        //   L + D-up   / D-down    rotate X  (pitch)      (snapped, per press)
        //   R + D-up   / D-down    raise / lower          (continuous)
        //   R + D-left / D-right   slide left / right     (continuous)
        if (withL) {
            if (edge & 8) {
                sUltrahand.baseRot.y += PACCI_UH_ROT_SNAP;
            }
            if (edge & 4) {
                sUltrahand.baseRot.y -= PACCI_UH_ROT_SNAP;
            }
            if (edge & 1) {
                sUltrahand.baseRot.x += PACCI_UH_ROT_SNAP;
            }
            if (edge & 2) {
                sUltrahand.baseRot.x -= PACCI_UH_ROT_SNAP;
            }
        } else if (withR) {
            if (dpad & 1) {
                sUhMode.heightOff += PACCI_UH_MOVE_RATE;
            }
            if (dpad & 2) {
                sUhMode.heightOff -= PACCI_UH_MOVE_RATE;
            }
            sUhMode.heightOff = CLAMP(sUhMode.heightOff, PACCI_UH_HEIGHT_MIN, PACCI_UH_HEIGHT_MAX);

            if (dpad & 8) {
                sUhMode.sideOff += PACCI_UH_MOVE_RATE;
            }
            if (dpad & 4) {
                sUhMode.sideOff -= PACCI_UH_MOVE_RATE;
            }
            sUhMode.sideOff = CLAMP(sUhMode.sideOff, PACCI_UH_SIDE_MIN, PACCI_UH_SIDE_MAX);
        } else {
            if (dpad & 1) {
                sUltrahand.distance += PACCI_UH_DIST_RATE;
            }
            if (dpad & 2) {
                sUltrahand.distance -= PACCI_UH_DIST_RATE;
            }
            sUltrahand.distance = CLAMP(sUltrahand.distance, PACCI_UH_DIST_MIN, PACCI_UH_DIST_MAX);

            // Left/right slides here too. The bare D-pad owns the whole GROUND plane, not just
            // the axis running away from Link: up/down is nearer and further, left/right is
            // across. R owns the vertical plane, and the two planes share the left-right axis,
            // so that binding appears in both on purpose rather than by accident.
            if (dpad & 8) {
                sUhMode.sideOff += PACCI_UH_MOVE_RATE;
            }
            if (dpad & 4) {
                sUhMode.sideOff -= PACCI_UH_MOVE_RATE;
            }
            sUhMode.sideOff = CLAMP(sUhMode.sideOff, PACCI_UH_SIDE_MIN, PACCI_UH_SIDE_MAX);
        }
        // Only the snapped rotations click; a continuous move would machine-gun it.
        if (withL && (edge & 15)) {
            Audio_PlaySoundGeneral(NA_SE_SY_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                   &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        }
    }

    // Z puts the orientation back to however the object was sitting when you grabbed it —
    // TotK's ZL. Untangling a piece you have over-rotated is otherwise seven more presses.
    if (Pacci_IsHoldingUltrahand() && CHECK_BTN_ALL(input->press.button, BTN_Z)) {
        sUltrahand.baseRot = sUltrahand.grabRot;
        Audio_PlaySoundGeneral(NA_SE_SY_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    }

    // Nothing grabbed yet? Show what A would take.
    if (!Pacci_IsHoldingUltrahand()) {
        Pacci_HighlightUltrahandTarget(play);
    }

    Pacci_UpdateUltrahand(play, player);
    // Strict order: the carry moves the root, the formation follows it, and only then
    // is it meaningful to ask where a new weld would land.
    Pacci_FuseFollow(play);
    Pacci_FuseUpdatePreview(play, player);
    return 1;
}

// ============================================================================
// TEARDOWN
// ============================================================================

// Putting the cane away drops whatever Ultrahand is carrying — you cannot hold an
// object with a cane you are no longer holding — but it deliberately does NOT undo
// a Flip or a Stone.
//
// That is the entire point of the move: you flip an enemy, switch to your sword,
// and hit it while it is down. Restoring flipped enemies on unequip made the skill
// useless, since the target righted itself the instant you reached for a weapon.
// The effect now runs to completion on its own — the flipped enemy's `update` IS
// our function, so it keeps ticking whether or not the cane is in hand.
//
// The cost stays where the design puts it: the cast locks Link in place for the
// animation with no invincibility, so using Flip is what leaves him open.
void Pacci_DropUltrahand(void) {
    if (sUltrahand.held != NULL) {
        // A fall, not an instant hand-back. This used to have to be instant: the fall was driven
        // from Pacci_UpdateUltrahand, which the cane stops calling the moment it is unequipped,
        // so a drop begun here would have frozen on its first frame. Pacci_UltrahandDropTick
        // runs from CustomItems_Update instead, which does not care what Link is holding.
        Pacci_UltrahandBeginDrop();
    }
}

// Hard teardown: undo EVERYTHING, flips included. Not called on unequip — this is
// for tearing the whole subsystem down (a new file, a full state reset).
void Pacci_ReleaseAll(PlayState* play) {
    Pacci_UhTintClear(); // borrowed draw pointers must not outlive the subsystem
    Pacci_UhLightOff(play);
    for (u8 i = 0; i < PACCI_MAX_AFFECTED; i++) {
        if (sPacciPool[i].actor != NULL) {
            Pacci_Restore(&sPacciPool[i]);
        }
    }
    Pacci_DropUltrahand();
}
