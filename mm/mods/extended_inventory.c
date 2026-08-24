/**
 * extended_inventory.c - Extended inventory system implementation
 *
 * Manages custom items in multiple inventory pages.
 * Page 1: Vanilla OOT items (slots 0-23)
 * Page 2: Custom items (slots 24-47)
 * Page 3: MM Masks (slots 48-71) — requires mm.o2r and CVar
 */

#include "extended_inventory.h"
#include "extended_equipment.h"
#include "z64.h"
#include "functions.h" // Item_Give, Player_UnsetMask (ExtInv_KeepMmMaskOrSell)
#include <string.h>
#include "assets/2s2h_assets.h" // custom item icon OTR paths (Skijer's NEI)
#include "transformation_masks/transformation_masks.h"
#include "transformation_masks/assets/mm_asset_loader.h"
#include "items/logic/weapon_upgrades.h" // NEI weapon-upgrade icon overrides
extern void* gItemIcons[];
extern uint8_t gItemSlots[];
static ExtendedInventoryState sExtInvState = { .currentPage = 0, .pageSwitchTimer = 0 };

// Page 2 item layout (slots 24-47)
// Note: ITEM_ROCS_FEATHER_SKIJER at slot 24 is progressive - becomes ITEM_ROCS_CAPE when upgraded (shares slot)
// Slot 15 (actual slot 39) now has ITEM_DESIRE_SENSOR instead of ITEM_ROCS_CAPE
const uint8_t gPage2Items[24] = { ITEM_ROCS_FEATHER_SKIJER,
                                  ITEM_WHIP,
                                  ITEM_SPINNER,
                                  ITEM_ELEMENTAL_WAND, // slot 27 — was ITEM_BOMB_ARROWS (now a flag)
                                  ITEM_ROD_FIRE,
                                  ITEM_DEMISE_DESTRUCTION,
                                  ITEM_DEKU_LEAF,
                                  ITEM_TIME_GATE,
                                  ITEM_BEETLE,
                                  ITEM_SWITCH_HOOK,
                                  ITEM_ROD_ICE,
                                  ITEM_ZONAI_PERMAFROST,
                                  ITEM_MOGMA_MITTS,
                                  ITEM_GUST_JAR,
                                  ITEM_BALL_AND_CHAIN,
                                  ITEM_DESIRE_SENSOR,
                                  ITEM_ROD_LIGHT,
                                  ITEM_HYLIAS_GRACE,
                                  ITEM_LANTERN,
                                  ITEM_MINISH_CAP,
                                  ITEM_POKEBALL,
                                  ITEM_CANE_OF_SOMARIA,
                                  ITEM_SHOVEL,
                                  ITEM_DOMINION_ROD };

// Age requirements for page 2 items
// Roc's items (slot 0/24) = AGE_REQ_NONE (both adult and child can use Feather AND Cape)
// Desire Sensor (slot 15/39) = AGE_REQ_NONE (both adult and child can use)
// Index 3 (slot 27) was AGE_REQ_ADULT for Bomb Arrows; the Elemental Wand that replaced it is
// age-free (MM has no age gate anyway — the medallions are what gate it).
const uint8_t gPage2ItemAgeReqs[24] = { AGE_REQ_NONE, AGE_REQ_NONE,  AGE_REQ_NONE, AGE_REQ_NONE,  AGE_REQ_NONE,
                                        AGE_REQ_NONE, AGE_REQ_CHILD, AGE_REQ_NONE, AGE_REQ_ADULT, AGE_REQ_CHILD,
                                        AGE_REQ_NONE, AGE_REQ_NONE,  AGE_REQ_NONE, AGE_REQ_CHILD, AGE_REQ_ADULT,
                                        AGE_REQ_NONE, AGE_REQ_NONE,  AGE_REQ_NONE, AGE_REQ_NONE,  AGE_REQ_CHILD,
                                        AGE_REQ_NONE, AGE_REQ_NONE,  AGE_REQ_NONE, AGE_REQ_NONE };

// Page 3: MM Masks layout (slots 48-71)
// Row 0: Postman, AllNight, Blast, Stone, GreatFairy, Deku
// Row 1: Keaton, Bremen, Bunny, DonGero, Scents, Goron
// Row 2: Romani, CircusLeader, Kafei, Couple, Truth, Zora
// Row 3: Kamaro, Gibdo, Garo, Captain, Giant, FierceDeity
const uint8_t gPage3MaskItems[24] = {
    ITEM_MM_MASK_POSTMAN,     ITEM_MM_MASK_ALL_NIGHT,     ITEM_MM_MASK_BLAST,  ITEM_MM_MASK_STONE,
    ITEM_MM_MASK_GREAT_FAIRY, ITEM_MM_MASK_DEKU,          ITEM_MM_MASK_KEATON, ITEM_MM_MASK_BREMEN,
    ITEM_MM_MASK_BUNNY,       ITEM_MM_MASK_DON_GERO,      ITEM_MM_MASK_SCENTS, ITEM_MM_MASK_GORON,
    ITEM_MM_MASK_ROMANI,      ITEM_MM_MASK_CIRCUS_LEADER, ITEM_MM_MASK_KAFEI,  ITEM_MM_MASK_COUPLE,
    ITEM_MM_MASK_TRUTH,       ITEM_MM_MASK_ZORA,          ITEM_MM_MASK_KAMARO, ITEM_MM_MASK_GIBDO,
    ITEM_MM_MASK_GARO,        ITEM_MM_MASK_CAPTAIN,       ITEM_MM_MASK_GIANT,  ITEM_MM_MASK_FIERCE_DEITY,
};

// MM masks age requirements: regular masks = AGE_REQ_NONE, transformation masks = AGE_REQ_CHILD
// Transformation masks (Deku, Goron, Zora, Fierce Deity) are child-only unless TimelessEquipment cheat
const uint8_t gPage3MaskAgeReqs[24] = {
    AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_CHILD, // [5]=Deku
    AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_CHILD, // [11]=Goron
    AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_CHILD, // [17]=Zora
    AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_NONE, AGE_REQ_CHILD, // [23]=Fierce Deity
};
ExtendedInventoryState* ExtInv_GetState(void) {
    return &sExtInvState;
}
void ExtInv_Reset(void) {
    sExtInvState.currentPage = 0;
    sExtInvState.pageSwitchTimer = 0;
}
// Clamp page if custom items or MM masks CVar was toggled off
void ExtInv_ClampPage(void) {
    if (sExtInvState.currentPage == 1 && !ExtInv_IsCustomItemsEnabled()) {
        sExtInvState.currentPage = 0;
    }
    if (sExtInvState.currentPage == 2 && !ExtInv_IsMmMasksEnabled()) {
        sExtInvState.currentPage = 0;
    }
}
void ExtInv_Update(void) {
    if (sExtInvState.pageSwitchTimer > 0) {
        sExtInvState.pageSwitchTimer--;
    }
}
bool ExtInv_CanSwitchPage(void) {
    return sExtInvState.pageSwitchTimer == 0;
}
void ExtInv_SwitchPage(void) {
    int available[3];
    int count = 0;
    available[count++] = 0; // Page 0 always available
    if (ExtInv_IsCustomItemsEnabled())
        available[count++] = 1;
    if (ExtInv_IsMmMasksEnabled())
        available[count++] = 2;

    if (count <= 1)
        return; // Only page 0, can't switch

    // Find current page in available list, advance to next
    int curIdx = 0;
    for (int i = 0; i < count; i++) {
        if (available[i] == sExtInvState.currentPage) {
            curIdx = i;
            break;
        }
    }
    sExtInvState.currentPage = available[(curIdx + 1) % count];
    sExtInvState.pageSwitchTimer = 15;
}
int ExtInv_GetCurrentPage(void) {
    return sExtInvState.currentPage;
}
int ExtInv_GetMaxPages(void) {
    int count = 1; // Page 0 always available
    if (ExtInv_IsCustomItemsEnabled())
        count++;
    if (ExtInv_IsMmMasksEnabled())
        count++;
    return count;
}
bool ExtInv_IsCustomItemsEnabled(void) {
    // Default ON — NEI features are enabled by default.
    return CVarGetInteger("gMods.CustomItems.Enabled", 1) != 0;
}
bool ExtInv_IsMmMasksEnabled(void) {
    // Default ON — NEI features are enabled by default.
    return CVarGetInteger("gMods.MmMasks.InventoryEnabled", 1) != 0;
}
bool ExtInv_IsOnlyTransformation(void) {
    return CVarGetInteger("gMods.MmMasks.OnlyTransformation", 0) != 0;
}
// OoT item-pause layout for page 0 (1:1 with the Shipwright build) — visual cell -> backing slot.
// MM-native cells use the real MM inventory slot; OoT-only cells use virtual slots (72+, backed
// by NeiSaveData). Wheels (keg/SW97/claw/rocs/trade) fold extra items onto these cells.
static const uint8_t sOotPage0Map[24] = {
    // ROW 1: Sticks | Nuts | Bombs(keg wheel) | Arrows(SW97 wheel) | Fire Arrows | Din's Fire
    SLOT_DEKU_STICK,
    SLOT_DEKU_NUT,
    SLOT_BOMB,
    SLOT_BOW,
    SLOT_ARROW_FIRE,
    VSLOT_DINS,
    // ROW 2: Slingshot(SW97) | Ocarina | Bombchu | Hookshot(claw wheel) | Ice Arrows | Farore's
    VSLOT_SLINGSHOT,
    SLOT_OCARINA,
    SLOT_BOMBCHU,
    SLOT_HOOKSHOT,
    SLOT_ARROW_ICE,
    VSLOT_FARORES,
    // ROW 3: Boomerang | Lens | Beans | Hammer | Light Arrows | Nayru's(rocs wheel)
    VSLOT_OOT_BOOMERANG,
    SLOT_LENS_OF_TRUTH,
    SLOT_MAGIC_BEANS,
    VSLOT_HAMMER,
    SLOT_ARROW_LIGHT,
    VSLOT_NAYRUS,
    // ROW 4: Bottles A | Bottles B | Net | Bottomless | Trade wheel | OoT Masks
    SLOT_BOTTLE_1,
    SLOT_BOTTLE_2,
    SLOT_BOTTLE_3,
    SLOT_BOTTLE_4,
    SLOT_TRADE_DEED,
    VSLOT_OOT_MASKS,
};

int ExtInv_GetInventorySlot(int visualSlot) {
    if (sExtInvState.currentPage == 0 && visualSlot >= 0 && visualSlot < 24) {
        return sOotPage0Map[visualSlot]; // OoT layout (Skijer's NEI)
    }
    return visualSlot + (sExtInvState.currentPage * 24);
}

// Ownership for the Nayru's Love <-> Roc's Feather cell. Exposed as functions because THREE places
// need the same answer — the cell getter, and the kaleido's canCycle/preview computation in both the
// handle and the draw pass. They used to each carry their own expression, and they had already
// drifted: the kaleido asked whether SLOT_ROCS (the page-2 PROGRESSIVE Roc's item) was owned, which
// is a different check entirely, so a rando-placed RG_ROCS_FEATHER produced a cell you could see but
// not cycle. Skijer's NEI
uint8_t NayrusWheel_HasRocs(void) {
    // THE ship-vanilla feather (ITEM_ROCS_FEATHER), and nothing else. There are two Roc's Feathers in
    // this fork and they are separate items that coexist freely:
    //
    //   ship-vanilla  ITEM_ROCS_FEATHER (0xA6)  — this wheel, paired with Nayru's Love, bit 0x8
    //   Skijer's      SLOT_ROCS (24)            — page 2, progressive into the Roc's Cape
    //
    // Owning one must never imply the other. Deliberately NOT reading SLOT_ROCS here: doing so hands
    // you the vanilla check for free the moment you pick up the progressive. SoH models it the same
    // way — RocsFeatherCycle.c gates purely on RAND_INF_OBTAINED_ROCS_FEATHER and never looks at the
    // Skijer item. Skijer's NEI
    return (Nei_Save()->ootSpellsOwned & 0x8) != 0;
}

uint8_t NayrusWheel_HasNayrus(void) {
    return (Nei_Save()->ootSpellsOwned & 0x4) != 0;
}

// THE give-path entry for this cell. Deliberately separate from ExtInv_SetOotSlotItem(VSLOT_NAYRUS,
// ...): that one is what the kaleido wheel calls on every cycle, so it must only move the SELECTION
// or spinning the wheel becomes an item generator. Granting also selects what you just received,
// which is what the player expects to see in the cell. Skijer's NEI
void NayrusWheel_Grant(uint8_t itemId) {
    NeiSaveData* nei = Nei_Save();
    if (itemId == ITEM_NAYRUS_LOVE) {
        nei->ootSpellsOwned |= 0x4;
        nei->nayruRocsMode = 0;
    } else if (itemId == ITEM_ROCS_FEATHER) {
        nei->ootSpellsOwned |= 0x8;
        nei->nayruRocsMode = 1;
    }
}

// --- OoT virtual slots (72+): backed by dedicated NeiSaveData fields ---
uint8_t ExtInv_GetOotSlotItem(int slot) {
    NeiSaveData* nei = Nei_Save();
    switch (slot) {
        case VSLOT_DINS:
            return (nei->ootSpellsOwned & 0x1) ? ITEM_DINS_FIRE : ITEM_NONE;
        case VSLOT_FARORES:
            return (nei->ootSpellsOwned & 0x2) ? ITEM_FARORES_WIND : ITEM_NONE;
        case VSLOT_NAYRUS:
            // Wheel: the SHIP-VANILLA Roc's Feather (ITEM_ROCS_FEATHER) <-> Nayru's Love.
            //
            // Not to be confused with Skijer's Roc's Feather, which is a different item living in
            // SLOT_ROCS on page 2 and upgrading into the Roc's Cape. The two coexist freely and
            // neither one's ownership says anything about the other — see NayrusWheel_HasRocs.
            // This cell used to read SLOT_ROCS, which made the vanilla feather a mere display mode
            // of an item you had to own separately: a rando-placed RG_ROCS_FEATHER showed nothing.
            {
                // Each entry of this wheel is independently obtainable, so the cell shows whichever
                // one you actually have — preferring the selected mode, falling back to the other.
                // Without the fallback, owning ONLY the feather while the mode still said "Nayru's"
                // left the cell empty and the item unreachable. (Bow, slingshot, lantern and
                // boomerang are deliberately NOT like this: their variants are modes of a weapon you
                // must own.) Skijer's NEI
                uint8_t hasRocs = NayrusWheel_HasRocs();
                uint8_t hasNayrus = NayrusWheel_HasNayrus();

                if (nei->nayruRocsMode && hasRocs) {
                    return ITEM_ROCS_FEATHER; // ship-vanilla feather (art + same jump behavior)
                }
                if (!nei->nayruRocsMode && hasNayrus) {
                    return ITEM_NAYRUS_LOVE;
                }
                if (hasRocs) {
                    return ITEM_ROCS_FEATHER;
                }
                return hasNayrus ? ITEM_NAYRUS_LOVE : ITEM_NONE;
            }
        case VSLOT_SLINGSHOT:
            // The cell always shows the plain slingshot now. The primed element is a FLAG
            // (sw97SlingElement) drawn as a medallion behind the icon, so this slot no longer
            // synthesizes a per-element item id. Skijer's NEI
            return nei->slingshotOwned ? ITEM_FAIRY_SLINGSHOT : ITEM_NONE;
        case VSLOT_OOT_BOOMERANG:
            return nei->ootBoomerangOwned ? ITEM_BOOMERANG : ITEM_NONE;
        case VSLOT_HAMMER:
            // Megaton Hammer shown when owned. Its Iron Knuckle's Axe upgrade is behavior-only
            // (no distinct OoT icon) — WeaponUpgrade_HasHammerAxe drives the in-game axe strike.
            return nei->ootHammerOwned ? ITEM_HAMMER : ITEM_NONE;
        case VSLOT_OOT_MASKS:
            // Was an unconditional ITEM_NONE ("per-item pass" — never written), which left cell 4,6
            // permanently empty. The kaleido cursor SKIPS empty cells, so the OoT child-trade mask
            // slot did not exist in MM at all. Synthesize it from the wheel, exactly like the trade
            // cell does. Skijer 2026-07-30
            return (OotMask_OwnedCount() > 0) ? OotMask_CellItem() : ITEM_NONE;
    }
    return ITEM_NONE;
}

// ─── OoT child-trade mask wheel (item cell 4,6 -> VSLOT_OOT_MASKS) ──────────────────────────────────
// The 8 OoT masks have no MM item id, so — same scheme as the unified trade wheel — ownership is a
// bitmask (nei->ootMasksOwned), the visible one is an index (nei->ootMaskCursor), the cell carries
// ITEM_OOT_MASK_PLACEHOLDER and the art is resolved from the index against the companion oot.o2r.
// NOTE OoT's own naming is inconsistent: Keaton/BunnyHood/Goron/Zora/MaskOfTruth read
// "gItemIcon<Name>Tex" but Skull/Spooky/Gerudo read "gItemIconMask<Name>Tex". Verified against
// soh/assets, not guessed. Skijer 2026-07-30
#define OOT_MASK_COUNT 8
static const char* sOotMaskIconPaths[OOT_MASK_COUNT] = {
    "__OTR__textures/icon_item_static/gItemIconKeatonMaskTex",  // 0 Keaton Mask
    "__OTR__textures/icon_item_static/gItemIconMaskSkullTex",   // 1 Skull Mask
    "__OTR__textures/icon_item_static/gItemIconMaskSpookyTex",  // 2 Spooky Mask
    "__OTR__textures/icon_item_static/gItemIconBunnyHoodTex",   // 3 Bunny Hood
    "__OTR__textures/icon_item_static/gItemIconGoronMaskTex",   // 4 Goron Mask
    "__OTR__textures/icon_item_static/gItemIconZoraMaskTex",    // 5 Zora Mask
    "__OTR__textures/icon_item_static/gItemIconMaskGerudoTex",  // 6 Gerudo Mask
    "__OTR__textures/icon_item_static/gItemIconMaskOfTruthTex", // 7 Mask of Truth
};

uint8_t OotMask_IsOwnedIndex(int index) {
    if (index < 0 || index >= OOT_MASK_COUNT) {
        return 0;
    }
    return (Nei_Save()->ootMasksOwned & (1u << index)) != 0;
}

void OotMask_SetOwnedIndex(int index, uint8_t on) {
    if (index < 0 || index >= OOT_MASK_COUNT) {
        return;
    }
    if (on) {
        Nei_Save()->ootMasksOwned |= (uint16_t)(1u << index);
    } else {
        Nei_Save()->ootMasksOwned &= (uint16_t) ~(1u << index);
    }
}

int OotMask_OwnedCount(void) {
    int n = 0;
    for (int i = 0; i < OOT_MASK_COUNT; i++) {
        if (OotMask_IsOwnedIndex(i)) {
            n++;
        }
    }
    return n;
}

int OotMask_OwnedAt(int ordinal) {
    int n = 0;
    for (int i = 0; i < OOT_MASK_COUNT; i++) {
        if (OotMask_IsOwnedIndex(i)) {
            if (n == ordinal) {
                return i;
            }
            n++;
        }
    }
    return -1;
}

// Self-healing like the trade cursor: an out-of-range or no-longer-owned index snaps to the first
// owned mask (-1 when none).
int OotMask_CursorIndex(void) {
    int cur = (int)Nei_Save()->ootMaskCursor;
    if (cur < 0 || cur >= OOT_MASK_COUNT || !OotMask_IsOwnedIndex(cur)) {
        cur = OotMask_OwnedAt(0);
        Nei_Save()->ootMaskCursor = (uint8_t)((cur < 0) ? 0 : cur);
    }
    return cur;
}

int OotMask_NeighborIndex(int dir) {
    int owned = OotMask_OwnedCount();
    int cur = OotMask_CursorIndex();
    if (owned <= 0 || cur < 0) {
        return -1;
    }
    int ord = 0;
    for (int i = 0; i < cur; i++) {
        if (OotMask_IsOwnedIndex(i)) {
            ord++;
        }
    }
    return OotMask_OwnedAt(((ord + dir) % owned + owned) % owned);
}

void OotMask_CursorStep(int dir) {
    int gi = OotMask_NeighborIndex(dir);
    if (gi >= 0) {
        Nei_Save()->ootMaskCursor = (uint8_t)gi;
    }
}

uint8_t OotMask_CellItem(void) {
    return (OotMask_CursorIndex() < 0) ? ITEM_NONE : (uint8_t)ITEM_OOT_MASK_PLACEHOLDER;
}

uint8_t OotMask_NeighborCellItem(int dir) {
    return (OotMask_NeighborIndex(dir) < 0) ? ITEM_NONE
                                            : (uint8_t)((dir < 0) ? ITEM_OOT_MASK_PREV : ITEM_OOT_MASK_NEXT);
}

const char* OotMask_IconPath(int index) {
    extern unsigned char OotAssets_Available(void);
    if (index < 0 || index >= OOT_MASK_COUNT || !OotAssets_Available()) {
        return NULL;
    }
    return sOotMaskIconPaths[index];
}

void ExtInv_SetOotSlotItem(int slot, uint8_t itemId) {
    NeiSaveData* nei = Nei_Save();
    switch (slot) {
        case VSLOT_DINS:
            nei->ootSpellsOwned =
                (itemId == ITEM_DINS_FIRE) ? (nei->ootSpellsOwned | 0x1) : (nei->ootSpellsOwned & ~0x1);
            break;
        case VSLOT_FARORES:
            nei->ootSpellsOwned =
                (itemId == ITEM_FARORES_WIND) ? (nei->ootSpellsOwned | 0x2) : (nei->ootSpellsOwned & ~0x2);
            break;
        case VSLOT_NAYRUS:
            // Wheel writes flip the SELECTION only — never ownership. This used to OR in the
            // ownership bit for whatever you cycled onto, which meant spinning the wheel handed you
            // Nayru's Love (or the feather) for free: the wheel was an item generator. Ownership now
            // comes exclusively from the give paths. Skijer's NEI
            if (itemId == ITEM_NAYRUS_LOVE) {
                nei->nayruRocsMode = 0;
            } else if (itemId != ITEM_NONE) {
                nei->nayruRocsMode = 1; // ITEM_ROCS_FEATHER selected
            }
            break;
        case VSLOT_SLINGSHOT:
            // Pure ownership now — the element lives in sw97SlingElement, not in this slot's value.
            nei->slingshotOwned = (itemId == ITEM_FAIRY_SLINGSHOT);
            break;
        case VSLOT_OOT_BOOMERANG:
            nei->ootBoomerangOwned = (itemId == ITEM_BOOMERANG);
            break;
        default:
            break;
    }
}

// Page-0 ammo digits are item-based (the layout is remapped, so slot-indexed tables don't apply)
uint8_t ExtInv_ItemHasAmmo(uint8_t itemId) {
    switch (itemId) {
        case ITEM_DEKU_STICK:
        case ITEM_DEKU_NUT:
        case ITEM_BOMB:
        case ITEM_BOMBCHU:
        case ITEM_BOW:
        case ITEM_POWDER_KEG:
        case ITEM_MAGIC_BEANS:
        // The elemental-bullet ids used to need their own arms here to share the seed pouch; the
        // slot holds a plain ITEM_FAIRY_SLINGSHOT now, so the one arm above covers every element.
        case ITEM_FAIRY_SLINGSHOT:
        case ITEM_PICTOGRAPH_BOX: // photo counter (MM native ammo-count on the picto)
            return true;
        default:
            return false;
    }
}
bool ExtInv_IsSlotOnCurrentPage(uint8_t slot) {
    int pageStart = sExtInvState.currentPage * 24;
    int pageEnd = pageStart + 23;
    return (slot >= pageStart && slot <= pageEnd);
}
int ExtInv_GetPageForSlot(uint8_t slot) {
    if (slot >= 48)
        return 2;
    if (slot >= 24)
        return 1;
    return 0;
}
uint8_t ExtInv_GetItemAgeReq(uint16_t itemId) {
    // MM Mask items: use per-mask age requirements from gPage3MaskAgeReqs
    // (kept on the page-3 table; NEI registry rows are no-op AGE_REQ_NONE).
    if (itemId >= ITEM_MM_MASK_POSTMAN && itemId <= ITEM_MM_MASK_FIERCE_DEITY) {
        for (int i = 0; i < 24; i++) {
            if (gPage3MaskItems[i] == itemId) {
                return gPage3MaskAgeReqs[i];
            }
        }
        return AGE_REQ_NONE;
    }
    // Page-2 custom items: unified NEI registry. Skijer's NEI
    const NeiItem* it = Nei_FindByItem(itemId);
    if (it != NULL) {
        return it->ageReq;
    }
    return 9;
}

// 2ship: MM has no gSlotAgeReqs / age system — vanilla slots are always allowed.

uint8_t ExtInv_GetSlotAgeReq(uint8_t slot) {
    // Transformation mask override: the per-form allowlist IS the age requirement.
    // Allowed slots return 9 (AGE_REQ_NONE = always passes), restricted slots return
    // opposite age (always fails → greyed out). This lets child Link use adult items
    // if the form permits it (e.g., Zora can use bow regardless of Link's age).
    if (TransformMasks_IsEnabled() && TransformMasks_IsTransformedAny() && slot < 72) {
        if (ExtInv_IsSlotTransformRestricted(slot)) {
            extern SaveContext gSaveContext;
            return 1 - gSaveContext.save.linkAge; // 2ship: linkAge lives under Save
        }
        return 9; // Allowed by form → bypass vanilla age check
    }

    // NEI: the Twilight clawshot upgrade makes the hookshot/longshot usable by child AND adult
    // (the clawshot is a child-friendly grapple). Owned-gated so child can still select it in the
    // kaleido to toggle clawshot mode.
    if (slot == SLOT_HOOKSHOT) {
        extern unsigned char TwilightUpgrade_HasClawshot(void);
        if (TwilightUpgrade_HasClawshot()) {
            return 9; // AGE_REQ_NONE
        }
    }

    // Vanilla slots (0-23): MM has no age gating → always allowed.
    if (slot < 24) {
        return 9; // AGE_REQ_NONE
    }
    // Custom slots (24-47): unified NEI registry. Skijer's NEI
    if (slot < 48) {
        const NeiItem* it = Nei_FindBySlot(slot);
        return it ? it->ageReq : 9;
    }
    // MM Mask slots (48-71) use gPage3MaskAgeReqs
    if (slot < 72) {
        return gPage3MaskAgeReqs[slot - 48];
    }
    return 9; // AGE_REQ_NONE for out-of-range
}

// 2ship: MM has no gEquipAgeReqs / age system — equipment is always allowed.
uint8_t ExtInv_GetEquipAgeReq(uint8_t row, uint8_t col) {
    // FD skin mode: allow swords (row 0) and shields (row 1), block tunics (row 2) and boots (row 3)
    if (TransformMasks_IsEnabled() && TransformMasks_IsFDSkinMode()) {
        extern SaveContext gSaveContext;
        if (row <= 1) {
            return 9; // AGE_REQ_NONE: swords/shields always available
        }
        // Tunics/boots: return opposite age to block them
        return 1 - gSaveContext.save.linkAge; // 2ship: linkAge lives under Save
    }

    // Other transformations (Goron/Zora/Deku): block all equipment changes
    if (TransformMasks_IsEnabled() && TransformMasks_IsTransformedAny()) {
        extern SaveContext gSaveContext;
        return 1 - gSaveContext.save.linkAge; // 2ship: linkAge lives under Save
    }

    // NEI: the Great Fairy's Sword upgrade makes the Biggoron Sword (row 0 = swords, col 3 = BGS)
    // usable by BOTH child and adult (normally adult-only).
    if (row == 0 && col == 3 && WeaponUpgrade_HasGreatFairy()) {
        return 9; // AGE_REQ_NONE
    }

    return 9; // AGE_REQ_NONE — MM has no equipment age gating
}

extern void* MmMasks_LoadNameTex(uint16_t itemId);
extern const char* MmMasks_GetNamePath(uint16_t itemId);

// Single source of truth for page-2 custom item icon + name-texture art.
// Both ExtInv_GetItemIcon and ExtInv_GetCustomItemNameTex index this table so
// the two associations can no longer drift apart.
//   icon == NULL  -> the icon getter falls through to its own special handling
//                    (used by ITEM_LANTERN, whose icon depends on fire type).
// Items needing dynamic/path-based art (Chateau Romani, MM masks, prop-hunt,
// SW97 medallions/arrows) are intentionally NOT in this table and stay handled
// by the surrounding special-case logic in each getter.
typedef struct {
    uint16_t itemId;
    void* icon;
    void* nameTex;
} CustomItemAsset;

static const CustomItemAsset sCustomItemAssets[] = {
    { ITEM_ROCS_FEATHER_SKIJER, (void*)gItemIconRocsFeatherTex, (void*)gRocsFeatherNameTex },            // 0x9D
    { ITEM_ROCS_CAPE, (void*)gItemIconRocsCapeTex, (void*)gRocsCapeNameTex },                            // 0x9E
    { ITEM_DESIRE_SENSOR, (void*)gItemIconDesireSensorTex, (void*)gDesireSensorNameTex },                // 0x9F
    { ITEM_HYLIAS_GRACE, (void*)gItemIconHyliaGraceTex, (void*)gHyliaGraceNameTex },                     // 0xA0
    { ITEM_ZONAI_PERMAFROST, (void*)gItemIconZonaiPermafrostTex, (void*)gZonaiPermafrostNameTex },       // 0xA1
    { ITEM_DEMISE_DESTRUCTION, (void*)gItemIconDemiseDestructionTex, (void*)gDemiseDestructionNameTex }, // 0xA2
    { ITEM_DEKU_LEAF, (void*)gItemIconDekuLeafTex, (void*)gDekuLeafNameTex },                            // 0xA3
    { ITEM_SWITCH_HOOK, (void*)gItemIconSwitchHookTex, (void*)gSwitchHookNameTex },                      // 0xA4
    { ITEM_MOGMA_MITTS, (void*)gItemIconMogmaMittsTex, (void*)gMogmaMittsNameTex },                      // 0xA5
    { ITEM_GUST_JAR, (void*)gItemIconGustJarTex, (void*)gGustJarNameTex },                               // 0xA6
    { ITEM_BALL_AND_CHAIN, (void*)gItemIconBallAndChainTex, (void*)gBallAndChainNameTex },               // 0xA7
    { ITEM_WHIP, (void*)gItemIconWhipTex, (void*)gWhipNameTex },                                         // 0xA8
    { ITEM_SPINNER, (void*)gItemIconSpinnerTex, (void*)gSpinnerNameTex },                                // 0xA9
    { ITEM_CANE_OF_SOMARIA, (void*)gItemIconCaneOfSomariaTex, (void*)gCaneOfSomariaNameTex },            // 0xAA
    { ITEM_DOMINION_ROD, (void*)gItemIconDominionRodTex, (void*)gDominionRodNameTex },                   // 0xAB
    { ITEM_TIME_GATE, (void*)gItemIconTimeGateTex, (void*)gTimeGateNameTex },                            // 0xAC
    // Bomb Arrows keeps its icon/name row even though it owns no inventory cell any more: the
    // wheel's corner badge and the get-item textbox still look them up by item id.
    { ITEM_BOMB_ARROWS, (void*)gItemIconBombArrowsTex, (void*)gBombArrowsNameTex }, // 0xAD
    // Elemental Wand's icon/name are per-MODE, resolved in ExtInv_GetItemIcon /
    // ExtInv_GetCustomItemNameTex; this row is only the fallback.
    { ITEM_ELEMENTAL_WAND, (void*)gItemIconSandRodTex, (void*)gSandRodNameTex }, // 0xD0
    { ITEM_ROD_FIRE, (void*)gItemIconFireRodTex, (void*)gFireRodNameTex },       // 0xAE
    { ITEM_ROD_ICE, (void*)gItemIconIceRodTex, (void*)gIceRodNameTex },          // 0xAF
    { ITEM_ROD_LIGHT, (void*)gItemIconLightRodTex, (void*)gLightRodNameTex },    // 0xB0
    { ITEM_BEETLE, (void*)gItemIconBeetleTex, (void*)gBeetleNameTex },           // 0xB1
    { ITEM_SHOVEL, (void*)gItemIconShovelTex, (void*)gShovelNameTex },           // 0xB2
    { ITEM_MINISH_CAP, (void*)gItemIconMinishCapTex, (void*)gMinishCapNameTex }, // 0xB3
    // Lantern: name texture is constant, but the icon is chosen dynamically by
    // fire type -> icon left NULL so the icon getter handles it below.
    { ITEM_LANTERN, NULL, (void*)gLanternNameTex }, // 0xB4
    { ITEM_POKEBALL, (void*)gItemIconPokeballTex, (void*)gPokeballNameTex },
    // Bottle Randomizer extra items (Skijer's NEI). Net + Bottomless Bottle; SLOT_BOTTLE_3/4.
    { ITEM_NET, (void*)gItemIconNetTex, (void*)gNetNameTex },                                         // 0xF4
    { ITEM_BOTTOMLESS_BOTTLE, (void*)gItemIconBottomlessBottleTex, (void*)gBottomlessBottleNameTex }, // 0xF5
};

static const CustomItemAsset* ExtInv_FindCustomItemAsset(uint16_t itemId) {
    for (size_t i = 0; i < sizeof(sCustomItemAssets) / sizeof(sCustomItemAssets[0]); i++) {
        if (sCustomItemAssets[i].itemId == itemId) {
            return &sCustomItemAssets[i];
        }
    }
    return NULL;
}

void* ExtInv_GetCustomItemNameTex(uint16_t itemId, uint8_t language) {
    // 2026-08-06 page-2 additions — EXT (u16) ids; their IA4 name textures come from the
    // generate_names.py pipeline. Path strings, resolved by the RSP like every custom name.
    switch (itemId) {
        case EXT_ITEM_SHEIKAH_SLATE:
            return (void*)"__OTR__textures/item_name_custom/gSheikahSlateNameTex";
        case EXT_ITEM_PHANTOM_HOURGLASS:
            return (void*)"__OTR__textures/item_name_custom/gPhantomHourglassNameTex";
        case EXT_ITEM_SHADOW_CRYSTAL:
            return (void*)"__OTR__textures/item_name_custom/gShadowCrystalNameTex";
        case EXT_ITEM_ROD_OF_SEASONS:
            return (void*)"__OTR__textures/item_name_custom/gRodOfSeasonsNameTex";
        default:
            break;
    }

    // Elemental Wand: one item id, six names — the name follows the active rod.
    if (itemId == ITEM_ELEMENTAL_WAND) {
        return Wand_ModeNameTex(Wand_GetMode());
    }

    // Unified trade wheel: 23 entries behind one placeholder id, so the name comes from the index
    // cursor (same reasoning as the icon override). Skijer 2026-07-29
    if (itemId == ITEM_TRADE_PLACEHOLDER) {
        extern s32 TradeAdult_CursorIndex(void);
        extern const char* TradeAdult_NamePath(s32 index);
        const char* tradeName = TradeAdult_NamePath(TradeAdult_CursorIndex());
        if (tradeName != NULL && ResourceMgr_FileExists(tradeName)) {
            return (void*)tradeName;
        }
        return NULL;
    }

    // OoT child-trade mask wheel: 8 masks behind one placeholder id, name from the index cursor.
    if (itemId == ITEM_OOT_MASK_PLACEHOLDER) {
        static const char* kOotMaskNames[] = {
            "__OTR__textures/item_name_static/gKeatonMaskItemNameENGTex",
            "__OTR__textures/item_name_static/gSkullMaskItemNameENGTex",
            "__OTR__textures/item_name_static/gSpookyMaskItemNameENGTex",
            "__OTR__textures/item_name_static/gBunnyHoodItemNameENGTex",
            "__OTR__textures/item_name_static/gGoronMaskItemNameENGTex",
            "__OTR__textures/item_name_static/gZoraMaskItemNameENGTex",
            "__OTR__textures/item_name_static/gGerudoMaskItemNameENGTex",
            // NOTE lowercase "of" — OoT's icon is gItemIconMaskOfTruthTex but the NAME texture is
            // gMaskofTruthItemNameENGTex. Verified against soh/assets.
            "__OTR__textures/item_name_static/gMaskofTruthItemNameENGTex",
        };
        int mi = OotMask_CursorIndex();
        if (mi >= 0 && mi < (int)(sizeof(kOotMaskNames) / sizeof(kOotMaskNames[0])) &&
            ResourceMgr_FileExists(kOotMaskNames[mi])) {
            return (void*)kOotMaskNames[mi];
        }
        return NULL;
    }

    // Dual Cane: same reasoning as the icon override — one item id, four names.
    if (itemId == ITEM_CANE_OF_SOMARIA && Nei_CaneOwned()) {
        switch (Nei_CaneGetType()) {
            case 1:
                return (void*)"__OTR__textures/item_name_custom/gTrirodNameTex";
            case 2:
                return (void*)"__OTR__textures/item_name_custom/gCaneOfPacciNameTex";
            case 3:
                return (void*)"__OTR__textures/item_name_custom/gUltrahandNameTex";
            default:
                break;
        }
    }
    (void)language;
    // Skijer's NEI hookshot overhaul — name overrides BEFORE the generic OoT-path routing:
    //   - Ultrashot: level 3 of the hookshot chain keeps the Longshot ICON but the name reads
    //     "Ultrashot" (custom tex; the Light-medallion corner marker is drawn at the icon sites).
    //   - Clawshot: MM's vanilla hookshot IS behaviorally the clawshot, so the MM-native
    //     hookshot cell ALWAYS reads "Clawshot" (display only — RI_HOOKSHOT keeps its
    //     "Hookshot" rando name: it represents tier 1 of the OoT progressive chain
    //     (FCI_HOOKSHOT: Hookshot/Longshot/Ultrashot), while FCI_CLAWSHOT = RI_CLAWSHOT
    //     carries the clawshot identity; renaming here avoids two items named "Clawshot"
    //     in rando text while the kaleido matches the item's real behavior).
    if ((itemId == ITEM_LONGSHOT_OOT) && (Nei_Save()->ootHookshotLevel >= 3)) {
        return (void*)gUltrashotNameTex;
    }
    if (itemId == ITEM_HOOKSHOT) {
        return (void*)gClawshotNameTex;
    }
    // OoT-ported page-0 items + Megaton Hammer + Boomerang: use OoT's OWN name textures from the
    // companion oot.o2r (item_name_static), FileExists-gated. Mirrors ExtInv_GetItemIcon's OoT-icon
    // handling above — these ids have no MM/custom name-texture asset, only OoT's.
    {
        extern u8 ResourceMgr_FileExists(const char* resName);
        const char* ootPath = NULL;
        switch (itemId) {
            case ITEM_DINS_FIRE:
                ootPath = "__OTR__textures/item_name_static/gDinsFireItemNameENGTex";
                break;
            case ITEM_FARORES_WIND:
                ootPath = "__OTR__textures/item_name_static/gFaroresWindItemNameENGTex";
                break;
            case ITEM_NAYRUS_LOVE:
                ootPath = "__OTR__textures/item_name_static/gNayrusLoveItemNameENGTex";
                break;
            case ITEM_FAIRY_SLINGSHOT:
                ootPath = "__OTR__textures/item_name_static/gFairySlingshotItemNameENGTex";
                break;
            case ITEM_HOOKSHOT_OOT:
                ootPath = "__OTR__textures/item_name_static/gHookshotItemNameENGTex";
                break;
            case ITEM_LONGSHOT_OOT:
                ootPath = "__OTR__textures/item_name_static/gLongshotItemNameENGTex";
                break;
            case ITEM_BOOMERANG:
                ootPath = "__OTR__textures/item_name_static/gBoomerangItemNameENGTex";
                break;
            case ITEM_HAMMER:
                ootPath = "__OTR__textures/item_name_static/gMegatonHammerItemNameENGTex";
                break;
            default:
                break;
        }
        if (ootPath != NULL && ResourceMgr_FileExists(ootPath)) {
            return (void*)ootPath;
        }
    }

    // MM masks + MM-native bottle contents (gold dust / seahorse / spring water / etc.) are
    // NATIVE in 2ship and handled by the MM item system / mask kaleido — NOT here. Only the
    // genuinely-custom page-2 items have their own name textures.
    const CustomItemAsset* asset = ExtInv_FindCustomItemAsset(itemId);
    if (asset) {
        return asset->nameTex;
    }
    return NULL;
}
extern void* MmMasks_LoadIcon(uint16_t itemId);
extern const char* MmMasks_GetIconPath(uint16_t itemId);
extern void* MmAssets_LoadFDSwordIcon(void);
extern const char* MmAssets_GetChateauIconPath(void);

// SM64 Mario caps — direct icon lookup, decoupled from the OOT spells. The caps
// are their own custom behavior (D-pad → Sm64Mario_HandleCapDpad), not an
// extension of Din's/Nayru's/Farore's. cap: 0 = Vanish, 1 = Metal, 2 = Wing,
// 3 = Fire Flower. Used by the corner power-up HUD draw in z_parameter.c.
void* ExtInv_GetCapIcon(uint8_t cap) {
    switch (cap) {
        case 0:
            return (void*)gItemIconVanishCapTex;
        case 1:
            return (void*)gItemIconMetalCapTex;
        case 2:
            return (void*)gItemIconWingCapTex;
        case 3:
            return (void*)gItemIconFireFlowerTex;
        default:
            return NULL;
    }
}

// mods/nei_save.cpp — Dual Cane context variables (see the icon override below).
uint8_t Nei_CaneOwned(void);
uint8_t Nei_CaneActiveSkill(void);

void* ExtInv_GetItemIcon(uint16_t itemId) {

    // ── Unified trade wheel: 23 entries share one placeholder id ─────────────
    // Same situation as the Dual Cane below — the icon cannot come from the id, because the 14 OoT
    // trade entries have no MM item id at all (ITEM_NONE). It comes from the save's index cursor,
    // whose OoT art lives in the companion oot.o2r. Skijer 2026-07-29
    if (itemId == ITEM_TRADE_PLACEHOLDER || itemId == ITEM_TRADE_PREV || itemId == ITEM_TRADE_NEXT) {
        extern s32 TradeAdult_CursorIndex(void);
        extern s32 TradeAdult_NeighborIndex(s32 dir);
        extern const char* TradeAdult_IconPath(s32 index);
        s32 idx = (itemId == ITEM_TRADE_PLACEHOLDER) ? TradeAdult_CursorIndex()
                                                     : TradeAdult_NeighborIndex(itemId == ITEM_TRADE_PREV ? -1 : 1);
        const char* tradePath = TradeAdult_IconPath(idx);
        if (tradePath != NULL && ResourceMgr_FileExists(tradePath)) {
            return (void*)tradePath;
        }
        return NULL; // no oot.o2r → empty cell rather than a wrong icon
    }

    // OoT child-trade mask wheel — same three-marker scheme (cell / prev / next).
    if (itemId == ITEM_OOT_MASK_PLACEHOLDER || itemId == ITEM_OOT_MASK_PREV || itemId == ITEM_OOT_MASK_NEXT) {
        int mi = (itemId == ITEM_OOT_MASK_PLACEHOLDER) ? OotMask_CursorIndex()
                                                       : OotMask_NeighborIndex(itemId == ITEM_OOT_MASK_PREV ? -1 : 1);
        const char* maskPath = OotMask_IconPath(mi);
        if (maskPath != NULL && ResourceMgr_FileExists(maskPath)) {
            return (void*)maskPath;
        }
        return NULL;
    }

    // ── Dual Cane: the cell's icon is simply which of the four is in hand ────
    // Four entries share one item id, so the icon cannot come from the id — it comes
    // from the context variable. Trirod and Ultrahand are their own entries here,
    // NOT the third level of the cane that unlocked them.
    //
    // OTR paths rather than the generated gItemIcon* symbols, because those only
    // exist after an asset re-extract; the FD sword override below does the same.
    if (itemId == ITEM_CANE_OF_SOMARIA && Nei_CaneOwned()) {
        switch (Nei_CaneGetType()) {
            case 1: // Trirod
                return (void*)"__OTR__textures/icon_item_custom/gItemIconTrirodTex";
            case 2: // Cane of Pacci
                return (void*)"__OTR__textures/icon_item_custom/gItemIconCaneOfPacciTex";
            case 3: // Ultrahand
                return (void*)"__OTR__textures/icon_item_custom/gItemIconUltrahandTex";
            default:
                break; // Cane of Somaria keeps the cell's own icon
        }
    }
    // OoT page-0 items: vanilla OoT icons from the companion oot.o2r (FileExists-gated; a
    // missing companion just leaves the cell blank until per-item MM art lands).
    {
        extern u8 ResourceMgr_FileExists(const char* resName);
        const char* ootPath = NULL;
        switch (itemId) {
            case ITEM_DINS_FIRE:
                ootPath = "__OTR__textures/icon_item_static/gItemIconDinsFireTex";
                break;
            case ITEM_FARORES_WIND:
                ootPath = "__OTR__textures/icon_item_static/gItemIconFaroresWindTex";
                break;
            case ITEM_NAYRUS_LOVE:
                ootPath = "__OTR__textures/icon_item_static/gItemIconNayrusLoveTex";
                break;
            case ITEM_FAIRY_SLINGSHOT:
                ootPath = "__OTR__textures/icon_item_static/gItemIconSlingshotTex";
                break;
            case ITEM_HOOKSHOT_OOT:
                ootPath = "__OTR__textures/icon_item_static/gItemIconHookshotTex";
                break;
            case ITEM_LONGSHOT_OOT:
                ootPath = "__OTR__textures/icon_item_static/gItemIconLongshotTex";
                break;
            case ITEM_BOOMERANG:
                ootPath = "__OTR__textures/icon_item_static/gItemIconBoomerangTex";
                break;
            case ITEM_HAMMER:
                ootPath = "__OTR__textures/icon_item_static/gItemIconHammerTex";
                break;
            case ITEM_ROCS_FEATHER:
                ootPath = "__OTR__textures/icon_item_static/gRocsFeatherTex";
                break; // ship-vanilla art (baked)
            default:
                break;
        }
        if (ootPath != NULL && ResourceMgr_FileExists(ootPath)) {
            return (void*)ootPath;
        }
        // Skijer's NEI slingshot pass: MM natively kept OoT's slingshot icon art in mm.o2r
        // (gItemIcons[ITEM_SLINGSHOT] in z_inventory.c) — use it when the companion is absent,
        // instead of falling through to the gItemIcons[0] (ocarina) default.
        if (itemId == ITEM_FAIRY_SLINGSHOT) {
            extern TexturePtr gItemIcons[];
            return (void*)gItemIcons[ITEM_SLINGSHOT];
        }
    }

    // SW97 medallions: the OoT medallion quest icons, 24x24 (SoH 1:1 — draw sites use
    // ExtInv_GetItemIconSize; a 24x24 RGBA32 drawn as 32x32 reads as garbage). The elemental
    // arrow/bullet ids that used to alias onto these are gone; the composite draw sites now ask for
    // the medallion directly via Sw97_ElementIcon(). Skijer's NEI
    switch (itemId) {
        case ITEM_MEDALLION_FOREST:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionForestTex";
        case ITEM_MEDALLION_FIRE:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionFireTex";
        case ITEM_MEDALLION_WATER:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionWaterTex";
        case ITEM_MEDALLION_SPIRIT:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionSpiritTex";
        case ITEM_MEDALLION_SHADOW:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionShadowTex";
        case ITEM_MEDALLION_LIGHT:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionLightTex";
        // Elemental Wand: one item id, six icons — the cell shows whichever rod is active.
        case ITEM_ELEMENTAL_WAND:
            return Wand_ModeIcon(Wand_GetMode());
        // 2026-08-06 page-2 additions — EXT (u16) inventory ids, own icons (icon_item_custom PNGs).
        // Resolved before any generic fallback, which would index vanilla art with an id > 0xFF.
        case EXT_ITEM_SHEIKAH_SLATE:
            // Once any rune is lit the cell/HUD icon carries the ACTIVE rune's badge (wand idiom).
            if (Nei_Save()->slateRunesOwned != 0) {
                return Slate_RuneIcon(Slate_GetRune());
            }
            return (void*)"__OTR__textures/icon_item_custom/gItemIconSheikahSlateTex";
        case EXT_ITEM_PHANTOM_HOURGLASS:
            return (void*)"__OTR__textures/icon_item_custom/gItemIconPhantomHourglassTex";
        case EXT_ITEM_SHADOW_CRYSTAL:
            return (void*)"__OTR__textures/icon_item_custom/gItemIconShadowCrystalTex";
        case EXT_ITEM_ROD_OF_SEASONS:
            return (void*)"__OTR__textures/icon_item_custom/gItemIconRodOfSeasonsTex";
        // Spiritual Stones (companion quest icons, 24x24 — same idiom as the medallions above).
        case EXT_ITEM_SPIRITUAL_STONE_KOKIRI:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconKokiriEmeraldTex";
        case EXT_ITEM_SPIRITUAL_STONE_GORON:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconGoronRubyTex";
        case EXT_ITEM_SPIRITUAL_STONE_ZORA:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconZoraSapphireTex";
        default:
            break;
    }

    // Extended equipment: override A button icon when ext sword/shield is active
    // Suppressed during kaleido equipment screen so vanilla icons show there
    if (ExtEquip_IsEnabled() && !gExtEquipSuppressIconOverride) {
        u8 extSword = ExtEquip_GetCurrent(EQUIP_TYPE_SWORD);
        if (extSword > 0 && (itemId == ITEM_SWORD_KOKIRI || itemId == ITEM_SWORD_MASTER || itemId == ITEM_SWORD_BGS ||
                             itemId == ITEM_SWORD_KNIFE)) {
            void* icon = ExtEquip_GetIcon(EQUIP_TYPE_SWORD, extSword);
            if (icon)
                return icon;
        }
    }

    // FD skin mode: show FD sword icon for any equipped sword
    if (TransformMasks_IsFDSkinMode() && (itemId == ITEM_SWORD_KOKIRI || itemId == ITEM_SWORD_MASTER ||
                                          itemId == ITEM_SWORD_BGS || itemId == ITEM_SWORD_KNIFE)) {
        void* fdIcon = MmAssets_LoadFDSwordIcon();
        if (fdIcon)
            return fdIcon;
    }

    // NEI weapon upgrades — show the MM upgrade icon when the upgrade is owned and the matching
    // base weapon is equipped. The Kokiri top level (Gilded) and the BGS upgrade (GFS) each have
    // a display toggle in the Custom Items menu. Falls through to the vanilla icon (gItemIcons)
    // if mm.o2r lacks the asset. Real Master Sword keeps the vanilla Master icon (a glow is
    // applied at render time, not here).
    if (itemId == ITEM_SWORD_KOKIRI && WeaponUpgrade_KokiriLevel() >= 1) {
        u8 showGilded = WeaponUpgrade_HasGilded() && CVarGetInteger("gEnhancements.SkijerNEI.GildedUsesGildedLook", 1);
        void* up = MmAssets_LoadResource(showGilded ? "__OTR__icon_item_static_yar/gItemIconGildedSwordTex"
                                                    : "__OTR__icon_item_static_yar/gItemIconRazorSwordTex");
        if (up)
            return up;
    }
    if (itemId == ITEM_SWORD_BGS && WeaponUpgrade_HasGreatFairy() &&
        CVarGetInteger("gEnhancements.SkijerNEI.BgsUsesGfsLook", 1)) {
        void* up = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconGreatFairysSwordTex");
        if (up)
            return up;
    }
    // Hammer → Iron Knuckle's Axe: show the axe icon while the upgrade is owned.
    if (itemId == ITEM_HAMMER && WeaponUpgrade_HasHammerAxe()) {
        return (void*)gItemIconDrillshaftTex;
    }

    // SM64 caps are NO LONGER tied to the OOT spells — they're custom behavior
    // triggered straight from the D-pad (Sm64Mario_HandleCapDpad). The cap icons
    // are looked up directly via ExtInv_GetCapIcon (below), so there's no
    // spell→cap icon override here anymore.

    // SM64 Mario mask — the toggle item that locks to C-Down via
    // gSm64MarioMaskForce. Pressing C-Down with this equipped flips
    // gSm64Mario on/off (handled in mod_menu / z_player hook).
    if (itemId == ITEM_MARIO_MASK) {
        return (void*)gItemIconMarioMaskTex;
    }

    // Twilight Upgrade icon swap — when the corresponding mode is active
    // (persistent toggle via A in kaleido), swap hookshot/longshot/
    // boomerang icons to the upgraded variant.
    //
    // Clawshot specifically uses the MM (Majora's Mask) hookshot icon
    // straight from mm.o2r so the visual is 1:1 with TP's clawshot. The
    // local placeholder PNG (gItemIconClawshotTex) is only used if mm.o2r
    // isn't loaded — keeps the rest of the system working in environments
    // without the MM archive. Gale boomerang still uses its local
    // placeholder; no MM equivalent ported yet.
    {
        extern unsigned char TwilightUpgrade_IsClawshotActive(void);
        extern unsigned char TwilightUpgrade_IsGaleBoomerangActive(void);
        if ((itemId == ITEM_HOOKSHOT || itemId == ITEM_LONGSHOT) && TwilightUpgrade_IsClawshotActive()) {
            void* mmIcon = MmAssets_LoadHookshotIcon();
            if (mmIcon)
                return mmIcon;
            return (void*)gItemIconClawshotTex;
        }
        if (itemId == ITEM_BOOMERANG && TwilightUpgrade_IsGaleBoomerangActive()) {
            return (void*)gItemIconGaleBoomerangTex;
        }
    }

    // (The Pictograph-Box-over-Lens icon override lived here. Gone with the NEI pictobox: MM draws
    // the real pictograph icon from its own slot.)

    // Power Keg shares the Bomb slot. When power-keg mode is selected (kaleido A-toggle), show the
    // Power Keg icon in the slot instead of the Bomb. Skijer's NEI
    {
        extern unsigned char PowerKeg_IsOwned(void);
        extern unsigned char PowerKeg_IsOnBombActive(void);
        if (itemId == ITEM_BOMB && PowerKeg_IsOwned() && PowerKeg_IsOnBombActive()) {
            void* t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconPowderKegTex");
            if (t) {
                return t;
            }
        }
    }

    // MM bottle-content custom items (Bottle Randomizer) — load the content icon from mm.o2r.
    // These use the best-known mm.o2r texture names; verify/adjust the exact names on test. Falls
    // through to the registry/fallback if mm.o2r isn't loaded. (Chateau Romani 0xB6 keeps its own
    // gItemIconChateauRomaniTex; Magic Mushroom 0xDD keeps its own.) Skijer's NEI
    {
        void* t = NULL;
        if (itemId == ITEM_GOLD_DUST)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledGoldDustTex");
        else if (itemId == ITEM_HOT_SPRING_WATER)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconHotSpringWaterTex");
        else if (itemId == ITEM_DEKU_PRINCESS)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledDekuPrincessTex");
        else if (itemId == ITEM_SEAHORSE)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledSeahorseTex");
        else if (itemId == ITEM_SPRING_WATER)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconSpringWaterTex");
        else if (itemId == ITEM_ZORA_EGG)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledZoraEggTex");
        else if (itemId == ITEM_HYLIAN_LOACH)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledHylianLoachTex");
        else if (itemId == ITEM_OBABA_DRINK)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconEmptyBottle2Tex");
        if (t)
            return t;
    }

    // MM adult trade-quest items (Skijer's NEI) — shown in the SLOT_TRADE_ADULT 2D-grid wheel. Icons
    // from mm.o2r. The Pendant of Memories (ITEM_EXT_BOOTS_2) is the combat item, so it gets its icon
    // from the ext-equipment block below. Special Delivery to Mama reuses MM's "Letter to Mama" icon.
    {
        void* t = NULL;
        if (itemId == ITEM_MM_MOONS_TEAR)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconMoonsTearTex");
        else if (itemId == ITEM_MM_DEED_LAND)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconLandDeedTex");
        else if (itemId == ITEM_MM_DEED_SWAMP)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconSwampDeedTex");
        else if (itemId == ITEM_MM_DEED_MOUNTAIN)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconMountainDeedTex");
        else if (itemId == ITEM_MM_DEED_OCEAN)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconOceanDeedTex");
        else if (itemId == ITEM_MM_ROOM_KEY)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconRoomKeyTex");
        else if (itemId == ITEM_MM_LETTER_KAFEI)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconLetterToKafeiTex");
        else if (itemId == ITEM_MM_SPECIAL_DELIVERY)
            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconLetterToMamaTex");
        if (t)
            return t;
    }

    if (itemId < 156) {
        return (itemId < 0x9A) ? (void*)gItemIcons[itemId] : NULL; // NEI: never index OOB (garbage tex)
    }
    // ITEM_EXT_BOOTS_2 is the one shared id: as an INVENTORY / trade-wheel / C-button item it is the
    // Pendant of Memories, while grid slot (BOOTS, 2) is the Climb Boots (whose icon the kaleido reads
    // straight from ExtEquip_GetIcon, not from here). Skijer 2026-07-29
    if (itemId == ITEM_EXT_BOOTS_2) {
        void* pendantIcon = ExtEquip_GetPendantIcon();
        if (pendantIcon != NULL) {
            return pendantIcon;
        }
    }
    // Extended equipment items (0xE0-0xEB): return ext equip icon
    // Must check BEFORE MM masks since ranges overlap
    if (itemId >= ITEM_EXT_SWORD_1 && itemId <= ITEM_EXT_BOOTS_3) {
        u8 equipType = (itemId - ITEM_EXT_SWORD_1) / 3; // 0=sword,1=shield,2=tunic,3=boots
        u8 index = (itemId - ITEM_EXT_SWORD_1) % 3 + 1; // 1-3
        void* icon = ExtEquip_GetIcon(equipType, index);
        if (icon)
            return icon;
        return gItemIcons[0];
    }
    // MM Mask items: return OTR path string so the RSP resolves actual texture
    // dimensions from resource metadata (HD mod textures render at native resolution).
    if (itemId >= ITEM_MM_MASK_POSTMAN && itemId <= ITEM_MM_MASK_FIERCE_DEITY) {
        // Bunny Hood: use OOT icon (same appearance, enables OOT behavior)
        if (itemId == ITEM_MM_MASK_BUNNY) {
            return gItemIcons[ITEM_MASK_BUNNY];
        }
        const char* path = MmMasks_GetIconPath(itemId);
        if (path)
            return (void*)path;
        return gItemIcons[0]; // Fallback
    }
    // Page-2 custom items with a constant icon: unified NEI registry. Skijer's NEI
    // (ITEM_LANTERN has a NULL iconTex because its icon is dynamic; it falls
    // through to the dedicated case in the switch below.)
    {
        const NeiItem* it = Nei_FindByItem(itemId);
        if (it != NULL && it->iconTex != NULL) {
            return it->iconTex;
        }
    }
    switch (itemId) {
        // Prop Hunt button icons (0xD7-0xDC). Shown only while a hider is
        // in "prop mode" — the C-buttons + D-pad display these cycling/
        // category hints instead of vanilla item art.
        case ITEM_PH_ICON_POT:
            return (void*)gItemIconPropHuntPotTex;
        case ITEM_PH_ICON_ENEMY:
            return (void*)gItemIconPropHuntEnemyTex;
        case ITEM_PH_ICON_NPC:
            return (void*)gItemIconPropHuntNpcTex;
        case ITEM_PH_ICON_CHANGE:
            return (void*)gItemIconPropHuntChangeTex;
        case ITEM_PH_ICON_PREV:
            return (void*)gItemIconPropHuntPrevTex;
        case ITEM_PH_ICON_NEXT:
            return (void*)gItemIconPropHuntNextTex;

        case ITEM_LANTERN: { // 0xB4
            extern u8 Lantern_GetFireType(void);
            switch (Lantern_GetFireType()) {
                case 1:
                    return (void*)gItemIconLanternFireTex; // Regular (orange)
                case 2:
                    return (void*)gItemIconLanternBlueTex; // Blue
                case 3:
                    return (void*)gItemIconLanternPoeTex; // Poe (purple)
                case 4:
                    return (void*)gItemIconLanternGreenTex; // Green
                default:
                    return (void*)gItemIconLanternTex; // Unlit
            }
        }
        case ITEM_POKEBALL:
            return (void*)gItemIconPokeballTex;

        // SW97 Medallion items (spell mode — show medallion quest icons)
        case ITEM_MEDALLION_FOREST:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionForestTex";
        case ITEM_MEDALLION_FIRE:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionFireTex";
        case ITEM_MEDALLION_WATER:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionWaterTex";
        case ITEM_MEDALLION_SPIRIT:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionSpiritTex";
        case ITEM_MEDALLION_SHADOW:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionShadowTex";
        case ITEM_MEDALLION_LIGHT:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionLightTex";

        // (The twelve SW97 arrow/bullet ids used to alias onto the medallion icons here. They are
        // gone — the composite draws ask for the medallion directly via Sw97_ElementIcon().)

        // Elemental Wand: one item id, six icons — the cell shows whichever rod is active.
        case ITEM_ELEMENTAL_WAND:
            return Wand_ModeIcon(Wand_GetMode());

        // Spiritual Stones (companion quest icons, 24x24 — mirror the medallion cases above)
        case EXT_ITEM_SPIRITUAL_STONE_KOKIRI:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconKokiriEmeraldTex";
        case EXT_ITEM_SPIRITUAL_STONE_GORON:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconGoronRubyTex";
        case EXT_ITEM_SPIRITUAL_STONE_ZORA:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconZoraSapphireTex";

        case ITEM_CHATEAU_ROMANI: { // 0xB5
            const char* path = MmAssets_GetChateauIconPath();
            if (path)
                return (void*)path;
            return gItemIcons[ITEM_MILK_BOTTLE]; // Fallback to milk icon
        }
        default:
            return gItemIcons[0];
    }
}
// Returns 1 if the player owns the given MM mask item (extended inventory page 3, slots 48-71).
// Used by the trade-mask sale actors (En_Heishi2/Keaton, En_Mm/Bunny): masks with an MM
// counterpart are permanent items — selling them grants the reward without losing the mask.
int32_t ExtInv_HasMmMask(uint16_t itemId) {
    if (itemId < ITEM_MM_MASK_POSTMAN || itemId > ITEM_MM_MASK_FIERCE_DEITY) {
        return 0;
    }
    for (int i = 0; i < 24; i++) {
        if (gPage3MaskItems[i] == itemId) {
            return Nei_GetOwnedItem((uint8_t)(48 + i)) == itemId; // Skijer's NEI
        }
    }
    return 0;
}

// Trade-mask sale helper for En_Mm (Bunny Hood) / En_Heishi2 (Keaton Mask): if the
// player does NOT own the given MM counterpart mask, take the worn OOT trade mask and
// hand back ITEM_SOLD_OUT (vanilla behavior). When the MM counterpart IS owned the mask
// is permanent, so it is kept and no SOLD_OUT is given. Collapses the byte-identical
// idiom both actors previously inlined.
void ExtInv_KeepMmMaskOrSell(PlayState* play, uint16_t maskItem) {
    if (!ExtInv_HasMmMask(maskItem)) {
        Player_UnsetMask(play);
        Item_Give(play, ITEM_SOLD_OUT);
    }
}

// ── SW97 primed element + Elemental Wand (Skijer's NEI) ──────────────────────────────────────────
// Single source of truth for "which element is the bow/slingshot primed with" and "which rod is the
// wand showing". Hosted here (not in nei_save) because this file is plain C, already includes
// z64item.h + nei_save.h, already owns the element->medallion icon map and ExtInv_GetItemIconSize,
// and every consumer — the kaleido, z_parameter.c, z_player.c — already links against it. No new .c
// file means no .vcxproj edit.
//
// Mirror of the SoH implementation, with two deliberate divergences: medallion ownership reads the
// parallel OoT quest store (MM's native QUEST_* flags are stubbed), and the slingshot item is
// ITEM_FAIRY_SLINGSHOT rather than ITEM_SLINGSHOT.

#define SW97_ENABLED() CVarGetInteger("gEnhancements.SkijerNEI.SW97Medallions", 0)

// Element -> medallion item id, for the icon composited behind the weapon.
static const uint16_t sSw97ElemIcon[SW97_ELEM_COUNT] = {
    ITEM_NONE,             // SW97_ELEM_NONE
    ITEM_MEDALLION_FIRE,   // SW97_ELEM_FIRE
    ITEM_MEDALLION_WATER,  // SW97_ELEM_ICE
    ITEM_MEDALLION_LIGHT,  // SW97_ELEM_LIGHT
    ITEM_MEDALLION_SHADOW, // SW97_ELEM_DARK
    ITEM_MEDALLION_SPIRIT, // SW97_ELEM_SOUL
    ITEM_MEDALLION_FOREST, // SW97_ELEM_WIND
    ITEM_BOMB_ARROWS,      // SW97_ELEM_BOMB
};
// Element -> OoT quest bit that unlocks it (NeiSaveData.ootQuestItems, not MM's stubbed flags).
static const uint8_t sSw97ElemQuest[SW97_ELEM_COUNT] = {
    0,
    OOT_QUEST_MEDALLION_FIRE,
    OOT_QUEST_MEDALLION_WATER,
    OOT_QUEST_MEDALLION_LIGHT,
    OOT_QUEST_MEDALLION_SHADOW,
    OOT_QUEST_MEDALLION_SPIRIT,
    OOT_QUEST_MEDALLION_FOREST,
    0,
};
// Vanilla elemental-arrow SLOT that also unlocks the element (0xFF = no vanilla equivalent). Ported
// from SoH's sArrowWheelVanillaArrow so a player with fire arrows but no medallion still gets the
// fire entry — 2ship never had this clause; adding it is the parity fix.
static const uint8_t sSw97ElemVanillaSlot[SW97_ELEM_COUNT] = {
    0xFF, SLOT_ARROW_FIRE, SLOT_ARROW_ICE, SLOT_ARROW_LIGHT, 0xFF, 0xFF, 0xFF, 0xFF,
};

uint16_t Sw97_ElementIcon(uint8_t elem) {
    return (elem < SW97_ELEM_COUNT) ? sSw97ElemIcon[elem] : ITEM_NONE;
}

uint8_t BombArrows_RandoMode(void) {
    return (uint8_t)CVarGetInteger("gMods.BombArrows.Mode", BOMB_ARROWS_RANDO_OFF);
}

uint8_t Sw97_BombArrowsOwned(void) {
    extern u8 TwilightUpgrade_HasBombArrows(void);
    if (Nei_Save()->bombArrowsOwned || TwilightUpgrade_HasBombArrows()) {
        return 1;
    }
    return (BombArrows_RandoMode() == BOMB_ARROWS_RANDO_BOMB_BAG) && (CUR_UPG_VALUE(UPG_BOMB_BAG) > 0);
}

uint8_t Sw97_ElementOwned(uint8_t elem) {
    if (elem == SW97_ELEM_NONE) {
        return 1; // the plain weapon is always an option
    }
    if (elem == SW97_ELEM_BOMB) {
        return Sw97_BombArrowsOwned();
    }
    if (elem >= SW97_ELEM_COUNT) {
        return 0;
    }
    if (Nei_Save()->ootQuestItems & (1u << sSw97ElemQuest[elem])) {
        return 1;
    }
    return (sSw97ElemVanillaSlot[elem] != 0xFF) &&
           (gSaveContext.save.saveInfo.inventory.items[sSw97ElemVanillaSlot[elem]] != ITEM_NONE);
}

// Is `elem` a legal value for this weapon at all?
//
// Bombs ride BOTH weapons: bomb arrows on the bow, bomb bullets on the slingshot. They share one
// ownership flag (bombArrowsOwned) but each weapon keeps its own primed element, so a bomb-primed
// bow and a wind-primed slingshot coexist like any other pair. Skijer's NEI
static uint8_t Sw97_ElementAllowed(uint8_t isSling, uint8_t elem) {
    (void)isSling;
    return Sw97_ElementOwned(elem);
}

uint8_t Sw97_ElementCount(uint8_t isSling) {
    uint8_t n = 0;
    for (uint8_t e = 0; e < SW97_ELEM_COUNT; e++) {
        if (Sw97_ElementAllowed(isSling, e)) {
            n++;
        }
    }
    return n;
}

uint8_t Sw97_ElementAt(uint8_t isSling, uint8_t index) {
    uint8_t n = 0;
    for (uint8_t e = 0; e < SW97_ELEM_COUNT; e++) {
        if (Sw97_ElementAllowed(isSling, e)) {
            if (n == index) {
                return e;
            }
            n++;
        }
    }
    return SW97_ELEM_NONE;
}

uint8_t Sw97_GetElement(uint8_t isSling) {
    NeiSaveData* nei = Nei_Save();
    uint8_t e = isSling ? nei->sw97SlingElement : nei->sw97BowElement;
    if (!Sw97_ElementAllowed(isSling, e)) {
        // Self-heal: a medallion can be lost (or the option toggled) after the flag was set.
        e = SW97_ELEM_NONE;
        if (isSling) {
            nei->sw97SlingElement = e;
        } else {
            nei->sw97BowElement = e;
        }
    }
    return e;
}

void Sw97_SetElement(uint8_t isSling, uint8_t elem) {
    if (!Sw97_ElementAllowed(isSling, elem)) {
        return;
    }
    if (isSling) {
        Nei_Save()->sw97SlingElement = elem;
    } else {
        Nei_Save()->sw97BowElement = elem;
    }
}

uint8_t Sw97_ElementNeighbor(uint8_t isSling, uint8_t elem, int32_t dir) {
    uint8_t n = Sw97_ElementCount(isSling);
    if (n <= 1) {
        return elem;
    }
    for (uint8_t i = 0; i < n; i++) {
        if (Sw97_ElementAt(isSling, i) == elem) {
            return Sw97_ElementAt(isSling, (uint8_t)((i + n + (dir > 0 ? 1 : -1)) % n));
        }
    }
    return Sw97_ElementAt(isSling, 0);
}

// THE accessor. Everything downstream — the arrow-type decode, the item action, the HUD composite,
// the pause grid — goes through this and nothing else, so the "CVar off" path stays byte-identical
// to the pre-refactor behavior and the bow-only rule for bombs lives in exactly one place.
uint8_t Sw97_EffectiveElement(uint8_t isSling) {
    if (!SW97_ENABLED()) {
        return SW97_ELEM_NONE;
    }
    return Sw97_GetElement(isSling);
}

uint8_t Sw97_IsBowItem(uint16_t item) {
    return item == ITEM_BOW;
}
uint8_t Sw97_IsSlingItem(uint16_t item) {
    return item == ITEM_FAIRY_SLINGSHOT;
}

// The composite HUD icon is built from iconItemSegment[], which only refreshes when the button's
// item is (re)loaded. Changing the element does not change the item, so every setter has to ask for
// the reload by hand — otherwise layer 1 keeps the previous weapon and it reads as a texture bug.
void Sw97_RefreshButtonIcons(PlayState* play) {
    void Interface_LoadItemIconImpl(PlayState * play, u8 btn);
    for (int32_t i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
        uint8_t item = BUTTON_ITEM_EQUIP(0, i);
        if (Sw97_IsBowItem(item) || Sw97_IsSlingItem(item)) {
            Interface_LoadItemIconImpl(play, (u8)i);
        }
    }
}

// Same iconItemSegment caching, generalized: any button (C or D-pad) holding `itemId` gets its icon
// re-resolved. For the mode-following items (Elemental Wand rods, Sheikah Slate runes) whose icon
// changes without the button item ever changing.
void ExtInv_RefreshButtonIconsForItem(PlayState* play, uint16_t itemId) {
    void Interface_LoadItemIconImpl(PlayState * play, u8 btn);
    void Interface_Dpad_LoadItemIconImpl(PlayState * play, u8 btn);
    for (int32_t i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
        uint8_t it = GET_CUR_FORM_BTN_ITEM(i);
        uint16_t eff = (it == ITEM_EXT_BUTTON) ? EXT_BUTTON_ITEM(0, i) : it; // u16 items park a marker
        if (eff == itemId) {
            Interface_LoadItemIconImpl(play, (u8)i);
        }
    }
    for (int32_t i = EQUIP_SLOT_D_RIGHT; i <= EQUIP_SLOT_D_UP; i++) {
        if (DPAD_GET_CUR_FORM_BTN_ITEM(i) == itemId) { // D-pad has no EXT-marker slots
            Interface_Dpad_LoadItemIconImpl(play, (u8)i);
        }
    }
}

// Is this button item a weapon primed with bombs right now? THE predicate for "bombs are live" —
// bomb arrows on the bow, bomb bullets on the slingshot. Every HUD badge, input dispatch and
// cleanup guard goes through here so the two weapons can never drift apart. Skijer's NEI
uint8_t Sw97_ItemHasBombs(uint16_t item) {
    if (Sw97_IsBowItem(item)) {
        return Sw97_EffectiveElement(0) == SW97_ELEM_BOMB;
    }
    if (Sw97_IsSlingItem(item)) {
        return Sw97_EffectiveElement(1) == SW97_ELEM_BOMB;
    }
    return 0;
}

// Is Bomb Arrows the primed element on some button right now? Replaces the old
// IsItemEquipped(ITEM_BOMB_ARROWS), which scanned for a literal id that no longer lands there.
uint8_t Sw97_BombArrowsOnButton(void) {
    for (int32_t i = EQUIP_SLOT_B; i <= EQUIP_SLOT_C_RIGHT; i++) {
        if (Sw97_ItemHasBombs(BUTTON_ITEM_EQUIP(0, i))) {
            return 1;
        }
    }
    return 0;
}

// One-shot save migration off the per-element item ids. Idempotent via sw97LayoutVersion.
//
// The legacy ids are GONE from z64item.h, so they are spelled as raw values here on purpose — this
// function is the only place that must still recognise them. This sweep is NOT optional in 2ship:
// 0xA7..0xAC are live ITEM_MAP_POINT_* values again, so an unmigrated save would leave a map point
// sitting on a C-button.
void Sw97_MigrateLayout(PlayState* play) {
    NeiSaveData* nei = Nei_Save();
    if (nei->sw97LayoutVersion >= 1) {
        return;
    }

    // The slingshot's element was already a persisted index (1..6) with the SAME numbering as
    // SW97_ELEM_FIRE..WIND, so this is a plain copy.
    if ((nei->slingshotWheel >= 1) && (nei->slingshotWheel <= 6)) {
        nei->sw97SlingElement = nei->slingshotWheel;
    }
    nei->slingshotWheel = 0;

    // The bow's element was never persisted separately — it lived in the SLOT_BOW cell.
    {
        uint8_t bowCell = gSaveContext.save.saveInfo.inventory.items[SLOT_BOW];
        if ((bowCell >= 0xD0) && (bowCell <= 0xD5)) {
            nei->sw97BowElement = (uint8_t)(SW97_ELEM_FIRE + (bowCell - 0xD0));
            gSaveContext.save.saveInfo.inventory.items[SLOT_BOW] = ITEM_BOW;
        }
    }

    // Buttons, for every player form — MM keeps a separate equips block per form.
    for (int32_t form = 0; form < PLAYER_FORM_MAX; form++) {
        for (int32_t i = EQUIP_SLOT_B; i <= EQUIP_SLOT_C_RIGHT; i++) {
            uint8_t item = BUTTON_ITEM_EQUIP(form, i);
            if ((item >= 0xD0) && (item <= 0xD5)) {
                nei->sw97BowElement = (uint8_t)(SW97_ELEM_FIRE + (item - 0xD0));
                BUTTON_ITEM_EQUIP(form, i) = ITEM_BOW;
            } else if ((item >= 0xA7) && (item <= 0xAC)) {
                nei->sw97SlingElement = (uint8_t)(SW97_ELEM_FIRE + (item - 0xA7));
                BUTTON_ITEM_EQUIP(form, i) = ITEM_FAIRY_SLINGSHOT;
            } else if (item == ITEM_BOMB_ARROWS) {
                nei->sw97BowElement = SW97_ELEM_BOMB;
                BUTTON_ITEM_EQUIP(form, i) = ITEM_BOW;
            }
        }
    }

    // Page-2 slot 27 held ITEM_BOMB_ARROWS; the cell belongs to the Elemental Wand now.
    if (Nei_GetOwnedItem(SLOT_BOMB_ARROWS) == ITEM_BOMB_ARROWS) {
        nei->bombArrowsOwned = 1;
        Nei_SetOwnedItem(SLOT_BOMB_ARROWS, ITEM_NONE);
    }

    nei->sw97LayoutVersion = 1;
    Sw97_RefreshButtonIcons(play);
}

// ── Elemental Wand — six rods in one page-2 cell ─────────────────────────────────────────────────
static const uint8_t sWandQuest[WAND_MODE_COUNT] = {
    OOT_QUEST_MEDALLION_SPIRIT, // Sand Rod
    OOT_QUEST_MEDALLION_FOREST, // Tornado Rod
    OOT_QUEST_MEDALLION_WATER,  // Water Rod
    OOT_QUEST_MEDALLION_FIRE,   // Meteor Rod
    OOT_QUEST_MEDALLION_LIGHT,  // Storm Rod
    OOT_QUEST_MEDALLION_SHADOW, // Shadow Scepter
};
static const uint16_t sWandMedallion[WAND_MODE_COUNT] = {
    ITEM_MEDALLION_SPIRIT, ITEM_MEDALLION_FOREST, ITEM_MEDALLION_WATER,
    ITEM_MEDALLION_FIRE,   ITEM_MEDALLION_LIGHT,  ITEM_MEDALLION_SHADOW,
};
// Placeholder art note: until the six real PNGs land, dropping the rod texture files in place is the
// only change needed — these paths are already the final ones.
static void* const sWandIcon[WAND_MODE_COUNT] = {
    (void*)gItemIconSandRodTex,   (void*)gItemIconTornadoRodTex, (void*)gItemIconWaterRodTex,
    (void*)gItemIconMeteorRodTex, (void*)gItemIconStormRodTex,   (void*)gItemIconShadowScepterTex,
};
static void* const sWandNameTex[WAND_MODE_COUNT] = {
    (void*)gSandRodNameTex,   (void*)gTornadoRodNameTex, (void*)gWaterRodNameTex,
    (void*)gMeteorRodNameTex, (void*)gStormRodNameTex,   (void*)gShadowScepterNameTex,
};

void* Wand_ModeIcon(uint8_t mode) {
    return (mode < WAND_MODE_COUNT) ? sWandIcon[mode] : sWandIcon[0];
}
void* Wand_ModeNameTex(uint8_t mode) {
    return (mode < WAND_MODE_COUNT) ? sWandNameTex[mode] : sWandNameTex[0];
}

uint8_t Wand_RandoMode(void) {
    return (uint8_t)CVarGetInteger("gRando.Options.RO_ELEMENTAL_WAND_SHUFFLE", WAND_RANDO_MEDALLIONS);
}

uint16_t Wand_ModeMedallion(uint8_t mode) {
    return (mode < WAND_MODE_COUNT) ? sWandMedallion[mode] : ITEM_NONE;
}

// Which rods are usable. All three randomizer treatments share the SAME slot flag; they differ only
// in what unlocks an individual mode.
uint8_t Wand_ModeOwned(uint8_t mode) {
    if (mode >= WAND_MODE_COUNT) {
        return 0;
    }
    switch (Wand_RandoMode()) {
        case WAND_RANDO_SINGLE:
            return Nei_Save()->wandRodsOwned != 0; // one item lights all six
        case WAND_RANDO_ELEMENTAL:
            return (Nei_Save()->wandRodsOwned & (1 << mode)) != 0;
        case WAND_RANDO_MEDALLIONS:
        default:
            return (Nei_Save()->ootQuestItems & (1u << sWandQuest[mode])) != 0;
    }
}

void Wand_GrantMode(uint8_t mode) {
    if (mode >= WAND_MODE_COUNT) {
        return;
    }
    if (Wand_RandoMode() == WAND_RANDO_SINGLE) {
        Nei_Save()->wandRodsOwned = (1 << WAND_MODE_COUNT) - 1;
    } else {
        Nei_Save()->wandRodsOwned |= (1 << mode);
    }
    // Obtaining ANY rod hands over the slot if it isn't there yet.
    ExtInv_SetSlotItem(SLOT_ELEMENTAL_WAND, ITEM_ELEMENTAL_WAND);
}

uint8_t Wand_ModeCount(void) {
    uint8_t n = 0;
    for (uint8_t m = 0; m < WAND_MODE_COUNT; m++) {
        if (Wand_ModeOwned(m)) {
            n++;
        }
    }
    return n;
}

uint8_t Wand_ModeAt(uint8_t index) {
    uint8_t n = 0;
    for (uint8_t m = 0; m < WAND_MODE_COUNT; m++) {
        if (Wand_ModeOwned(m)) {
            if (n == index) {
                return m;
            }
            n++;
        }
    }
    return WAND_MODE_SAND;
}

uint8_t Wand_GetMode(void) {
    uint8_t m = Nei_Save()->wandMode;
    if (!Wand_ModeOwned(m)) {
        m = Wand_ModeAt(0);
        Nei_Save()->wandMode = m;
    }
    return m;
}

void Wand_SetMode(uint8_t mode) {
    if (Wand_ModeOwned(mode)) {
        Nei_Save()->wandMode = mode;
    }
}

uint8_t Wand_ModeNeighbor(uint8_t mode, int32_t dir) {
    uint8_t n = Wand_ModeCount();
    if (n <= 1) {
        return mode;
    }
    for (uint8_t i = 0; i < n; i++) {
        if (Wand_ModeAt(i) == mode) {
            return Wand_ModeAt((uint8_t)((i + n + (dir > 0 ? 1 : -1)) % n));
        }
    }
    return Wand_ModeAt(0);
}

// ── Sheikah Slate — four runes in one page-2 cell (wand idiom, no rando-mode split: each rune is
// always its own sibling item, "random" order comes from where the seed hides them) ──────────────
static void* const sSlateRuneMiniIcon[SLATE_RUNE_COUNT] = {
    (void*)"__OTR__textures/icon_item_custom/gItemIconSlateRuneBombTex",
    (void*)"__OTR__textures/icon_item_custom/gItemIconSlateRuneStasisTex",
    (void*)"__OTR__textures/icon_item_custom/gItemIconSlateRuneCryonisTex",
    (void*)"__OTR__textures/icon_item_custom/gItemIconSlateRuneMasterCycleTex",
};
static void* const sSlateRuneIcon[SLATE_RUNE_COUNT] = {
    (void*)"__OTR__textures/icon_item_custom/gItemIconSheikahSlateBombTex",
    (void*)"__OTR__textures/icon_item_custom/gItemIconSheikahSlateStasisTex",
    (void*)"__OTR__textures/icon_item_custom/gItemIconSheikahSlateCryonisTex",
    (void*)"__OTR__textures/icon_item_custom/gItemIconSheikahSlateMasterCycleTex",
};

void* Slate_RuneMiniIcon(uint8_t rune) {
    return (rune < SLATE_RUNE_COUNT) ? sSlateRuneMiniIcon[rune] : sSlateRuneMiniIcon[0];
}
void* Slate_RuneIcon(uint8_t rune) {
    return (rune < SLATE_RUNE_COUNT) ? sSlateRuneIcon[rune] : sSlateRuneIcon[0];
}

uint8_t Slate_RuneOwned(uint8_t rune) {
    if (rune >= SLATE_RUNE_COUNT) {
        return 0;
    }
    return (Nei_Save()->slateRunesOwned & (1 << rune)) != 0;
}

void Slate_GrantRune(uint8_t rune) {
    if (rune >= SLATE_RUNE_COUNT) {
        return;
    }
    Nei_Save()->slateRunesOwned |= (1 << rune);
    // The freshly obtained rune becomes the active one — this is also what makes the get-item
    // textbox icon (resolved through Slate_GetRune) show the rune that was just granted.
    Nei_Save()->slateMode = rune;
    // Obtaining ANY rune hands over the slate itself if it isn't there yet.
    ExtInv_GiveItem(SLOT_SHEIKAH_SLATE, EXT_ITEM_SHEIKAH_SLATE);
}

uint8_t Slate_RuneCount(void) {
    uint8_t n = 0;
    for (uint8_t r = 0; r < SLATE_RUNE_COUNT; r++) {
        if (Slate_RuneOwned(r)) {
            n++;
        }
    }
    return n;
}

uint8_t Slate_RuneAt(uint8_t index) {
    uint8_t n = 0;
    for (uint8_t r = 0; r < SLATE_RUNE_COUNT; r++) {
        if (Slate_RuneOwned(r)) {
            if (n == index) {
                return r;
            }
            n++;
        }
    }
    return SLATE_RUNE_BOMB;
}

uint8_t Slate_GetRune(void) {
    uint8_t r = Nei_Save()->slateMode;
    if (!Slate_RuneOwned(r)) {
        r = Slate_RuneAt(0);
        Nei_Save()->slateMode = r;
    }
    return r;
}

void Slate_SetRune(uint8_t rune) {
    if (Slate_RuneOwned(rune)) {
        Nei_Save()->slateMode = rune;
    }
}

uint8_t Slate_RuneNeighbor(uint8_t rune, int32_t dir) {
    uint8_t n = Slate_RuneCount();
    if (n <= 1) {
        return rune;
    }
    for (uint8_t i = 0; i < n; i++) {
        if (Slate_RuneAt(i) == rune) {
            return Slate_RuneAt((uint8_t)((i + n + (dir > 0 ? 1 : -1)) % n));
        }
    }
    return Slate_RuneAt(0);
}

uint8_t ExtInv_GetItemSlot(uint16_t itemId) {
    if (itemId < 52) {
        return gItemSlots[itemId];
    }
    // Page 2 items (incl. ROCS_CAPE -> shared SLOT_ROCS): unified NEI registry. Skijer's NEI
    const NeiItem* it = Nei_FindByItem(itemId);
    if (it != NULL && it->slot != NEI_NO_SLOT) {
        return it->slot;
    }
    // Page 3 MM Mask items
    for (int i = 0; i < 24; i++) {
        if (gPage3MaskItems[i] == itemId) {
            return 48 + i;
        }
    }
    return 0xFF;
}

// Icon texture size for the kaleido draws: the SW97 medallion/arrow icons are 24x24 quest
// icons (drawing them as 32x32 RGBA32 misreads the buffer into garbage). Everything else 32.
uint8_t ExtInv_GetItemIconSize(uint16_t itemId) {
    switch (itemId) {
        // Unified trade wheel: cell + the two preview markers all draw an OoT icon_item_static
        // texture, which is 32x32. Without this they'd fall through to the default and render as
        // garbage (a 24x24 read as 32x32). Skijer 2026-07-30
        case ITEM_TRADE_PLACEHOLDER:
        case ITEM_TRADE_PREV:
        case ITEM_TRADE_NEXT:
        case ITEM_OOT_MASK_PLACEHOLDER:
        case ITEM_OOT_MASK_PREV:
        case ITEM_OOT_MASK_NEXT:
            return 32;
        case ITEM_MEDALLION_FOREST:
        case ITEM_MEDALLION_FIRE:
        case ITEM_MEDALLION_WATER:
        case ITEM_MEDALLION_SPIRIT:
        case ITEM_MEDALLION_SHADOW:
        case ITEM_MEDALLION_LIGHT:
        // (The twelve SW97 arrow/bullet ids used to be listed here as 24x24 too. They are gone; the
        // composite draw sites size the medallion layer explicitly.)
        // Spiritual Stones use the 24x24 companion quest icons (same as medallions).
        case EXT_ITEM_SPIRITUAL_STONE_KOKIRI:
        case EXT_ITEM_SPIRITUAL_STONE_GORON:
        case EXT_ITEM_SPIRITUAL_STONE_ZORA:
            return 24;
        default:
            return 32;
    }
}
