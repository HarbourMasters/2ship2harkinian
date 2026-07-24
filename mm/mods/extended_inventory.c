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
                                  ITEM_BOMB_ARROWS,
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
const uint8_t gPage2ItemAgeReqs[24] = { AGE_REQ_NONE, AGE_REQ_NONE,  AGE_REQ_NONE, AGE_REQ_ADULT, AGE_REQ_NONE,
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
// by NeiSaveData). Wheels (keg/SW97/claw/picto/rocs/trade) fold extra items onto these cells.
static const uint8_t sOotPage0Map[24] = {
    // ROW 1: Sticks | Nuts | Bombs(keg wheel) | Arrows(SW97 wheel) | Fire Arrows | Din's Fire
    SLOT_DEKU_STICK, SLOT_DEKU_NUT, SLOT_BOMB, SLOT_BOW, SLOT_ARROW_FIRE, VSLOT_DINS,
    // ROW 2: Slingshot(SW97) | Ocarina | Bombchu | Hookshot(claw wheel) | Ice Arrows | Farore's
    VSLOT_SLINGSHOT, SLOT_OCARINA, SLOT_BOMBCHU, SLOT_HOOKSHOT, SLOT_ARROW_ICE, VSLOT_FARORES,
    // ROW 3: Boomerang | Lens(picto wheel) | Beans | Hammer | Light Arrows | Nayru's(rocs wheel)
    VSLOT_OOT_BOOMERANG, SLOT_LENS_OF_TRUTH, SLOT_MAGIC_BEANS, VSLOT_HAMMER, SLOT_ARROW_LIGHT, VSLOT_NAYRUS,
    // ROW 4: Bottles A | Bottles B | Net | Bottomless | Trade wheel | OoT Masks
    SLOT_BOTTLE_1, SLOT_BOTTLE_2, SLOT_BOTTLE_3, SLOT_BOTTLE_4, SLOT_TRADE_DEED, VSLOT_OOT_MASKS,
};

int ExtInv_GetInventorySlot(int visualSlot) {
    if (sExtInvState.currentPage == 0 && visualSlot >= 0 && visualSlot < 24) {
        return sOotPage0Map[visualSlot]; // OoT layout (Skijer's NEI)
    }
    return visualSlot + (sExtInvState.currentPage * 24);
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
            // Wheel: Roc's Feather (functional page-2 item) <-> Nayru's Love
            if (nei->nayruRocsMode && Nei_GetOwnedItem(SLOT_ROCS) != ITEM_NONE) {
                return ITEM_ROCS_FEATHER; // ship-vanilla feather (art + same jump behavior)
            }
            return (nei->ootSpellsOwned & 0x4) ? ITEM_NAYRUS_LOVE : ITEM_NONE;
        case VSLOT_SLINGSHOT:
            if (!nei->slingshotOwned) {
                return ITEM_NONE;
            }
            // Wheel: plain Fairy Slingshot <-> SW97 elemental bullets (CVar-gated, twin of the
            // bow cell's SW97 arrow wheel). Skijer's NEI slingshot pass.
            if ((nei->slingshotWheel >= 1) && (nei->slingshotWheel <= 6) &&
                CVarGetInteger("gEnhancements.SkijerNEI.SW97Medallions", 0)) {
                return ITEM_SW97_BULLET_FIRE + (nei->slingshotWheel - 1);
            }
            return ITEM_FAIRY_SLINGSHOT;
        case VSLOT_OOT_BOOMERANG:
            return nei->ootBoomerangOwned ? ITEM_BOOMERANG : ITEM_NONE;
        case VSLOT_HAMMER:
            // Megaton Hammer shown when owned. Its Iron Knuckle's Axe upgrade is behavior-only
            // (no distinct OoT icon) — WeaponUpgrade_HasHammerAxe drives the in-game axe strike.
            return nei->ootHammerOwned ? ITEM_HAMMER : ITEM_NONE;
        case VSLOT_OOT_MASKS:
            return ITEM_NONE; // OoT child-trade masks wheel: per-item pass
    }
    return ITEM_NONE;
}

void ExtInv_SetOotSlotItem(int slot, uint8_t itemId) {
    NeiSaveData* nei = Nei_Save();
    switch (slot) {
        case VSLOT_DINS:
            nei->ootSpellsOwned = (itemId == ITEM_DINS_FIRE) ? (nei->ootSpellsOwned | 0x1)
                                                             : (nei->ootSpellsOwned & ~0x1);
            break;
        case VSLOT_FARORES:
            nei->ootSpellsOwned = (itemId == ITEM_FARORES_WIND) ? (nei->ootSpellsOwned | 0x2)
                                                                : (nei->ootSpellsOwned & ~0x2);
            break;
        case VSLOT_NAYRUS:
            // Wheel writes: selecting Roc's flips the mode; selecting Nayru's flips back
            if (itemId == ITEM_NAYRUS_LOVE) {
                nei->ootSpellsOwned |= 0x4;
                nei->nayruRocsMode = 0;
            } else if (itemId != ITEM_NONE) {
                nei->nayruRocsMode = 1; // ITEM_ROCS_FEATHER selected
            }
            break;
        case VSLOT_SLINGSHOT:
            if (itemId == ITEM_FAIRY_SLINGSHOT) {
                nei->slingshotOwned = 1;
                nei->slingshotWheel = 0;
            } else if ((itemId >= ITEM_SW97_BULLET_FIRE) && (itemId <= ITEM_SW97_BULLET_WIND)) {
                // Wheel write: pick an elemental bullet (ownership unchanged)
                nei->slingshotOwned = 1;
                nei->slingshotWheel = (itemId - ITEM_SW97_BULLET_FIRE) + 1;
            } else {
                nei->slingshotOwned = 0;
                nei->slingshotWheel = 0;
            }
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
        case ITEM_FAIRY_SLINGSHOT:
        case ITEM_SW97_BULLET_FIRE: // Skijer's NEI: elemental bullets share the seed pouch
        case ITEM_SW97_BULLET_ICE:
        case ITEM_SW97_BULLET_LIGHT:
        case ITEM_SW97_BULLET_DARK:
        case ITEM_SW97_BULLET_SOUL:
        case ITEM_SW97_BULLET_WIND:
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
    { ITEM_ROCS_FEATHER_SKIJER, (void*)gItemIconRocsFeatherTex,       (void*)gRocsFeatherNameTex },       // 0x9D
    { ITEM_ROCS_CAPE,           (void*)gItemIconRocsCapeTex,          (void*)gRocsCapeNameTex },          // 0x9E
    { ITEM_DESIRE_SENSOR,       (void*)gItemIconDesireSensorTex,      (void*)gDesireSensorNameTex },      // 0x9F
    { ITEM_HYLIAS_GRACE,        (void*)gItemIconHyliaGraceTex,        (void*)gHyliaGraceNameTex },        // 0xA0
    { ITEM_ZONAI_PERMAFROST,    (void*)gItemIconZonaiPermafrostTex,   (void*)gZonaiPermafrostNameTex },   // 0xA1
    { ITEM_DEMISE_DESTRUCTION,  (void*)gItemIconDemiseDestructionTex, (void*)gDemiseDestructionNameTex }, // 0xA2
    { ITEM_DEKU_LEAF,           (void*)gItemIconDekuLeafTex,          (void*)gDekuLeafNameTex },          // 0xA3
    { ITEM_SWITCH_HOOK,         (void*)gItemIconSwitchHookTex,        (void*)gSwitchHookNameTex },        // 0xA4
    { ITEM_MOGMA_MITTS,         (void*)gItemIconMogmaMittsTex,        (void*)gMogmaMittsNameTex },        // 0xA5
    { ITEM_GUST_JAR,            (void*)gItemIconGustJarTex,           (void*)gGustJarNameTex },           // 0xA6
    { ITEM_BALL_AND_CHAIN,      (void*)gItemIconBallAndChainTex,      (void*)gBallAndChainNameTex },      // 0xA7
    { ITEM_WHIP,                (void*)gItemIconWhipTex,              (void*)gWhipNameTex },              // 0xA8
    { ITEM_SPINNER,             (void*)gItemIconSpinnerTex,           (void*)gSpinnerNameTex },           // 0xA9
    { ITEM_CANE_OF_SOMARIA,     (void*)gItemIconCaneOfSomariaTex,     (void*)gCaneOfSomariaNameTex },     // 0xAA
    { ITEM_DOMINION_ROD,        (void*)gItemIconDominionRodTex,       (void*)gDominionRodNameTex },       // 0xAB
    { ITEM_TIME_GATE,           (void*)gItemIconTimeGateTex,          (void*)gTimeGateNameTex },          // 0xAC
    { ITEM_BOMB_ARROWS,         (void*)gItemIconBombArrowsTex,        (void*)gBombArrowsNameTex },        // 0xAD
    { ITEM_ROD_FIRE,            (void*)gItemIconFireRodTex,           (void*)gFireRodNameTex },           // 0xAE
    { ITEM_ROD_ICE,             (void*)gItemIconIceRodTex,            (void*)gIceRodNameTex },            // 0xAF
    { ITEM_ROD_LIGHT,           (void*)gItemIconLightRodTex,          (void*)gLightRodNameTex },          // 0xB0
    { ITEM_BEETLE,              (void*)gItemIconBeetleTex,            (void*)gBeetleNameTex },            // 0xB1
    { ITEM_SHOVEL,              (void*)gItemIconShovelTex,            (void*)gShovelNameTex },            // 0xB2
    { ITEM_MINISH_CAP,          (void*)gItemIconMinishCapTex,         (void*)gMinishCapNameTex },         // 0xB3
    // Lantern: name texture is constant, but the icon is chosen dynamically by
    // fire type -> icon left NULL so the icon getter handles it below.
    { ITEM_LANTERN,             NULL,                                 (void*)gLanternNameTex },           // 0xB4
    { ITEM_POKEBALL,            (void*)gItemIconPokeballTex,          (void*)gPokeballNameTex },
    // Bottle Randomizer extra items (Skijer's NEI). Net + Bottomless Bottle; SLOT_BOTTLE_3/4.
    { ITEM_NET,                 (void*)gItemIconNetTex,               (void*)gNetNameTex },               // 0xF4
    { ITEM_BOTTOMLESS_BOTTLE,   (void*)gItemIconBottomlessBottleTex,  (void*)gBottomlessBottleNameTex },  // 0xF5
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
            case ITEM_DINS_FIRE:       ootPath = "__OTR__textures/item_name_static/gDinsFireItemNameENGTex"; break;
            case ITEM_FARORES_WIND:    ootPath = "__OTR__textures/item_name_static/gFaroresWindItemNameENGTex"; break;
            case ITEM_NAYRUS_LOVE:     ootPath = "__OTR__textures/item_name_static/gNayrusLoveItemNameENGTex"; break;
            case ITEM_FAIRY_SLINGSHOT: ootPath = "__OTR__textures/item_name_static/gFairySlingshotItemNameENGTex"; break;
            case ITEM_HOOKSHOT_OOT:    ootPath = "__OTR__textures/item_name_static/gHookshotItemNameENGTex"; break;
            case ITEM_LONGSHOT_OOT:    ootPath = "__OTR__textures/item_name_static/gLongshotItemNameENGTex"; break;
            case ITEM_BOOMERANG:       ootPath = "__OTR__textures/item_name_static/gBoomerangItemNameENGTex"; break;
            case ITEM_HAMMER:          ootPath = "__OTR__textures/item_name_static/gMegatonHammerItemNameENGTex"; break;
            default: break;
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
        case 0: return (void*)gItemIconVanishCapTex;
        case 1: return (void*)gItemIconMetalCapTex;
        case 2: return (void*)gItemIconWingCapTex;
        case 3: return (void*)gItemIconFireFlowerTex;
        default: return NULL;
    }
}

void* ExtInv_GetItemIcon(uint16_t itemId) {
    // OoT page-0 items: vanilla OoT icons from the companion oot.o2r (FileExists-gated; a
    // missing companion just leaves the cell blank until per-item MM art lands).
    {
        extern u8 ResourceMgr_FileExists(const char* resName);
        const char* ootPath = NULL;
        switch (itemId) {
            case ITEM_DINS_FIRE:       ootPath = "__OTR__textures/icon_item_static/gItemIconDinsFireTex"; break;
            case ITEM_FARORES_WIND:    ootPath = "__OTR__textures/icon_item_static/gItemIconFaroresWindTex"; break;
            case ITEM_NAYRUS_LOVE:     ootPath = "__OTR__textures/icon_item_static/gItemIconNayrusLoveTex"; break;
            case ITEM_FAIRY_SLINGSHOT: ootPath = "__OTR__textures/icon_item_static/gItemIconSlingshotTex"; break;
            case ITEM_HOOKSHOT_OOT:    ootPath = "__OTR__textures/icon_item_static/gItemIconHookshotTex"; break;
            case ITEM_LONGSHOT_OOT:    ootPath = "__OTR__textures/icon_item_static/gItemIconLongshotTex"; break;
            case ITEM_BOOMERANG:       ootPath = "__OTR__textures/icon_item_static/gItemIconBoomerangTex"; break;
            case ITEM_HAMMER:          ootPath = "__OTR__textures/icon_item_static/gItemIconHammerTex"; break;
            case ITEM_ROCS_FEATHER:    ootPath = "__OTR__textures/icon_item_static/gRocsFeatherTex"; break; // ship-vanilla art (baked)
            default: break;
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

    // SW97 medallions + elemental arrows: the OoT medallion quest icons, 24x24 (SoH 1:1 —
    // draw sites use ExtInv_GetItemIconSize; a 24x24 RGBA32 drawn as 32x32 reads as garbage).
    switch (itemId) {
        case ITEM_MEDALLION_FOREST:
        case ITEM_SW97_ARROW_WIND:
        case ITEM_SW97_BULLET_WIND:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionForestTex";
        case ITEM_MEDALLION_FIRE:
        case ITEM_SW97_ARROW_FIRE:
        case ITEM_SW97_BULLET_FIRE:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionFireTex";
        case ITEM_MEDALLION_WATER:
        case ITEM_SW97_ARROW_ICE:
        case ITEM_SW97_BULLET_ICE:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionWaterTex";
        case ITEM_MEDALLION_SPIRIT:
        case ITEM_SW97_ARROW_SOUL:
        case ITEM_SW97_BULLET_SOUL:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionSpiritTex";
        case ITEM_MEDALLION_SHADOW:
        case ITEM_SW97_ARROW_DARK:
        case ITEM_SW97_BULLET_DARK:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionShadowTex";
        case ITEM_MEDALLION_LIGHT:
        case ITEM_SW97_ARROW_LIGHT:
        case ITEM_SW97_BULLET_LIGHT:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionLightTex";
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

    // Pictograph Box shares the Lens of Truth slot. When the slot's pictobox mode is selected (kaleido
    // A-toggle), show the pictobox icon in the slot instead of the Lens — clear feedback for the swap,
    // mirroring the Clawshot/Gale overrides above. Skijer's NEI
    {
        extern unsigned char Picto_IsOwned(void);
        extern unsigned char Picto_IsOnLensActive(void);
        if (itemId == ITEM_LENS_OF_TRUTH && Picto_IsOwned() && Picto_IsOnLensActive()) {
            void* t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconPictographBoxTex");
            if (t) {
                return t;
            }
        }
    }

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
        if (itemId == ITEM_GOLD_DUST)             t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledGoldDustTex");
        else if (itemId == ITEM_HOT_SPRING_WATER) t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconHotSpringWaterTex");
        else if (itemId == ITEM_DEKU_PRINCESS)    t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledDekuPrincessTex");
        else if (itemId == ITEM_SEAHORSE)         t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledSeahorseTex");
        else if (itemId == ITEM_SPRING_WATER)     t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconSpringWaterTex");
        else if (itemId == ITEM_ZORA_EGG)         t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledZoraEggTex");
        else if (itemId == ITEM_HYLIAN_LOACH)     t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconBottledHylianLoachTex");
        else if (itemId == ITEM_OBABA_DRINK)      t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconEmptyBottle2Tex");
        if (t) return t;
    }

    // MM adult trade-quest items (Skijer's NEI) — shown in the SLOT_TRADE_ADULT 2D-grid wheel. Icons
    // from mm.o2r. The Pendant of Memories (ITEM_EXT_BOOTS_2) is the combat item, so it gets its icon
    // from the ext-equipment block below. Special Delivery to Mama reuses MM's "Letter to Mama" icon.
    {
        void* t = NULL;
        if (itemId == ITEM_MM_MOONS_TEAR)            t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconMoonsTearTex");
        else if (itemId == ITEM_MM_DEED_LAND)        t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconLandDeedTex");
        else if (itemId == ITEM_MM_DEED_SWAMP)       t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconSwampDeedTex");
        else if (itemId == ITEM_MM_DEED_MOUNTAIN)    t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconMountainDeedTex");
        else if (itemId == ITEM_MM_DEED_OCEAN)       t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconOceanDeedTex");
        else if (itemId == ITEM_MM_ROOM_KEY)         t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconRoomKeyTex");
        else if (itemId == ITEM_MM_LETTER_KAFEI)     t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconLetterToKafeiTex");
        else if (itemId == ITEM_MM_SPECIAL_DELIVERY) t = MmAssets_LoadResource("__OTR__icon_item_static_yar/gItemIconLetterToMamaTex");
        if (t) return t;
    }

    if (itemId < 156) {
        return (itemId < 0x9A) ? (void*)gItemIcons[itemId] : NULL; // NEI: never index OOB (garbage tex)
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
        case ITEM_PH_ICON_POT:    return (void*)gItemIconPropHuntPotTex;
        case ITEM_PH_ICON_ENEMY:  return (void*)gItemIconPropHuntEnemyTex;
        case ITEM_PH_ICON_NPC:    return (void*)gItemIconPropHuntNpcTex;
        case ITEM_PH_ICON_CHANGE: return (void*)gItemIconPropHuntChangeTex;
        case ITEM_PH_ICON_PREV:   return (void*)gItemIconPropHuntPrevTex;
        case ITEM_PH_ICON_NEXT:   return (void*)gItemIconPropHuntNextTex;

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

        // SW97 Arrow items (arrow mode — SAME medallion icons)
        case ITEM_SW97_ARROW_FIRE:
        case ITEM_SW97_BULLET_FIRE: // slingshot-wheel bullet twins share the medallion icons
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionFireTex";
        case ITEM_SW97_ARROW_ICE:
        case ITEM_SW97_BULLET_ICE:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionWaterTex";
        case ITEM_SW97_ARROW_LIGHT:
        case ITEM_SW97_BULLET_LIGHT:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionLightTex";
        case ITEM_SW97_ARROW_DARK:
        case ITEM_SW97_BULLET_DARK:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionShadowTex";
        case ITEM_SW97_ARROW_SOUL:
        case ITEM_SW97_BULLET_SOUL:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionSpiritTex";
        case ITEM_SW97_ARROW_WIND:
        case ITEM_SW97_BULLET_WIND:
            return (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionForestTex";

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
        case ITEM_MEDALLION_FOREST:
        case ITEM_MEDALLION_FIRE:
        case ITEM_MEDALLION_WATER:
        case ITEM_MEDALLION_SPIRIT:
        case ITEM_MEDALLION_SHADOW:
        case ITEM_MEDALLION_LIGHT:
        case ITEM_SW97_ARROW_FIRE:
        case ITEM_SW97_ARROW_ICE:
        case ITEM_SW97_ARROW_LIGHT:
        case ITEM_SW97_ARROW_DARK:
        case ITEM_SW97_ARROW_SOUL:
        case ITEM_SW97_ARROW_WIND:
        case ITEM_SW97_BULLET_FIRE:
        case ITEM_SW97_BULLET_ICE:
        case ITEM_SW97_BULLET_LIGHT:
        case ITEM_SW97_BULLET_DARK:
        case ITEM_SW97_BULLET_SOUL:
        case ITEM_SW97_BULLET_WIND:
        // Spiritual Stones use the 24x24 companion quest icons (same as medallions).
        case EXT_ITEM_SPIRITUAL_STONE_KOKIRI:
        case EXT_ITEM_SPIRITUAL_STONE_GORON:
        case EXT_ITEM_SPIRITUAL_STONE_ZORA:
            return 24;
        default:
            return 32;
    }
}
