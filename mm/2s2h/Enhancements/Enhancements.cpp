#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Cycle
    RegisterEndOfCycleSaveHooks();

    // Masks
    RegisterFastTransformation();
    RegisterFierceDeityAnywhere();
    RegisterNoBlastMaskCooldown();

    // Minigames
    RegisterAlwaysWinDoggyRace();

    // Songs
    RegisterEnableSunsSong();
    
    // Restorations
    RegisterPowerCrouchStab();
    RegisterSideRoll();
    
    // Cutscenes
    RegisterSkipEntranceCutscenes();
    RegisterHideTitleCards();
    
    // Modes
    RegisterPlayAsKafei();

}
