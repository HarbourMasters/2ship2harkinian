#include "Enhancements.h"

void InitEnhancements() {
    // Camera
    RegisterCameraFreeLook();
    RegisterDebugCam();

    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();
    RegisterUnbreakableRazorSword(); 

    // Cycle & Saving
    RegisterEndOfCycleSaveHooks();
    RegisterPermanentOwlStatues();

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
