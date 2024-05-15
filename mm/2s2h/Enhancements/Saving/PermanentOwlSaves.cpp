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

    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeEndOfCycleSave>([]() {
        // Remove Owl Save on time cycle reset.
        if (CVarGetInteger("gEnhancements.Saving.PermanentOwlSaves", 0)) {

            // Reset some gSaveContext stuff that usually happens when loading into a new savefile
            // when there's an owl save present. Fixes some unexpected behaviour that otherwise happens.
            if (gSaveContext.save.isFirstCycle) {
                gSaveContext.save.entrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
                gSaveContext.save.day = 0;
                gSaveContext.save.time = CLOCK_TIME(6, 0) - 1;
            } else {
                gSaveContext.save.entrance = ENTRANCE(CUTSCENE, 0);
                gSaveContext.nextCutsceneIndex = 0;
                gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
            }

            func_80147314(&gPlayState->sramCtx, gSaveContext.fileNum);

            gSaveContext.save.isOwlSave = 0;
        }
    });
}
