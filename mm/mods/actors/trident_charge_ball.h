/**
 * trident_charge_ball.h — Trident (ext sword 3) charged energy ball.
 *
 * The projectiles own their routes and impacts. MM has no shared BossSuperDamage
 * hook, so the max-charge boss volley resolves its FD hits directly and follows
 * split-boss parent links to the root actor.
 *
 * The implementation (trident_charge_ball.c) is TEXT-INCLUDED from
 * extended_equipment.c — it is not a standalone translation unit and is not in
 * the vcxproj. Only the accessors below are exported.
 *
 * Skijer's NEI
 */

#ifndef TRIDENT_CHARGE_BALL_H
#define TRIDENT_CHARGE_BALL_H

#include "z64.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * True while a charge ball is in flight OR inside the post-impact grace window.
 *
 * The grace window is NOT optional. Actors update in category order (PLAYER = 2
 * before BOSS = 9). The ball is updated from the player-side dispatch, so on the
 * frame after its AT lands it sees its own AT_HIT and dies BEFORE the boss gets
 * to read BUMP_HIT — and the boss would then find this predicate already false
 * and silently drop the super hit. This is the exact bug that was chased down
 * for Mario's fireball (sm64_mario_items.c, sFireGraceTimer / MARIO_FB_GRACE).
 */
u8 TridentChargeBall_IsActive(void);

/** Returns the forced FD damage carried by one of this module's projectile actors,
 *  or zero for every unrelated actor. Used by MM's damage-table resolver. */
u8 TridentChargeBall_GetFierceDamage(struct Actor* projectile);

/** Spawn a charged ball at `pos` aimed at the lock-on (or nearest enemy).
 *  `charge01` is the 0..1 charge level and only scales the visual. */
struct Actor* TridentChargeBall_Spawn(PlayState* play, Vec3f* pos, f32 charge01);

/** Per-frame tick for the grace timer. Called from ExtEquip_Update so the
 *  window still expires when no ball is alive. */
void TridentChargeBall_Tick(void);

/** Clears stale actor-pool pointers when a scene/player session is initialized. */
void TridentChargeBall_Forget(void);

/** Phantom Ganon's light ball (flight B): light-arrow damage, homes on the lock-on
 *  or the nearest enemy, drawn with gPhantomEnergyBallDL. Never claims super damage. */
struct Actor* TridentChargeBall_SpawnLight(PlayState* play, Vec3f* pos);

/** The max-charge release. In a boss room the held sphere breaks in place into six
 *  double-speed, 12-damage DMG_SWORD_BEAM (Fierce Deity) light trails; it is never
 *  launched. Outside boss encounters it keeps the ordinary four-seeker burst. */
struct Actor* TridentChargeBall_SpawnMax(PlayState* play, Vec3f* pos, s32 damage);

/** One lit mote, no actor and no collider — the streak under Link's feet as the
 *  Phantom Ganon flight takes off. */
void TridentChargeBall_DropSpark(PlayState* play, Vec3f* pos);

#ifdef __cplusplus
}
#endif

#endif // TRIDENT_CHARGE_BALL_H
