/**
 * trade_items.c - MM adult trade-quest items for the SLOT_TRADE_ADULT 2D-grid wheel (Skijer's NEI).
 *
 * The adult trade slot becomes a grid selector over every adult trade item the player owns: OoT's
 * (Pocket Egg..Claim Check) plus the 9 Majora's Mask ones. Ownership is a bitmask in the nei save
 * (Nei_Save()->tradeAdultOwned), indexed by the NEI trade index below (matches nei_save.h).
 *
 * The Pendant of Memories is the combat Ext Boots 2 item (equip_pendant.c). Granting it sets BOTH the
 * trade bit (shows in the wheel + syncs to MM for the Anju exchange) AND the Ext Boots 2 ownership bit
 * (so it's equippable to a C-button as the Mortal Draw / Ground Pound / Parry Leap moveset).
 *
 * Included by custom_items.c (unity build -> z_player.c). Helpers are non-static so the kaleido grid
 * and the grant menu can call them via extern prototypes.
 */

#include "mods/nei_save.h"
#include "mods/extended_equipment.h" // ExtEquip_GiveItem/HasItem, ITEM_EXT_BOOTS_2

#define TRADE_ADULT_COUNT 23
#define TRADE_ADULT_PENDANT 19 // Pendant of Memories (== ITEM_EXT_BOOTS_2)

// NEI trade index -> inventory item id. Order MUST match the tradeAdultOwned bit layout (nei_save.h).
// APPEND ONLY — the index is the save bit, so reordering invalidates existing saves.
// In THIS (MM) build every OoT entry is ITEM_NONE (nei_oot_compat.h): MM has no equivalent item and no
// free u8 id. They are still fully visible/cyclable — the wheel runs off the index cursor and resolves
// art through sTradeIconPaths below, never through the id. Skijer 2026-07-29
static const u8 sTradeAdultItems[TRADE_ADULT_COUNT] = {
    ITEM_POCKET_EGG,
    ITEM_POCKET_CUCCO,
    ITEM_COJIRO,
    ITEM_ODD_MUSHROOM, // 0-3  (OoT adult)
    ITEM_ODD_POTION,
    ITEM_SAW,
    ITEM_SWORD_BROKEN,
    ITEM_PRESCRIPTION, // 4-7  (OoT adult)
    ITEM_FROG,
    ITEM_EYEDROPS,
    ITEM_CLAIM_CHECK,   // 8-10 (OoT adult)
    ITEM_MM_MOONS_TEAR, // 11
    ITEM_MM_DEED_LAND,
    ITEM_MM_DEED_SWAMP,
    ITEM_MM_DEED_MOUNTAIN,
    ITEM_MM_DEED_OCEAN, // 12-15
    ITEM_MM_ROOM_KEY,
    ITEM_MM_LETTER_KAFEI,
    ITEM_MM_SPECIAL_DELIVERY, // 16-18
    ITEM_EXT_BOOTS_2,         // 19 Pendant of Memories
    ITEM_WEIRD_EGG,
    ITEM_CHICKEN,
    ITEM_LETTER_ZELDA, // 20-22 (OoT child)
};

// Per-index icon art. The OoT entries come from the companion oot.o2r through the OoT asset loader
// (OotAssets_LoadTexOrDList); the MM entries are NULL and fall back to MM's own gItemIcons via the
// item id, which those DO have. Indexed exactly like sTradeAdultItems. Skijer 2026-07-29
static const char* sTradeIconPaths[TRADE_ADULT_COUNT] = {
    "__OTR__textures/icon_item_static/gItemIconPocketEggTex",         // 0
    "__OTR__textures/icon_item_static/gItemIconPocketCuccoTex",       // 1
    "__OTR__textures/icon_item_static/gItemIconCojiroTex",            // 2
    "__OTR__textures/icon_item_static/gItemIconOddMushroomTex",       // 3
    "__OTR__textures/icon_item_static/gItemIconOddPotionTex",         // 4
    "__OTR__textures/icon_item_static/gItemIconPoachersSawTex",       // 5
    "__OTR__textures/icon_item_static/gItemIconBrokenGoronsSwordTex", // 6
    "__OTR__textures/icon_item_static/gItemIconPrescriptionTex",      // 7
    "__OTR__textures/icon_item_static/gItemIconEyeballFrogTex",       // 8
    "__OTR__textures/icon_item_static/gItemIconEyeDropsTex",          // 9
    "__OTR__textures/icon_item_static/gItemIconClaimCheckTex",        // 10
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, // 11-15 MM native
    NULL,
    NULL,
    NULL,                                                        // 16-18 MM native
    NULL,                                                        // 19 Pendant (ext-equip art)
    "__OTR__textures/icon_item_static/gItemIconWeirdEggTex",     // 20
    "__OTR__textures/icon_item_static/gItemIconChickenTex",      // 21
    "__OTR__textures/icon_item_static/gItemIconZeldasLetterTex", // 22
};

// Per-index name texture, same deal as the icons (item_name_static from the companion oot.o2r).
// NOTE OoT's own inconsistency: the icon is gItemIconEyeballFrogTex (lowercase b) but the name is
// gEyeBallFrogItemNameENGTex (capital B). Verified against soh/assets, not guessed. Skijer 2026-07-29
static const char* sTradeNamePaths[TRADE_ADULT_COUNT] = {
    "__OTR__textures/item_name_static/gPocketEggItemNameENGTex",         // 0
    "__OTR__textures/item_name_static/gPocketCuccoItemNameENGTex",       // 1
    "__OTR__textures/item_name_static/gCojiroItemNameENGTex",            // 2
    "__OTR__textures/item_name_static/gOddMushroomItemNameENGTex",       // 3
    "__OTR__textures/item_name_static/gOddPotionItemNameENGTex",         // 4
    "__OTR__textures/item_name_static/gPoachersSawItemNameENGTex",       // 5
    "__OTR__textures/item_name_static/gBrokenGoronsSwordItemNameENGTex", // 6
    "__OTR__textures/item_name_static/gPrescriptionItemNameENGTex",      // 7
    "__OTR__textures/item_name_static/gEyeBallFrogItemNameENGTex",       // 8
    "__OTR__textures/item_name_static/gEyeDropsItemNameENGTex",          // 9
    "__OTR__textures/item_name_static/gClaimCheckItemNameENGTex",        // 10
    NULL,
    NULL,
    NULL,
    NULL,
    NULL, // 11-15 MM native
    NULL,
    NULL,
    NULL,                                                           // 16-18 MM native
    NULL,                                                           // 19 Pendant
    "__OTR__textures/item_name_static/gWeirdEggItemNameENGTex",     // 20
    "__OTR__textures/item_name_static/gCuccoItemNameENGTex",        // 21
    "__OTR__textures/item_name_static/gZeldasLetterItemNameENGTex", // 22
};

// OoT icon path for a trade index, or NULL when the entry uses MM's own art (or oot.o2r is absent).
const char* TradeAdult_IconPath(s32 index) {
    extern unsigned char OotAssets_Available(void);
    if (index < 0 || index >= TRADE_ADULT_COUNT) {
        return NULL;
    }
    if (sTradeIconPaths[index] == NULL || !OotAssets_Available()) {
        return NULL;
    }
    return sTradeIconPaths[index];
}

// OoT name-texture path for a trade index, or NULL when MM's own name applies.
const char* TradeAdult_NamePath(s32 index) {
    extern unsigned char OotAssets_Available(void);
    if (index < 0 || index >= TRADE_ADULT_COUNT) {
        return NULL;
    }
    if (sTradeNamePaths[index] == NULL || !OotAssets_Available()) {
        return NULL;
    }
    return sTradeNamePaths[index];
}

s32 TradeAdult_Count(void) {
    return TRADE_ADULT_COUNT;
}

u8 TradeAdult_ItemId(s32 index) {
    if (index < 0 || index >= TRADE_ADULT_COUNT) {
        return ITEM_NONE;
    }
    return sTradeAdultItems[index];
}

s32 TradeAdult_IndexOfItem(u8 item) {
    // ITEM_NONE never identifies a trade item. In the MM build the 11 OoT trade ids have no MM
    // equivalent and alias to ITEM_NONE (nei_oot_compat.h), so without this guard an empty slot
    // (ITEM_NONE) matched entry 0 and every "is this a trade item?" test answered "yes, Pocket Egg" —
    // which is why the wheel behaved as if it held an item it could not draw. Skijer's NEI
    if (item == ITEM_NONE) {
        return -1;
    }
    for (s32 i = 0; i < TRADE_ADULT_COUNT; i++) {
        if (sTradeAdultItems[i] == item) {
            return i;
        }
    }
    return -1;
}

u8 TradeAdult_IsOwnedIndex(s32 index) {
    if (index < 0 || index >= TRADE_ADULT_COUNT) {
        return 0;
    }
    // Skijer 2026-07-29: the Pendant used to ALSO count as owned via the ext BOOTS-2 grid bit. That
    // slot is the CLIMB BOOTS now, so owning boots would have read as owning the Pendant. One source
    // of truth: this bitmask (mirror of soh).
    return (Nei_Save()->tradeAdultOwned & (1u << index)) != 0;
}

u8 TradeAdult_IsOwnedItem(u8 item) {
    s32 idx = TradeAdult_IndexOfItem(item);
    return (idx >= 0) ? TradeAdult_IsOwnedIndex(idx) : 0;
}

void TradeAdult_GiveIndex(s32 index) {
    if (index < 0 || index >= TRADE_ADULT_COUNT) {
        return;
    }
    Nei_Save()->tradeAdultOwned |= (1u << index); // trade flag: wheel + MM sync (Anju exchange)
    // Nothing else for the Pendant: its moveset runs off ExtEquip_PendantActive() (ownership +
    // toggle), and the old BOOTS-2 "combat flag" bit belongs to the Climb Boots now.
}

void TradeAdult_GiveItem(u8 item) {
    s32 idx = TradeAdult_IndexOfItem(item);
    if (idx >= 0) {
        TradeAdult_GiveIndex(idx);
    }
}

// Set or clear an item's "obtained" flag (the save-editor toggle). Clearing the Pendant also drops its
// Ext Boots 2 combat ownership so the two stay in lockstep.
void TradeAdult_SetOwnedIndex(s32 index, u8 on) {
    if (index < 0 || index >= TRADE_ADULT_COUNT) {
        return;
    }
    if (on) {
        TradeAdult_GiveIndex(index);
        return;
    }
    Nei_Save()->tradeAdultOwned &= ~(1u << index);
}

// Owned-item count (drives the 2D-grid layout: rows/cols sized to how many the player holds).
s32 TradeAdult_OwnedCount(void) {
    s32 n = 0;
    for (s32 i = 0; i < TRADE_ADULT_COUNT; i++) {
        if (TradeAdult_IsOwnedIndex(i)) {
            n++;
        }
    }
    return n;
}

// The ordinal-th owned trade item -> its global trade index (or -1). The 2D-grid wheel lays out only
// the items the player holds, so it walks owned ordinals.
s32 TradeAdult_OwnedAt(s32 ordinal) {
    s32 n = 0;
    for (s32 i = 0; i < TRADE_ADULT_COUNT; i++) {
        if (TradeAdult_IsOwnedIndex(i)) {
            if (n == ordinal) {
                return i;
            }
            n++;
        }
    }
    return -1;
}

// An item's position within the owned list (or -1 if not owned). Used to start the grid cursor on the
// item currently shown in the slot.
s32 TradeAdult_OrdinalOf(u8 item) {
    s32 target = TradeAdult_IndexOfItem(item);
    if (target < 0 || !TradeAdult_IsOwnedIndex(target)) {
        return -1;
    }
    s32 n = 0;
    for (s32 i = 0; i < target; i++) {
        if (TradeAdult_IsOwnedIndex(i)) {
            n++;
        }
    }
    return n;
}

// Prev/next owned trade item relative to the one in the slot — feeds the shared adult-trade cycle wheel
// (KaleidoScope_HandleItemCycleExtras), so the wheel cycles every owned trade item (vanilla + MM).
u8 TradeAdult_NextItem(u8 cur) {
    s32 owned = TradeAdult_OwnedCount();
    if (owned <= 0) {
        return ITEM_NONE;
    }
    s32 ord = TradeAdult_OrdinalOf(cur);
    s32 nextOrd = (ord < 0) ? 0 : (ord + 1) % owned;
    s32 gi = TradeAdult_OwnedAt(nextOrd);
    return (gi >= 0) ? TradeAdult_ItemId(gi) : ITEM_NONE;
}

u8 TradeAdult_PrevItem(u8 cur) {
    s32 owned = TradeAdult_OwnedCount();
    if (owned <= 0) {
        return ITEM_NONE;
    }
    s32 ord = TradeAdult_OrdinalOf(cur);
    s32 prevOrd = (ord < 0) ? 0 : (ord + owned - 1) % owned;
    s32 gi = TradeAdult_OwnedAt(prevOrd);
    return (gi >= 0) ? TradeAdult_ItemId(gi) : ITEM_NONE;
}

// Fold a trade item the player is holding (a vanilla one obtained the normal way, or the rando-current)
// into the owned set so it joins the wheel alongside the granted MM items.
void TradeAdult_FoldCurrent(u8 item) {
    s32 idx = TradeAdult_IndexOfItem(item);
    if (idx >= 0 && !TradeAdult_IsOwnedIndex(idx)) {
        Nei_Save()->tradeAdultOwned |= (1u << idx);
    }
}

// ── Index cursor (MM) ───────────────────────────────────────────────────────────────────────────────
// MM cannot address these items by inventory id (the 11+3 OoT entries are ITEM_NONE here), so the
// unified wheel remembers a trade INDEX in the save and the cell only carries ITEM_TRADE_PLACEHOLDER.
// Skijer 2026-07-29

// Current index, self-healing: if it is out of range or points at something no longer owned, snap to
// the first owned entry (or -1 when the player holds nothing).
s32 TradeAdult_CursorIndex(void) {
    s32 cur = (s32)Nei_Save()->tradeAdultCursor;
    if (cur < 0 || cur >= TRADE_ADULT_COUNT || !TradeAdult_IsOwnedIndex(cur)) {
        cur = TradeAdult_OwnedAt(0); // -1 when nothing is owned
        Nei_Save()->tradeAdultCursor = (u8)((cur < 0) ? 0 : cur);
    }
    return cur;
}

// Step the cursor by whole owned entries (dir = +1 / -1), wrapping. No-op with 0 or 1 item.
void TradeAdult_CursorStep(s32 dir) {
    s32 owned = TradeAdult_OwnedCount();
    if (owned <= 1) {
        return;
    }
    s32 cur = TradeAdult_CursorIndex();
    s32 ord = (cur < 0) ? 0 : TradeAdult_OrdinalOf(TradeAdult_ItemId(cur));
    // OrdinalOf() goes through the item id, which is ITEM_NONE for the OoT entries — recompute by
    // index instead so those are not all treated as ordinal 0.
    ord = 0;
    for (s32 i = 0; i < cur; i++) {
        if (TradeAdult_IsOwnedIndex(i)) {
            ord++;
        }
    }
    s32 nextOrd = ((ord + dir) % owned + owned) % owned;
    s32 gi = TradeAdult_OwnedAt(nextOrd);
    if (gi >= 0) {
        Nei_Save()->tradeAdultCursor = (u8)gi;
    }
}

// What the inventory cell should hold for the current cursor: the real MM id when the entry has one
// (Moon's Tear, the Title Deeds, Room Key, ...), otherwise the placeholder that the icon override
// resolves back through TradeAdult_IconPath().
u8 TradeAdult_CellItem(void) {
    s32 cur = TradeAdult_CursorIndex();
    if (cur < 0) {
        return ITEM_NONE;
    }
    u8 id = TradeAdult_ItemId(cur);
    return (id != ITEM_NONE) ? id : (u8)ITEM_TRADE_PLACEHOLDER;
}

// Ordinal of a trade index within the owned set (by INDEX, not by item id — OrdinalOf() goes through
// the id, which is ITEM_NONE for every OoT entry and would collapse them all onto ordinal 0).
static s32 TradeAdult_OrdinalOfIndex(s32 index) {
    s32 n = 0;
    for (s32 i = 0; i < index; i++) {
        if (TradeAdult_IsOwnedIndex(i)) {
            n++;
        }
    }
    return n;
}

// Cell item of the neighbouring owned entry (dir = +1 / -1) without moving the cursor — feeds the
// wheel's left/right preview.
// Trade index of the neighbouring owned entry (dir = +1 / -1), or -1.
s32 TradeAdult_NeighborIndex(s32 dir) {
    s32 owned = TradeAdult_OwnedCount();
    if (owned <= 0) {
        return -1;
    }
    s32 cur = TradeAdult_CursorIndex();
    if (cur < 0) {
        return -1;
    }
    s32 ord = TradeAdult_OrdinalOfIndex(cur);
    return TradeAdult_OwnedAt(((ord + dir) % owned + owned) % owned);
}

u8 TradeAdult_NeighborCellItem(s32 dir) {
    s32 gi = TradeAdult_NeighborIndex(dir);
    if (gi < 0) {
        return ITEM_NONE;
    }
    u8 id = TradeAdult_ItemId(gi);
    if (id != ITEM_NONE) {
        return id; // has a real MM id — draw it directly
    }
    // No MM id: hand back a SIDE-SPECIFIC marker so the two previews resolve to different art and the
    // draw's `slotItem != leftItem` guard doesn't collapse them. Skijer 2026-07-30
    return (u8)((dir < 0) ? ITEM_TRADE_PREV : ITEM_TRADE_NEXT);
}
