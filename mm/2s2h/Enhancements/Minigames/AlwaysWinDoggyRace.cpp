#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "Enhancements/Enhancements.h"

extern "C" {
    #include <z64save.h>
    extern SaveContext gSaveContext;
    extern u8 gItemSlots[77];
}


void RegisterAlwaysWinDoggyRace() {
    REGISTER_VB_SHOULD(GI_VB_DOGGY_RACE_SET_MAX_SPEED, {
        uint8_t selectedOption = CVarGetInteger("gEnhancements.Minigames.AlwaysWinDoggyRace", 0);
        if (selectedOption == ALWAYS_WIN_DOGGY_RACE_ALWAYS || (selectedOption == ALWAYS_WIN_DOGGY_RACE_MASKOFTRUTH &&
                                                               (INV_CONTENT(ITEM_MASK_TRUTH) == ITEM_MASK_TRUTH))) {
            *should = true;
        }
    });
}
