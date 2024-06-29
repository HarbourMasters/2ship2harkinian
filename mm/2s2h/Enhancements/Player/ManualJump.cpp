#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterManualJump() {
    REGISTER_VB_SHOULD(GI_VB_MANUAL_JUMP, {
        if (CVarGetInteger("gEnhancements.Player.ManualJump", 0)) {
            *should = false;
        }
    });
}