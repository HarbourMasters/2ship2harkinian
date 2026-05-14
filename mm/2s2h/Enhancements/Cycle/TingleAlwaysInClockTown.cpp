#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
}

#define CVAR_NAME "gEnhancements.Cycle.TingleAlwaysInClockTown"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

static ActorEntry* GetTingleActorEntry() {
    ActorEntry* entry = gPlayState->setupActorList;
    for (size_t i = 0; i < gPlayState->numSetupActors; i++, entry++) {
        if (entry->id == ACTOR_EN_BAL) {
            return entry;
        }
    }

    return NULL;
}

static void SetTingleAlwaysInClockTown(s16 sceneId, s8 roomNum) {
    if (sceneId != SCENE_BACKTOWN) {
        return;
    }

    ActorEntry* tingleEntry = GetTingleActorEntry();
    if (tingleEntry == NULL) {
        return;
    }

    tingleEntry->rot.x |= 0x7;
    tingleEntry->rot.z |= 0x7F;
}

static void RegisterTingleAlwaysInClockTown() {
    COND_HOOK(AfterRoomSceneCommands, CVAR, SetTingleAlwaysInClockTown);
}

static RegisterShipInitFunc initFunc(RegisterTingleAlwaysInClockTown, { CVAR_NAME });
