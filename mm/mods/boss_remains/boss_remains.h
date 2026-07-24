/**
 * boss_remains.h - The four boss remains (Odolwa/Goht/Gyorg/Twinmold) as custom
 * wearable "masks" (Skijer's NEI).
 *
 * The remains are already real u8 MM items (ITEM_REMAINS_ODOLWA..TWINMOLD =
 * 0x5D..0x60), so they fit directly in the u8 buttonItems / dpadItems arrays —
 * no extButtons/u16 infra needed (unlike the spiritual stones). Their worn-on-
 * face geometry is the Moon Child's masks (gMoonChild*MaskDL in object_ob),
 * which ARE face-fitted, drawn on Link's head node.
 *
 * Three phases:
 *   1) Equip a remains to a C or D-pad slot from the MM quest page.
 *   2) Press that button in-game to don/doff it on Link's face (mask toggle).
 *   3) While worn, A does the remains' own action (+ summon a friendly ally) —
 *      phase-3 is a single test action for now.
 */
#ifndef BOSS_REMAINS_H
#define BOSS_REMAINS_H

#include "z64.h"

#ifdef __cplusplus
extern "C" {
#endif

// Phase 1 — equip a boss remains (ITEM_REMAINS_ODOLWA..TWINMOLD) to a C or D-pad
// slot from the MM native kaleido quest page. Call from KaleidoScope_UpdateQuestCursor
// while the cursor is on a remains. Returns true if it consumed a button press.
s32 BossRemains_TryEquipAtCursor(PlayState* play, Input* input);

// Phase 2 — per-frame input tick (from Player_UpdateCommon): a press on the C/D-pad
// button that holds a remains toggles wearing that remains on Link's face.
void BossRemains_TickInput(PlayState* play, Player* player);

// Phase 2 — draw the worn remains mask on Link's face. Call from
// Player_PostLimbDrawGameplay at the PLAYER_LIMB_HEAD node (head matrix current).
void BossRemains_DrawWornMask(PlayState* play, Player* player);

// The currently-worn remains item id (ITEM_REMAINS_*), or ITEM_NONE if none.
s16 BossRemains_GetWorn(void);

// Force-remove any worn remains (e.g. when a native mask is donned or on transform).
void BossRemains_ClearWorn(void);

// Phase 3 (test) — the custom A action while a remains is worn. Returns true if it
// handled the A press (so the default roll is suppressed). Call from the roll trigger.
s32 BossRemains_TryActionA(PlayState* play, Player* player);

// Odolwa A-action accessors, read by z_player.c:
//   BossRemains_RunSpeedMul: 1.5x while Odolwa's running state is active (applied at Player_Action_13),
//   else 1.0x. BossRemains_IsOdolwaRunning: true while that state is active (drives the red trail).
f32 BossRemains_RunSpeedMul(void);
s32 BossRemains_IsOdolwaRunning(void);

// True whenever the Odolwa remains is worn — gates the sword/shield swap + faster sword attacks.
s32 BossRemains_IsOdolwaWorn(void);

// Draw Odolwa's red running afterimage (frozen-pose ghosts of Link, dark-red fog). Call from the
// player draw (z_player.c), next to CustomItems_OverrideDraw. No-op unless Odolwa is running.
void BossRemains_DrawOdolwaTrail(Player* player, PlayState* play);

// Draw Odolwa's sword / shield in Link's hands (called from Player_PostLimbDrawGameplay at the
// LEFT_HAND / RIGHT_HAND limbs, where the native ones are hidden while the Odolwa remains is worn).
void BossRemains_DrawOdolwaSword(PlayState* play, Player* player);
void BossRemains_DrawOdolwaShield(PlayState* play, Player* player);

// Sword swing with magic → fire a moth projectile forward (FD-beam style). Call from the melee-attack
// setup (z_player.c func_80833864). Self-guards on Odolwa-worn + available magic.
void BossRemains_OdolwaSwordMoth(PlayState* play, Player* player);

// ── Goht ─────────────────────────────────────────────────────────────────────
// True while the Goht remains is worn.
s32 BossRemains_IsGohtWorn(void);
// True while the bull charge is running. Read by the walk-off handler (z_player.c func_8083827C) so a
// charging Goht is exempt from the fall action / auto-hop / ledge-grab, exactly like the Goron roll
// (Player_Action_96) — that's what lets a ledge or ramp launch Link instead of dropping him.
s32 BossRemains_IsGohtCharging(void);

// ── Gyorg ────────────────────────────────────────────────────────────────────
// True while the Gyorg remains is worn — drives human Link's Zora-style free 3D dive/swim, water-
// current immunity, "can't walk in water", and damage resilience (each z_player.c hook adds its own
// in-water test on top of this).
s32 BossRemains_IsGyorgWorn(void);
// Spawn Gyorg's fish school (En_Tanron3) near Link. Called from the R (swim) / R+B (land) summon.
void BossRemains_GyorgSummonFish(PlayState* play, Player* player);
// Draw Gyorg's whirlpool funnel (B while swimming) — the spinning vortex that pulls in + damages nearby
// enemies. Call from the player draw (z_player.c), next to the Goht cone. No-op unless it's active.
void BossRemains_DrawGyorgWhirlpool(Player* player, PlayState* play);
// True while any remains that takes over the A button is worn (Odolwa runs, Goht bull-charges) —
// suppresses the roll at its choke point func_80836B3C.
s32 BossRemains_SuppressRoll(void);
// The bull QUAKE POUND (jumpAT hop → landing quake → jumpATend). While Goht is worn the sword is
// unequipped, so this is triggered directly from R+A in BossRemains_GohtPostAction (not the melee path).
s32 BossRemains_GohtQuakeStart(PlayState* play, Player* player);
// Per-frame Goht driver (bull charge speed/anim/crash, quake landing, HESS slide). Call from
// Player_UpdateCommon AFTER the action func so the overrides win.
void BossRemains_GohtPostAction(PlayState* play, Player* player);
// Majora-red bull-charge cone around Link. Call from the player draw, next to the Odolwa trail.
void BossRemains_DrawGohtCone(Player* player, PlayState* play);
// Goht's charging-thunder VFX (growing light orb + crossed bolts) in front of Link while B is held.
// Call from the player draw, next to the cone.
void BossRemains_DrawGohtChargingThunder(Player* player, PlayState* play);

#ifdef __cplusplus
}
#endif

#endif // BOSS_REMAINS_H
