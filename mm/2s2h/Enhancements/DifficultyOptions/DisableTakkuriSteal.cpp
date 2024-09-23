#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "DisableTakkuriSteal.h"

void RegisterDisableTakkuriSteal() {
    REGISTER_VB_SHOULD(GI_VB_THIEF_BIRD_STEAL, {
        if (CVarGetInteger("gEnhancements.Cheats.DisableTakkuriSteal", 0)) {
            *should = false;
        }
    });
}