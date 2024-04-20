#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterFierceDeityAnywhere() {
    REGISTER_VB_SHOULD(GI_VB_DISABLE_FD_MASK, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            *should = false;
        }
    });
}
