/**
 * extended_equipment.c - Extended equipment system (cheat)
 *
 * Core system: page switching, equip/unequip, icon/name lookup, behavior dispatch.
 * Follows the same pattern as extended_inventory.c.
 *
 * When the cheat CVar is enabled, pressing L on the equipment page toggles
 * to a second page showing 12 new equipment pieces (3 per category).
 * Equipped state is persisted via CVars.
 */

#include "extended_equipment.h"
#include "nei_save.h" // Skijer's NEI
#include "transformation_masks/transformation_masks.h"
#include "transformation_masks/assets/mm_asset_loader.h"
#include "pak_loader/pak_loader.h"

extern MmPlayerTransformation MmForm_GetCurrentForm(void);
// trade_items.c ships no header; declared locally, as the save editor does. The Pendant of
// Memories lives on the adult trade wheel — that bit is its ONLY ownership flag since the ext
// BOOTS-2 grid slot became the Climb Boots (Skijer 2026-07-29).
#define TRADE_ADULT_PENDANT 19
extern u8 TradeAdult_IsOwnedIndex(s32 index);
extern void TradeAdult_GiveIndex(s32 index);
#include <string.h>
#include <math.h>
#include "z64.h"
#include "z64player.h"
#include "z64save.h"
#include "functions.h"
#include "variables.h"

extern SaveContext gSaveContext;
extern s32 CVarGetInteger(const char* name, s32 defaultValue);
extern f32 CVarGetFloat(const char* name, f32 defaultValue);

// Cane of Byrna 3D model: blue-tinted variant of the Somaria cane, loaded from
// soh.o2r (objects/object_somaria/g_byrna_cane_dl — shares the Somaria tri
// geometry). No inline C model. LoadGfxByName crashes on a missing path, so gate.
extern u8 ResourceMgr_FileExists(const char* resName);
extern Gfx* ResourceMgr_LoadGfxByName(const char* path);

static Gfx* Byrna_GetCaneDL(void) {
    static Gfx* sCached = NULL;
    static u8 sTried = 0;
    if (!sTried) {
        sTried = 1;
        const char* otr = "__OTR__objects/object_somaria/g_byrna_cane_dl";
        if (ResourceMgr_FileExists(otr)) {
            sCached = ResourceMgr_LoadGfxByName(otr);
        }
    }
    return sCached;
}

// NEI Weapon Upgrades — the Hammer upgrade (Iron Knuckle's Axe) is driven from here,
// independent of the extended-equipment cheat. Accessors are defined in
// mods/items/logic/weapon_upgrades.c (linked via the custom_items.c TU).
#include "items/logic/weapon_upgrades.h"

// Unity build includes
#include "equipment/ext_equip_icon_assets.h" // custom equipment icon dg-macros (dgItemIcon*Tex)
#include "equipment/ext_equip_icons.c"
#include "equipment/ext_equip_names.c"
#include "equipment/ext_equip_behavior.c"

// Age requirements (mirror extended_inventory.h to avoid header cycle)
// 2ship: MM has no LINK_AGE_ADULT/CHILD. Keep OoT's numeric values (ADULT=0,
// CHILD=1, NONE=9) so the age-req tables compile; the age CHECK below is
// neutralized because MM never gates equipment by Link's age.
#ifndef AGE_REQ_NONE
#define AGE_REQ_NONE 9
#endif
#ifndef AGE_REQ_ADULT
#define AGE_REQ_ADULT 0
#endif
#ifndef AGE_REQ_CHILD
#define AGE_REQ_CHILD 1
#endif

// Per-piece age requirement: [equipType][index-1] (2026-07-29 layout)
//   SWORD:  Cane of Byrna,    Four Sword,    Trident
//   SHIELD: Goddess Shield,   Kite Shield,   Shield of Ikana
//   TUNIC:  Champion's Tunic, Magic Tunic,   Sage's
//   BOOTS:  Pegasus Boots,    Climb Boots,   Roc Boots
static const u8 sExtEquipAgeReqs[4][3] = {
    { AGE_REQ_NONE,  AGE_REQ_CHILD, AGE_REQ_ADULT },
    { AGE_REQ_NONE,  AGE_REQ_NONE,  AGE_REQ_CHILD },
    { AGE_REQ_NONE,  AGE_REQ_NONE,  AGE_REQ_NONE },
    { AGE_REQ_NONE,  AGE_REQ_NONE,  AGE_REQ_ADULT },
};

u8 ExtEquip_GetAgeReq(s16 equipType, u8 index) {
    if (equipType < 0 || equipType >= 4 || index < 1 || index > 3)
        return AGE_REQ_NONE;
    return sExtEquipAgeReqs[equipType][index - 1];
}

u8 ExtEquip_CheckAgeReq(s16 equipType, u8 index) {
    if (CVarGetInteger("gCheats.TimelessEquipment", 0))
        return 1;
    u8 req = ExtEquip_GetAgeReq(equipType, index);
    if (req == AGE_REQ_NONE) {
        return 1;
    }
    // MM's linkAge is a dead field: z64save.h says it "still exists in MM, but is always set to 0
    // (always adult)". Comparing against it meant every AGE_REQ_CHILD piece — Four Sword, Shield of
    // Ikana — was permanently unequippable here, and the MM kaleido doesn't even beep on refusal, so
    // it looked like the item simply hadn't crossed from OoT. MM's real age switch is the Time Gate:
    // timeGateAdultMode forces OoT adult Link, and without it Link is the child. Skijer's NEI
    u8 age = Nei_Save()->timeGateAdultMode ? AGE_REQ_ADULT : AGE_REQ_CHILD;
    return req == age;
}

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------
ExtendedEquipmentState gExtEquipState;
u8 gExtEquipSuppressIconOverride = 0;
f32 gChampionSlowFactor = 1.0f;

// Transform backup: stores equipped ext equipment indices before transformation
static u8 sTransformBackup[4] = { 0 }; // [EQUIP_TYPE_SWORD..BOOTS]
static u8 sTransformBackupValid = 0;

#define EXT_EQUIP_PAGE_SWITCH_COOLDOWN 15

// ---------------------------------------------------------------------------
// Page management
// ---------------------------------------------------------------------------

void ExtEquip_Init(void) {
    memset(&gExtEquipState, 0, sizeof(gExtEquipState));
    memset(&gExtEquipBehavior, 0, sizeof(gExtEquipBehavior));

    // Load equipped state from save data (per-file, persisted only on game save) // Skijer's NEI
    gExtEquipState.currentExtSword = Nei_Save()->extEquipSword;
    gExtEquipState.currentExtShield = Nei_Save()->extEquipShield;
    gExtEquipState.currentExtTunic = Nei_Save()->extEquipTunic;
    gExtEquipState.currentExtBoots = Nei_Save()->extEquipBoots;

    // Migrate the old tunic layout (Cape/Spirit/Champion) to
    // Champion/Spirit/Sage's. Equipped implies owned.
    if (Nei_Save()->extTunicLayoutVersion < 1) {
        u8 hadCape = ExtEquip_HasItem(EQUIP_TYPE_TUNIC, 1);
        u8 hadChampion = ExtEquip_HasItem(EQUIP_TYPE_TUNIC, 3);
        u8 legacyEquippedTunic = gExtEquipState.currentExtTunic;

        ExtEquip_RemoveItem(EQUIP_TYPE_TUNIC, 1);
        ExtEquip_RemoveItem(EQUIP_TYPE_TUNIC, 3);
        if (hadCape || legacyEquippedTunic == 1) {
            Nei_Save()->capeOwned = 1;
        }
        if (hadChampion || legacyEquippedTunic == 3) {
            ExtEquip_GiveItem(EQUIP_TYPE_TUNIC, 1);
        }
        if (legacyEquippedTunic == 3) {
            gExtEquipState.currentExtTunic = 1;
            Nei_Save()->extEquipTunic = 1;
        } else if (legacyEquippedTunic == 1) {
            gExtEquipState.currentExtTunic = 0;
            Nei_Save()->extEquipTunic = 0;
        }
        Nei_Save()->extTunicLayoutVersion = 1;
    }
    // Migrate the old boots layout (Pegasus / Pendant / Water Dragon Scale) to the real-boots layout
    // (Pegasus / Climb / Roc). The Pendant keeps living on the left column, so its ownership moves off
    // the BOOTS-2 bit onto the adult trade wheel; the dead Dragon-Scale bit is cleared so it can't
    // read as "owns the Roc Boots". Equipped implies owned.
    if (Nei_Save()->extBootsLayoutVersion < 1) {
        if (ExtEquip_HasItem(EQUIP_TYPE_BOOTS, 2) || gExtEquipState.currentExtBoots == 2) {
            TradeAdult_GiveIndex(TRADE_ADULT_PENDANT); // keep the Pendant owned on the trade wheel
        }
        ExtEquip_RemoveItem(EQUIP_TYPE_BOOTS, 2);
        ExtEquip_RemoveItem(EQUIP_TYPE_BOOTS, 3);
        if (gExtEquipState.currentExtBoots == 2 || gExtEquipState.currentExtBoots == 3) {
            gExtEquipState.currentExtBoots = 0;
            Nei_Save()->extEquipBoots = 0;
        }
        Nei_Save()->extBootsLayoutVersion = 1;
    }

    // Clamp to valid range
    if (gExtEquipState.currentExtSword > 3)
        gExtEquipState.currentExtSword = 0;
    if (gExtEquipState.currentExtShield > 3)
        gExtEquipState.currentExtShield = 0;
    if (gExtEquipState.currentExtTunic > 3)
        gExtEquipState.currentExtTunic = 0;
    if (gExtEquipState.currentExtBoots > 3)
        gExtEquipState.currentExtBoots = 0;

    // Existing saves may already have an extended piece equipped from the core
    // equipment page while the legacy cheat CVar is off. Restore the complete
    // behavior/draw path for that persisted loadout.
    if (gExtEquipState.currentExtSword != 0 || gExtEquipState.currentExtShield != 0 ||
        gExtEquipState.currentExtTunic != 0 || gExtEquipState.currentExtBoots != 0) {
        CVarSetInteger(CVAR_EXT_EQUIP_ENABLED, 1);
    }

    // Generate placeholder icons
    ExtEquip_GenerateIcons();
}

void ExtEquip_Update(void) {
    if (gExtEquipState.pageSwitchTimer > 0) {
        gExtEquipState.pageSwitchTimer--;
    }

    // If cheat was disabled, reset page and clear equipped state
    if (!ExtEquip_IsEnabled()) {
        gExtEquipState.equipPage = 0;
        // Don't clear equipped state here — it persists in CVars
        // and will be re-applied when cheat is re-enabled
    }
}

int ExtEquip_GetPage(void) {
    if (!ExtEquip_IsEnabled()) {
        return 0;
    }
    return gExtEquipState.equipPage;
}

void ExtEquip_SwitchPage(void) {
    if (!ExtEquip_IsEnabled())
        return;

    gExtEquipState.equipPage = (gExtEquipState.equipPage == 0) ? 1 : 0;
    gExtEquipState.pageSwitchTimer = EXT_EQUIP_PAGE_SWITCH_COOLDOWN;

    // When switching to vanilla page, restore original sword if Byrna was overriding it
    if (gExtEquipState.equipPage == 0 && gExtEquipBehavior.byrnaActive) {
        Byrna_Cleanup();
    }
}

u8 ExtEquip_CanSwitch(void) {
    return gExtEquipState.pageSwitchTimer <= 0;
}

u8 ExtEquip_IsEnabled(void) {
    return CVarGetInteger(CVAR_EXT_EQUIP_ENABLED, 0) != 0;
}

// ---------------------------------------------------------------------------
// Equip / Unequip
// ---------------------------------------------------------------------------

static void ExtEquip_SetCurrentByType(s16 equipType, u8 index) {
    switch (equipType) {
        case EQUIP_TYPE_SWORD:
            gExtEquipState.currentExtSword = index;
            Nei_Save()->extEquipSword = index; // Skijer's NEI
            break;
        case EQUIP_TYPE_SHIELD:
            gExtEquipState.currentExtShield = index;
            Nei_Save()->extEquipShield = index; // Skijer's NEI
            // Equipping an ext shield (index > 0) overrides any vanilla Deku skin so the draw/behavior
            // don't disagree (Deku model over an Ikana=Mirror value). Not cleared on unequip (index 0)
            // so the vanilla Deku set right before ExtEquip_Unequip in the kaleido survives.
            if (index != 0) {
                Nei_Save()->vanillaShieldSkin = 0;
            }
            break;
        case EQUIP_TYPE_TUNIC:
            gExtEquipState.currentExtTunic = index;
            Nei_Save()->extEquipTunic = index; // Skijer's NEI
            break;
        case EQUIP_TYPE_BOOTS:
            gExtEquipState.currentExtBoots = index;
            Nei_Save()->extEquipBoots = index; // Skijer's NEI
            break;
    }
}

// ---------------------------------------------------------------------------
// Ownership
// ---------------------------------------------------------------------------

static u32 ExtEquip_GetBit(s16 equipType, u8 index) {
    return 1 << (EXT_EQUIP_OWNED_SHIFT + equipType * 3 + (index - 1));
}

u8 ExtEquip_HasItem(s16 equipType, u8 index) {
    if (index == 0 || index > 3 || equipType < 0 || equipType > 3)
        return 0;
    return (Nei_Save()->extEquipOwnedBits & ExtEquip_GetBit(equipType, index)) != 0; // Skijer's NEI
}

// ---------------------------------------------------------------------------
// Retired slots — NONE left (Skijer 2026-07-29, kaleido re-layout). History:
//   TUNIC 1 (Magic Cape)          -> left column; ownership in Nei_Save()->capeOwned. Slot re-used
//                                    by the Champion's Tunic (2026-07-16).
//   BOOTS 2 (Pendant of Memories) -> left column; ownership in Nei_Save()->pendantOwned. Slot
//                                    re-used by the CLIMB BOOTS.
//   BOOTS 3 (Water Dragon Scale)  -> deleted (Zora swim = ZORA TUNIC's permanent effect). Slot
//                                    re-used by the ROC BOOTS.
// Kept as a function so the kaleido/save-editor call sites stay put if a slot is ever parked again.
// ---------------------------------------------------------------------------
u8 ExtEquip_SlotRetired(s16 equipType, u8 index) {
    (void)equipType;
    (void)index;
    return false;
}

u8 ExtEquip_CapeOwned(void) {
    return Nei_Save()->capeOwned;
}

void ExtEquip_GiveCape(void) {
    Nei_Save()->capeOwned = 1;
}

u8 ExtEquip_CapeVisible(void) {
    return ExtEquip_CapeOwned() && !Nei_Save()->capeHidden;
}

void ExtEquip_ToggleCapeVisibility(void) {
    Nei_Save()->capeHidden = !Nei_Save()->capeHidden;
}

u8 ExtEquip_IsChampionTunic(void) {
    return ExtEquip_IsEnabled() && ExtEquip_GetCurrent(EQUIP_TYPE_TUNIC) == 1;
}

// Magic Tunic (slot 2) — TP-style Magic Armor. Every effect it has (damage absorption, the
// underwater-breath skip, the orange visual) is gated on actually carrying rupees; broke, the
// tunic is dead weight. Mirrors soh's ExtEquip_IsSpiritTunic / ExtEquip_SpiritHasMoney.
u8 ExtEquip_IsSpiritTunic(void) {
    return ExtEquip_IsEnabled() && ExtEquip_GetCurrent(EQUIP_TYPE_TUNIC) == 2;
}

u8 ExtEquip_SpiritHasMoney(void) {
    return ExtEquip_IsSpiritTunic() && (gSaveContext.save.saveInfo.playerData.rupees > 0);
}

u8 ExtEquip_IsSagesTunic(void) {
    return ExtEquip_IsEnabled() && ExtEquip_GetCurrent(EQUIP_TYPE_TUNIC) == 3;
}

u8 ExtEquip_HasSagesResistance(SagesResistance resistance) {
    static const u8 sQuestItems[] = {
        OOT_QUEST_MEDALLION_WATER,  OOT_QUEST_MEDALLION_FIRE,   OOT_QUEST_MEDALLION_LIGHT,
        OOT_QUEST_MEDALLION_SHADOW, OOT_QUEST_MEDALLION_SPIRIT, OOT_QUEST_MEDALLION_FOREST,
    };

    return ExtEquip_IsSagesTunic() && resistance >= SAGES_RESIST_ICE &&
           resistance <= SAGES_RESIST_WIND &&
           (Nei_Save()->ootQuestItems & (1u << sQuestItems[resistance])) != 0;
}

// Sage's Tunic damage flash: when a medallion resistance absorbs a hit, the tunic briefly dyes
// itself with that medallion's color, then fades back to white. Continuous sources (wind, shock)
// keep refreshing the timer, so the dye holds while the medallion is still "feeding" the tunic.
#define SAGES_FLASH_HOLD_FRAMES 20
#define SAGES_FLASH_FADE_FRAMES 30
static const u8 sSagesMedallionColors[6][3] = {
    { 60, 130, 235 },  // ICE     <- Water Medallion
    { 235, 60, 30 },   // FIRE    <- Fire Medallion
    { 245, 225, 80 },  // THUNDER <- Light Medallion
    { 155, 70, 220 },  // STUN    <- Shadow Medallion
    { 240, 140, 40 },  // FALL    <- Spirit Medallion
    { 70, 195, 90 },   // WIND    <- Forest Medallion
};
static s16 sSagesFlashTimer = 0;
static u8 sSagesFlashResist = 0;

void ExtEquip_SagesFlash(SagesResistance resistance) {
    if (resistance > SAGES_RESIST_WIND) {
        return;
    }
    sSagesFlashResist = resistance;
    sSagesFlashTimer = SAGES_FLASH_HOLD_FRAMES + SAGES_FLASH_FADE_FRAMES;
}

void ExtEquip_SagesFlashTick(void) {
    if (sSagesFlashTimer > 0) {
        sSagesFlashTimer--;
    }
}

void ExtEquip_GetSagesTunicColor(u8* r, u8* g, u8* b) {
    *r = 235;
    *g = 240;
    *b = 245;
    if (ExtEquip_IsSagesTunic() && sSagesFlashTimer > 0) {
        const u8* m = sSagesMedallionColors[sSagesFlashResist];
        s32 num = (sSagesFlashTimer >= SAGES_FLASH_FADE_FRAMES) ? SAGES_FLASH_FADE_FRAMES : sSagesFlashTimer;

        *r = (u8)(*r + (((s32)m[0] - *r) * num) / SAGES_FLASH_FADE_FRAMES);
        *g = (u8)(*g + (((s32)m[1] - *g) * num) / SAGES_FLASH_FADE_FRAMES);
        *b = (u8)(*b + (((s32)m[2] - *b) * num) / SAGES_FLASH_FADE_FRAMES);
    }
}

// Pendant of Memories — TWO separate flags (Skijer 2026-07-31, user decision):
//
//   OWN     (Nei_Save()->pendantOwned)     the EQUIPMENT piece. Holding the pendant in the adult
//                                          trade slot GRANTS it, and from then on it is PERMANENT:
//                                          the trade item can be handed away, the equipment piece
//                                          cannot. This is what the kaleido equipment upgrade column
//                                          shows and lets you toggle, and what FleetSync carries.
//   EFFECT  (Nei_Save()->pendantEffectOff) the moveset on/off toggle (A on that cell).
//
// Before this, PendantOwned read the trade wheel DIRECTLY, so trading the pendant away silently
// deleted the equipment piece and its whole moveset.
void ExtEquip_GivePendant(void) {
    Nei_Save()->pendantOwned = 1;
}

u8 ExtEquip_PendantOwned(void) {
    // Latch on observation: this catches every acquisition path (trade grant, rando, save load,
    // FleetSync) without having to hook each one. Idempotent, and it never clears.
    if (!Nei_Save()->pendantOwned && TradeAdult_IsOwnedIndex(TRADE_ADULT_PENDANT)) {
        ExtEquip_GivePendant();
    }
    return Nei_Save()->pendantOwned;
}

u8 ExtEquip_PendantActive(void) {
    return ExtEquip_PendantOwned() && !Nei_Save()->pendantEffectOff;
}

void ExtEquip_TogglePendantEffect(void) {
    Nei_Save()->pendantEffectOff = !Nei_Save()->pendantEffectOff;
}

void ExtEquip_GiveItem(s16 equipType, u8 index) {
    if (index == 0 || index > 3 || equipType < 0 || equipType > 3)
        return;
    Nei_Save()->extEquipOwnedBits |= ExtEquip_GetBit(equipType, index); // Skijer's NEI
}

// Clear vanilla equipment base that was set for ext equipment.
// Called only from explicit toggle-off paths (not from vanilla equip path,
// which sets its own vanilla equipment before calling ExtEquip_Unequip).
static void ExtEquip_ClearVanillaEquip(s16 equipType) {
    switch (equipType) {
        case EQUIP_TYPE_SWORD:
            // 2ship: MM's Inventory_ChangeEquipment(s16) only touches the shield, so set
            // the sword nibble directly via SET_EQUIP_VALUE (faithful to OoT's 2-arg call).
            SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_NONE);
            gSaveContext.save.saveInfo.equips.buttonItems[0][0] = ITEM_NONE; // 2ship: nested equips, B button
            break;
        case EQUIP_TYPE_SHIELD:
            Inventory_ChangeEquipment(EQUIP_VALUE_SHIELD_NONE); // 2ship: 1-arg (shield-only) in MM
            break;
        default:
            break;
    }
}

void ExtEquip_RemoveItem(s16 equipType, u8 index) {
    if (index == 0 || index > 3 || equipType < 0 || equipType > 3)
        return;
    Nei_Save()->extEquipOwnedBits &= ~ExtEquip_GetBit(equipType, index); // Skijer's NEI
    // If currently equipped, unequip and clear vanilla base
    if (ExtEquip_GetCurrent(equipType) == index) {
        ExtEquip_Unequip(equipType);
        ExtEquip_ClearVanillaEquip(equipType);
    }
}

void ExtEquip_Equip(s16 equipType, u8 index) {
    if (index == 0 || index > 3)
        return;

    // Freed/retired slots can never be equipped (reserved for the new boots). Skijer 2026-07-16
    if (ExtEquip_SlotRetired(equipType, index))
        return;

    // Pikachu cannot use extended equipment
    if (TransformMasks_IsTransformedAny() && MmForm_GetCurrentForm() == MM_PLAYER_FORM_PIKACHU)
        return;

    // Must own the item to equip it
    if (!ExtEquip_HasItem(equipType, index))
        return;

    // Age restriction
    if (!ExtEquip_CheckAgeReq(equipType, index))
        return;

    // The MM equipment sub-page is a core inventory page and remains reachable even
    // when the legacy cheat toggle is off. Equipping an owned piece is therefore the
    // authoritative opt-in: enable its behavior/draw path as part of the equip action.
    if (!ExtEquip_IsEnabled()) {
        CVarSetInteger(CVAR_EXT_EQUIP_ENABLED, 1);
    }

    // If already equipped, toggle off (unequip)
    u8 current = ExtEquip_GetCurrent(equipType);
    if (current == index) {
        ExtEquip_Unequip(equipType);
        ExtEquip_ClearVanillaEquip(equipType);
        return;
    }

    // Set extended equipment (also syncs to gSaveContext.ship)
    ExtEquip_SetCurrentByType(equipType, index);

    // Set vanilla equipment base for ext equipment
    // Ext swords use Kokiri Sword as base (model + IA), ext shields use Mirror Shield
    switch (equipType) {
        case EQUIP_TYPE_SWORD:
            // Ext swords don't change vanilla sword equipment or B button item.
            // The sword model/IA override is handled by the behavior/draw system.
            // This prevents giving BGS/Kokiri Sword if the player doesn't own them.
            break;
        case EQUIP_TYPE_SHIELD:
            // Shield of Ikana (slot 3) uses Mirror Shield model
            // 2ship: 1-arg Inventory_ChangeEquipment (shield-only) in MM.
            // OoT's Hylian shield == MM's Hero shield.
            if (index == 3) {
                Inventory_ChangeEquipment(EQUIP_VALUE_SHIELD_MIRROR);
            } else {
                Inventory_ChangeEquipment(EQUIP_VALUE_SHIELD_HERO);
            }
            break;
        case EQUIP_TYPE_TUNIC:
            // 2ship: MM's Inventory_ChangeEquipment(s16) only touches the shield; write the
            // tunic nibble directly (EQUIP_TYPE_TUNIC is an OoT remnant in MM, harmless bit).
            SET_EQUIP_VALUE(EQUIP_TYPE_TUNIC, EQUIP_VALUE_TUNIC_KOKIRI);
            break;
        case EQUIP_TYPE_BOOTS:
            // Ext boots are accessories — don't change vanilla boots
            break;
    }
}

void ExtEquip_Unequip(s16 equipType) {
    // Restore sword state if Byrna was active
    if (equipType == EQUIP_TYPE_SWORD && gExtEquipBehavior.byrnaActive) {
        Byrna_Cleanup();
    }

    ExtEquip_SetCurrentByType(equipType, 0);
    // NOTE: vanilla equipment is NOT cleared here — callers that need it
    // (toggle-off, remove) call ExtEquip_ClearVanillaEquip separately.
    // The vanilla equip path (z_kaleido_equipment.c) calls ExtEquip_Unequip
    // after already setting vanilla equipment, so clearing here would undo it.
}

// ---------------------------------------------------------------------------
// Transform integration
// ---------------------------------------------------------------------------

void ExtEquip_UnequipForTransform(void) {
    if (!ExtEquip_IsEnabled())
        return;
    if (sTransformBackupValid)
        return; // Already backed up (form-to-form switch)

    sTransformBackup[EQUIP_TYPE_SWORD] = gExtEquipState.currentExtSword;
    sTransformBackup[EQUIP_TYPE_SHIELD] = gExtEquipState.currentExtShield;
    sTransformBackup[EQUIP_TYPE_TUNIC] = gExtEquipState.currentExtTunic;
    sTransformBackup[EQUIP_TYPE_BOOTS] = gExtEquipState.currentExtBoots;
    sTransformBackupValid = 1;

    for (s16 t = EQUIP_TYPE_SWORD; t <= EQUIP_TYPE_BOOTS; t++) {
        if (ExtEquip_GetCurrent(t) != 0) {
            ExtEquip_Unequip(t);
        }
    }
}

void ExtEquip_RestoreFromTransform(void) {
    if (!sTransformBackupValid)
        return;
    if (!ExtEquip_IsEnabled()) {
        sTransformBackupValid = 0;
        return;
    }

    for (s16 t = EQUIP_TYPE_SWORD; t <= EQUIP_TYPE_BOOTS; t++) {
        if (sTransformBackup[t] != 0 && ExtEquip_HasItem(t, sTransformBackup[t])) {
            ExtEquip_Equip(t, sTransformBackup[t]);
        }
    }
    sTransformBackupValid = 0;
}

void ExtEquip_ClearTransformBackup(void) {
    sTransformBackupValid = 0;
    memset(sTransformBackup, 0, sizeof(sTransformBackup));
}

void ExtEquip_ToggleFromCButton(u16 itemId) {
    if (itemId < ITEM_EXT_SWORD_1 || itemId > ITEM_EXT_BOOTS_3)
        return;

    // Pikachu cannot use extended equipment
    if (TransformMasks_IsTransformedAny() && MmForm_GetCurrentForm() == MM_PLAYER_FORM_PIKACHU)
        return;

    // Map itemId to equipType + index
    // ITEM_EXT_SWORD_1=0xE0, _2=0xE1, _3=0xE2
    // ITEM_EXT_SHIELD_1=0xE3, _2=0xE4, _3=0xE5
    // ITEM_EXT_TUNIC_1=0xE6, _2=0xE7, _3=0xE8
    // ITEM_EXT_BOOTS_1=0xE9, _2=0xEA, _3=0xEB
    u16 offset = itemId - ITEM_EXT_SWORD_1; // 0-11
    s16 equipType = offset / 3;             // 0=sword, 1=shield, 2=tunic, 3=boots
    u8 index = (offset % 3) + 1;            // 1-3

    // Age restriction (allow unequip even if age fails — player can always remove)
    u8 current = ExtEquip_GetCurrent(equipType);
    if (current != index && !ExtEquip_CheckAgeReq(equipType, index)) {
        Audio_PlaySoundGeneral(NA_SE_SY_ERROR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        return;
    }

    // Toggle: if already equipped with this index, unequip; otherwise equip
    if (current == index) {
        ExtEquip_Unequip(equipType);
        ExtEquip_ClearVanillaEquip(equipType);
        Audio_PlaySoundGeneral(NA_SE_IT_SHIELD_REMOVE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    } else {
        ExtEquip_Equip(equipType, index);
        Audio_PlaySoundGeneral(NA_SE_SY_DECIDE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    }
}

u8 ExtEquip_GetCurrent(s16 equipType) {
    switch (equipType) {
        case EQUIP_TYPE_SWORD:
            return gExtEquipState.currentExtSword;
        case EQUIP_TYPE_SHIELD:
            return gExtEquipState.currentExtShield;
        case EQUIP_TYPE_TUNIC:
            return gExtEquipState.currentExtTunic;
        case EQUIP_TYPE_BOOTS:
            return gExtEquipState.currentExtBoots;
        default:
            return 0;
    }
}

// ---------------------------------------------------------------------------
// Icons / Names
// ---------------------------------------------------------------------------

// Icon lookup table: [type][index-1] = OTR path string.
// Skijer 2026-07-29 (kaleido re-layout) — page 2 is now:
//   swords  Cane of Byrna (dummy, behavior moved to the GFS line) / Four Sword / Trident
//   shields Goddess Shield / Kite Shield / Shield of Ikana (MM mirror shield)
//   tunics  Champion's / Magic Tunic / Sage's (white)
//   boots   Pegasus Boots / Climb Boots / Roc Boots
static const char* sExtEquipIconPaths[4][3] = {
    // Swords
    { dgItemIconCaneOfByrnaTex, dgItemIconFourSwordTex, dgItemIconTridentTex },
    // Shields
    { dgItemIconGoddessShieldTex, dgItemIconKiteShieldTex,
      "__OTR__icon_item_static_yar/gItemIconMirrorShieldTex" }, // Shield of Ikana (MM mirror shield)
    // Tunics
    { dgItemIconChampionsTunicTex, dgItemIconMagicTunicTex, dgItemIconSagesTunicTex },
    // Boots
    { dgItemIconPegasusBootsTex, dgItemIconClimbBootsTex, dgItemIconRocBootsTex },
};

// Left-column passives (Magic Cape / Pendant of Memories) — they left the ext grid, so their kaleido
// icons come from here, NOT ExtEquip_GetIcon(grid). Mirror of the soh getters.
void* ExtEquip_GetCapeIcon(void) {
    return (void*)dgItemIconMagicCapeTex;
}
void* ExtEquip_GetPendantIcon(void) {
    return (void*)"__OTR__icon_item_static_yar/gItemIconPendantOfMemoriesTex";
}

void* ExtEquip_GetIcon(s16 equipType, u8 index) {
    if (equipType < 0 || equipType >= 4 || index < 1 || index > 3) {
        return NULL;
    }

    return (void*)sExtEquipIconPaths[equipType][index - 1];
}

u16 ExtEquip_GetItemId(s16 equipType, u8 index) {
    // Map (type, index) to ITEM_EXT_xxx
    // type 0 (sword): 0xE0 + (index-1)
    // type 1 (shield): 0xE3 + (index-1)
    // type 2 (tunic): 0xE6 + (index-1)
    // type 3 (boots): 0xE9 + (index-1)
    if (index < 1 || index > 3 || equipType < 0 || equipType >= 4) {
        return 0;
    }
    return ITEM_EXT_SWORD_1 + (equipType * 3) + (index - 1);
}

// Set by the equipment kaleido while it is naming a PAGE-2 GRID cell (same idiom as
// gExtEquipSuppressIconOverride). Only ITEM_EXT_BOOTS_2 (0xEA) is ambiguous: as an inventory/trade-wheel
// id it is the Pendant of Memories, as a grid slot it is the Climb Boots. Skijer 2026-07-29
u8 gExtEquipGridNameContext = 0;

void* ExtEquip_GetNameTex(u16 itemId, u8 language) {
    return ExtEquip_LookupNameTex(itemId, language);
}

// ---------------------------------------------------------------------------
// Behavior
// ---------------------------------------------------------------------------

ExtEquipBehaviorState gExtEquipBehavior;

void ExtEquip_UpdateBehavior(void* playerVoid, void* playVoid) {
    Player* player = (Player*)playerVoid;
    PlayState* play = (PlayState*)playVoid;

    // NEI weapon upgrades are NOT extended equipment — they run whenever the upgrade is owned,
    // regardless of the ext-equipment cheat. Gate on the local player (read global save state +
    // local input for the throw).
    if (gPlayState == NULL || player == GET_PLAYER(gPlayState)) {
        if (WeaponUpgrade_HasHammerAxe()) {
            IKAxe_Behavior(player, play);
        } else {
            IKAxe_Cleanup();
        }
        if (WeaponUpgrade_HasGreatFairy()) {
            GreatFairySword_Behavior(player, play);
        }
        // OoT vanilla tunics/boots (Goron fireproof / Iron anti-knockback / Hover float) — run
        // whenever the piece is equipped, independent of the ext-equipment cheat. Skijer's NEI
        VanillaTB_Behavior(player, play);

        // Upgrade-column passives (Skijer 2026-07-16) — OWNERSHIP-based, independent of the
        // ext-equipment cheat (they no longer live in the ext grid): Magic Cape cloth while visible
        // (the half-cost passive is at the MAGIC_REQ cost sites), Pendant moveset while toggled ON.
        if (ExtEquip_CapeVisible()) {
            MagicCape_Behavior(player, play);
        }
        MagicCape_Cleanup();
        if (ExtEquip_PendantActive()) {
            Pendant_Behavior(player, play);
        } else {
            Pendant_Reset();
        }
    }

    if (!ExtEquip_IsEnabled()) {
        Champion_Cleanup(play);
        return;
    }

    ExtEquip_DispatchBehavior(player, play);
}

void ExtEquip_OnMeleeHit(void* playerVoid, void* playVoid) {
    Player* player = (Player*)playerVoid;
    PlayState* play = (PlayState*)playVoid;

    // Great Fairy's Sword recovers HP+MP on hit, independent of the ext-equipment cheat.
    if (WeaponUpgrade_HasGreatFairy() && player->heldItemAction == PLAYER_IA_SWORD_BIGGORON) {
        GreatFairySword_OnMeleeHit(player, play);
    }

    if (!ExtEquip_IsEnabled())
        return;

    ExtEquip_OnMeleeHitDispatch(player, play);
}

void ExtEquip_DrawBehavior(void* playerVoid, void* playVoid) {
    Player* player = (Player*)playerVoid;
    PlayState* play = (PlayState*)playVoid;

    // Skip remote dummy players. HarpoonDummyPlayer_Draw delegates to
    // Player_Draw for skeleton/anim parity, which routes here. But these draws
    // read GLOBAL state (the LOCAL player's slots / save) — drawing Four Sword
    // clones / Pegasus wind cone / Magic Cape / IK Axe reticle / Water-Dragon
    // barrier on remote dummies would render the local player's effects on every
    // peer's body. Gate on "this player is the local player actor".
    if (gPlayState != NULL) {
        Player* localPlayer = GET_PLAYER(gPlayState);
        if (player != localPlayer) {
            return;
        }
    }

    // Hammer upgrade (Iron Knuckle's Axe) aim reticle is drawn from BowReticle.cpp (crash-free),
    // not here — the old FirstPerson_DrawReticle path NaN-crashed for the melee hammer.

    // Magic Cape cloth: ownership-based upgrade-column passive, independent of the ext cheat
    // (Skijer 2026-07-16). Requires the companion oot.o2r for the gMant* assets.
    if (ExtEquip_CapeVisible()) {
        MagicCape_Draw(player, play);
    }

    if (!ExtEquip_IsEnabled())
        return;

    ExtEquip_DrawDispatch(player, play);
}

void ExtEquip_DrawSwordDL(void* playVoid) {
    PlayState* play = (PlayState*)playVoid;

    // Hammer upgrade: draw the Iron Knuckle's Axe in place of the hammer DL.
    // IKAxe_DrawAxe self-guards on heldItemAction == HAMMER / throw state, and the
    // hammer DL itself is hidden via ExtEquip_ShouldHideSwordDL. Independent of cheat.
    if (WeaponUpgrade_HasHammerAxe()) {
        IKAxe_DrawAxe(play);
    }

    if (gExtEquipState.currentExtSword == 1) {
        // Byrna: draw blue cane DL only when sword is held (not sheathed)
        Player* drawPlayer = GET_PLAYER(play);
        if (Player_GetMeleeWeaponHeld(drawPlayer) != 0) {
            Gfx* byrnaDL = Byrna_GetCaneDL();
            if (byrnaDL != NULL) {
                OPEN_DISPS(play->state.gfxCtx);
                gSPDisplayList(POLY_OPA_DISP++, byrnaDL);
                CLOSE_DISPS(play->state.gfxCtx);
            }
        }
    }
}

// item_cane_of_somaria.c — the Dual Cane borrows the two-handed BGS model group for
// its stance, so the sword DL that group normally draws has to be suppressed.
u8 Cane_IsActive(void);

// mods/extended_player.h defines this, but pulling that header in here just for one
// constant drags the whole custom-item action table with it. Guarded so the real
// definition always wins if the include order ever changes.
#ifndef PLAYER_IA_NET
#define PLAYER_IA_NET 0x7E
#endif

u8 ExtEquip_ShouldHideSwordDL(void) {
    // Dual Cane: the BGS model group is used for the two-handed POSE only. Same
    // arrangement as the Cane of Byrna below, which also swaps the blade for its
    // own model. Checked before the ext-equipment gate because the cane is a
    // page-2 custom item, not ext equipment.
    if (Cane_IsActive()) {
        return 1;
    }

    // Net: it borrows Player_UpperAction_Sword so that drawing another item can take
    // it out of Link's hands, but that action makes the engine draw the equipped
    // BLADE too — which read as pulling the sword out on top of the net. The net has
    // its own model, so the sword's is suppressed exactly like the cane's.
    if (gPlayState != NULL) {
        Player* netPlayer = GET_PLAYER(gPlayState);

        if ((netPlayer != NULL) && (netPlayer->heldItemAction == PLAYER_IA_NET)) {
            return 1;
        }
    }

    // Hammer upgrade: hide the hammer DL only while the axe is actually being drawn
    // (in free mode / putaway, don't hide — vanilla shows the open hand). Independent
    // of the ext-equipment cheat.
    if (WeaponUpgrade_HasHammerAxe() && gExtEquipBehavior.ikAxeDrawing)
        return 1;

    if (!ExtEquip_IsEnabled())
        return 0;

    // Cane of Byrna replaces the sword model with its own draw
    if (gExtEquipState.currentExtSword == 1)
        return 1;

    return 0;
}

const char* ExtEquip_GetShieldDLOverride(void) {
    if (!ExtEquip_IsEnabled())
        return NULL;

    // Divine (1), Kite (2), Shield of Ikana (3): hide OOT shield, draw custom in PostLimbDraw
    if (gExtEquipState.currentExtShield >= 1 && gExtEquipState.currentExtShield <= 3)
        return "HIDE";

    return NULL;
}

// Deku shield skin (vanilla equipment page): equipped AS Hero but should draw OoT's Deku shield.
// Unlike the ext shields (Divine/Kite/Ikana, which draw a separate model over an open hand via a
// custom transform), the Deku uses OoT's correctly-oriented CHILD combined hand+shield / shield+
// sheath DLs directly (a plain hand-DL swap in z_player_lib.c) — no custom transform, no rotation
// issues. Cheat-independent. Skijer's NEI
u8 ExtEquip_IsDekuSkinActive(void) {
    return Nei_Save()->vanillaShieldSkin == 1;
}

// OoT Mirror shield skin (vanilla page col3): equipped as the MM Mirror value (2) but draws OoT's
// adult mirror shield with a child hand (z_player_lib.c scan-patches the adult fist → child fist).
// Distinct from the MM Mirror (= Shield of Ikana on the ext page). Skijer's NEI
u8 ExtEquip_IsOotMirrorSkinActive(void) {
    return Nei_Save()->vanillaShieldSkin == 2;
}

// Cached MM Mirror Shield DLs (loaded once from mm.o2r with hash pre-resolution)
static Gfx* sCachedMmShieldHandDL = NULL;
static Gfx* sCachedMmShieldBackDL = NULL;
static u8 sMmShieldLoadAttempted = 0;

static void ExtEquip_LoadMmShieldDLs(void) {
    if (sMmShieldLoadAttempted)
        return;
    sMmShieldLoadAttempted = 1;

    sCachedMmShieldHandDL =
        (Gfx*)TransformMasks_LoadMmDL("objects/object_link_child/gLinkHumanRightHandHoldingMirrorShieldDL");
    // Use the plain shield DL (no embedded matrix) for back — we control the transform
    sCachedMmShieldBackDL = (Gfx*)TransformMasks_LoadMmDL("objects/object_link_child/gLinkHumanMirrorShieldDL");
}

// Shared draw for the cached MM Mirror Shield DLs (hand + back differ only in
// which cached DL is passed). Drawn on XLU to avoid corrupting the OPA pipeline
// (prevents black tint on the tunic).
static void DrawCachedShieldDL(void* playVoid, Gfx* dl) {
    PlayState* play = (PlayState*)playVoid;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Push();
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, dl);
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

// Custom soh.o2r shield models (brought in from kite_shield.blend via the
// blend_to_nei -> c2obj_nei pipeline). Cached gated loads.
//   slot 1 = Divine Shield (object_nei_divine_shield)
//   slot 2 = Kite Shield    (object_nei_kite_shield)
static Gfx* ExtEquip_GetCachedDL(const char* otr, Gfx** cache, u8* tried) {
    if (!*tried) {
        *tried = 1;
        if (ResourceMgr_FileExists(otr)) {
            *cache = ResourceMgr_LoadGfxByName(otr);
        }
    }
    return *cache;
}

static Gfx* ExtEquip_GetKiteShieldDL(void) {
    static Gfx* sCached = NULL;
    static u8 sTried = 0;
    return ExtEquip_GetCachedDL("__OTR__objects/object_nei_kite_shield/g_kite_shield_dl", &sCached, &sTried);
}

static Gfx* ExtEquip_GetDivineShieldDL(void) {
    static Gfx* sCached = NULL;
    static u8 sTried = 0;
    return ExtEquip_GetCachedDL("__OTR__objects/object_nei_divine_shield/g_divine_shield_dl", &sCached, &sTried);
}

// Shared transform that seats a custom shield model in Link's shield-limb space.
// The model is drawn relative to the sheath/hand limb matrix, whose LOCAL space is
// huge (~6000 N64 units across — the Hylian shield collider quad size in z_player_lib.c).
// Divine + Kite share this placement (both modeled in the same space).
// Final, visually-tuned values (degrees for rotation, N64 units for offset).
#define CUSTOM_SHIELD_SCALE  44.2f
#define CUSTOM_SHIELD_ROT_X  (-95.0f * (M_PI / 180.0f))
#define CUSTOM_SHIELD_ROT_Y  (-27.0f * (M_PI / 180.0f))
#define CUSTOM_SHIELD_ROT_Z  (-99.0f * (M_PI / 180.0f))
#define CUSTOM_SHIELD_OFF_X  (-508.0f)
#define CUSTOM_SHIELD_OFF_Y  (-372.0f)
#define CUSTOM_SHIELD_OFF_Z  (-5.0f)

// Shared custom-equipment draw template (see EquipDrawModel in extended_equipment.h). Collapses the
// open/push/setup/color/TRS/load/draw/pop/close boilerplate that every ext-equipment DL repeated.
void ExtEquip_DrawModel(void* playVoid, const EquipDrawModel* m) {
    PlayState* play = (PlayState*)playVoid;
    Gfx* gfx;

    if (m->dl == NULL) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    gfx = m->xlu ? POLY_XLU_DISP : POLY_OPA_DISP;

    if (m->setupDL) {
        gfx = Gfx_SetupDL(gfx, SETUPDL_25); // material-25 setup (MM idiom); 0 = the DL brings its own
    }

    Matrix_Push();
    Matrix_Translate(m->translate.x, m->translate.y, m->translate.z, MTXMODE_APPLY);
    Matrix_RotateXF(m->rotate.x, MTXMODE_APPLY);
    Matrix_RotateYF(m->rotate.y, MTXMODE_APPLY);
    Matrix_RotateZF(m->rotate.z, MTXMODE_APPLY);
    Matrix_Scale(m->scale.x, m->scale.y, m->scale.z, MTXMODE_APPLY);

    if (m->setColor) {
        gDPSetPrimColor(gfx++, 0, 0, m->prim.r, m->prim.g, m->prim.b, m->prim.a);
        gDPSetEnvColor(gfx++, m->env.r, m->env.g, m->env.b, m->env.a);
    }
    gSPMatrix(gfx++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(gfx++, m->dl);

    Matrix_Pop();

    if (m->xlu) {
        POLY_XLU_DISP = gfx;
    } else {
        POLY_OPA_DISP = gfx;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

static void DrawCustomShieldDL(void* playVoid, Gfx* dl) {
    // Drawn on XLU (like the MM Ikana shield): a custom DL leaves its combiner/texture state on the
    // pipe; on OPA that bleeds onto the limbs drawn after it (black tunic). The XLU pass runs after all
    // OPA limbs, so the body stays clean. The DL supplies its own render state, so setupDL = 0.
    EquipDrawModel m = {
        .dl = dl,
        .xlu = 1,
        .setupDL = 0,
        .translate = { CUSTOM_SHIELD_OFF_X, CUSTOM_SHIELD_OFF_Y, CUSTOM_SHIELD_OFF_Z },
        .rotate = { CUSTOM_SHIELD_ROT_X, CUSTOM_SHIELD_ROT_Y, CUSTOM_SHIELD_ROT_Z },
        .scale = { CUSTOM_SHIELD_SCALE, CUSTOM_SHIELD_SCALE, CUSTOM_SHIELD_SCALE },
        .setColor = 0,
    };

    ExtEquip_DrawModel(playVoid, &m);
}

// Hand + back draws share the per-slot model dispatch (onBack picks hand vs sheath
// DL for the MM Mirror Shield; the custom models use one DL for both).
static void ExtEquip_DrawShieldCommon(void* playVoid, u8 onBack) {
    if (!ExtEquip_IsEnabled())
        return;

    switch (gExtEquipState.currentExtShield) {
        case 1: // Divine Shield: custom soh.o2r model
            DrawCustomShieldDL(playVoid, ExtEquip_GetDivineShieldDL());
            break;
        case 2: // Kite Shield: custom soh.o2r model
            DrawCustomShieldDL(playVoid, ExtEquip_GetKiteShieldDL());
            break;
        case 3: { // Shield of Ikana: MM Mirror Shield from mm.o2r
            ExtEquip_LoadMmShieldDLs();
            Gfx* mmDL = onBack ? sCachedMmShieldBackDL : sCachedMmShieldHandDL;
            if (mmDL != NULL)
                DrawCachedShieldDL(playVoid, mmDL);
            break;
        }
    }
}

void ExtEquip_DrawShieldDL(void* playVoid) {
    ExtEquip_DrawShieldCommon(playVoid, 0);
}

// Draw the ext shield on Link's back (sheath position)
void ExtEquip_DrawShieldBackDL(void* playVoid) {
    ExtEquip_DrawShieldCommon(playVoid, 1);
}

// Common prologue for the per-piece dispatch wrappers below: bail out unless
// the cheat is enabled AND the given slot is currently equipped with `index`.
// (ExtEquip_GetCurrent returns the same field these used to read directly.)
#define EXT_EQUIP_REQUIRE(type, index)                                      \
    if (!ExtEquip_IsEnabled() || ExtEquip_GetCurrent(type) != (index))      \
    return

void ExtEquip_DrawWaistScale(void* playVoid) {
    EXT_EQUIP_REQUIRE(EQUIP_TYPE_BOOTS, 3);

    PlayState* play = (PlayState*)playVoid;
    DScale_DrawWaistScale(play);
}

void ExtEquip_DrawAnklet(void* playVoid, s32 isRightFoot) {
    EXT_EQUIP_REQUIRE(EQUIP_TYPE_BOOTS, 1);

    PlayState* play = (PlayState*)playVoid;
    Pegasus_DrawAnklet(play, isRightFoot);
}

void ExtEquip_UpdateAnkletPhysics(void* playerVoid) {
    EXT_EQUIP_REQUIRE(EQUIP_TYPE_BOOTS, 1);

    Player* player = (Player*)playerVoid;
    Pegasus_UpdateWingPhysics(player);
}

// ExtEquip_CaptureCapeShoulderPos removed (Skijer 2026-07-16): the cape anchors on
// player->bodyPartsPos natively; no PostLimbDraw capture needed.

void ExtEquip_DrawBreastplate(void* playVoid) {
    EXT_EQUIP_REQUIRE(EQUIP_TYPE_TUNIC, 2);

    PlayState* play = (PlayState*)playVoid;
    Breastplate_Draw(play);
}

u8 ExtEquip_IkanaDeathSave(void* playVoid) {
    if (!Ikana_ShouldRevive())
        return 0;

    PlayState* play = (PlayState*)playVoid;
    Ikana_ConsumeDeathSave(play);
    return 1;
}
