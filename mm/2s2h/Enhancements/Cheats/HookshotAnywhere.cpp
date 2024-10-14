#include "HookshotAnywhere.h"
#include <libultraship/bridge.h>
#include "GameInteractor/GameInteractor.h"

void RegisterHookshotAnywhere() {
    REGISTER_VB_SHOULD(VB_BE_HOOKSHOT_SURFACE, {
        if (CVarGetInteger("gCheats.HookshotAnywhere", 0)) {
            *should = true;
        }
    });
}