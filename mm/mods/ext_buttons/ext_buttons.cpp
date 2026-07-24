/**
 * ext_buttons.cpp — Extended-button infrastructure accessor API (see ext_buttons.h).
 *
 * Globbed and compiled as its own translation unit by mm/CMakeLists.txt (mods/*.cpp). Exports
 * C-linkage helpers so the u8 buttonItems marker (ITEM_EXT_BUTTON) and the parallel u16
 * extButtons array stay in lockstep.
 */
#include "ext_buttons.h"

extern "C" {

u16 ExtButton_GetItem(s32 form, s32 btn) {
    u8 raw = BUTTON_ITEM_EQUIP(form, btn);
    if (raw == ITEM_EXT_BUTTON) {
        return EXT_BUTTON_ITEM(form, btn);
    }
    return (u16)raw;
}

void ExtButton_SetItem(s32 form, s32 btn, u16 extId) {
    BUTTON_ITEM_EQUIP(form, btn) = ITEM_EXT_BUTTON;
    EXT_BUTTON_ITEM(form, btn) = extId;
}

void ExtButton_ClearItem(s32 form, s32 btn) {
    BUTTON_ITEM_EQUIP(form, btn) = ITEM_NONE;
    EXT_BUTTON_ITEM(form, btn) = 0;
}

} // extern "C"
