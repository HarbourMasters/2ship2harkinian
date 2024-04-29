#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Masks
    RegisterFierceDeityAnywhere();

    // Minigames
    RegisterAlwaysWinDoggyRace();
    
    // Time Savers
    RegisterTimeSaversHooks();
}
