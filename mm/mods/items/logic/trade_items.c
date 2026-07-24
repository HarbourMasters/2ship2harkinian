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

#define TRADE_ADULT_COUNT 20
#define TRADE_ADULT_PENDANT 19 // last index = Pendant of Memories (== ITEM_EXT_BOOTS_2)

// NEI trade index -> inventory item id. Order MUST match the tradeAdultOwned bit layout (nei_save.h).
static const u8 sTradeAdultItems[TRADE_ADULT_COUNT] = {
    ITEM_POCKET_EGG,    ITEM_POCKET_CUCCO,    ITEM_COJIRO,          ITEM_ODD_MUSHROOM, // 0-3  (OoT)
    ITEM_ODD_POTION,    ITEM_SAW,             ITEM_SWORD_BROKEN,    ITEM_PRESCRIPTION, // 4-7  (OoT)
    ITEM_FROG,          ITEM_EYEDROPS,        ITEM_CLAIM_CHECK,                        // 8-10 (OoT)
    ITEM_MM_MOONS_TEAR,                                                                // 11
    ITEM_MM_DEED_LAND,  ITEM_MM_DEED_SWAMP,   ITEM_MM_DEED_MOUNTAIN, ITEM_MM_DEED_OCEAN, // 12-15
    ITEM_MM_ROOM_KEY,   ITEM_MM_LETTER_KAFEI, ITEM_MM_SPECIAL_DELIVERY,                // 16-18
    ITEM_EXT_BOOTS_2,                                                                  // 19 Pendant of Memories
};

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
    if (index == TRADE_ADULT_PENDANT) {
        // Owned if either flag is set (granted via the trade path or the combat equipment page).
        return ((Nei_Save()->tradeAdultOwned & (1u << index)) != 0) || ExtEquip_HasItem(EQUIP_TYPE_BOOTS, 2);
    }
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
    if (index == TRADE_ADULT_PENDANT) {
        ExtEquip_GiveItem(EQUIP_TYPE_BOOTS, 2); // combat flag: C-equippable moveset (equip_pendant.c)
    }
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
    if (index == TRADE_ADULT_PENDANT) {
        // Bit 26 = ExtEquip owned (16 + EQUIP_TYPE_BOOTS*3 + (index-1)); see extended_equipment.c.
        Nei_Save()->extEquipOwnedBits &= ~(1u << 26);
    }
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
