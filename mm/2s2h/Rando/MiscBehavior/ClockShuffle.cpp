#include "ClockShuffle.h"
#include "Rando/Rando.h"
#include "Rando/Logic/Logic.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "overlays/actors/ovl_En_Test4/z_en_test4.h"
}

static constexpr u16 DAWN_TIME = CLOCK_TIME(6, 0);
static constexpr u16 DUSK_TIME = CLOCK_TIME(18, 0);

int Rando::ClockItems::GetHalfDayIndexFromClockItem(RandoItemId clockItemId) {
    switch (clockItemId) {
        case RI_TIME_DAY_1:
            return HALF_DAY1_DAY;
        case RI_TIME_NIGHT_1:
            return HALF_DAY1_NIGHT;
        case RI_TIME_DAY_2:
            return HALF_DAY2_DAY;
        case RI_TIME_NIGHT_2:
            return HALF_DAY2_NIGHT;
        case RI_TIME_DAY_3:
            return HALF_DAY3_DAY;
        case RI_TIME_NIGHT_3:
            return HALF_DAY3_NIGHT;
        default:
            return INVALID;
    }
}

RandoItemId Rando::ClockItems::GetClockItemFromHalfDayIndex(int halfDayIndex) {
    if (halfDayIndex < 0 || halfDayIndex >= HALF_COUNT) {
        return RI_UNKNOWN;
    }

    static const RandoItemId clockItemMap[] = {
        RI_TIME_DAY_1,   // HALF_DAY1_DAY   (index 0)
        RI_TIME_NIGHT_1, // HALF_DAY1_NIGHT (index 1)
        RI_TIME_DAY_2,   // HALF_DAY2_DAY   (index 2)
        RI_TIME_NIGHT_2, // HALF_DAY2_NIGHT (index 3)
        RI_TIME_DAY_3,   // HALF_DAY3_DAY   (index 4)
        RI_TIME_NIGHT_3, // HALF_DAY3_NIGHT (index 5)
    };

    return clockItemMap[halfDayIndex];
}

u8 Rando::ClockItems::GetAllOwnedHalfDaysMask() {
    u8 ownedMask = 0;
    for (int i = 0; i < HALF_COUNT; ++i) {
        if (Rando::Logic::OwnsClockHalfDay(i)) {
            ownedMask |= (1 << i);
        }
    }
    return ownedMask;
}

int Rando::ClockItems::FindEarliestOwnedHalfDay(bool searchFromEnd) {
    if (searchFromEnd) {
        for (int i = HALF_COUNT - 1; i >= 0; --i) {
            if (Rando::Logic::OwnsClockHalfDay(i)) {
                return i;
            }
        }
    } else {
        for (int i = 0; i < HALF_COUNT; ++i) {
            if (Rando::Logic::OwnsClockHalfDay(i)) {
                return i;
            }
        }
    }
    return Rando::ClockItems::INVALID;
}

static int FindNextOwnedHalfDayAfter(int startHalfDay, u8 ownedMask) {
    if (startHalfDay < 0 || startHalfDay >= Rando::ClockItems::HALF_COUNT) {
        return Rando::ClockItems::TERMINAL_STATE;
    }

    for (int halfDayIndex = startHalfDay + 1; halfDayIndex < Rando::ClockItems::HALF_COUNT; ++halfDayIndex) {
        if (ownedMask & (1 << halfDayIndex)) {
            return halfDayIndex;
        }
    }

    return Rando::ClockItems::TERMINAL_STATE;
}

bool Rando::ClockItems::IsClockItem(RandoItemId itemId) {
    return (itemId >= RI_TIME_DAY_1 && itemId <= RI_TIME_NIGHT_3) || itemId == RI_TIME_PROGRESSIVE;
}

bool Rando::ClockItems::IsDayClock(RandoItemId itemId) {
    return itemId == RI_TIME_DAY_1 || itemId == RI_TIME_DAY_2 || itemId == RI_TIME_DAY_3;
}

static bool IsNight(u16 time) {
    return (time < DAWN_TIME) || (time >= DUSK_TIME);
}

static u16 GetConfiguredTerminalTime() {
    int terminalMinutes = RANDO_SAVE_OPTIONS[RO_CLOCK_TERMINAL_TIME];
    return CLOCK_TIME(0, terminalMinutes);
}

static bool IsInTerminalRange(s32 day, u16 time) {
    if (day != 3) {
        return false;
    }

    u16 terminalTime = GetConfiguredTerminalTime();

    if (terminalTime >= DUSK_TIME) {
        return (time >= terminalTime) || (time < DAWN_TIME);
    } else {
        return (time >= terminalTime && time < DAWN_TIME);
    }
}

static int GetHalfDayIndexFromTime(s32 day, u16 time) {
    if (day < 1) {
        return 0;
    }
    int halfDay = (day - 1) * 2;
    if (IsNight(time)) {
        halfDay++;
    }
    return halfDay;
}

static int GetCurrentHalfDayIndex() {
    u16 currentTime = gSaveContext.save.time;
    s32 currentDay = gSaveContext.save.day;

    if (currentDay >= 4 || currentDay == 0) {
        return Rando::ClockItems::TERMINAL_STATE;
    }

    if (IsInTerminalRange(currentDay, currentTime)) {
        return Rando::ClockItems::TERMINAL_STATE;
    }

    bool isNight = IsNight(currentTime);
    return (currentDay - 1) * 2 + (isNight ? 1 : 0);
}

void Rando::ClockShuffle::SetTimeToHalfDayStart(int halfDayIndex) {
    if (halfDayIndex < 0 || halfDayIndex >= Rando::ClockItems::HALF_COUNT) {
        return;
    }

    // Calculate day number and time from half-day index
    u8 dayNumber = (halfDayIndex / 2) + 1;
    u16 startTime = (halfDayIndex & 1) ? DUSK_TIME : DAWN_TIME;

    gSaveContext.save.day = dayNumber;
    gSaveContext.save.time = startTime;
}

// Vanilla reloads scenes at 6 o'clock transitions when there are day/night specific actors.
// Terminal state bypasses that boundary, so we match that behavior when skipping from day to terminal.
// Only triggers for scenes with day-only actors (e.g. Termina Field), not dungeons/grottos.
static bool SceneHasDayOnlyActors() {
    if (gPlayState == nullptr || gPlayState->setupActorList == nullptr) {
        return false;
    }

    s32 numActors = ABS(gPlayState->numSetupActors);
    ActorEntry* actorList = gPlayState->setupActorList;

    for (s32 i = 0; i < numActors; i++) {
        s32 actorHalfDayBit = ((actorList[i].rot.x & 7) << 7) | (actorList[i].rot.z & 0x7F);

        if (actorHalfDayBit == 0) {
            continue;
        }

        bool hasDawnBits = (actorHalfDayBit & HALFDAYBIT_DAWNS) != 0;
        bool hasNightBits = (actorHalfDayBit & HALFDAYBIT_NIGHTS) != 0;
        if (hasDawnBits && !hasNightBits) {
            return true;
        }
    }

    return false;
}

static bool SceneNeedsReloadForTimeSkip(s16 sceneId, int currentHalfDay, bool enteringTerminalState, bool wasDay) {
    if (sceneId == SCENE_BOWLING) {
        return true;
    }

    if (enteringTerminalState && wasDay && SceneHasDayOnlyActors()) {
        return true;
    }

    return false;
}

static void ForceSceneReload() {
    Player* player = GET_PLAYER(gPlayState);

    gPlayState->nextEntrance = gSaveContext.save.entrance;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_FADE_BLACK_FAST;

    Play_SetRespawnData(gPlayState, RESPAWN_MODE_RETURN, gSaveContext.save.entrance, gPlayState->roomCtx.curRoom.num,
                        PLAYER_PARAMS(0xFF, PLAYER_START_MODE_B), &player->actor.world.pos, player->actor.world.rot.y);

    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
    gSaveContext.respawnFlag = 2;
}

static bool CheckSkippedTime(u8* day, u16* time) {
    if (gPlayState->envCtx.sceneTimeSpeed == 0 || Player_InCsMode(gPlayState) || *day >= 4) {
        return false;
    }

    // Terminal state is always accessible regardless of Night 3 ownership
    if (IsInTerminalRange(*day, *time)) {
        return false;
    }

    int currentHalfDay = (*day < 1) ? 0 : ((*day - 1) * 2 + (IsNight(*time) ? 1 : 0));

    if (Rando::Logic::OwnsClockHalfDay(currentHalfDay)) {
        return false;
    }

    int nextHalfDay = Rando::ClockItems::TERMINAL_STATE;
    for (int i = currentHalfDay + 1; i < Rando::ClockItems::HALF_COUNT; ++i) {
        if (Rando::Logic::OwnsClockHalfDay(i)) {
            nextHalfDay = i;
            break;
        }
    }

    if (nextHalfDay == Rando::ClockItems::TERMINAL_STATE) {
        *day = 3;
        *time = GetConfiguredTerminalTime();
    } else {
        *day = (nextHalfDay / 2) + 1;
        *time = (nextHalfDay & 1) ? DUSK_TIME : DAWN_TIME;
    }

    return true;
}

static bool CheckAndSkipUnownedTime(Actor* timeActor) {
    EnTest4* enTest4 = (EnTest4*)timeActor;
    u8 day = gSaveContext.save.day;
    u16 time = gSaveContext.save.time + CLOCK_TIME(0, 1);

    bool wasDay = !IsNight(gSaveContext.save.time);

    if (IsNight(enTest4->prevTime) && !IsNight(time)) {
        day++;
    }

    int currentHalfDay = (day < 1) ? 0 : ((day - 1) * 2 + (IsNight(time) ? 1 : 0));

    if (day < 4 && CheckSkippedTime(&day, &time)) {
        gSaveContext.save.day = day;
        gSaveContext.save.time = time;

        // Detect if we're entering terminal state
        bool enteringTerminalState = (day == 3 && time == GetConfiguredTerminalTime());

        if (SceneNeedsReloadForTimeSkip(gPlayState->sceneId, currentHalfDay, enteringTerminalState, wasDay)) {
            ForceSceneReload();
        }

        if (time == DAWN_TIME) {
            // Set to NIGHT and decrement day so vanilla's dawn transition runs on the next frame.
            // EnTest4 will detect the night-to-day crossing and trigger the proper dawn sequence.
            enTest4->daytimeIndex = 0;
            gSaveContext.save.day--;
        } else {
            enTest4->daytimeIndex = 1;
            Interface_NewDay(gPlayState, gSaveContext.save.day);
            Environment_NewDay(&gPlayState->envCtx);
        }

        // Reset audio state to match the new time period
        Environment_PlaySceneSequence(gPlayState);

        // Reset screen scale to prevent lingering shrink effect from bell tolling
        gSaveContext.screenScaleFlag = false;
        gSaveContext.screenScale = 1000.0f;

        enTest4->prevTime = time - CLOCK_TIME(0, 1);
        return true;
    }
    return false;
}

bool Rando::ClockShuffle::IsTimeOwnedForClockShuffle(s32 day, u16 time) {
    if (!RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE]) {
        return true;
    }
    if (day < 1 || day > 3) {
        return true;
    }
    if (IsInTerminalRange(day, time)) {
        return true;
    }

    int halfDayIndex = GetHalfDayIndexFromTime(day, time);
    return Rando::Logic::OwnsClockHalfDay(halfDayIndex);
}

std::string Rando::ClockShuffle::GetTimeDescriptionForMessage(s32 day, u16 time) {
    if (IsInTerminalRange(day, time)) {
        return "%rFinal Hours%w";
    }

    bool isNight = IsNight(time);
    std::string description = isNight ? "%rNight of the " : "%rDawn of the ";

    if (day == 1) {
        description += "First";
    } else if (day == 2) {
        description += "Second";
    } else if (day == 3) {
        description += "Third";
    } else {
        description += "Unknown";
    }

    description += " Day%w";
    return description;
}

static void ProcessClockShuffleMessage(u16* textId, bool* loadFromMessageTable, bool isSongOfTime) {
    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
    int targetHalfDay;

    if (isSongOfTime) {
        targetHalfDay = Rando::ClockItems::FindEarliestOwnedHalfDay(false);
    } else {
        int currentHalfDay = GetCurrentHalfDayIndex();
        u8 ownedHalfDaysMask = Rando::ClockItems::GetAllOwnedHalfDaysMask();
        targetHalfDay = FindNextOwnedHalfDayAfter(currentHalfDay, ownedHalfDaysMask);
    }

    std::string destinationText;
    if (targetHalfDay == Rando::ClockItems::TERMINAL_STATE) {
        destinationText = "%rFinal Hours%w";
    } else {
        int targetDay = (targetHalfDay / 2) + 1;
        bool isNight = (targetHalfDay & 1);
        destinationText = isNight ? "%rNight of " : "%rDawn of the ";
        if (targetDay == 1) {
            destinationText += "First";
        } else if (targetDay == 2) {
            destinationText += "Second";
        } else if (targetDay == 3) {
            destinationText += "Third";
        }
        destinationText += " Day%w";
    }

    if (isSongOfTime) {
        entry.msg = "Save and return to " + destinationText + "?\n%gYes\nNo\xC2";
    } else {
        entry.msg = "Time moves strangely...\nProceed to " + destinationText + "?\n%gYes\nNo\xC2";
    }

    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

static void EnforceOwnedTime() {
    bool isCycleStart =
        (gSaveContext.save.day == 0 || (gSaveContext.save.day == 1 && gSaveContext.save.time == CLOCK_TIME(6, 0)));
    bool isPendingDayTelop = (CHECK_EVENTINF(EVENTINF_TRIGGER_DAYTELOP) && gSaveContext.respawnFlag == -4);

    if (!isCycleStart && !isPendingDayTelop) {
        return;
    }

    // Determine target half-day index
    int targetHalfDay = 0;
    if (isPendingDayTelop) {
        u8 nextDay = gSaveContext.save.day + 1;
        targetHalfDay = (nextDay < 1) ? 0 : ((nextDay - 1) * 2);
    } else {
        targetHalfDay = 0;
    }

    // Find next owned half-day
    int validHalfDay = Rando::ClockItems::TERMINAL_STATE;

    if (Rando::Logic::OwnsClockHalfDay(targetHalfDay)) {
        validHalfDay = targetHalfDay;
    } else {
        for (int i = targetHalfDay + 1; i < Rando::ClockItems::HALF_COUNT; ++i) {
            if (Rando::Logic::OwnsClockHalfDay(i)) {
                validHalfDay = i;
                break;
            }
        }
    }

    if (validHalfDay != Rando::ClockItems::TERMINAL_STATE) {
        if (validHalfDay == targetHalfDay) {
            return;
        }

        if (validHalfDay & 1) { // Night
            gSaveContext.save.day = (validHalfDay / 2) + 1;
            gSaveContext.save.time = DUSK_TIME;
        } else { // Day
            gSaveContext.save.day = (validHalfDay / 2);
            gSaveContext.save.time = CLOCK_TIME(6, 0) - 1;
        }
    } else {
        // Terminal State
        gSaveContext.save.day = 3;
        gSaveContext.save.time = GetConfiguredTerminalTime();
    }

    if (isPendingDayTelop) {
        CLEAR_EVENTINF(EVENTINF_TRIGGER_DAYTELOP);
        gSaveContext.respawnFlag = -8;
    }
}

void Rando::ClockShuffle::OnFileLoad() {
    bool shouldRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE];

    COND_HOOK(BeforePlayInit, shouldRegister, []() { EnforceOwnedTime(); });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EN_TEST4, shouldRegister, [](Actor* actor, bool* should) {
        // Skip time checks if a transition cutscene is in progress
        if (!CHECK_FLAG_ALL(gSaveContext.eventInf[1], 1 << 7)) {
            if (CheckAndSkipUnownedTime(actor)) {
                *should = false;
                return;
            }
        }
        *should = true;
    });

    COND_ID_HOOK(OnOpenText, 0x1B8A, shouldRegister, [](u16* textId, bool* loadFromMessageTable) {
        ProcessClockShuffleMessage(textId, loadFromMessageTable, true);
    });

    auto onDoubleTime = [](u16* textId, bool* loadFromMessageTable) {
        ProcessClockShuffleMessage(textId, loadFromMessageTable, false);
    };

    COND_ID_HOOK(OnOpenText, 0x1B91, shouldRegister, onDoubleTime);
    COND_ID_HOOK(OnOpenText, 0x1B90, shouldRegister, onDoubleTime);
    COND_ID_HOOK(OnOpenText, 0x1B8F, shouldRegister, onDoubleTime);
    COND_ID_HOOK(OnOpenText, 0x1B92, shouldRegister, onDoubleTime);
    COND_ID_HOOK(OnOpenText, 0x1B8E, shouldRegister, onDoubleTime);

    COND_VB_SHOULD(VB_TIME_UNTIL_MOON_CRASH_CALCULATION, shouldRegister, {
        *should = false;
        u32* timeVar = va_arg(args, u32*);
        u8 ownedHalfDaysMask = Rando::ClockItems::GetAllOwnedHalfDaysMask();
        u32 totalHours = 0;

        for (int halfDayIndex = 0; halfDayIndex < Rando::ClockItems::HALF_COUNT; ++halfDayIndex) {
            if (ownedHalfDaysMask & (1 << halfDayIndex)) {
                totalHours += 12;
            }
        }

        bool shouldIncludeTerminalHours = (GetCurrentHalfDayIndex() == Rando::ClockItems::TERMINAL_STATE) ||
                                          !Rando::Logic::OwnsClockHalfDay(Rando::ClockItems::HALF_DAY3_NIGHT);

        if (shouldIncludeTerminalHours) {
            u16 terminalTime = GetConfiguredTerminalTime();
            u32 terminalZeroed = ZERO_DAY_START(terminalTime);
            u32 timeDiff = (DAY_LENGTH - terminalZeroed) % DAY_LENGTH;
            u32 terminalHours = (timeDiff + CLOCK_TIME_HOUR - 1) / CLOCK_TIME_HOUR;
            totalHours += terminalHours;
        }

        u32 ownedTime = totalHours * CLOCK_TIME_HOUR;
        *timeVar = ownedTime;
    });
}
