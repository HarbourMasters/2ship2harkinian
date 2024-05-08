#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterHideTitleCards() {
    REGISTER_VB_SHOULD(GI_VB_SHOW_TITLE_CARD, {
        if (CVarGetInteger("gEnhancements.Cutscenes.HideTitleCards", 0)) {
            *should = false;
        }
    });
}
