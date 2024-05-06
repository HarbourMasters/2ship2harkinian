#include <libultraship/libultraship.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include <variables.h>
#include <functions.h>
}

void RegisterPermanentOwlStatues() {
    REGISTER_VB_SHOULD(GI_VB_DELETE_OWL_SAVE, {
        if (CVarGetInteger("gEnhancements.Saving.PermanentOwlSaves", 0)) {
            *should = false;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::AfterEndOfCycleSave>([]() {
        // Remove Owl Save on time cycle reset.
        if (CVarGetInteger("gEnhancements.Saving.PermanentOwlSaves", 0)) {
            func_80147314(&gPlayState->sramCtx, gSaveContext.fileNum);
        }
    });
}
