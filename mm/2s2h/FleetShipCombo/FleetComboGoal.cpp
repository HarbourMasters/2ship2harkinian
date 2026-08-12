// FleetComboGoal.cpp (MM side) — the combo's "Beat Both Bosses" ending gate.
//
// The goal spans two games, so Majora falling is only half of it. Vanilla sends you straight from
// Majora's Lair to Termina Field with cutsceneIndex 0xFFF7, which is the ending sequence. In a combo
// seed that is wrong unless Ganon is already down: the run is still going, and the player has to be
// able to walk back through the portal and finish Hyrule.
//
// So the first boss to fall records its bit in the SHARED save (FC_GOAL_*), the game is saved so the
// fact survives a quit, and the ending cutscene is dropped — you simply arrive in Termina Field and
// keep playing. Whichever boss dies SECOND sees both bits and lets the real ending run.
//
// OoT's half of this lives in hook_handlers.cpp (VB_SLAY_GANON), which already had the exact shape
// for it: the non-Ganon win conditions there suppress the kill, hand over the check and put Link back
// outside the castle. Skijer's NEI

#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "FleetShipCombo.h"
#include "FleetComboIds.h"
#include <libultraship/bridge/consolevariablebridge.h>

extern "C" {
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "mods/nei_save.h" // NeiSaveData / Nei_Save — shared combo goal flags
extern SaveContext gSaveContext;
}

#define CVAR_COMBO CVarGetInteger("isFleetShipCombo.Enabled", 0)

// Beat Both Bosses is goal mode 0; Triforce Hunt (1) keeps its own counter and is not gated here.
static bool BeatBothBosses() {
    return CVAR_COMBO != 0 && CVarGetInteger("gFleetCombo.GoalMode", 0) == 0;
}

void RegisterFleetComboGoal() {
    // Majora's Lair is left through a scene transition, so the arrival in Termina Field is where the
    // ending would begin. Catching it here rather than inside the boss actor keeps the decomp
    // untouched and works no matter which of Majora's phases landed the final blow.
    COND_ID_HOOK(OnSceneInit, SCENE_00KEIKOKU, BeatBothBosses(), [](s16 sceneId, s8 spawnNum) {
        if (gSaveContext.save.cutsceneIndex != 0xFFF7) {
            return; // ordinary arrival in Termina Field, nothing to do
        }
        NeiSaveData* nei = Nei_Save();
        if (nei == NULL) {
            return;
        }
        nei->comboGoalFlags |= FC_GOAL_MAJORA_BEATEN;
        if (nei->comboGoalFlags & FC_GOAL_GANON_BEATEN) {
            return; // Ganon already fell: this is the real ending, let it play
        }
        // Ganon is still standing, so the run continues. Drop the ending cutscene and leave the
        // player standing in Termina Field.
        gSaveContext.save.cutsceneIndex = 0;
        gSaveContext.nextCutsceneIndex = 0;
        // Persist immediately: the shared flag has to survive a quit here, or Hyrule would never
        // learn that Majora is already down and the run could never be finished. MM has no
        // "save now" call, so this is the owl-save recipe FleetSync already uses.
        if (gPlayState != NULL) {
            bool prevOwl = gSaveContext.save.isOwlSave;
            gSaveContext.save.isOwlSave = true;
            Play_SaveCycleSceneFlags(gPlayState);
            gSaveContext.save.saveInfo.playerData.savedSceneId = gPlayState->sceneId;
            func_8014546C(&gPlayState->sramCtx);
            Sram_SetFlashPagesOwlSave(&gPlayState->sramCtx,
                                      gFlashOwlSaveStartPages[gSaveContext.fileNum * FLASH_SAVE_MAIN_MULTIPLIER],
                                      gFlashOwlSaveNumPages[gSaveContext.fileNum * FLASH_SAVE_MAIN_MULTIPLIER]);
            Sram_StartWriteToFlashOwlSave(&gPlayState->sramCtx);
            gSaveContext.save.isOwlSave = prevOwl;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterFleetComboGoal, { "isFleetShipCombo.Enabled" });
