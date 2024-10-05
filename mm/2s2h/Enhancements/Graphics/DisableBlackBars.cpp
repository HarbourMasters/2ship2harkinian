#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterDisableBlackBars() {
    REGISTER_VB_SHOULD(VB_DISABLE_LETTERBOX, {
        if (CVarGetInteger("gEnhancements.Graphics.DisableBlackBars", 0)) {
            *should = true;
        }
    });
}
