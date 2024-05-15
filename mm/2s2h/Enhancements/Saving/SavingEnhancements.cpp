#include <libultraship/libultraship.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include <variables.h>
#include <functions.h>

}

static uint32_t frameCount = 0;
static uint32_t frameInterval = 0;
static uint32_t iconTimer = 0;

void RegisterSavingEnhancements() {
    REGISTER_VB_SHOULD(GI_VB_DELETE_OWL_SAVE, {
        if (CVarGetInteger("gEnhancements.Saving.PermanentOwlSaves", 0)) {
            *should = false;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeEndOfCycleSave>([]() {
        // Remove Owl Save on time cycle reset, needed when persisting owl saves and when
        // creating owl saves without the player being send back to the file select screen.
        if (CVarGetInteger("gEnhancements.Saving.PermanentOwlSaves", 0) ||
            CVarGetInteger("gEnhancements.Saving.PauseSave", 0) ||
            CVarGetInteger("gEnhancements.Saving.Autosave", 0)) {

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

            // Delete Owl Save
            func_80147314(&gPlayState->sramCtx, gSaveContext.fileNum);

            // Set it to not be an owl save so after reloading the save file it doesn't try to load at the owl's position in clock town
            gSaveContext.save.isOwlSave = 0;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {

        if (gPlayState == NULL || !CVarGetInteger("gEnhancements.Saving.Autosave", 0)) {
            return;
        }
        
        Player* player = GET_PLAYER(gPlayState);

        // 5 seconds (100 frames) of showing the owl save icon to signify autosave has happened.
        if (iconTimer != 0) {
            float opacity = 255.0;
            // Fade in icon
            if (iconTimer > 80) {
                opacity = 255.0 - (((iconTimer - 80.0) / 20.0) * 255);
            // Fade out icon
            } else if (iconTimer < 20) {
                opacity = (iconTimer / 20.0) * 255.0;
            }
            Interface_DrawAutosaveIcon(gPlayState, uint16_t(opacity));
            iconTimer--;
        }

        frameCount++;

        // Interval is set in minutes, need to be converted to frames.
        frameInterval = CVarGetInteger("gEnhancements.Saving.AutosaveInterval", 5) * 60 * 20;

        // If chosen interval elapsed and owl save available to create
        if (frameCount > frameInterval && gSaveContext.flashSaveAvailable &&
                gSaveContext.fileNum != 255 && !Player_InBlockingCsMode(gPlayState, player) &&
            gPlayState->pauseCtx.state == 0 && gPlayState->msgCtx.msgMode == 0) {

            // Reset frame count and set icon timer to show autosave icon for 5 seconds (100 frames)
            frameCount = 0;
            iconTimer = 100;

            // Create owl save
            gSaveContext.save.isOwlSave = 1;
            gSaveContext.save.shipSaveInfo.pauseSaveEntrance = gSaveContext.save.entrance;
            Play_SaveCycleSceneFlags(&gPlayState->state);
            gSaveContext.save.saveInfo.playerData.savedSceneId = gPlayState->sceneId;
            func_8014546C(&gPlayState->sramCtx);
            Sram_SetFlashPagesOwlSave(&gPlayState->sramCtx, gFlashOwlSaveStartPages[gSaveContext.fileNum * 2],
                                        gFlashOwlSaveNumPages[gSaveContext.fileNum * 2]);
            Sram_StartWriteToFlashOwlSave(&gPlayState->sramCtx);
            gSaveContext.save.isOwlSave = 0;
            gSaveContext.save.shipSaveInfo.pauseSaveEntrance = -1;
        }
    });
}
