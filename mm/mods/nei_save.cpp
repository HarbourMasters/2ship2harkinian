// Skijer's NEI — per-save custom state.
//
// 2ship port: this state now lives INSIDE 2ship's per-save struct at
// `gSaveContext.save.shipSaveInfo.nei` (NeiSaveData), and is serialized together
// with the rest of ShipSaveInfo by `2s2h/BenJsonConversions.hpp`. This replaces
// SoH's `SaveManager` "nei" section + the SaveData/LoadData calls, which 2ship
// does not have (2ship persists custom per-save state as ShipSaveInfo fields).
//
// Deferred to the FleetShipCombo phase: the cross-game pictograph/trade `.bin`
// sidecars (Picto_SyncWrite/Read, TradeItems_SyncWrite/Read) — re-add against
// MM-native picto/trade state when those subsystems are adapted.
#include <string.h>

extern "C" {
#include "z64save.h" // SaveContext, Save.shipSaveInfo.nei (pulls in nei_save.h)
}

extern "C" SaveContext gSaveContext;

extern "C" NeiSaveData* Nei_Save(void) {
    return &gSaveContext.save.shipSaveInfo.nei;
}

extern "C" uint8_t Nei_GetOwnedItem(uint8_t slot) {
    if (slot >= 24 && slot < 72) {
        return gSaveContext.save.shipSaveInfo.nei.ownedItems[slot - 24];
    }
    return 0xFF; // ITEM_NONE
}

extern "C" void Nei_SetOwnedItem(uint8_t slot, uint8_t v) {
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
    memset(n->ownedItems, 0xFF, sizeof(n->ownedItems));
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
    nei->ootHammerOwned = 1;    // Megaton Hammer
    nei->ootHookshotLevel = 3;  // Ultrashot (1 Hookshot / 2 Longshot / 3 Ultrashot)
    nei->clawshotOwned = 1;     // Clawshot (separate selectable variant on the hookshot cell)
    // Enable the existing L-tap clawshot toggle so Clawshot is selectable right away (the L-tap in
    // custom_items_common.c gates on TwilightUpgrade_HasClawshot). Bloque 5 will drive selection
    // from clawshotOwned directly; until then this lets the enemy-pull be tested via give-all.
    nei->twilightUpgrade |= 0x1; // TWILIGHT_UPGRADE_CLAWSHOT
    nei->ootMasksOwned = 0xFFFF;
    // Bullet bag capped at its real max level 3 (gUpgradeCapacities rows have 4 entries — a raw 7
    // read out of bounds); other upgrade fields keep the legacy "everything maxed" 7s.
    nei->ootUpgrades = (3 << 0) | (7 << 3) | (7 << 6) | (7 << 9) | (7 << 12);
    nei->slingshotSeeds = 50; // full 50-seed bag (OoT ITEM_BULLET_BAG_50 give)
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
