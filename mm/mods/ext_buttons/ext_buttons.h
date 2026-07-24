/**
 * ext_buttons.h — Extended-button infrastructure accessor API.
 *
 * The vanilla per-form C/B button array `equips.buttonItems[form][slot]` is u8, and the full u8
 * ItemId space is essentially exhausted. To C-equip custom items beyond the u8 id space, one
 * reserved u8 (ITEM_EXT_BUTTON, z64item.h) acts as a MARKER: when a slot holds ITEM_EXT_BUTTON,
 * the REAL (u16) id lives in the parallel array shipSaveInfo.extButtons.items[form][slot]
 * (EXT_BUTTON_ITEM macro, z64save.h). This mirrors the DpadSaveInfo/dpadEquips pattern.
 *
 * These accessors are the single place that keeps buttonItems + extButtons in sync. Callable from
 * C (z_parameter.c, the kaleido overlays) and C++ (mods) — the .cpp exports C linkage.
 */
#ifndef EXT_BUTTONS_H
#define EXT_BUTTONS_H

#include "z64.h"

#ifdef __cplusplus
extern "C" {
#endif

// Returns the effective item id for a button slot: the real u16 id from extButtons when the slot is
// marked ITEM_EXT_BUTTON, otherwise the plain u8 buttonItems value (widened to u16).
u16 ExtButton_GetItem(s32 form, s32 btn);

// Equip an extended (u16) item to a button slot: marks buttonItems with ITEM_EXT_BUTTON and stores
// the real id in extButtons. Does NOT touch cButtonSlots — callers set that as needed.
void ExtButton_SetItem(s32 form, s32 btn, u16 extId);

// Clear a button slot: buttonItems -> ITEM_NONE, extButtons -> 0.
void ExtButton_ClearItem(s32 form, s32 btn);

#ifdef __cplusplus
}
#endif

#endif // EXT_BUTTONS_H
