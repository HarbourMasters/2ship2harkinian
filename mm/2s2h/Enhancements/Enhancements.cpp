#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Masks
    RegisterFierceDeityAnywhere();
    
    // Time Savers
    RegisterTimeSaversHooks();
}
