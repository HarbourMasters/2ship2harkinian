#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterDisableTakkuriSteal() {
    REGISTER_VB_SHOULD(GI_VB_THIEF_BIRD_STEAL, {
        if (CVarGetInteger("gEnhancements.Cheats.DisableTakkuriSteal", 0)) {
            *should = false;
        }
    });
}