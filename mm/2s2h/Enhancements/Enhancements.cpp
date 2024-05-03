#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Cycle
    RegisterEndOfCycleSaveHooks();

    // Masks
    RegisterFierceDeityAnywhere();

    // Songs
    RegisterEnableSunsSong();
    
    // Time Savers
    RegisterTimeSaversHooks();
}
