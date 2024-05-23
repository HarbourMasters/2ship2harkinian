#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterInstantPutaway() {
    REGISTER_VB_SHOULD(GI_VB_PUTAWAY_TIMER, {
        if (CVarGetInteger("gEnhancements.PlayerActions.InstantPutaway", 0)) {
            *should = false;
        }
    });
}
