// Skijer's NEI — per-save custom state.
//
// 2ship port: this state now lives INSIDE 2ship's per-save struct at
// `gSaveContext.save.shipSaveInfo.nei` (NeiSaveData), and is serialized together
// with the rest of ShipSaveInfo by `2s2h/BenJsonConversions.hpp`. This replaces
// SoH's `SaveManager` "nei" section + the SaveData/LoadData calls, which 2ship
// does not have (2ship persists custom per-save state as ShipSaveInfo fields).
//
// Deferred to the FleetShipCombo phase: the cross-game trade `.bin` sidecar
// (TradeItems_SyncWrite/Read). The pictograph one is gone for good — MM owns the
// Pictograph Box natively, so a future photo bridge reads/writes
// gSaveContext.pictoPhotoI5 + save.saveInfo.pictoFlags0/1 directly.
#include <string.h>
#include <libultraship/bridge/consolevariablebridge.h> // CVar: gItemEditor.CaneWheelMode (cane wheel shape)

extern "C" {
#include "z64save.h" // SaveContext, Save.shipSaveInfo.nei (pulls in nei_save.h)
}

extern "C" SaveContext gSaveContext;

extern "C" NeiSaveData* Nei_Save(void) {
    return &gSaveContext.save.shipSaveInfo.nei;
}

extern "C" uint16_t Nei_GetOwnedItem(uint8_t slot) {
    if (slot >= 24 && slot < 72) {
        return gSaveContext.save.shipSaveInfo.nei.ownedItems[slot - 24];
    }
    return 0xFF; // ITEM_NONE
}

extern "C" void Nei_SetOwnedItem(uint8_t slot, uint16_t v) {
    if (slot >= 24 && slot < 72) {
        gSaveContext.save.shipSaveInfo.nei.ownedItems[slot - 24] = v;
    }
}

// Empty custom slots must be 0xFF (ITEM_NONE), not 0 (= ITEM_STICK), so a freshly
// zeroed save needs this normalization. TODO(port): call from 2ship's new-file
// init path; from_json also applies these defaults when the "nei" key is absent.
extern "C" void Nei_InitNewSave(void) {
    NeiSaveData* n = &gSaveContext.save.shipSaveInfo.nei;
    memset(n, 0, sizeof(*n));
    // ownedItems is u16 now, so memset(0xFF) would write 0xFFFF per entry — and the empty marker is
    // ITEM_NONE (0xFF), not 0xFFFF. Fill it element by element. Skijer's NEI
    for (int i = 0; i < (int)(sizeof(n->ownedItems) / sizeof(n->ownedItems[0])); i++) {
        n->ownedItems[i] = 0xFF;
    }
    memset(n->bottleSlots, 0xFF, sizeof(n->bottleSlots));
    n->bottomlessContent = 0xFF;
}

// Skijer's NEI (MM port): grant every OoT page-0 item — spells, slingshot (+seeds), OoT
// boomerang, the full hookshot chain, all OoT child-trade masks, and max OoT upgrades.
// Used by the Save Editor's "Give All OoT Items" button.
extern "C" void WeaponUpgrade_SetHammerAxe(uint8_t on);

extern "C" void Nei_GiveAllOotItems(void) {
    NeiSaveData* nei = Nei_Save();
    nei->ootSpellsOwned = 0x7; // Din's + Farore's + Nayru's
    nei->slingshotOwned = 1;
    nei->ootBoomerangOwned = 1;
    nei->ootHammerOwned = 1;   // Megaton Hammer
    nei->ootHookshotLevel = 3; // Ultrashot (1 Hookshot / 2 Longshot / 3 Ultrashot)
    nei->clawshotOwned = 1;    // Clawshot (separate selectable variant on the hookshot cell)
    // Enable the existing L-tap clawshot toggle so Clawshot is selectable right away (the L-tap in
    // custom_items_common.c gates on TwilightUpgrade_HasClawshot). Bloque 5 will drive selection
    // from clawshotOwned directly; until then this lets the enemy-pull be tested via give-all.
    nei->twilightUpgrade |= 0x1; // TWILIGHT_UPGRADE_CLAWSHOT
    nei->ootMasksOwned = 0xFFFF;
    // Bullet bag capped at its real max level 3 (gUpgradeCapacities rows have 4 entries — a raw 7
    // read out of bounds); other upgrade fields keep the legacy "everything maxed" 7s.
    nei->ootUpgrades = (3 << 0) | (7 << 3) | (7 << 6) | (7 << 9) | (7 << 12);
    nei->slingshotSeeds = 50;      // full 50-seed bag (OoT ITEM_BULLET_BAG_50 give)
    WeaponUpgrade_SetHammerAxe(1); // + its upgrade (Iron Knuckle's Axe behavior)

    // OoT quest-status page: grant the whole collect_register so the OoT quest page (L-flip)
    // shows everything. 6 medallions (0-5) + 12 songs (6-17) + 3 spiritual stones (18-20) +
    // Stone of Agony (21) + Gerudo Card (22) + the GS token bit (23); GS count maxed at 100.
    nei->ootQuestItems = 0x00FFFFFF; // bits 0..23 set
    nei->ootGsCount = 100;
}

// ─── Skijer's NEI slingshot pass: bullet-bag helpers (C-callable) ─────────────────────────────
// OoT gUpgradeCapacities[UPG_BULLET_BAG] = { 0, 30, 40, 50 } (soh z_inventory.c:27-33).
static const uint16_t sNeiBulletBagCapacities[4] = { 0, 30, 40, 50 };

extern "C" uint8_t Nei_BulletBagLevel(void) {
    uint16_t level = Nei_Save()->ootUpgrades & 0x7; // bulletBag = bits 0-2
    return (level > 3) ? 3 : (uint8_t)level;        // clamp legacy give-all 7s
}

extern "C" uint8_t Nei_SlingshotCapacity(void) {
    return (uint8_t)sNeiBulletBagCapacities[Nei_BulletBagLevel()];
}

extern "C" uint8_t Nei_SlingshotSeeds(void) {
    return Nei_Save()->slingshotSeeds;
}

// ─── OoT strength upgrade (Skijer's NEI) ──────────────────────────────────────────────────────
// ootUpgrades bits 9-11, the same store FleetSync exports as upgrades["strength"] and the kaleido
// equipment page reads. Clamped on read: Nei_GiveAllOotItems writes the legacy "everything maxed"
// 7 into every 3-bit field, and both the icon table and OoT's UPG_STRENGTH only go up to 3.
extern "C" uint8_t Nei_StrengthLevel(void) {
    uint16_t level = (Nei_Save()->ootUpgrades >> NEI_STRENGTH_SHIFT) & 0x7;
    return (level > NEI_STRENGTH_MAX) ? NEI_STRENGTH_MAX : (uint8_t)level;
}

extern "C" void Nei_SetStrengthLevel(uint8_t level) {
    NeiSaveData* nei = Nei_Save();
    if (level > NEI_STRENGTH_MAX) {
        level = NEI_STRENGTH_MAX;
    }
    nei->ootUpgrades =
        (uint16_t)((nei->ootUpgrades & ~(0x7 << NEI_STRENGTH_SHIFT)) | ((uint16_t)level << NEI_STRENGTH_SHIFT));
}

// Skijer's NEI hookshot overhaul — tiny accessor for TUs that don't pull the NEI headers in
// (z_parameter.c HUD): the OoT hookshot-chain level (1 Hookshot / 2 Longshot / 3 Ultrashot).
extern "C" uint8_t Nei_HookshotLevel(void) {
    return Nei_Save()->ootHookshotLevel;
}

// ─── Skijer's NEI hookshot overhaul: which variant currently fires from the hookshot cell ────────
// Clawshot takes priority when its wheel mode is active AND owned (it's the MM-native model with the
// enemy-pull behavior). Otherwise the OoT chain by ootHookshotLevel: 1 Hookshot, 2 Longshot,
// 3 Ultrashot. Defaults to Longshot for an unset/garbage level.
extern "C" uint8_t Nei_HookshotVariant(void) {
    NeiSaveData* nei = Nei_Save();
    if (nei->clawshotOwned && nei->clawshotModeActive) {
        return NEI_HOOK_VARIANT_CLAWSHOT;
    }
    switch (nei->ootHookshotLevel) {
        case 1:
            return NEI_HOOK_VARIANT_HOOKSHOT;
        case 3:
            return NEI_HOOK_VARIANT_ULTRASHOT;
        case 2:
        default:
            return NEI_HOOK_VARIANT_LONGSHOT;
    }
}

// ─── Skijer's NEI Dual Cane (Somaria / Pacci) ────────────────────────────────────────────────────
// Six separate obtainable skills share ONE kaleido slot. `caneSkills` is the 6-bit ownership mask
// (bits 0..2 Somaria: Statue/Block/Platform, bits 3..5 Pacci: Flip/Stone/Ultrahand). `caneType` and
// `caneSkillSel` are the player's wheel selection, persisted so it survives save/load.
//
// Every getter AUTO-CORRECTS: the skills can be obtained in any order, so the stored selection is
// routinely pointing at something not owned yet (a fresh file starts at type 0 / slot 0 and the
// player's first pickup may well be Pacci-Ultrahand). Correcting on read keeps the wheel, the HUD
// and the cast path from ever disagreeing about what the button does.

// Mirrors item_cane_of_somaria.h — kept local so this TU doesn't pull the item headers in.
#define NEI_CANE_SKILL_MAX 6
#define NEI_CANE_TYPE_MAX 4
#define NEI_CANE_SOMARIA_MASK 0x07
#define NEI_CANE_PACCI_MASK 0x38

static uint8_t Nei_CaneTypeMask(uint8_t type) {
    return (type == 1) ? NEI_CANE_PACCI_MASK : NEI_CANE_SOMARIA_MASK;
}

extern "C" uint8_t Nei_CaneSkillMask(void) {
    return Nei_Save()->caneSkills;
}

extern "C" uint8_t Nei_CaneHasSkill(uint8_t skill) {
    if (skill >= NEI_CANE_SKILL_MAX) {
        return 0;
    }
    return (Nei_Save()->caneSkills & (1 << skill)) ? 1 : 0;
}

extern "C" void Nei_CaneGrantSkill(uint8_t skill) {
    if (skill >= NEI_CANE_SKILL_MAX) {
        return;
    }
    Nei_Save()->caneSkills |= (uint8_t)(1 << skill);
}

extern "C" uint8_t Nei_CaneOwned(void) {
    return Nei_Save()->caneSkills != 0;
}

// Four cane entries, each gated by one skill bit: Somaria(bit0), Trirod(bit2),
// Pacci(bit3), Ultrahand(bit5). Trirod and Ultrahand are ADDITIONAL wheel entries
// at the end of their chain, not replacements for the cane that leads to them.
static uint8_t Nei_CaneTypeGateBit(uint8_t type) {
    switch (type) {
        case 1:
            return 2; // Trirod    <- Somaria L3
        case 2:
            return 3; // Pacci     <- Pacci L1
        case 3:
            return 5; // Ultrahand <- Pacci L3
        case 0:
        default:
            return 0; // Somaria   <- Somaria L1
    }
}

extern "C" uint8_t Nei_CaneTypeOwned(uint8_t type) {
    if (type >= NEI_CANE_TYPE_MAX) {
        return 0;
    }
    return (Nei_Save()->caneSkills & (1 << Nei_CaneTypeGateBit(type))) ? 1 : 0;
}

// ── Wheel configuration (user 2026-08-06): which of the four ride the wheel ──
// The cane cell can carry up to four entries (Somaria 0 / Trirod 1 / Pacci 2 / Ultrahand 3). The
// user wants FOUR wheel shapes, picked by the shared CVar gItemEditor.CaneWheelMode:
//   0 = S-T-P-U (everything owned — today's behaviour)
//   1 = T-U     (end-items only: the base canes leave the wheel once their upgrade exists)
//   2 = S-T-U   (Somaria stays; Pacci hides behind the Ultrahand)
//   3 = T-P-U   (Pacci stays; Somaria hides behind the Trirod)
// A base cane is only ever HIDDEN when its own end-item is owned — hiding the only thing you have
// would brick the cell. Same CVar name on the soh side so the option reads identically. Skijer's NEI
extern "C" uint8_t Nei_CaneTypeVisible(uint8_t type) {
    if (!Nei_CaneTypeOwned(type)) {
        return 0;
    }
    int mode = CVarGetInteger("gItemEditor.CaneWheelMode", 0);
    if (type == 0 && (mode == 1 || mode == 3) && Nei_CaneTypeOwned(1)) {
        return 0; // Somaria hidden behind an owned Trirod
    }
    if (type == 2 && (mode == 1 || mode == 2) && Nei_CaneTypeOwned(3)) {
        return 0; // Pacci hidden behind an owned Ultrahand
    }
    return 1;
}

// How many of the four the wheel actually shows — it only opens past one.
extern "C" uint8_t Nei_CaneTypeCount(void) {
    uint8_t n = 0;
    for (uint8_t i = 0; i < NEI_CANE_TYPE_MAX; i++) {
        n += Nei_CaneTypeVisible(i);
    }
    return n;
}

// Next VISIBLE type in `dir`, wrapping. Returns the current one when nothing else
// is visible, so a single-entry wheel is a no-op rather than a crash.
extern "C" uint8_t Nei_CaneNextType(int8_t dir) {
    uint8_t cur = Nei_CaneGetType();
    for (uint8_t step = 1; step <= NEI_CANE_TYPE_MAX; step++) {
        int16_t probe = (int16_t)cur + (int16_t)(dir >= 0 ? step : -step);
        while (probe < 0) {
            probe += NEI_CANE_TYPE_MAX;
        }
        probe %= NEI_CANE_TYPE_MAX;
        if (Nei_CaneTypeVisible((uint8_t)probe)) {
            return (uint8_t)probe;
        }
    }
    return cur;
}

extern "C" uint8_t Nei_CaneGetType(void) {
    NeiSaveData* nei = Nei_Save();
    uint8_t type = (nei->caneType < NEI_CANE_TYPE_MAX) ? nei->caneType : 0;

    // Visible beats merely owned: switching the wheel shape can hide the stored type, and the cell
    // must follow the wheel or it shows a cane the wheel can no longer reach. Skijer's NEI
    if (Nei_CaneTypeVisible(type)) {
        return type;
    }
    for (uint8_t i = 0; i < NEI_CANE_TYPE_MAX; i++) {
        if (Nei_CaneTypeVisible(i)) {
            return i;
        }
    }
    // Nothing visible (fresh file / pre-split save): fall back to owned, then 0.
    if (Nei_CaneTypeOwned(type)) {
        return type;
    }
    for (uint8_t i = 0; i < NEI_CANE_TYPE_MAX; i++) {
        if (Nei_CaneTypeOwned(i)) {
            return i;
        }
    }
    return 0;
}

extern "C" void Nei_CaneSetType(uint8_t type) {
    if (type < NEI_CANE_TYPE_MAX) {
        Nei_Save()->caneType = type;
    }
}

// ── Sub-selection, which now ONLY the Cane of Somaria has ────────────────────
// These used to assume "2 cane types x 3 skill slots", indexing caneSkillSel[type]
// and computing the skill bit as type*3 + slot. Both broke the moment the wheel
// grew to four entries: caneSkillSel is 2 wide so type 2/3 read and wrote PAST it,
// and type*3 addressed bits 6..11 of a 6-bit mask. That corrupted the active type,
// which is why the wheel showed the wrong icon for an entry.
//
// In the four-entry model only Somaria picks between things (statue / block /
// platform). Trirod, Pacci and Ultrahand each do one thing, so they have no
// sub-selection at all and never touch the array. Slot 0 of caneSkillSel is the
// only one used, so the field's serialized size is unchanged.

// Which of Somaria's three summons are available. Bit 0 (L1) gives the statue;
// bit 1 (L2) gives blocks AND platforms together.
static uint8_t Nei_SomariaSummonOwned(uint8_t slot) {
    uint8_t skills = Nei_Save()->caneSkills;

    switch (slot) {
        case 0:
            return (skills & (1 << 0)) ? 1 : 0; // Statue
        case 1:
        case 2:
            return (skills & (1 << 1)) ? 1 : 0; // Block + Platform, same level
        default:
            return 0;
    }
}

extern "C" uint8_t Nei_CaneGetSkillSlot(uint8_t type) {
    NeiSaveData* nei = Nei_Save();

    if (type != 0) {
        return 0; // only the Cane of Somaria has anything to choose between
    }

    uint8_t slot = nei->caneSkillSel[0];
    if ((slot < 3) && Nei_SomariaSummonOwned(slot)) {
        return slot;
    }
    // Selection points at something not unlocked — snap to the first that is.
    for (uint8_t i = 0; i < 3; i++) {
        if (Nei_SomariaSummonOwned(i)) {
            return i;
        }
    }
    return 0;
}

extern "C" void Nei_CaneSetSkillSlot(uint8_t type, uint8_t slot) {
    if ((type == 0) && (slot < 3) && Nei_SomariaSummonOwned(slot)) {
        Nei_Save()->caneSkillSel[0] = slot;
    }
}

// The CANE_SKILL_* the button would cast right now, derived from WHICH ENTRY is in
// hand — not from arithmetic on the type index.
extern "C" uint8_t Nei_CaneActiveSkill(void) {
    switch (Nei_CaneGetType()) {
        case 1:
            return 6; // Trirod — its own thing; no skill bit yet (behaviour pending)
        case 2:
            return 3; // Cane of Pacci -> Flip (hold adds Lift once that level is owned)
        case 3:
            return 5; // Ultrahand
        case 0:
        default:
            return Nei_CaneGetSkillSlot(0); // Somaria: 0 statue / 1 block / 2 platform
    }
}
