#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Songs.SoTDoesNotResetSpeed"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

static int32_t timeSpeedBackup = 0;

static void RememberTimeSpeed() {
    timeSpeedBackup = gSaveContext.save.timeSpeedOffset;
}

static void RestoreTimeSpeed() {
    gSaveContext.save.timeSpeedOffset = timeSpeedBackup;
}

static void RegisterSoTDoesNotReset() {
    COND_HOOK(BeforeEndOfCycleSave, CVAR, RememberTimeSpeed);
    COND_HOOK(AfterEndOfCycleSave, CVAR, RestoreTimeSpeed);
}

static RegisterShipInitFunc initFunc(RegisterSoTDoesNotReset, { CVAR_NAME });
