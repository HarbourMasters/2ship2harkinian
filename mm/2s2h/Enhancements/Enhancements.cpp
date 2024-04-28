#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Clock
    RegisterTextBasedClock();

    // Masks
    RegisterFierceDeityAnywhere();
    
    // Time Savers
    RegisterTimeSaversHooks();
}
