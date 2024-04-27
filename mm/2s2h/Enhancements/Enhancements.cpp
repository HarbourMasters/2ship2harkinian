#include "Enhancements.h"

void InitEnhancements() {
    // Cheats
    RegisterInfiniteCheats();
    RegisterMoonJumpOnL();

    // Masks
    RegisterFierceDeityAnywhere();
    RegisterNoBlastMaskCooldown();

    // Time Savers
    RegisterTimeSaversHooks();
}
