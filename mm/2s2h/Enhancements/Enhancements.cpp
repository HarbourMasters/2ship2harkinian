#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Clock
    RegisterTextBasedClock();
  
    // Cycle
    RegisterEndOfCycleSaveHooks();

    // Masks
    RegisterFastTransformation();
    RegisterFierceDeityAnywhere();

    // Restorations
    RegisterSideRoll();
    
    // Cutscenes
    RegisterSkipEntranceCutscenes();
    RegisterHideTitleCards();

}
