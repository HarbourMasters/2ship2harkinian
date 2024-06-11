#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterFierceDeityAnywhere() {
    REGISTER_VB_SHOULD(VB_DISABLE_FD_MASK, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            *should = false;
        }
    });

    REGISTER_VB_SHOULD(GI_VB_SWORD_BEAMS_ON_REGULAR_ENEMIES, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            *should = false;
        }
    });
}
