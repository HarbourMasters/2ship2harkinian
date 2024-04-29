#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Masks
    RegisterFierceDeityAnywhere();

    // Songs
    RegisterEnableSunsSong();
    
    // Time Savers
    RegisterTimeSaversHooks();
}
