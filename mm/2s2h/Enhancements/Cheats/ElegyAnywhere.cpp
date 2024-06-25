#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterElegyAnywhere() {
    REGISTER_VB_SHOULD(GI_VB_ELEGY_IN_IKANA_ONLY, {
        if (CVarGetInteger("gCheats.ElegyAnywhere", 0)) {
            *should = true;
        }
    });
}
