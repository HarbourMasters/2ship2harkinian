#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Cycle
    RegisterEndOfCycleSaveHooks();

    // Masks
    RegisterFierceDeityAnywhere();

    // Restorations
    RegisterSideRoll();
    
    // Time Savers
    RegisterTimeSaversHooks();
}
