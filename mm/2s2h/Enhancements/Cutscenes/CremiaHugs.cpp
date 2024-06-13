#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "Enhancements/Enhancements.h"

extern "C" {
#include <z64save.h>
}

void RegisterCremiaHugs() {
    REGISTER_VB_SHOULD(GI_VB_RANDOM_COMPARISON, {
        uint8_t selectedOption = CVarGetInteger("gEnhancements.Cutscenes.CremiaHugs", 0);
        if (selectedOption == CREMIA_REWARD_ALWAYS_HUG) {
            *should = true;
        } else if (selectedOption == CREMIA_REWARD_ALWAYS_RUPEE) {
            *should = false;
        }
    });
}
