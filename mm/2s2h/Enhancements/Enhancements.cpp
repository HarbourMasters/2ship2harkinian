#include "Enhancements.h"

void InitEnhancements() {
    // Camera
    RegisterCameraFreeLook();
    RegisterDebugCam();

    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();
    RegisterUnbreakableRazorSword(); 

    // Clock
    RegisterTextBasedClock();
  
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
    RegisterTatlISG();
    
    // Cutscenes
    RegisterSkipEntranceCutscenes();
    RegisterHideTitleCards();
    
    // Modes
    RegisterPlayAsKafei();

    // Player Movement
    RegisterClimbSpeed();

}
