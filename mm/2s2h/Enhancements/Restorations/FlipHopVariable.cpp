#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterVariableFlipHop() {
    REGISTER_VB_SHOULD(GI_VB_FLIP_HOP_VARIABLE, {
        if (CVarGetInteger("gEnhancements.Restorations.ConstantFlipsHops", 0)) {
            *should = false;
        }
    });
}
