#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterTimeSaversHooks() {
    REGISTER_VB_SHOULD(GI_VB_PLAY_ENTRANCE_CS, {
        if (CVarGetInteger("gEnhancements.TimeSavers.SkipEntranceCutscenes", 0)) {
            *should = false;
        }
    });
    REGISTER_VB_SHOULD(GI_VB_SHOW_TITLE_CARD, {
        if (CVarGetInteger("gEnhancements.TimeSavers.HideTitleCards", 0)) {
            *should = false;
        }
    });
}
