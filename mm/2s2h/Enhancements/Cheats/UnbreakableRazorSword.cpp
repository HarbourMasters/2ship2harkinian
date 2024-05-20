#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterUnbreakableRazorSword() {
    REGISTER_VB_SHOULD(GI_VB_LOWER_RAZOR_SWORD_DURABILITY, {
        if (CVarGetInteger("gCheats.UnbreakableRazorSword", 0)) {
            *should = false;
        }
    });
}
