#include "RealTimeSync.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "variables.h"
#include <time.h>

time_t Gameplay_GetRealTime() {
    time_t t1, t2;
    struct tm* tms;
    time(&t1);
    tms = localtime(&t1);
    tms->tm_hour = 0;
    tms->tm_min = 0;
    tms->tm_sec = 0;
    t2 = mktime(tms);
    return t1 - t2;
}

static uint32_t realTimeSyncGameStateUpdateHookId = 0;
void RegisterRealTimeSync() {
    if (realTimeSyncGameStateUpdateHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateUpdate>(realTimeSyncGameStateUpdateHookId);
        realTimeSyncGameStateUpdateHookId = 0;
    }

    if (CVarGetInteger("gCheats.RealTimeSync", 0)) {
        realTimeSyncGameStateUpdateHookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
            if (&gSaveContext == nullptr) return;

			const int maxRealDaySeconds = 86400;
			const int maxInGameDayTicks = 65536;

			int secs = (int)Gameplay_GetRealTime();
			float percent = (float)secs / (float)maxRealDaySeconds;

			int newIngameTime = maxInGameDayTicks * percent;

			gSaveContext.save.time = newIngameTime;
        });
    }
}
