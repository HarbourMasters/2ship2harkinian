#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/CustomItem/CustomItem.h"
#include "2s2h/Rando/Rando.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

#define CVAR_NAME CVAR_ENHANCEMENT("Cutscenes.SkipMiscInteractions")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSkipTurtleGoodbye() {
    COND_VB_SHOULD(VB_START_CUTSCENE, CVAR, {
        s16* csId = va_arg(args, s16*);
        if (gPlayState->sceneId == SCENE_SEA) {
            if (*csId == 36) {
                *should = false;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterSkipTurtleGoodbye, { CVAR_NAME });
