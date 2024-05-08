#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterNoBlackBars() {
    REGISTER_VB_SHOULD(GI_VB_SHOW_BLACK_BARS, {
        if (CVarGetInteger("gEnhancements.Graphics.NoBlackBars", 0)) {
            *should = false;
        }
    });
}
