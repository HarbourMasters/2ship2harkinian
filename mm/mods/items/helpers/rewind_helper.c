/**
 * rewind_helper.c - Shared trajectory recorder / rewinder (Skijer's NEI) — MM backend
 *
 * See rewind_helper.h for the contract. MM specifics: actor lists are `.first`
 * and the explosives category is ACTORCAT_EXPLOSIVES.
 */

#include "rewind_helper.h"
#include "timestop_helper.h"
#include "functions.h"
#include "macros.h"
#include "variables.h"

#define REWIND_LIST_HEAD(list) ((list).first)

// Categories worth recording. Anything the player can plausibly want to recall:
// thrown/rolling props, enemies, bombs, and the misc/BG movers (platforms, blocks).
static const u8 sRewindCats[] = {
    ACTORCAT_PROP, ACTORCAT_ENEMY, ACTORCAT_EXPLOSIVES, ACTORCAT_MISC, ACTORCAT_BG,
};

typedef struct {
    Actor* actor;
    RewindFrame frames[REWIND_FRAMES];
    s16 count;    // valid frames (<= REWIND_FRAMES)
    s16 writeIdx; // next slot to write
    s16 cursor;   // frames behind newest while scrubbing (0 = present)
    u8 used;
    u8 scrubbing;
} RewindSlot;

static RewindSlot sRewindSlots[REWIND_SLOTS];
static u8 sRewindEnabled = 0;
static u8 sRewindSeen[REWIND_SLOTS];
static s32 sRewindFrameCounter = 0;
static s16 sRewindLastSceneId = -1;

// ---------------------------------------------------------------------------
// Slot bookkeeping
// ---------------------------------------------------------------------------

static s32 Rewind_FindSlot(Actor* actor) {
    s32 i;

    if (actor == NULL) {
        return -1;
    }
    for (i = 0; i < REWIND_SLOTS; i++) {
        if (sRewindSlots[i].used && (sRewindSlots[i].actor == actor)) {
            return i;
        }
    }
    return -1;
}

static void Rewind_ClearSlot(s32 idx) {
    sRewindSlots[idx].actor = NULL;
    sRewindSlots[idx].count = 0;
    sRewindSlots[idx].writeIdx = 0;
    sRewindSlots[idx].cursor = 0;
    sRewindSlots[idx].used = 0;
    sRewindSlots[idx].scrubbing = 0;
}

static void Rewind_BindSlot(s32 idx, Actor* actor) {
    Rewind_ClearSlot(idx);
    sRewindSlots[idx].actor = actor;
    sRewindSlots[idx].used = 1;
}

/** Push the actor's current transform onto its ring. */
static void Rewind_Record(RewindSlot* slot) {
    RewindFrame* f = &slot->frames[slot->writeIdx];
    Actor* actor = slot->actor;

    f->pos = actor->world.pos;
    f->velocity = actor->velocity;
    f->rot = actor->world.rot;
    f->speed = actor->speed;

    slot->writeIdx = (slot->writeIdx + 1) % REWIND_FRAMES;
    if (slot->count < REWIND_FRAMES) {
        slot->count++;
    }
}

/** The frame `back` steps behind the newest one (back == 0 is the present). */
static RewindFrame* Rewind_FrameAt(RewindSlot* slot, s32 back) {
    s32 idx;

    if ((back < 0) || (back >= slot->count)) {
        return NULL;
    }
    idx = slot->writeIdx - 1 - back;
    while (idx < 0) {
        idx += REWIND_FRAMES;
    }
    return &slot->frames[idx % REWIND_FRAMES];
}

/**
 * Write a recorded transform back onto the actor.
 * prevPos is set alongside world.pos and bgCheckFlags cleared so the engine does
 * not treat the jump as a collision sweep and snap the actor to a wall — the same
 * trick the Switch Hook's swap uses. home.pos is deliberately left alone: it is
 * an actor's patrol/spawn anchor, and moving it would corrupt AI.
 */
static void Rewind_Apply(Actor* actor, RewindFrame* f, u8 restoreVelocity) {
    actor->world.pos = f->pos;
    actor->prevPos = f->pos;
    actor->world.rot = f->rot;
    actor->shape.rot = f->rot;
    actor->bgCheckFlags = 0;

    if (restoreVelocity) {
        actor->velocity = f->velocity;
        actor->speed = f->speed;
    } else {
        actor->velocity.x = 0.0f;
        actor->velocity.y = 0.0f;
        actor->velocity.z = 0.0f;
        actor->speed = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void Rewind_SetEnabled(u8 enabled) {
    if (!enabled && sRewindEnabled) {
        Rewind_Reset();
    }
    sRewindEnabled = enabled;
}

u8 Rewind_IsEnabled(void) {
    return sRewindEnabled;
}

s32 Rewind_GetLength(Actor* actor) {
    s32 idx = Rewind_FindSlot(actor);

    return (idx < 0) ? 0 : sRewindSlots[idx].count;
}

s32 Rewind_HasHistory(Actor* actor, s32 minFrames) {
    return Rewind_GetLength(actor) >= minFrames;
}

s32 Rewind_Begin(Actor* actor) {
    s32 idx = Rewind_FindSlot(actor);

    if ((idx < 0) || (sRewindSlots[idx].count < 2)) {
        return 0;
    }
    sRewindSlots[idx].cursor = 0;
    sRewindSlots[idx].scrubbing = 1;
    return 1;
}

s32 Rewind_Scrub(Actor* actor, s32 dir) {
    s32 idx = Rewind_FindSlot(actor);
    RewindSlot* slot;
    RewindFrame* f;
    s32 newCursor;

    if ((idx < 0) || (actor == NULL) || (actor->update == NULL)) {
        return 0;
    }
    slot = &sRewindSlots[idx];
    if (!slot->scrubbing) {
        return 0;
    }

    // dir < 0 walks into the past, which means a LARGER cursor.
    newCursor = slot->cursor - dir;
    if (newCursor < 0) {
        newCursor = 0;
    } else if (newCursor > (slot->count - 1)) {
        newCursor = slot->count - 1;
    }
    if (newCursor == slot->cursor) {
        // Already pinned at an end — still hold the actor at that frame so it
        // does not drift, but report "no movement" so the caller can stop.
        f = Rewind_FrameAt(slot, slot->cursor);
        if (f != NULL) {
            Rewind_Apply(actor, f, 0);
            actor->freezeTimer = TIMECTL_FREEZE_REFRESH;
        }
        return 0;
    }

    slot->cursor = (s16)newCursor;
    f = Rewind_FrameAt(slot, slot->cursor);
    if (f == NULL) {
        return 0;
    }
    Rewind_Apply(actor, f, 0);
    // The actor must not run its own update while we drive it, or its AI would
    // immediately fight the position we just wrote.
    actor->freezeTimer = TIMECTL_FREEZE_REFRESH;
    return 1;
}

void Rewind_End(Actor* actor, u8 keepMomentum) {
    s32 idx = Rewind_FindSlot(actor);
    RewindSlot* slot;
    RewindFrame* f;

    if (idx < 0) {
        return;
    }
    slot = &sRewindSlots[idx];
    slot->scrubbing = 0;

    if ((actor != NULL) && (actor->update != NULL)) {
        f = Rewind_FrameAt(slot, slot->cursor);
        if (f != NULL) {
            Rewind_Apply(actor, f, keepMomentum);
        }
        actor->freezeTimer = 0;
    }

    // History after the release point is now fiction — drop it so a second
    // rewind does not replay a future that never happened.
    if (slot->cursor > 0) {
        s32 drop = slot->cursor;

        slot->writeIdx = slot->writeIdx - drop;
        while (slot->writeIdx < 0) {
            slot->writeIdx += REWIND_FRAMES;
        }
        slot->writeIdx %= REWIND_FRAMES;
        slot->count -= (s16)drop;
        if (slot->count < 0) {
            slot->count = 0;
        }
    }
    slot->cursor = 0;
}

s32 Rewind_IsScrubbing(Actor* actor) {
    s32 i;

    if (actor != NULL) {
        s32 idx = Rewind_FindSlot(actor);
        return (idx >= 0) && sRewindSlots[idx].scrubbing;
    }
    for (i = 0; i < REWIND_SLOTS; i++) {
        if (sRewindSlots[i].used && sRewindSlots[i].scrubbing) {
            return 1;
        }
    }
    return 0;
}

void Rewind_Reset(void) {
    s32 i;

    for (i = 0; i < REWIND_SLOTS; i++) {
        Rewind_ClearSlot(i);
    }
    sRewindFrameCounter = 0;
}

// ---------------------------------------------------------------------------
// Per-frame pool maintenance + recording
// ---------------------------------------------------------------------------

/**
 * Fill free slots with the nearest untracked candidates. Only runs on the rescan
 * interval — rebinding every frame would thrash slots (and wipe their history)
 * whenever two actors trade places in the distance ordering.
 */
static void Rewind_RefillPool(PlayState* play, Player* player) {
    u32 c;
    s32 i;

    for (i = 1; i < REWIND_SLOTS; i++) {
        Actor* best = NULL;
        f32 bestDistSq = REWIND_TRACK_RANGE * REWIND_TRACK_RANGE;

        if (sRewindSlots[i].used) {
            continue;
        }
        for (c = 0; c < ARRAY_COUNT(sRewindCats); c++) {
            Actor* actor = REWIND_LIST_HEAD(play->actorCtx.actorLists[sRewindCats[c]]);

            while (actor != NULL) {
                if ((actor->update != NULL) && (Rewind_FindSlot(actor) < 0)) {
                    f32 dx = actor->world.pos.x - player->actor.world.pos.x;
                    f32 dy = actor->world.pos.y - player->actor.world.pos.y;
                    f32 dz = actor->world.pos.z - player->actor.world.pos.z;
                    f32 distSq = (dx * dx) + (dy * dy) + (dz * dz);

                    if (distSq < bestDistSq) {
                        bestDistSq = distSq;
                        best = actor;
                    }
                }
                actor = actor->next;
            }
        }
        if (best == NULL) {
            break; // nothing left in range — the remaining slots stay free
        }
        Rewind_BindSlot(i, best);
    }
}

void Rewind_Tick(PlayState* play) {
    Player* player;
    u32 c;
    s32 i;

    if (!sRewindEnabled || (play == NULL)) {
        return;
    }

    // Actor pointers do not survive a scene load, and neither does the history.
    if (sRewindLastSceneId != (s16)play->sceneId) {
        sRewindLastSceneId = (s16)play->sceneId;
        Rewind_Reset();
        return;
    }

    player = GET_PLAYER(play);
    if (player == NULL) {
        return;
    }

    // Slot 0 is Link's, always. Rebinding wipes it, which is correct: a different
    // Player actor means a different life.
    if (sRewindSlots[REWIND_LINK_SLOT].actor != &player->actor) {
        Rewind_BindSlot(REWIND_LINK_SLOT, &player->actor);
    }

    // Drop slots whose actor has died. Actor memory is pooled, so a stale pointer
    // could otherwise alias a freshly spawned actor — validate by presence in the
    // live lists rather than by dereferencing the pointer.
    for (i = 1; i < REWIND_SLOTS; i++) {
        sRewindSeen[i] = 0;
    }
    for (c = 0; c < ARRAY_COUNT(sRewindCats); c++) {
        Actor* actor = REWIND_LIST_HEAD(play->actorCtx.actorLists[sRewindCats[c]]);

        while (actor != NULL) {
            s32 idx = Rewind_FindSlot(actor);

            if (idx > 0) {
                sRewindSeen[idx] = 1;
            }
            actor = actor->next;
        }
    }
    for (i = 1; i < REWIND_SLOTS; i++) {
        if (sRewindSlots[i].used && !sRewindSeen[i]) {
            Rewind_ClearSlot(i);
        }
    }

    sRewindFrameCounter++;
    if ((sRewindFrameCounter % REWIND_RESCAN_INTERVAL) == 0) {
        Rewind_RefillPool(play, player);
    }

    // Nothing moves during a hard time stop, so recording would just fill the
    // ring with duplicates and throw away real history.
    if (TimeCtl_IsFrozen()) {
        return;
    }

    for (i = 0; i < REWIND_SLOTS; i++) {
        if (sRewindSlots[i].used && !sRewindSlots[i].scrubbing && (sRewindSlots[i].actor != NULL) &&
            (sRewindSlots[i].actor->update != NULL)) {
            Rewind_Record(&sRewindSlots[i]);
        }
    }
}
