#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterPowerCrouchStab() {
    REGISTER_VB_SHOULD(GI_VB_PATCH_POWER_CROUCH_STAB, {
        if (CVarGetInteger("gEnhancements.Restorations.PowerCrouchStab", 0)) {
            *should = false;
        }
    });
}
