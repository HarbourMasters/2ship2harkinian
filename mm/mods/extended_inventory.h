/**
 * extended_inventory.h - Extended inventory system for custom items
 *
 * Manages a multi-page inventory system (up to 72 total slots).
 * Page 1: Vanilla OOT items (slots 0-23)
 * Page 2: Custom items (slots 24-47)
 * Page 3: MM Masks (slots 48-71) — requires mm.o2r and CVar enabled
 *
 * Page switching: Press L/A button in pause menu to cycle pages.
 */
#ifndef EXTENDED_INVENTORY_H
#define EXTENDED_INVENTORY_H
#include <stdint.h>
#include <stdbool.h>
#include "z64item.h"
#include "nei_save.h" // Skijer's NEI — custom-slot storage (24..71)

// ─── Skijer's NEI — OoT-only item-name → MM compat aliases ───────────────────
// The ported NEI sources reference some OoT item names that do not exist in MM.
// Each is mapped here (guarded with #ifndef so a future central definition in
// z64item.h always wins, exactly like _nei_dmg_compat.h does for DMG_* names):
//   • Names with a real MM-native equivalent alias straight to it (faithful).
//   • Genuinely OoT-only names (no MM analog) get a DISTINCT sentinel id in a
//     free slot of MM's item-id space so the icon-override equality checks
//     compile and only ever match that exact custom id (never a real MM item).
// Free MM item-id gaps used below: 0xDE-0xDF, 0xEC-0xEF, 0xF3-0xF6 (NET=0xF7,
// BOTTOMLESS=0xF8 are taken; 0xF0-0xF2 / 0xFC-0xFE are MM enum entries).
//
// OoT Lens of Truth → MM native Lens of Truth (0x0E). Exact semantic match.
#ifndef ITEM_LENS
#define ITEM_LENS ITEM_LENS_OF_TRUTH
#endif
// MM adult trade-quest items → their real MM-native ItemId enum values.
#ifndef ITEM_MM_MOONS_TEAR
#define ITEM_MM_MOONS_TEAR ITEM_MOONS_TEAR
#endif
#ifndef ITEM_MM_DEED_LAND
#define ITEM_MM_DEED_LAND ITEM_DEED_LAND
#endif
#ifndef ITEM_MM_DEED_SWAMP
#define ITEM_MM_DEED_SWAMP ITEM_DEED_SWAMP
#endif
#ifndef ITEM_MM_DEED_MOUNTAIN
#define ITEM_MM_DEED_MOUNTAIN ITEM_DEED_MOUNTAIN
#endif
#ifndef ITEM_MM_DEED_OCEAN
#define ITEM_MM_DEED_OCEAN ITEM_DEED_OCEAN
#endif
#ifndef ITEM_MM_ROOM_KEY
#define ITEM_MM_ROOM_KEY ITEM_ROOM_KEY
#endif
#ifndef ITEM_MM_LETTER_KAFEI
#define ITEM_MM_LETTER_KAFEI ITEM_LETTER_TO_KAFEI
#endif
#ifndef ITEM_MM_SPECIAL_DELIVERY
#define ITEM_MM_SPECIAL_DELIVERY ITEM_LETTER_MAMA
#endif
// OoT page-0 items (Skijer's NEI MM port) — ids in the free 0xA0..0xA5 gap. (0xCA..0xCF is
// taken by Beetle/Shovel/MinishCap/Lantern/Chateau/Pokeball in custom_items.h; 0xB5 is Mario.)
// NOTE: OUTSIDE any #ifndef guard block (the ITEM_SWORD_MASTER guard is skipped when another
// header already defines it, which silently hid these the first time around).
#ifndef ITEM_DINS_FIRE
#define ITEM_DINS_FIRE 0xA0
#define ITEM_FARORES_WIND 0xA1
#define ITEM_NAYRUS_LOVE 0xA2
#define ITEM_FAIRY_SLINGSHOT 0xA3
#define ITEM_HOOKSHOT_OOT 0xA4
#define ITEM_LONGSHOT_OOT 0xA5
#define ITEM_ROCS_FEATHER 0xA6 // SHIP-VANILLA Roc's Feather (art: gRocsFeatherTex; distinct from the Skijer item)
#endif

// Virtual inventory slots (72+) for OoT page-0 cells without an MM-native slot; backed by
// dedicated NeiSaveData fields via ExtInv_GetOotSlotItem/ExtInv_SetOotSlotItem.
#define VSLOT_DINS 72
#define VSLOT_FARORES 73
#define VSLOT_NAYRUS 74
#define VSLOT_SLINGSHOT 75
#define VSLOT_OOT_BOOMERANG 76
#define VSLOT_OOT_MASKS 77
#define VSLOT_HAMMER 78

uint8_t ExtInv_GetOotSlotItem(int slot);
void ExtInv_SetOotSlotItem(int slot, uint8_t itemId);
uint8_t ExtInv_ItemHasAmmo(uint8_t itemId); // page-0 ammo digits are item-based (layout is remapped)
uint8_t ExtInv_GetItemIconSize(uint16_t itemId); // 24 for SW97 medallion/arrow quest icons, else 32

// OoT swords with no MM analog (icon-override sentinels — distinct, non-MM ids).
#ifndef ITEM_SWORD_MASTER
#define ITEM_SWORD_MASTER 0xDE
#endif
#ifndef ITEM_SWORD_BGS
#define ITEM_SWORD_BGS 0xDF
#endif
#ifndef ITEM_SWORD_KNIFE
#define ITEM_SWORD_KNIFE 0xEC
#endif
// OoT Megaton Hammer / Boomerang — no MM item (sentinels for icon overrides).
#ifndef ITEM_HAMMER
#define ITEM_HAMMER 0xED
#endif
#ifndef ITEM_BOOMERANG
#define ITEM_BOOMERANG 0xEE
#endif
// OoT "Sold Out" — MM has no sold-out item; sentinel id (Item_Give no-ops on it).
#ifndef ITEM_SOLD_OUT
#define ITEM_SOLD_OUT 0xEF
#endif
// OoT medallions (SW97 spell-mode icon sentinels) — no MM analog, distinct ids.
#ifndef ITEM_MEDALLION_FOREST
#define ITEM_MEDALLION_FOREST 0xF3
#endif
#ifndef ITEM_MEDALLION_FIRE
#define ITEM_MEDALLION_FIRE 0xF4
#endif
#ifndef ITEM_MEDALLION_WATER
#define ITEM_MEDALLION_WATER 0xF5
#endif
#ifndef ITEM_MEDALLION_SPIRIT
#define ITEM_MEDALLION_SPIRIT 0xF6
#endif
#ifndef ITEM_MEDALLION_SHADOW
#define ITEM_MEDALLION_SHADOW 0xF9
#endif
#ifndef ITEM_MEDALLION_LIGHT
#define ITEM_MEDALLION_LIGHT 0xFA
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Skijer's NEI — custom inventory slots 24..71 live in gNeiSave (NOT in the
// vanilla SaveContext, which only has items[0..23]). These dispatch helpers
// read/write the right backing store by slot index. Use them anywhere a slot
// could be >= 24.
static inline uint8_t ExtInv_GetSlotItem(int slot) {
    extern SaveContext gSaveContext;
    if (slot >= 0 && slot < 24) {
        uint8_t item = gSaveContext.save.saveInfo.inventory.items[slot]; // 2ship: nested inventory
        // Pictograph Box shares the Lens of Truth slot: when owned, the slot is selectable AND
        // equippable even without the real Lens (the in-game C-button routes to the pictobox, and the
        // icon swaps via ExtInv_GetItemIcon). Without this, an empty Lens slot can't be equipped at all.
        // Skijer's NEI
        if (slot == SLOT_LENS_OF_TRUTH && item == ITEM_NONE) {
            extern unsigned char Picto_IsOwned(void);
            if (Picto_IsOwned()) {
                return ITEM_LENS_OF_TRUTH;
            }
        }
        // Clawshot shares the Hookshot slot and is INDEPENDENT of the base hookshot chain: when the
        // Clawshot is owned but no native hookshot is in the slot, synthesize ITEM_HOOKSHOT so the
        // hookshot cell renders, is cursor-selectable, and equips. Held ITEM_HOOKSHOT + clawshotOwned
        // fires the clawshot variant (Nei_ArmsHookVariant). Mirrors the Pictograph-Box handling above.
        // Skijer's NEI
        if (slot == SLOT_HOOKSHOT && item == ITEM_NONE) {
            if (Nei_Save()->clawshotOwned) {
                return ITEM_HOOKSHOT;
            }
        }
        return item;
    }
    // 2ship: page 2 (visual slots 48..71) = the REAL MM mask inventory (inventory.items[24..47]).
    // MM natively stores its 24 masks there — the mask kaleido page was replaced by the equipment
    // page, so this is where masks are browsed/equipped now ("tal cual" the mask page design).
    if (slot >= 48 && slot < 72) {
        return gSaveContext.save.saveInfo.inventory.items[slot - 24];
    }
    // OoT page-0 virtual slots (Din's/Farore's/Nayru's, slingshot, OoT boomerang, hammer...)
    if (slot >= 72 && slot < 80) {
        return ExtInv_GetOotSlotItem(slot);
    }
    return Nei_GetOwnedItem((uint8_t)slot);
}
static inline void ExtInv_SetSlotItem(int slot, uint8_t itemId) {
    extern SaveContext gSaveContext;
    if (slot >= 0 && slot < 24) {
        gSaveContext.save.saveInfo.inventory.items[slot] = itemId; // 2ship: nested inventory
    } else if (slot >= 48 && slot < 72) {
        gSaveContext.save.saveInfo.inventory.items[slot - 24] = itemId; // real MM mask slots
    } else if (slot >= 72 && slot < 80) {
        ExtInv_SetOotSlotItem(slot, itemId); // OoT page-0 virtual slots
    } else {
        Nei_SetOwnedItem((uint8_t)slot, itemId);
    }
}

// Skijer's NEI — item-action func ptr types (shared with extended_player.h via
// the NEI_ITEM_ACTION_FUNC_TYPES guard so neither header redefines them).
#ifndef NEI_ITEM_ACTION_FUNC_TYPES
#define NEI_ITEM_ACTION_FUNC_TYPES
struct Player;
struct PlayState;
typedef int32_t (*ItemActionUpdateFunc)(struct Player* player, struct PlayState* play);
typedef void (*ItemActionInitFunc)(struct PlayState* play, struct Player* player);
#endif

// Skijer's NEI — rando draw-func ptr type. Same signature/type as ItemTableTypes.h's
// CustomDrawFunc (typedef redefinition to the same type is legal in C11/C++), so
// includers of this header don't have to pull in the item-tables header.
#ifndef NEI_CUSTOM_DRAW_FUNC_TYPE
#define NEI_CUSTOM_DRAW_FUNC_TYPE
struct GetItemEntry;
typedef void (*CustomDrawFunc)(struct PlayState*, struct GetItemEntry*);
#endif

// Skijer's NEI — unified custom-item registry row (single source of truth).
// item: ITEM_xxx (or NEI_NO_ITEM for IA-only rows). slot: page-2/3 inventory
// slot, or NEI_NO_SLOT. ageReq: AGE_REQ_*. iconTex: page-2 icon (NULL = dynamic).
// rg: RandomizerGet for this item (NEI_NO_RG if none / non-uniform). drawFunc:
// rando get-item 3D model (NULL = none). name*: textbox message strings (NULL = none).
#define NEI_NO_ITEM ((int16_t)-1)
#define NEI_NO_SLOT ((uint8_t)0xFF)
#define NEI_NO_RG ((int16_t)0) // RG_NONE

typedef struct {
    int16_t item;
    int16_t ia;
    uint8_t modelGroup;
    uint8_t slot;
    uint8_t ageReq;
    void* iconTex;
    ItemActionUpdateFunc updateFn;
    ItemActionInitFunc initFn;
    CustomDrawFunc drawFunc; // Skijer's NEI
    int16_t rg;              // Skijer's NEI
    const char* nameEn;      // Skijer's NEI
    const char* nameFr;      // Skijer's NEI
    const char* nameDe;      // Skijer's NEI
} NeiItem;

const NeiItem* Nei_FindByItem(int32_t item);
const NeiItem* Nei_FindBySlot(uint8_t slot);
const NeiItem* Nei_FindByRg(int16_t rg); // Skijer's NEI
void ExtInv_DebugGiveAll(void);          // NEI debug: grant all custom items to their slots

typedef struct {
    int currentPage;         // 0 = vanilla, 1 = custom items, 2 = MM masks
    int16_t pageSwitchTimer; // Cooldown to prevent rapid switching
} ExtendedInventoryState;

/**
 * @return Pointer to the global extended inventory state
 */
ExtendedInventoryState* ExtInv_GetState(void);

/**
 * Reset inventory state to defaults (page 0, no cooldown)
 */
void ExtInv_Reset(void);

/**
 * Update inventory state each frame (handles page switch cooldown)
 */
void ExtInv_Update(void);

/**
 * Clamp currentPage if it exceeds max pages (e.g., MM masks CVar toggled off)
 */
void ExtInv_ClampPage(void);

/**
 * @return true if page switch cooldown has elapsed
 */
bool ExtInv_CanSwitchPage(void);

/**
 * Cycle to next page (0 → 1 → 2 → 0, or fewer if MM masks disabled)
 */
void ExtInv_SwitchPage(void);

/**
 * @return Current inventory page (0, 1, or 2)
 */
int ExtInv_GetCurrentPage(void);

/**
 * @return Maximum number of pages (2 or 3 depending on MM masks CVar)
 */
int ExtInv_GetMaxPages(void);

/**
 * @return true if custom items (page 2) CVar is enabled
 */
bool ExtInv_IsCustomItemsEnabled(void);

/**
 * @return true if MM masks inventory CVar is enabled
 */
bool ExtInv_IsMmMasksEnabled(void);

/**
 * @return true if "Only Transformation Masks" sub-option is enabled
 */
bool ExtInv_IsOnlyTransformation(void);

/**
 * Convert visual slot (0-23) to actual inventory slot based on current page
 * @param visualSlot - The slot position shown on screen (0-23)
 * @return Actual inventory slot index (0-71)
 */
int ExtInv_GetInventorySlot(int visualSlot);

/**
 * @param slot - Inventory slot to check
 * @return true if slot belongs to current page
 */
bool ExtInv_IsSlotOnCurrentPage(uint8_t slot);

/**
 * @param slot - Inventory slot
 * @return Page number (0, 1, or 2) where this slot belongs
 */
int ExtInv_GetPageForSlot(uint8_t slot);

/**
 * @param itemId - Item ID (ITEM_xxx constant)
 * @return Age requirement (AGE_REQ_ADULT, AGE_REQ_CHILD, or AGE_REQ_NONE)
 */
uint8_t ExtInv_GetItemAgeReq(uint16_t itemId);

/**
 * @param slot - Inventory slot
 * @return Age requirement for items in this slot
 */
uint8_t ExtInv_GetSlotAgeReq(uint8_t slot);

/**
 * @param row - Equipment row (0=swords, 1=shields, 2=tunics, 3=boots)
 * @param col - Equipment column within row
 * @return Age requirement, accounting for transformation restrictions
 */
uint8_t ExtInv_GetEquipAgeReq(uint8_t row, uint8_t col);

/**
 * @param itemId - Custom item ID
 * @param language - Language index for localization
 * @return Pointer to item name texture, or NULL if not found
 */
void* ExtInv_GetCustomItemNameTex(uint16_t itemId, uint8_t language);

/**
 * @param itemId - Item ID
 * @return Pointer to item icon texture
 */
void* ExtInv_GetItemIcon(uint16_t itemId);

/**
 * Returns the SM64 cap icon directly (decoupled from OOT spells).
 * @param cap - 0 = Vanish, 1 = Metal, 2 = Wing
 * @return Pointer to the cap icon texture, or NULL
 */
void* ExtInv_GetCapIcon(uint8_t cap);

/**
 * @param itemId - Item ID
 * @return Inventory slot for this item, or 0xFF if not found
 */
uint8_t ExtInv_GetItemSlot(uint16_t itemId);

/**
 * @param itemId - MM mask item ID (ITEM_MM_MASK_*)
 * @return 1 if the player owns this MM mask (extended inventory page 3)
 */
int32_t ExtInv_HasMmMask(uint16_t itemId);

/**
 * Trade-mask sale helper for En_Mm (Bunny Hood) / En_Heishi2 (Keaton Mask).
 * If the player does NOT own the given MM counterpart mask, unsets the worn OOT
 * trade mask and gives ITEM_SOLD_OUT; otherwise keeps the (permanent) mask.
 * @param play - Current PlayState
 * @param maskItem - MM mask item ID counterpart (ITEM_MM_MASK_BUNNY / ITEM_MM_MASK_KEATON)
 */
void ExtInv_KeepMmMaskOrSell(struct PlayState* play, uint16_t maskItem);
extern const uint8_t gPage2Items[24];
// 2ship: MM has no adult/child age system (no LINK_AGE_ADULT/CHILD). These keep
// OoT's numeric values (ADULT=0, CHILD=1, NONE=9) so the age-req data tables still
// compile, but the age CHECK functions are neutralized to "always allowed" since
// MM never gates items by Link's age.
#define AGE_REQ_ADULT 0
#define AGE_REQ_CHILD 1
#define AGE_REQ_NONE 9
extern const uint8_t gPage2ItemAgeReqs[24];
// Roc's Feather Skijer and Roc's Cape share slot 24 (progressive upgrade system)
#define SLOT_ROCS 24                // Shared slot for Roc's Feather/Cape progressive
#define SLOT_ROCS_FEATHER_SKIJER 24 // Alias for compatibility
#define SLOT_ROCS_CAPE 24           // Now same slot as Feather (upgrade replaces it)
#define SLOT_WHIP 25
#define SLOT_SPINNER 26
#define SLOT_BOMB_ARROWS 27
#define SLOT_FIRE_ROD 28
#define SLOT_DEMISE_DESTRUCTION 29
#define SLOT_DEKU_LEAF 30
#define SLOT_TIME_GATE 31
#define SLOT_BEETLE 32
#define SLOT_SWITCH_HOOK 33
#define SLOT_ICE_ROD 34
#define SLOT_ZONAI_PERMAFROST 35
#define SLOT_MOGMA_MITTS 36
#define SLOT_GUST_JAR 37
#define SLOT_BALL_AND_CHAIN 38
#define SLOT_DESIRE_SENSOR 39
#define SLOT_LIGHT_ROD 40
#define SLOT_HYLIAS_GRACE 41
#define SLOT_LANTERN 42
#define SLOT_MINISH_CAP 43
#define SLOT_POKEBALL 44
#define SLOT_CANE_OF_SOMARIA 45
#define SLOT_SHOVEL 46
#define SLOT_DOMINION_ROD 47

// Page 3: MM Mask slots (48-71)
#define SLOT_MM_MASK_POSTMAN 48
#define SLOT_MM_MASK_ALL_NIGHT 49
#define SLOT_MM_MASK_BLAST 50
#define SLOT_MM_MASK_STONE 51
#define SLOT_MM_MASK_GREAT_FAIRY 52
#define SLOT_MM_MASK_DEKU 53
#define SLOT_MM_MASK_KEATON 54
#define SLOT_MM_MASK_BREMEN 55
#define SLOT_MM_MASK_BUNNY 56
#define SLOT_MM_MASK_DON_GERO 57
#define SLOT_MM_MASK_SCENTS 58
#define SLOT_MM_MASK_GORON 59
#define SLOT_MM_MASK_ROMANI 60
#define SLOT_MM_MASK_CIRCUS_LEADER 61
#define SLOT_MM_MASK_KAFEI 62
#define SLOT_MM_MASK_COUPLE 63
#define SLOT_MM_MASK_TRUTH 64
#define SLOT_MM_MASK_ZORA 65
#define SLOT_MM_MASK_KAMARO 66
#define SLOT_MM_MASK_GIBDO 67
#define SLOT_MM_MASK_GARO 68
#define SLOT_MM_MASK_CAPTAIN 69
#define SLOT_MM_MASK_GIANT 70
#define SLOT_MM_MASK_FIERCE_DEITY 71

extern const uint8_t gPage3MaskItems[24];
extern const uint8_t gPage3MaskAgeReqs[24];
static inline uint8_t ExtInv_GetPage2AgeReq(uint8_t slot) {
    if (slot >= 24 && slot < 48) {
        return gPage2ItemAgeReqs[slot - 24];
    }
    return 9;
}
static inline bool ExtInv_CheckAgeReqForSlot(uint8_t slot, bool isAdult) {
    uint8_t req = ExtInv_GetSlotAgeReq(slot);
    return (req == 9) || (req == 0 && isAdult) || (req == 1 && !isAdult);
}
static inline bool ExtInv_ShouldRenderGrayscale(uint8_t slot, bool isAdult) {
    return !ExtInv_CheckAgeReqForSlot(slot, isAdult);
}
static inline void ExtInv_InitializePage2Items(void) { // Skijer's NEI
    for (int i = 0; i < 24; i++) {
        if (Nei_GetOwnedItem((uint8_t)(24 + i)) == ITEM_NONE) {
            Nei_SetOwnedItem((uint8_t)(24 + i), gPage2Items[i]);
        }
    }
}
static inline void ExtInv_ClearPage2Items(void) { // Skijer's NEI
    for (int i = 24; i < 48; i++) {
        Nei_SetOwnedItem((uint8_t)i, ITEM_NONE);
    }
}
static inline void ExtInv_GiveItem(uint8_t slot, uint8_t itemId) { // Skijer's NEI
    if (slot >= 24 && slot < 48) {
        Nei_SetOwnedItem(slot, itemId);
    }
}
static inline void ExtInv_SetItemById(uint8_t itemId) { // Skijer's NEI
    uint8_t slot = ExtInv_GetItemSlot(itemId);
    if (slot != 0xFF) {
        ExtInv_SetSlotItem(slot, itemId);
    }
}

// Page 3: MM Mask helpers
// "Only Transformation" mode: transformation masks go to rightmost column (positions 5,11,17,23)
// Deku=pos5(slot53), Goron=pos11(slot59), Zora=pos17(slot65), FierceDeity=pos23(slot71)
#define SLOT_MM_ONLY_DEKU 53
#define SLOT_MM_ONLY_GORON 59
#define SLOT_MM_ONLY_ZORA 65
#define SLOT_MM_ONLY_FIERCE 71

static inline void ExtInv_InitializePage3Masks(void) {
    // No-op: masks are given individually (save editor, randomizer, etc.)
    // This function exists for future initialization if needed
}
static inline void ExtInv_ClearPage3Masks(void) { // Skijer's NEI
    for (int i = 48; i < 72; i++) {
        Nei_SetOwnedItem((uint8_t)i, ITEM_NONE);
    }
}
static inline void ExtInv_GiveMask(uint8_t slot, uint8_t itemId) { // Skijer's NEI
    if (slot >= 48 && slot < 72) {
        Nei_SetOwnedItem(slot, itemId);
    }
}

static inline void ExtInv_VerifyConsistency(void) {
    const int expectedSize = 24;
    const int actualSize = sizeof(gPage2Items) / sizeof(gPage2Items[0]);
    if (actualSize != expectedSize) {}
    const int ageReqSize = sizeof(gPage2ItemAgeReqs) / sizeof(gPage2ItemAgeReqs[0]);
    if (ageReqSize != expectedSize) {}
}

// =============================================================================
// Transformation Mask Item Restriction
//
// When a transformation mask is active, items not in the form's allow list
// are restricted (grayed out in KaleidoScope, can't be equipped/used).
// This integrates with CHECK_AGE_REQ_ITEM/SLOT macros so all existing
// usage sites automatically get the restriction without per-site changes.
// =============================================================================

extern u8 TransformMasks_IsEnabled(void);
extern u8 TransformMasks_IsTransformedAny(void);
extern u8 TransformMasks_IsFDSkinMode(void);
extern u8 TransformMasks_IsItemAllowed(s32 item);
extern u8 TransformMasks_IsSlotAllowed(uint8_t slot);

// Returns true if item is restricted by active transformation mask
static inline bool ExtInv_IsTransformRestricted(int itemId) {
    if (itemId == ITEM_NONE)
        return false; // Empty slot = no restriction
    if (!TransformMasks_IsEnabled() || !TransformMasks_IsTransformedAny())
        return false;
    return !TransformMasks_IsItemAllowed(itemId);
}

// Returns true if the item in a given slot is restricted by active transformation mask
// Uses slot-based arrays (72 elements per form) for direct lookup
static inline bool ExtInv_IsSlotTransformRestricted(uint8_t slot) {
    if (slot >= 72)
        return false;
    if (!TransformMasks_IsEnabled() || !TransformMasks_IsTransformedAny())
        return false;
    return !TransformMasks_IsSlotAllowed(slot);
}

#ifdef __cplusplus
}
#endif
#endif
