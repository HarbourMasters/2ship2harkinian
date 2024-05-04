#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterSkipEntranceCutscenes() {
    REGISTER_VB_SHOULD(GI_VB_PLAY_ENTRANCE_CS, {
        if (CVarGetInteger("gEnhancements.Cutscenes.SkipEntranceCutscenes", 0)) {
            *should = false;
        }
    });
}
