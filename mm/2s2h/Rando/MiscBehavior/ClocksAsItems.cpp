#include "MiscBehavior.h"
#include "Rando/Utils.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

// Apply a one-time correction on file load so we never spawn into an unowned half-day.
void Rando::MiscBehavior::ClockSpawnTime() {
    if (!RANDO_SAVE_OPTIONS[RO_CLOCKS_AS_ITEMS]) {
        return;
    }

    u8 owned = Rando::TimeUtils::GetOwnedHalfDaysMask();
    if (gSaveContext.save.day >= 4) {
        return;
    }

    const u16 sixAM = CLOCK_TIME(6, 0);
    const u16 sixPM = CLOCK_TIME(18, 0);
    int currentHalf = Rando::TimeUtils::GetCurrentHalfDay(gSaveContext.save.day, gSaveContext.save.time);
    if ((owned & (1 << currentHalf)) != 0) {
        return;
    }

    int next = 6;
    for (int i = currentHalf + 1; i < 6; ++i) {
        if (owned & (1 << i)) {
            next = i;
            break;
        }
    }

    if (next < 6) {
        gSaveContext.save.day = (u8)((next / 2) + 1);
        // Nudge past exact boundary to allow scene BGM to start immediately on load
        gSaveContext.save.time = (next & 1) ? (sixPM + CLOCK_TIME_MINUTE) : (sixAM + CLOCK_TIME_MINUTE);
        gSaveContext.save.isNight = ((next & 1) != 0);
    } else {
        // No subsequent owned half exists; place at start of Final Hours (D3 00:00)
        gSaveContext.save.day = 3;
        gSaveContext.save.time = CLOCK_TIME(0, 0);
        gSaveContext.save.isNight = true;
    }
}
