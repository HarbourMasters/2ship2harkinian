#include "ClockShuffle.h"
#include "Rando/Rando.h"
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64game.h"
}

namespace Rando {

// ============================================================================
// CLOCK ITEM MANAGEMENT
// ============================================================================

namespace ClockItems {

// Internal half-day indices
enum ClockHalfIndex : int {
    HALF_DAY1_DAY = 0,   // Day 1, 6:00 AM - 5:59 PM
    HALF_DAY1_NIGHT = 1, // Day 1, 6:00 PM - 5:59 AM
    HALF_DAY2_DAY = 2,   // Day 2, 6:00 AM - 5:59 PM
    HALF_DAY2_NIGHT = 3, // Day 2, 6:00 PM - 5:59 AM
    HALF_DAY3_DAY = 4,   // Day 3, 6:00 AM - 5:59 PM
    HALF_DAY3_NIGHT = 5, // Day 3, 6:00 PM - 5:59 AM
    TERMINAL_STATE = 6,  // Terminal state (fallback for invalid/end states)
    HALF_COUNT = 6,      // Total number of regular half-days (0-5)
};

// Convert a rando item ID to its corresponding half-day index
int GetHalfDayIndexFromClockItem(RandoItemId clockItemId) {
    switch (clockItemId) {
        case RI_CLOCK_DAY_1:
            return HALF_DAY1_DAY;
        case RI_CLOCK_NIGHT_1:
            return HALF_DAY1_NIGHT;
        case RI_CLOCK_DAY_2:
            return HALF_DAY2_DAY;
        case RI_CLOCK_NIGHT_2:
            return HALF_DAY2_NIGHT;
        case RI_CLOCK_DAY_3:
            return HALF_DAY3_DAY;
        case RI_CLOCK_NIGHT_3:
            return HALF_DAY3_NIGHT;
        default:
            return -1; // Invalid item
    }
}

// Convert a half-day index back to its rando item ID
RandoItemId GetClockItemFromHalfDayIndex(int halfDayIndex) {
    if (halfDayIndex < 0 || halfDayIndex >= HALF_COUNT) {
        return RI_UNKNOWN;
    }

    // Map each half-day index to its corresponding rando item
    static const RandoItemId clockItemMap[] = {
        RI_CLOCK_DAY_1,   // HALF_DAY1_DAY   (index 0)
        RI_CLOCK_NIGHT_1, // HALF_DAY1_NIGHT (index 1)
        RI_CLOCK_DAY_2,   // HALF_DAY2_DAY   (index 2)
        RI_CLOCK_NIGHT_2, // HALF_DAY2_NIGHT (index 3)
        RI_CLOCK_DAY_3,   // HALF_DAY3_DAY   (index 4)
        RI_CLOCK_NIGHT_3, // HALF_DAY3_NIGHT (index 5)
    };

    return clockItemMap[halfDayIndex];
}

bool DoesPlayerOwnHalfDay(int halfDayIndex) {
    if (halfDayIndex < 0 || halfDayIndex >= HALF_COUNT) {
        return false;
    }

    // Get the check ID for this half-day (RC_CLOCK_DAY_1 + index)
    RandoCheckId checkId = static_cast<RandoCheckId>(RC_CLOCK_DAY_1 + halfDayIndex);

    // Check if the player has obtained this check
    return RANDO_SAVE_CHECKS[checkId].obtained;
}

void GivePlayerHalfDay(int halfDayIndex) {
    if (halfDayIndex < 0 || halfDayIndex >= HALF_COUNT) {
        return;
    }

    // Get the check ID for this half-day
    RandoCheckId checkId = static_cast<RandoCheckId>(RC_CLOCK_DAY_1 + halfDayIndex);

    // Mark both the check and cycle as obtained
    RANDO_SAVE_CHECKS[checkId].obtained = true;
    RANDO_SAVE_CHECKS[checkId].cycleObtained = true;
}

void TakeAwayHalfDay(int halfDayIndex) {
    if (halfDayIndex < 0 || halfDayIndex >= HALF_COUNT) {
        return;
    }

    // Get the check ID for this half-day
    RandoCheckId checkId = static_cast<RandoCheckId>(RC_CLOCK_DAY_1 + halfDayIndex);

    // Mark both the check and cycle as not obtained
    RANDO_SAVE_CHECKS[checkId].obtained = false;
    RANDO_SAVE_CHECKS[checkId].cycleObtained = false;
}

u8 GetAllOwnedHalfDaysMask() {
    u8 ownedMask = 0;

    // Check each half-day and set the corresponding bit if owned
    for (int halfDayIndex = 0; halfDayIndex < HALF_COUNT; ++halfDayIndex) {
        if (DoesPlayerOwnHalfDay(halfDayIndex)) {
            ownedMask |= (1 << halfDayIndex);
        }
    }

    return ownedMask;
}

int FindEarliestOwnedHalfDay(bool searchFromEnd) {
    if (searchFromEnd) {
        // Search from the end (latest half-days first)
        for (int halfDayIndex = HALF_COUNT - 1; halfDayIndex >= 0; --halfDayIndex) {
            if (DoesPlayerOwnHalfDay(halfDayIndex)) {
                return halfDayIndex;
            }
        }
    } else {
        // Search from the beginning (earliest half-days first)
        for (int halfDayIndex = 0; halfDayIndex < HALF_COUNT; ++halfDayIndex) {
            if (DoesPlayerOwnHalfDay(halfDayIndex)) {
                return halfDayIndex;
            }
        }
    }

    return -1; // No owned half-days found
}

int FindNextOwnedHalfDayAfter(int startHalfDay, u8 ownedMask) {
    if (startHalfDay < -1 || startHalfDay >= HALF_COUNT) {
        return TERMINAL_STATE; // Invalid input, go to terminal state
    }

    // Search for the next owned half-day after the start point
    for (int halfDayIndex = startHalfDay + 1; halfDayIndex < HALF_COUNT; ++halfDayIndex) {
        if (ownedMask & (1 << halfDayIndex)) {
            return halfDayIndex;
        }
    }

    return TERMINAL_STATE; // No owned half-days found after start point
}

} // namespace ClockItems

namespace ClockShuffle {

// ============================================================================
// INTERNAL TYPES AND DATA
// ============================================================================

// Configuration for each half-day's timing
struct HalfDayTimeConfig {
    u8 dayNumber;  // Which day (1, 2, or 3)
    u16 startTime; // When this half-day begins (6:00 AM or 6:00 PM)
    u16 endTime;   // When this half-day ends (5:59 AM or 5:59 PM)
};

// ============================================================================
// TIME CONFIGURATION DATA
// ============================================================================

constexpr u16 DAWN_TIME = CLOCK_TIME(6, 0);           // 6:00 AM - start of day
constexpr u16 DUSK_TIME = CLOCK_TIME(18, 0);          // 6:00 PM - start of night
constexpr u16 DAWN_END_TIME = CLOCK_TIME(5, 59);      // 5:59 AM - end of night
constexpr u16 DUSK_END_TIME = CLOCK_TIME(17, 59);     // 5:59 PM - end of day
constexpr u16 TERMINAL_STATE_TIME = CLOCK_TIME(0, 0); // 12:00 AM - terminal state time (used for fallback states)
constexpr u16 DAY_0_0559_TIME = CLOCK_TIME(6, 0) - 1; // Day 0, 5:59 AM - Cycle Reset Time
// Vanilla uses CLOCK_TIME(6, 0) - 1 = 16383, NOT CLOCK_TIME(5, 59) = 16338 for cycle resets
// This 45-unit difference has to be accounted for.

// Configuration for each half-day's timing and behavior
constexpr HalfDayTimeConfig HALF_DAY_CONFIGS[] = {
    /* HALF_DAY1_DAY   */ { 1, DAWN_TIME, DUSK_END_TIME }, // Day 1: 6:00 AM - 5:59 PM
    /* HALF_DAY1_NIGHT */ { 1, DUSK_TIME, DAWN_END_TIME }, // Day 1: 6:00 PM - 5:59 AM
    /* HALF_DAY2_DAY   */ { 2, DAWN_TIME, DUSK_END_TIME }, // Day 2: 6:00 AM - 5:59 PM
    /* HALF_DAY2_NIGHT */ { 2, DUSK_TIME, DAWN_END_TIME }, // Day 2: 6:00 PM - 5:59 AM
    /* HALF_DAY3_DAY   */ { 3, DAWN_TIME, DUSK_END_TIME }, // Day 3: 6:00 AM - 5:59 PM
    /* HALF_DAY3_NIGHT */ { 3, DUSK_TIME, DAWN_END_TIME }, // Day 3: 6:00 PM - 5:59 AM
};

// ============================================================================
// INTERNAL STATE MANAGEMENT
// ============================================================================

static int sLastKnownHalfDay = -1;
static bool sIsRedirecting = false;
static int sRedirectTarget = -1;
static HOOK_ID sPlayDestroyHook = 0;

// ============================================================================
// TIME DETECTION AND CONFIGURATION
// ============================================================================

const HalfDayTimeConfig* GetHalfDayTimeConfig(int halfDayIndex) {
    if (halfDayIndex < 0 || halfDayIndex >= ClockItems::HALF_COUNT) {
        return nullptr;
    }

    return &HALF_DAY_CONFIGS[halfDayIndex];
}

bool IsCurrentlyNightTime(u16 gameTime) {
    return (gameTime >= DUSK_TIME) || (gameTime < DAWN_TIME);
}

int GetCurrentHalfDayIndex() {
    const u16 currentTime = gSaveContext.save.time;
    const s32 currentDay = gSaveContext.save.day;

    // Handle moon crash/game over sequence: Day 4 or higher means the moon has fallen and the game is ending.
    if (currentDay >= 4) {
        return ClockItems::TERMINAL_STATE; // Use TERMINAL_STATE as a sentinel for post-crash state.
    }

    // The game uses Day 0 as a transition before placing the player at the correct half-day.
    if (currentDay == 0) {
        return ClockItems::TERMINAL_STATE; // Signal: handle Day 0 as a reset/redirect.
    }

    // This is what ClockShuffle treats as a "buffer" period before the moon crash.
    // TODO: Make the buffer length configurable.
    if (currentDay == 3 && currentTime >= TERMINAL_STATE_TIME && currentTime < DAWN_TIME) {
        return ClockItems::TERMINAL_STATE;
    }

    const bool isNight = IsCurrentlyNightTime(currentTime);

    // Figure out which half-day we're in:
    // - Each day has two halves: day (index 0) and night (index 1)
    // - Day 1's day is index 0, night is 1; Day 2's day is 2, night is 3, etc.
    // - So: (currentDay - 1) * 2 gives us the starting index for that day (0 for Day 1, 2 for Day 2, 4 for Day 3)
    // - If it's night, add 1; if it's day, add 0.
    // Example: Day 2 night → (2-1)*2 + 1 = 2 + 1 = 3
    return (currentDay - 1) * 2 + (isNight ? 1 : 0);
}

// ============================================================================
// TIME MANIPULATION FUNCTIONS
// ============================================================================

// Apply a new time to the game, updating all related state
void SetGameTime(u8 day, u16 time) {
    gSaveContext.save.day = day;
    gSaveContext.save.time = time;
    gSaveContext.save.isNight = IsCurrentlyNightTime(time);
    gSaveContext.save.eventDayCount = day;
}

void SetTimeToHalfDayStart(int halfDayIndex) {
    // Don't try to set time for terminal state
    if (halfDayIndex == ClockItems::TERMINAL_STATE) {
        return;
    }

    // Get the configuration for this half-day
    const HalfDayTimeConfig* config = GetHalfDayTimeConfig(halfDayIndex);

    // Set time to the start of this half-day
    SetGameTime(config->dayNumber, config->startTime);
}

// Force a scene transition to reload the current area
void ForceSceneReload() {
    Player* player = GET_PLAYER(gPlayState);

    // Set up the transition parameters
    gPlayState->nextEntrance = gSaveContext.save.entrance;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_BLACK_FAST;

    // Set up respawn data to return to the same location
    Play_SetRespawnData(&gPlayState->state, RESPAWN_MODE_RETURN, gSaveContext.save.entrance,
                        gPlayState->roomCtx.curRoom.num, PLAYER_PARAMS(0xFF, PLAYER_INITMODE_B),
                        &player->actor.world.pos, player->actor.world.rot.y);

    // Configure the transition
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
    gSaveContext.respawnFlag = 2;
}

// ============================================================================
// TRANSITION MANAGEMENT
// ============================================================================

void ProcessHalfDayTransition(Actor* timeActor, int fromHalfDay, int toHalfDay) {
    // Special half-days are always considered "owned": Terminal state, Day 0, Day 4+
    const bool playerOwnsTarget =
        (toHalfDay == ClockItems::TERMINAL_STATE || toHalfDay < 0 || toHalfDay >= ClockItems::HALF_COUNT)
            ? true
            : ClockItems::DoesPlayerOwnHalfDay(toHalfDay);

    // If they own it, let vanilla logic handle it
    if (playerOwnsTarget) {
        return;
    }

    // Get all owned half-days and find the next one after the target
    const u8 ownedHalfDaysMask = ClockItems::GetAllOwnedHalfDaysMask();
    int nextOwnedHalfDay = ClockItems::FindNextOwnedHalfDayAfter(toHalfDay, ownedHalfDaysMask);

    // Set up redirect state using simple globals
    sIsRedirecting = true;
    sRedirectTarget = nextOwnedHalfDay;

    // Set up a hook to apply the time change after the scene is destroyed
    sPlayDestroyHook = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayDestroy>([nextOwnedHalfDay]() {
        if (nextOwnedHalfDay == ClockItems::TERMINAL_STATE) {
            // Set time to terminal state (Day 3, midnight)
            SetGameTime(3, TERMINAL_STATE_TIME);
        } else {
            // Set time to the start of the next owned half-day
            const HalfDayTimeConfig* targetConfig = GetHalfDayTimeConfig(nextOwnedHalfDay);
            if (targetConfig) {
                SetGameTime(targetConfig->dayNumber, targetConfig->startTime);
            }
        }

        // Update global tracking to the target half-day after the time change
        sLastKnownHalfDay = nextOwnedHalfDay;
        sIsRedirecting = false;
        sRedirectTarget = -1;

        // Clean up the hook
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnPlayDestroy>(sPlayDestroyHook);
        sPlayDestroyHook = 0;
    });

    // Force a scene transition to apply the time change
    ForceSceneReload();
}

// ============================================================================
// EVENT HANDLERS
// ============================================================================

// Called when the time transition actor (EnTest4) is about to update
void OnTimeTransitionDetected(Actor* timeActor, bool* should) {
    // We only handle days 0 through 3
    if (gSaveContext.save.day >= 4) {
        return;
    }

    // Get the current half-day we're transitioning to
    const int currentHalfDay = GetCurrentHalfDayIndex();

    // Only process if we have a previous state and it changed
    if (sLastKnownHalfDay != -1 && currentHalfDay != sLastKnownHalfDay) {
        // Check if we're already processing a redirect
        if (sIsRedirecting) {
            // We're already redirecting somewhere, ignore this transition
            return;
        }

        const bool isTargetTerminalState = (currentHalfDay == ClockItems::TERMINAL_STATE);
        const bool isFromTerminalState = (sLastKnownHalfDay == ClockItems::TERMINAL_STATE);
        const bool playerOwnsTarget = isTargetTerminalState ? true : ClockItems::DoesPlayerOwnHalfDay(currentHalfDay);

        // If transitioning from terminal state, always allow it (terminal state allows transitions out)
        if (isFromTerminalState) {
            // Update tracking and let the actor update normally
            sLastKnownHalfDay = currentHalfDay;
            return;
        }

        if (!playerOwnsTarget) {
            // Prevent the time actor from updating this frame - we'll handle the redirect
            *should = false;
            ProcessHalfDayTransition(timeActor, sLastKnownHalfDay, currentHalfDay);
            return;
        }
    }

    sLastKnownHalfDay = currentHalfDay;
}

// ============================================================================
// PUBLIC API
// ============================================================================

void OnFileLoad() {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EN_TEST4, RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE],
                 [](Actor* actor, bool* should) { OnTimeTransitionDetected(actor, should); });

    // Initial time correction
    if (gPlayState == nullptr) {
        const int earliestOwnedHalfDay = ClockItems::FindEarliestOwnedHalfDay(false);
        if (earliestOwnedHalfDay != -1) {
            SetTimeToHalfDayStart(earliestOwnedHalfDay);
        }
    }

    COND_HOOK(OnPlayDestroy, IS_RANDO && RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE], []() {
        if (gSaveContext.save.day == 0 && gSaveContext.save.time == DAY_0_0559_TIME) {
            if (ClockItems::DoesPlayerOwnHalfDay(ClockItems::HALF_DAY1_DAY)) {
                // Player owns Day 1, allow natural progression
            } else {
                const int earliestOwnedHalfDay = ClockItems::FindEarliestOwnedHalfDay(false);
                if (earliestOwnedHalfDay != -1) {
                    SetTimeToHalfDayStart(earliestOwnedHalfDay);
                }
            }
        }
    });
}

} // namespace ClockShuffle
} // namespace Rando
