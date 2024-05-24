#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterUnrestrictedItems() {
    REGISTER_VB_SHOULD(GI_VB_ITEM_BE_RESTRICTED, {
        if (CVarGetInteger("gCheats.UnrestrictedItems", 0)) {
            *should = false;
        }
    });
}
