#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Cutscenes.SkipMiscInteractions"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// Skips the cutscene where Kafei runs back to town after completing Sakon's Hideout
static void RegisterSkipBellCutscene() {
    COND_VB_SHOULD(VB_START_CUTSCENE, CVAR, {
        if (gPlayState->sceneId == SCENE_ALLEY) {
            s16* csId = va_arg(args, s16*);
            if (*csId == 12) {
                *should = false;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterSkipBellCutscene, { CVAR_NAME });
