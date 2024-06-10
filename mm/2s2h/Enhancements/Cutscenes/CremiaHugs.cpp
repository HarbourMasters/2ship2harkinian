#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterCremiaHugs() {
    REGISTER_VB_SHOULD(GI_VB_RANDOM_CREMIA_HUGS, {
        if (CVarGetInteger("gEnhancements.Cutscenes.CremiaHugs", 0)) {
            *should = false;
        }
    });
}
