#ifndef RANDO_UTILS_H
#define RANDO_UTILS_H

#include "Rando/Types.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

namespace Rando {
namespace TimeUtils {

static inline u8 GetOwnedHalfDaysMask() {
    u8 owned = 0;
    owned |= Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY1) ? (1 << 0) : 0;
    owned |= Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT1) ? (1 << 1) : 0;
    owned |= Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY2) ? (1 << 2) : 0;
    owned |= Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT2) ? (1 << 3) : 0;
    owned |= Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY3) ? (1 << 4) : 0;
    owned |= Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT3) ? (1 << 5) : 0;
    return owned;
}

static inline int GetCurrentHalfDay(u8 day, u16 time) {
    const u16 sixAM = CLOCK_TIME(6, 0);
    const u16 sixPM = CLOCK_TIME(18, 0);
    int currentHalf;
    if (day == 0) {
        currentHalf = 0;
    } else {
        bool night = (time < sixAM) || (time >= sixPM);
        currentHalf = (int)((day - 1) * 2 + (night ? 1 : 0));
    }
    if (currentHalf < 0)
        currentHalf = 0;
    if (currentHalf > 5)
        currentHalf = 5;
    return currentHalf;
}

static inline int HalfIndexFromClockItem(RandoItemId randoItemId) {
    switch (randoItemId) {
        case RI_CLOCK_DAY_1:
            return 0;
        case RI_CLOCK_NIGHT_1:
            return 1;
        case RI_CLOCK_DAY_2:
            return 2;
        case RI_CLOCK_NIGHT_2:
            return 3;
        case RI_CLOCK_DAY_3:
            return 4;
        case RI_CLOCK_NIGHT_3:
            return 5;
        default:
            return -1;
    }
}

static inline void OwnHalfDay(int halfIndex) {
    switch (halfIndex) {
        case 0:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY1);
            break;
        case 1:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT1);
            break;
        case 2:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY2);
            break;
        case 3:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT2);
            break;
        case 4:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY3);
            break;
        case 5:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT3);
            break;
        default:
            return;
    }
    gSaveContext.save.shipSaveInfo.rando.unlockedHalfDays |= (1 << halfIndex);
}

static inline void UnownHalfDay(int halfIndex) {
    switch (halfIndex) {
        case 0:
            Flags_ClearRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY1);
            break;
        case 1:
            Flags_ClearRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT1);
            break;
        case 2:
            Flags_ClearRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY2);
            break;
        case 3:
            Flags_ClearRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT2);
            break;
        case 4:
            Flags_ClearRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY3);
            break;
        case 5:
            Flags_ClearRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT3);
            break;
        default:
            return;
    }
    gSaveContext.save.shipSaveInfo.rando.unlockedHalfDays &= ~(1 << halfIndex);
}

static inline bool HasOwnedHalf(int halfIndex) {
    switch (halfIndex) {
        case 0:
            return Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY1);
        case 1:
            return Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT1);
        case 2:
            return Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY2);
        case 3:
            return Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT2);
        case 4:
            return Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_DAY3);
        case 5:
            return Flags_GetRandoInf(RANDO_INF_OBTAINED_CLOCK_NIGHT3);
        default:
            return false;
    }
}

static inline RandoItemId ClockItemFromHalfIndex(int halfIndex) {
    switch (halfIndex) {
        case 0:
            return RI_CLOCK_DAY_1;
        case 1:
            return RI_CLOCK_NIGHT_1;
        case 2:
            return RI_CLOCK_DAY_2;
        case 3:
            return RI_CLOCK_NIGHT_2;
        case 4:
            return RI_CLOCK_DAY_3;
        case 5:
            return RI_CLOCK_NIGHT_3;
        default:
            return RI_UNKNOWN;
    }
}

// Return the first owned half-day index to remove based on progressive mode ordering
// descending == true: check order 0,1,2,3,4,5 (Day1 -> Night3)
// descending == false: check order 5,4,3,2,1,0 (Night3 -> Day1)
static inline int FindOwnedHalfForProgressiveRemoval(bool descending) {
    if (descending) {
        for (int i = 0; i <= 5; ++i) {
            if (HasOwnedHalf(i))
                return i;
        }
    } else {
        for (int i = 5; i >= 0; --i) {
            if (HasOwnedHalf(i))
                return i;
        }
    }
    return -1;
}

} // namespace TimeUtils
} // namespace Rando

#endif
