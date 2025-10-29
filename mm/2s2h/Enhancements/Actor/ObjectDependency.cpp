#include "public/bridge/consolevariablebridge.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "Rando/Rando.h"

extern "C" {
#include "variables.h"
}

void RegisterObjectDependency() {
    COND_VB_SHOULD(VB_ENABLE_OBJECT_DEPENDENCY, true, {
        if (CVarGetInteger("gDeveloperTools.DisableObjectDependency", 0) ||
            (IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_TRAPS])) {
            *should = false;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterObjectDependency, {});
