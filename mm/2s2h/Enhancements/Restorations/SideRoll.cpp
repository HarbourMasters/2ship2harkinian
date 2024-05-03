#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterSideRoll() {
    REGISTER_VB_SHOULD(GI_VB_PATCH_SIDEROLL, {
        if (CVarGetInteger("gEnhancements.Restorations.SideRoll", 0)) {
            *should = false;
        }
    });
}
