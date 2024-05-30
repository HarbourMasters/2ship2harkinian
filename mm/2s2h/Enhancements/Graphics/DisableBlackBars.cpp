#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "DisableBlackBars.h"

void RegisterDisableBlackBars() {
    REGISTER_VB_SHOULD(GI_VB_DISABLE_LETTERBOX, {
        if (CVarGetInteger("gEnhancements.Graphics.DisableBlackBars", 0)) {
            *should = true;
        }
    });
}
