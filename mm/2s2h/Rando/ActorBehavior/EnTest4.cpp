#include "ActorBehavior.h"
#include "Rando/Utils.h"
#include "public/bridge/consolevariablebridge.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

void Rando::ActorBehavior::InitEnTest4Behavior() {
    // Enforce Clocks-as-Items time skip to the next owned half-day and first-load spawn fix
    COND_ID_HOOK(OnActorUpdate, ACTOR_EN_TEST4, IS_RANDO, [](Actor* actor) {
        if (!RANDO_SAVE_OPTIONS[RO_CLOCKS_AS_ITEMS])
            return;
        if (gPlayState == nullptr)
            return;
        if (gPlayState->envCtx.sceneTimeSpeed == 0)
            return;
        if (Play_InCsMode(gPlayState))
            return;

        u8 owned = Rando::TimeUtils::GetOwnedHalfDaysMask();

        // If we already have the current half-day, do nothing. Else, jump forward to next owned half-day.
        u8 day = (u8)gSaveContext.save.day;
        u16 time = gSaveContext.save.time;
        if (day >= 4)
            return;
        int currentHalf = Rando::TimeUtils::GetCurrentHalfDay(day, time);

        if ((owned & (1 << currentHalf)) == 0) {
            int next = 6;
            for (int i = currentHalf + 1; i < 6; ++i) {
                if (owned & (1 << i)) {
                    next = i;
                    break;
                }
            }
            if (next < 6) {
                gSaveContext.save.day = (u8)((next / 2) + 1);
                gSaveContext.save.time = (next & 1) ? CLOCK_TIME(18, 0) : CLOCK_TIME(6, 0);
            } else {
                // No proceeding owned half-day: place at start of Final Hours (D3 00:00) and let vanilla progress.
                // If we're already within Final Hours (D3 00:00 -> D3 06:00), avoid reapplying
                if (gSaveContext.save.day == 3 && gSaveContext.save.time < CLOCK_TIME(6, 0)) {
                    return;
                }

                gSaveContext.save.day = 3;
                gSaveContext.save.time = CLOCK_TIME(0, 0);
                gSaveContext.save.isNight = true;
            }
        }
    });
}
