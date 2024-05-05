#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterTatlISG() {
    REGISTER_VB_SHOULD(GI_VB_PATCH_TATL_ISG, {
        if (CVarGetInteger("gEnhancements.Restorations.TatlISG", 0)) {
            *should = false;
        }
    });
}
