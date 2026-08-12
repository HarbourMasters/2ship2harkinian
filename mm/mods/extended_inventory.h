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

// OoT child-trade mask wheel (item cell 4,6 -> VSLOT_OOT_MASKS). The 8 masks have no MM item id, so
// ownership is the nei->ootMasksOwned bitmask and the visible one is the nei->ootMaskCursor index —
// same scheme as the unified trade wheel. Defined in extended_inventory.c but used earlier in that
// file (ExtInv_GetOotSlotItem), so they need prototypes here. Skijer 2026-07-30
uint8_t OotMask_IsOwnedIndex(int index);
void OotMask_SetOwnedIndex(int index, uint8_t on);
int OotMask_OwnedCount(void);
int OotMask_OwnedAt(int ordinal);
int OotMask_CursorIndex(void);
int OotMask_NeighborIndex(int dir);
void OotMask_CursorStep(int dir);
uint8_t OotMask_CellItem(void);
uint8_t OotMask_NeighborCellItem(int dir);
const char* OotMask_IconPath(int index);
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
// u16, not u8: page-2 slots (24..47) live in the widened NeiSaveData.ownedItems and can hold an EXT
// id above 0xFF. The vanilla-backed ranges (0..23 page 1, 48..71 MM masks) are still u8 values —
// they read from gSaveContext inventory.items — they just come back widened. Skijer's NEI
static inline uint16_t ExtInv_GetSlotItem(int slot) {
    extern SaveContext gSaveContext;
    if (slot >= 0 && slot < 24) {
        uint16_t item = gSaveContext.save.saveInfo.inventory.items[slot]; // 2ship: nested inventory
        // (The Pictograph Box used to be synthesized onto the Lens of Truth cell here, back when it
        // was a NEI custom item. MM has it natively in its own slot, so the Lens cell is just the
        // Lens again.)
        // Clawshot shares the Hookshot slot and is INDEPENDENT of the base hookshot chain: when the
        // Clawshot is owned but no native hookshot is in the slot, synthesize ITEM_HOOKSHOT so the
        // hookshot cell renders, is cursor-selectable, and equips. Held ITEM_HOOKSHOT + clawshotOwned
        // fires the clawshot variant (Nei_ArmsHookVariant). Skijer's NEI
        // Power Keg shares the Bomb cell. Same reasoning as the Clawshot
        // below: ownership lives in its own flag, not in this slot, so owning ONLY the keg left the
        // cell empty — and an empty cell is skipped by the kaleido cursor, which made the keg
        // unreachable. Every wheel entry has to be obtainable on its own. (Bow, slingshot, lantern
        // and boomerang are deliberately excluded from this rule: their variants are modes of a
        // weapon you must own first.) Skijer's NEI
        if (slot == SLOT_BOMB && item == ITEM_NONE) {
            if (Nei_Save()->powerKegOwned) {
                return ITEM_POWDER_KEG;
            }
        }
        if (slot == SLOT_HOOKSHOT && item == ITEM_NONE) {
            if (Nei_Save()->clawshotOwned) {
                return ITEM_HOOKSHOT;
            }
        }
        // Unified trade wheel (cell 4,5). Same synthesis as the two above: ownership lives in the
        // tradeAdultOwned bitmask, not in this inventory slot, so a trade item earned in OoT — or one
        // MM put in SLOT_TRADE_KEY_MAMA / SLOT_TRADE_COUPLE instead — left this slot ITEM_NONE. The
        // kaleido's cursor SKIPS empty cells (z_kaleido_item.c:1320), so the cell was unreachable and
        // everything in the wheel stayed hidden until a native deed happened to land here. Seeding it
        // from the wheel handler was too late: the cursor test runs first. Skijer 2026-07-30
        if (slot == SLOT_TRADE_DEED && item == ITEM_NONE) {
            extern s32 TradeAdult_OwnedCount(void);
            extern u8 TradeAdult_CellItem(void);
            if (TradeAdult_OwnedCount() > 0) {
                return TradeAdult_CellItem();
            }
            // Nothing in the bitmask YET, but MM may have just put a trade item in one of its other
            // two native homes — SLOT_TRADE_KEY_MAMA (Room Key / Letter to Mama) or SLOT_TRADE_COUPLE
            // (Letter to Kafei / Pendant). Neither has a cell in the OoT page-0 layout, so without
            // this the item is invisible AND unreachable: the wheel's fold that would register it runs
            // after the cursor test. Surface it read-only; the fold sets the bit the same frame.
            // Skijer 2026-07-30
            {
                uint16_t other = gSaveContext.save.saveInfo.inventory.items[SLOT_TRADE_KEY_MAMA];
                if (other == ITEM_NONE) {
                    other = gSaveContext.save.saveInfo.inventory.items[SLOT_TRADE_COUPLE];
                }
                if (other != ITEM_NONE) {
                    return other;
                }
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
// Same widening as the getter. The vanilla-backed ranges still store u8, so an EXT id (>0xFF) can
// only be put in a page-2 slot; writing one to a vanilla slot truncates, which is why the casts
// below are explicit rather than implicit. Skijer's NEI
static inline void ExtInv_SetSlotItem(int slot, uint16_t itemId) {
    extern SaveContext gSaveContext;
    if (slot >= 0 && slot < 24) {
        gSaveContext.save.saveInfo.inventory.items[slot] = (uint8_t)itemId; // 2ship: nested inventory
    } else if (slot >= 48 && slot < 72) {
        gSaveContext.save.saveInfo.inventory.items[slot - 24] = (uint8_t)itemId; // real MM mask slots
    } else if (slot >= 72 && slot < 80) {
        ExtInv_SetOotSlotItem(slot, (uint8_t)itemId); // OoT page-0 virtual slots
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

// ── SW97 primed element (Skijer's NEI) ───────────────────────────────────────
// The bow/slingshot element is a FLAG, not the item on the button. Everything downstream reads
// Sw97_EffectiveElement() and nothing else — that is where the CVar gate and the "bombs are
// bow-only" rule live. `isSling` is 0 for the bow, 1 for the slingshot; the two carry independent
// elements on purpose (spirit arrows and wind bullets may be primed at the same time).
uint8_t Sw97_ElementOwned(uint8_t elem);
uint8_t Sw97_ElementCount(uint8_t isSling);
uint8_t Sw97_ElementAt(uint8_t isSling, uint8_t index);
uint8_t Sw97_GetElement(uint8_t isSling);
void Sw97_SetElement(uint8_t isSling, uint8_t elem);
uint8_t Sw97_ElementNeighbor(uint8_t isSling, uint8_t elem, int32_t dir);
uint16_t Sw97_ElementIcon(uint8_t elem);
uint8_t Sw97_EffectiveElement(uint8_t isSling);
uint8_t Sw97_IsBowItem(uint16_t item);
uint8_t Sw97_IsSlingItem(uint16_t item);
uint8_t Sw97_BombArrowsOwned(void);
// Nayru's Love <-> Roc's Feather cell ownership. Both the cell getter and the kaleido wheel must
// ask these, never re-derive them — the two had already drifted apart once.
uint8_t NayrusWheel_HasRocs(void);
uint8_t NayrusWheel_HasNayrus(void);
// Give-path grant for that cell. Give paths MUST use this, never ExtInv_SetSlotItem/SetOotSlotItem:
// those are the wheel's selection writes and confer no ownership by design.
void NayrusWheel_Grant(uint8_t itemId);

// True when `item` is a weapon primed with bombs: bomb arrows (bow) or bomb bullets (slingshot).
uint8_t Sw97_ItemHasBombs(uint16_t item);
uint8_t Sw97_BombArrowsOnButton(void);
uint8_t BombArrows_RandoMode(void);
void Sw97_RefreshButtonIcons(struct PlayState* play);
void Sw97_MigrateLayout(struct PlayState* play); // one-shot, gated by NeiSaveData.sw97LayoutVersion

// ── Elemental Wand (Skijer's NEI) ────────────────────────────────────────────
uint8_t Wand_RandoMode(void);
uint8_t Wand_ModeOwned(uint8_t mode);
void Wand_GrantMode(uint8_t mode);
uint8_t Wand_ModeCount(void);
uint8_t Wand_ModeAt(uint8_t index);
uint8_t Wand_GetMode(void);
void Wand_SetMode(uint8_t mode);
uint8_t Wand_ModeNeighbor(uint8_t mode, int32_t dir);
uint16_t Wand_ModeMedallion(uint8_t mode);
void* Wand_ModeIcon(uint8_t mode);
void* Wand_ModeNameTex(uint8_t mode);

// ── Sheikah Slate runes (Skijer's NEI) — wand idiom over SLOT_SHEIKAH_SLATE ──
uint8_t Slate_RuneOwned(uint8_t rune);
void Slate_GrantRune(uint8_t rune); // also hands over the slot on the first rune
uint8_t Slate_RuneCount(void);      // owned runes
uint8_t Slate_RuneAt(uint8_t index);
uint8_t Slate_GetRune(void);        // active rune (self-healing to an owned one)
void Slate_SetRune(uint8_t rune);
uint8_t Slate_RuneNeighbor(uint8_t rune, int32_t dir);
void* Slate_RuneMiniIcon(uint8_t rune); // 24x24 rune glyph (wheel previews / textbox)
void* Slate_RuneIcon(uint8_t rune);     // 32x32 slate-with-rune-badge (cell / HUD)
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
// Slot 27 used to be Bomb Arrows. Bomb Arrows are the 7th value of the bow's element flag now
// (SW97_ELEM_BOMB) and own no cell; the Elemental Wand took the freed cell. SLOT_BOMB_ARROWS is
// KEPT as a reserved marker because call sites still reference the name — it must never be used to
// store an item again, and gPage2Items[3] must never be shifted (each index maps to a
// NeiSaveData::ownedItems byte, so shifting corrupts every existing save).
#define SLOT_BOMB_ARROWS 27 // RESERVED — do not store into
#define SLOT_ELEMENTAL_WAND 27
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
#define SLOT_DESIRE_SENSOR 39 // RETIRED cell (Desire Sensor -> Quartz of Motion, collect page); 39 is the Sheikah Slate now
#define SLOT_LIGHT_ROD 40
#define SLOT_HYLIAS_GRACE 41 // RETIRED item (2026-08-06); 41 is the Phantom Hourglass now
#define SLOT_LANTERN 42
#define SLOT_MINISH_CAP 43
#define SLOT_POKEBALL 44 // moved to the Broken Items equipment page (Pikachu form); 44 is the Shadow Crystal now
#define SLOT_CANE_OF_SOMARIA 45
#define SLOT_SHOVEL 46 // shared cell: Shovel <-> Dominion Rod wheel (ownership = shovelOwned/dominionOwned flags)
#define SLOT_DOMINION_ROD 47 // RETIRED cell (rod moved onto the shovel wheel); 47 is the Rod of Seasons now
// 2026-08-06 re-layout — the four cells the moves above freed. These are the USER's canonical page-2
// layout positions; do not re-shuffle them. Old defines above keep their numbers so existing code
// compiles, but the CELL belongs to the new item. Skijer's NEI
#define SLOT_SHEIKAH_SLATE 39
#define SLOT_PHANTOM_HOURGLASS 41
#define SLOT_SHADOW_CRYSTAL 44
#define SLOT_ROD_OF_SEASONS 47

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
// itemId is u16 so a page-2 slot can be given an EXT id (>0xFF); the store behind it is u16 too.
// Skijer's NEI
static inline void ExtInv_GiveItem(uint8_t slot, uint16_t itemId) {
    if (slot >= 24 && slot < 48) {
        Nei_SetOwnedItem(slot, itemId);
    }
}
static inline void ExtInv_SetItemById(uint16_t itemId) { // Skijer's NEI
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
