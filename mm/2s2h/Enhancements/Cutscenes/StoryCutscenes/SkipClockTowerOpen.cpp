#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
void Flags_SetWeekEventReg(s32 flag);
extern SaveContext gSaveContext;
}

void RegisterSkipClockTowerOpen() {
    REGISTER_VB_SHOULD(GI_VB_PLAY_TRANSITION_CS, {
        // Cutscene where clock tower opens. Will only show if the player is in Clock Town
        if (ENTRANCE(SOUTH_CLOCK_TOWN, 0) && gSaveContext.save.cutsceneIndex == 0xFFF1 &&
            CVarGetInteger("gEnhancements.Cutscenes.SkipStoryCutscenes", 0)) {
            // Setting the respawn flag to 2 will respawn the player where they were before the cutscene, consistent
            // with the normal behavior
            gSaveContext.respawnFlag = 2;
            gSaveContext.save.cutsceneIndex = 0;
            SET_WEEKEVENTREG(WEEKEVENTREG_CLOCK_TOWER_OPENED);
        }
    });
}
