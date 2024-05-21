#include <libultraship/libultraship.h>
#include "BenPort.h"
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include <variables.h>
#include <functions.h>
}

static uint32_t autosaveInterval = 0;
static uint32_t iconTimer = 0;
static uint64_t currentTimestamp = 0;
static uint64_t lastSaveTimestamp = GetUnixTimestamp();

static uint32_t autosaveGameStateUpdateHookId = 0;

void DeleteOwlSave() {
    // Remove Owl Save on time cycle reset, needed when persisting owl saves and/or when
    // creating owl saves without the player being send back to the file select screen.

    // Delete Owl Save
    func_80147314(&gPlayState->sramCtx, gSaveContext.fileNum);

    // Set it to not be an owl save so after reloading the save file it doesn't try to load at the owl's position in
    // clock town
    gSaveContext.save.isOwlSave = 0;
}

void DrawAutosaveIcon() {
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
}

void HandleAutoSave() {
    // Check if the interval has passed in minutes.
    autosaveInterval = CVarGetInteger("gEnhancements.Saving.AutosaveInterval", 5) * 60000;
    currentTimestamp = GetUnixTimestamp();
    if ((currentTimestamp - lastSaveTimestamp) < autosaveInterval) {
        return;
    }

    Player* player = GET_PLAYER(gPlayState);
    if (player == NULL) {
        return;
    }

    // If owl save available to create, do it and reset the interval.
    if (gSaveContext.flashSaveAvailable && gSaveContext.fileNum != 255 &&
        !Player_InBlockingCsMode(gPlayState, player) && gPlayState->pauseCtx.state == 0 &&
        gPlayState->msgCtx.msgMode == 0) {

        // Reset timestamp, set icon timer to show autosave icon for 5 seconds (100 frames)
        lastSaveTimestamp = GetUnixTimestamp();
        iconTimer = 100;

        // Create owl save
        gSaveContext.save.isOwlSave = true;
        gSaveContext.save.shipSaveInfo.pauseSaveEntrance = gSaveContext.save.entrance;
        Play_SaveCycleSceneFlags(&gPlayState->state);
        gSaveContext.save.saveInfo.playerData.savedSceneId = gPlayState->sceneId;
        func_8014546C(&gPlayState->sramCtx);
        Sram_SetFlashPagesOwlSave(&gPlayState->sramCtx, gFlashOwlSaveStartPages[gSaveContext.fileNum * 2],
                                  gFlashOwlSaveNumPages[gSaveContext.fileNum * 2]);
        Sram_StartWriteToFlashOwlSave(&gPlayState->sramCtx);
        gSaveContext.save.isOwlSave = false;
        gSaveContext.save.shipSaveInfo.pauseSaveEntrance = -1;
    }
}

void RegisterSavingEnhancements() {
    REGISTER_VB_SHOULD(GI_VB_DELETE_OWL_SAVE, {
        if (CVarGetInteger("gEnhancements.Saving.PersistentOwlSaves", 0)) {
            *should = false;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeEndOfCycleSave>([]() { 
        DeleteOwlSave();
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeMoonCrashSaveReset>([]() { 
        DeleteOwlSave();
    });
}

void RegisterAutosave() {
    if (autosaveGameStateUpdateHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateUpdate>(autosaveGameStateUpdateHookId);
        autosaveGameStateUpdateHookId = 0;
    }

    if (CVarGetInteger("gEnhancements.Saving.Autosave", 0)) {
        autosaveGameStateUpdateHookId =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
                if (gPlayState == nullptr) {
                    return;
                }

                DrawAutosaveIcon();
                HandleAutoSave();
            });
    }
}
