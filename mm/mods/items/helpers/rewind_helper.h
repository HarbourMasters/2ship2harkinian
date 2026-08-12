/**
 * rewind_helper.h - Shared trajectory recorder / rewinder (Skijer's NEI)
 *
 * Backs the Phantom Hourglass' Tears-of-the-Kingdom style Recall.
 *
 * DESIGN — this is deliberately NOT a savestate. Only the MOTION of an actor is
 * recorded: world position, rotation, velocity and speed. Health, magic, rupees,
 * inventory, actor damage state, chest/switch/scene flags and anything in
 * gSaveContext are never touched, so rewinding an object leaves all of that
 * exactly as it is. Kill an enemy, rewind the rock that killed it — the enemy
 * stays dead. That is the intended behaviour, and it comes for free from
 * recording transforms only.
 *
 * Recording is a fixed static pool — no allocation. Slot 0 is permanently
 * reserved for Link (needed by the L+R self-rewind, which must be able to look
 * backwards at history the player never explicitly selected); the remaining
 * slots track the nearest interesting actors on a rolling basis.
 *
 * Everything is a no-op until Rewind_SetEnabled(1), so the module costs nothing
 * until the Phantom Hourglass exists.
 */

#ifndef REWIND_HELPER_H
#define REWIND_HELPER_H

#include "z64.h"

#define REWIND_SLOTS 8              // 1 for Link + 7 world actors
#define REWIND_FRAMES 200           // ~10 s of history at 20 fps gameplay
#define REWIND_TRACK_RANGE 700.0f   // how far out world actors get recorded
#define REWIND_RESCAN_INTERVAL 20   // frames between pool re-evaluations
#define REWIND_LINK_SLOT 0

/** One recorded frame. Motion only — see the header comment. */
typedef struct {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ Vec3f velocity;
    /* 0x18 */ Vec3s rot;
    /* 0x1E */ s16 pad;
    /* 0x20 */ f32 speed;
} RewindFrame; // size = 0x24

/** Master switch. Off (the default) makes Rewind_Tick a single compare. */
void Rewind_SetEnabled(u8 enabled);
u8 Rewind_IsEnabled(void);

/**
 * Per-frame recorder. Call unconditionally from CustomItems_Update.
 * Recording pauses while the world is frozen (nothing meaningful moves, and it
 * would otherwise flood the ring with duplicate frames) and for any actor that
 * is currently being scrubbed.
 */
void Rewind_Tick(PlayState* play);

/** Non-zero if `actor` has at least `minFrames` of usable history. */
s32 Rewind_HasHistory(struct Actor* actor, s32 minFrames);

/** How many recorded frames `actor` has (0 if untracked). */
s32 Rewind_GetLength(struct Actor* actor);

/**
 * Enter scrub mode for `actor`. The read cursor starts at the newest frame and
 * the actor stops being recorded until Rewind_End. Returns 0 if it has no history.
 */
s32 Rewind_Begin(struct Actor* actor);

/**
 * Move the read cursor by `dir` frames (negative = into the past, positive =
 * back toward the present) and write that transform onto the actor.
 * @return 0 when the cursor is already at the requested end of the buffer.
 */
s32 Rewind_Scrub(struct Actor* actor, s32 dir);

/**
 * Leave scrub mode.
 * @param keepMomentum non-zero restores the velocity recorded at the cursor
 *                     (object carries on as it was); zero drops it to rest,
 *                     which is what ToTK's Recall does when it lets go.
 */
void Rewind_End(struct Actor* actor, u8 keepMomentum);

/** Non-zero while `actor` is being scrubbed (NULL asks "is anything scrubbing?"). */
s32 Rewind_IsScrubbing(struct Actor* actor);

/** Drop the whole pool. Called automatically on scene change. */
void Rewind_Reset(void);

#endif // REWIND_HELPER_H
