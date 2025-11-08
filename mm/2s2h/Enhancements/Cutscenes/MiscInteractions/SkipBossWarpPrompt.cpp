#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

extern "C" {
#include "functions.h"
}

#define CVAR_NAME CVAR_ENHANCEMENT("Cutscenes.SkipMiscInteractions")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSkipBossWarpPrompt() {
    COND_ID_HOOK(ShouldActorInit, ACTOR_DOOR_WARP1, CVAR,
                 [](Actor* actor, bool* should) { SET_WEEKEVENTREG(WEEKEVENTREG_86_80); });
}

static RegisterShipInitFunc initFunc(RegisterSkipBossWarpPrompt, { CVAR_NAME });
