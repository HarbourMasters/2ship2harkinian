#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Masks
    RegisterFierceDeityAnywhere();

    // Restorations
    RegisterPowerCrouchStab();
    
    // Time Savers
    RegisterTimeSaversHooks();
}
