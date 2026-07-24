/**
 * Ball and Chain Item — Twilight Princess behavior. Skijer's NEI
 *
 * Hold the item button: Link spins the ball around himself, winding up over
 * ~1.5s while walking slowly and turning with heavy inertia (capped yaw step).
 * Release: the ball launches in a heavy ballistic arc, bounces off the floor
 * (up to 2 hard bounces with quake/rumble), clanks off walls and drops, rests
 * a beat, then retracts along the chain back to Link's hand.
 *
 * Damage: hammer-tier (4) with hammer + fire flags applied at once, so it
 * smashes hammer objects AND melts ice actors via their own damage tables.
 */

#ifndef ITEM_BALLCHAIN_H
#define ITEM_BALLCHAIN_H

#include "z64.h"
#include "../custom_items.h"

// =============================================================================
// States
// =============================================================================
#define BALLCHAIN_STATE_INACTIVE 0 // Not equipped
#define BALLCHAIN_STATE_EQUIP 1    // Holding ball, can walk slowly
#define BALLCHAIN_STATE_SPINNING 2 // Spinning around Link, winding up
#define BALLCHAIN_STATE_THROWN 3   // Ball flying/bouncing/retracting, Link braced in place

// Thrown sub-phases (bcPhase) — TP arc lifecycle. Skijer's NEI
#define BALLCHAIN_PHASE_FLY 0     // Ballistic flight (gravity + bounces)
#define BALLCHAIN_PHASE_REST 1    // Resting a beat on the ground after the last bounce
#define BALLCHAIN_PHASE_RETRACT 2 // Reeling back along the chain to Link's hand

// =============================================================================
// Physics Constants (all per 20fps logic frame) — Skijer's NEI
// =============================================================================

// Launch (heavy TP arc)
#define BALLCHAIN_LAUNCH_SPEED_MIN 22.0f // Launch speed with no wind-up
#define BALLCHAIN_LAUNCH_SPEED_MAX 32.0f // Launch speed fully wound up
#define BALLCHAIN_LAUNCH_VY 7.0f         // Extra upward kick at launch (arc height)
#define BALLCHAIN_GRAVITY (-1.5f)        // Gravity on the flying ball
#define BALLCHAIN_TERMINAL_VY 30.0f      // Fall speed clamp

// Ground bounce / rest
#define BALLCHAIN_BOUNCE_FACTOR 0.45f  // velocity.y kept (inverted) on a floor bounce
#define BALLCHAIN_BOUNCE_XZ_KEEP 0.65f // XZ speed kept on a floor bounce
#define BALLCHAIN_MAX_BOUNCES 2        // Hard floor bounces before the ball settles
#define BALLCHAIN_REST_FRAMES 12       // Beat on the ground before retracting (~0.6s)

// Wall bounce — the ball REFLECTS off walls (ricochet) instead of dropping dead against them.
#define BALLCHAIN_WALL_BOUNCE_FACTOR 0.55f // XZ speed kept after reflecting off a wall normal

// Chain / retract
#define BALLCHAIN_CHAIN_MAX 380.0f    // Chain length — the ball can never fly past this
#define BALLCHAIN_RETRACT_SPEED 34.0f // Reel-in speed back to Link's hand
#define BALLCHAIN_RETURN_DIST 65.0f   // Distance to consider "returned"
#define BALLCHAIN_THROWN_TIMEOUT 200  // Hard safety: force the ball back after 10s

// Throw pose
#define BALLCHAIN_THROW_LEAN 3000 // Upper body forward lean while the ball is out

// Spin orbit (TP: ball orbits Link, speed ramps up over ~1.5s)
#define BALLCHAIN_SPIN_RADIUS 20.0f     // Orbit radius around Link (tight, OoT-style — the radius the user likes)
#define BALLCHAIN_SPIN_HEIGHT_MIN 28.0f // Orbit height at spin start
#define BALLCHAIN_SPIN_HEIGHT_MAX 38.0f // Orbit height fully wound up
#define BALLCHAIN_SPIN_SPEED_MIN 0x600  // Starting spin angular velocity (~1.4s/rev)
#define BALLCHAIN_SPIN_SPEED_MAX 0x2200 // Max spin angular velocity (~0.38s/rev)
#define BALLCHAIN_CHARGE_MAX 30         // Frames to full wind-up (1.5s @20fps)

// Heavy movement while spinning
#define BALLCHAIN_TURN_RATE 0x380     // Capped yaw step per frame (TP heavy inertia)
#define BALLCHAIN_SPIN_WALK_MULT 0.3f // Walk speed multiplier while spinning
#define BALLCHAIN_SPEED_MULT 0.4f     // Walk speed multiplier while just holding the ball

// Equip state — TWO-HANDED grip: the held ball sits at the MIDPOINT of both hands (the same point
// the chain is drawn from), so it stays centered between Link's hands for every facing. Skijer's NEI
#define BALLCHAIN_EQUIP_HEIGHT 20.0f
#define BALLCHAIN_EQUIP_Y_OFFSET 5.0f // small drop below the two-hand midpoint
#define BALLCHAIN_EQUIP_SCALE 0.06f
#define BALLCHAIN_SPIN_SCALE 0.1f

// Pose lean
#define BALLCHAIN_LEAN_MULT 3500.0f          // Upper body lean factor
#define BALLCHAIN_LEAN_TILT 40.0f            // Orbit tilt from stick
#define BALLCHAIN_STICK_DEADZONE 8.0f        // Lib_GetControlStickData magnitude deadzone

// =============================================================================
// Collision
// =============================================================================
#define BALLCHAIN_COL_RADIUS 20
#define BALLCHAIN_COL_HEIGHT 20
#define BALLCHAIN_WALL_RADIUS 20.0f
#define BALLCHAIN_WALL_HEIGHT 20.0f
// Proximity reach for shattering ICE actors — the throw is fast (tunnels past thin icicles), so ice
// is destroyed by proximity here instead of relying on the collider overlapping. Skijer's NEI
#define BALLCHAIN_ICE_REACH 140.0f       // icicles + ice enemies (freezard/Eeno)
#define BALLCHAIN_BIGICE_REACH 240.0f    // LARGE blocks (Obj_Ice_Poly / Bigicicle / Iceblock) — huge actors,
                                         // measured to world.pos, so they need a bigger reach to register
#define BALLCHAIN_BREAKABLE_REACH 80.0f // pots/crates/grass/rocks
#define BALLCHAIN_BALL_RADIUS 12.0f // Visual/floor-contact radius of the ball
#define BALLCHAIN_DAMAGE 4          // Hammer tier

// Goron-punch damage ONLY — NO fire. Skijer's NEI:
//  - A fire flag would light torches (Obj_Syokudai) and other fire triggers, which we do NOT want.
//  - The fast throw/retract tunnels past thin ice + small enemies, so ALL ice is destroyed by
//    PROXIMITY, each driving its OWN native break/melt so drops/path-flags/puzzles fire, no fire
//    needed (BallChain_CheckDestructibles):
//      * Bg_Icicle / Obj_Bigicicle break on ANY player AC hit — we raise their collider AC_HIT flag.
//      * Obj_Ice_Poly (red meltable ice) needs a strict fire hit OR its switch flag — we set the flag.
//      * Obj_Iceblock (frozen-enemy cube) — we zero its meltTimer (native fire/timer melt path);
//        the ICEBERG platform variant is skipped so we don't destroy something you stand on.
//      * En_Fz (Freezard) — we drive its fire-melt damage-effect (2) → native melt + loot drop.
//      * En_Snowman (Eeno) — we call its native EnSnowman_SetupMelt → melt + loot drop.
//    (Ice puzzle/pushable blocks and ice platforms are otherwise left alone — melting breaks progression.)
#define BALLCHAIN_DMG_FLAGS (DMG_GORON_PUNCH | DMG_GORON_POUND)

// =============================================================================
// Sound Effects
// =============================================================================
#define BALLCHAIN_SFX_SWING NA_SE_IT_HAMMER_SWING
#define BALLCHAIN_SFX_HIT NA_SE_IT_HAMMER_HIT             // enemy hit + ground thud
#define BALLCHAIN_SFX_WHOOSH NA_SE_IT_SWORD_SWING_HARD    // once per orbit revolution
// Retract clink — MUST be a ONE-SHOT. NA_SE_IT_HOOKSHOT_CHAIN is a LOOPING sample (the hookshot
// re-triggers it every frame as a flagged sfx); fired via Audio_PlaySoundGeneral it starts a loop
// that never stops — that was the "sonido de volver a mano nunca se detiene" bug. Skijer's NEI
#define BALLCHAIN_SFX_CHAIN NA_SE_IT_HOOKSHOT_REFLECT      // one-shot metallic clink while reeling in
#define BALLCHAIN_SFX_WALL_BOUNCE NA_SE_IT_SHIELD_BOUND   // metal clank on wall hit
#define BALLCHAIN_SFX_VOICE_ADULT NA_SE_VO_LI_SWORD_N
#define BALLCHAIN_SFX_VOICE_CHILD NA_SE_VO_LI_SWORD_N_KID

// =============================================================================
// State Aliases (mapped to gCustomItemState fields)
// =============================================================================
#define bcActive gCustomItemState.ballAndChainThrown     // u8: Item is active
#define bcState gCustomItemState.timer2                  // s16: Current state (INACTIVE/EQUIP/SPINNING/THROWN)
#define bcCharge gCustomItemState.timer1                 // s16: Wind-up frames (0..CHARGE_MAX); thrown-safety counter
#define bcSpinAngle gCustomItemState.somariaCooldown     // s16: Current spin angle (binary angle)
#define bcThrowYaw gCustomItemState.sharedYaw            // s16: Link facing / throw direction yaw
#define bcBallPos gCustomItemState.sharedProjectilePos   // Vec3f: Ball world position
#define bcBallVel gCustomItemState.ballAndChainVel       // Vec3f: Ball velocity (thrown) — Skijer's NEI
#define bcPhase gCustomItemState.ballAndChainPhase       // u8: Thrown sub-phase — Skijer's NEI
#define bcBounces gCustomItemState.ballAndChainBounces   // u8: Floor bounces this throw — Skijer's NEI
#define bcRestTimer gCustomItemState.ballAndChainRestTimer // s16: Rest beat / retract clink counter — Skijer's NEI
#define bcCollider gCustomItemState.ballAndChainCollider // ColliderCylinder: Damage collider
#define bcFirstPerson gCustomItemState.ballAndChainFirstPersonActive // u8: First person aim mode
#define bcTrailIndex gCustomItemState.ballAndChainTrailIndex   // s32: EffectBlure trail index — Skijer's NEI
#define bcTrailActive gCustomItemState.ballAndChainTrailActive // u8: trail allocated
#define bcTrailTick gCustomItemState.ballAndChainTrailTick     // u8: sparse-feed frame counter

#endif
