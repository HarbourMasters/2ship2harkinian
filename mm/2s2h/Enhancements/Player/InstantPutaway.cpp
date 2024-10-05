#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterInstantPutaway() {
    REGISTER_VB_SHOULD(VB_RESET_PUTAWAY_TIMER, {
        if (CVarGetInteger("gEnhancements.Player.InstantPutaway", 0)) {
            *should = false;
        }
    });
}
