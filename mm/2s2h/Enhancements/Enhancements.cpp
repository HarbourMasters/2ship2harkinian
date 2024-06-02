#include "Enhancements.h"

void InitEnhancements() {
    // Camera
    RegisterCameraInterpolationFixes();
    RegisterCameraFreeLook();
    RegisterDebugCam();

    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();
    RegisterUnbreakableRazorSword();
    RegisterUnrestrictedItems();

    // Clock
    RegisterTextBasedClock();

    // Cycle & Saving
    RegisterEndOfCycleSaveHooks();
    RegisterSavingEnhancements();
    RegisterAutosave();

    // Masks
    RegisterFastTransformation();
    RegisterFierceDeityAnywhere();
    RegisterBlastMaskKeg();
    RegisterNoBlastMaskCooldown();

    // Minigames
    RegisterAlwaysWinDoggyRace();

    // Songs
    RegisterEnableSunsSong();

    // Restorations
    RegisterPowerCrouchStab();
    RegisterSideRoll();
    RegisterTatlISG();
    RegisterVariableFlipHop();

    // Cutscenes
    RegisterCutscenes();

    // Modes
    RegisterPlayAsKafei();

    // Player Movement
    RegisterClimbSpeed();
}
