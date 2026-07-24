// Skijer's NEI
#ifndef NEI_SAVE_H
#define NEI_SAVE_H

#include <stdint.h>
#include "2s2h/FleetShipCombo/FleetComboIds.h" // FC_COMBO_OBTAINED_FC_SIZE

#ifdef __cplusplus
extern "C" {
#endif

// Skijer's NEI: per-save state, serialized via the "nei" SaveManager section
// (NOT in the vanilla SaveContext, which is kept 100% upstream).
typedef struct NeiSaveData {
    uint8_t ownedItems[48];      // custom inventory slots 24..71 (page-2 items + MM masks)
    uint32_t extEquipOwnedBits;  // ext-equipment ownership (was inventory.equipment high bits)
    uint8_t lanternFireType;
    uint8_t lanternCapturedTypes;
    uint8_t twilightUpgrade;
    uint8_t clawshotModeActive;
    uint8_t galeBoomerangModeActive;
    uint8_t weaponUpgrades;
    uint8_t extEquipSword;
    uint8_t extEquipShield;
    uint8_t extEquipTunic;
    uint8_t extEquipBoots;
    // Bottle randomizer (Skijer's NEI). The bottle inventory is 8 slots shown in the save editor as a
    // 4x2 grid: "Bottle A" = slots 0-3, "Bottle B" = slots 4-7. Each holds an OoT ITEM_ content id,
    // ITEM_BOTTLE (empty bottle), or 0xFF (empty slot). This is the OoT-side "which content is in
    // which bottle" state; the kaleido Wheel A/B each cycle their 4 slots. (Cross-game sharing reads/
    // writes these on game switch — layered on later via FscShared.) Wheels A/B map to the vanilla
    // SLOT_BOTTLE_1/2; Net + Bottomless take SLOT_BOTTLE_3/4.
    uint8_t bottleSlots[8];       // 0xFF = empty; ITEM_BOTTLE = empty bottle; else a content id
    uint8_t bottomlessBottleMode; // Bottomless Bottle OWNED (SLOT_BOTTLE_4 item). Skijer's NEI
    uint8_t netEquipped;          // Net OWNED (SLOT_BOTTLE_3 item; behavior deferred)
    // Bottomless Bottle "ammo": SLOT_BOTTLE_4 holds a real bottle content, but instead of emptying in
    // one use it has a per-content use-counter. Each empty (drink/sell) decrements bottomlessCount;
    // while >0 the content auto-refills, at 0 it becomes an empty Bottomless Bottle. (Net has none.)
    uint8_t bottomlessContent;    // content id in the Bottomless Bottle, or ITEM_BOTTLE/0xFF when empty
    uint8_t bottomlessCount;      // remaining uses of bottomlessContent (the counter shown on the icon)
    uint8_t powerKegOwned;        // Power Keg owned (granted via menu); shares the Bomb slot via a
                                  // kaleido wheel, USE gated by form + strength (see power_keg.c)
    uint8_t powerKegCount;        // Power Keg "ammo": how many kegs the player carries (its own
                                  // counter; each use consumes 1). Skijer's NEI
    uint8_t powerKegMode;         // keg mode selected on the Bomb slot (kaleido wheel toggle) — persists
                                  // so the slot doesn't revert to bombs on reload. Skijer's NEI
    // MM adult trade-quest items (Skijer's NEI). Bitmask over a NEI trade index: 0-10 = the OoT items
    // (ITEM_POCKET_EGG..ITEM_CLAIM_CHECK), 11 = Moon's Tear, 12-15 = the four Title Deeds, 16 = Room Key,
    // 17 = Letter to Kafei, 18 = Special Delivery to Mama, 19 = Pendant of Memories. The 2D-grid wheel on
    // SLOT_TRADE_ADULT shows every owned entry. The Pendant's bit is set alongside its combat ownership
    // (extEquipOwnedBits, Ext Boots 2) — both flags on grant. See trade_items.c.
    uint32_t tradeAdultOwned;
    // Pictograph Box (Skijer's NEI). Stored in Majora's Mask's EXACT save layout so a 2Ship bridge
    // can consume it: pictoFlags0/1 are the 64 PICTO_VALID_* bits (set by Snap_SetFlag when a mapped
    // OoT actor is validly photographed), pictoPhotoI5 is the last photo compressed to I5 (160x112).
    // OoT itself gives no reward for these — they exist only to be read by MM/2Ship. See snap.h.
    uint8_t pictoboxOwned;          // Pictobox item owned (granted via CVar/menu)
    uint8_t pictoHasPhoto;          // a photo has been kept (gates the "Replace?" warn before capture)
    uint32_t pictoFlags0;           // MM pictoFlags0: PICTO_VALID_* bits 0x00..0x1F
    uint32_t pictoFlags1;           // MM pictoFlags1: PICTO_VALID_* bits 0x20..0x3F
    uint8_t pictoPhotoI5[11200];    // MM PICTO_PHOTO_COMPRESSED_SIZE = (160*112)*5/8 (I5, last photo)

    // --- OoT page-0 items (Skijer's NEI, MM port: the OoT item-pause layout on MM's page 0).
    //     Appended at the END so older shipSaveInfo blobs stay readable. ---
    uint8_t ootSpellsOwned;   // bit0 Din's Fire, bit1 Farore's Wind, bit2 Nayru's Love
    uint8_t slingshotOwned;   // Fairy Slingshot owned
    uint8_t slingshotSeeds;   // Deku Seed ammo
    uint8_t ootBoomerangOwned;
    uint8_t ootHammerOwned;   // Megaton Hammer (L1). Its upgrade (Iron Knuckle's Axe, L2) is the
                              // WEAPON_UPGRADE_HAMMER_AXE bit in `weaponUpgrades`.
    uint8_t ootHookshotLevel; // OoT chain: 0 none, 1 Hookshot, 2 Longshot
    uint8_t ootHookMode;      // hookshot-cell wheel: 0 = Clawshot (MM native, functional), 1 = OoT mode
    uint8_t nayruRocsMode;    // nayru-cell wheel: 0 = Nayru's Love, 1 = Roc's Feather
    uint8_t lensPictoMode;    // lens-cell wheel: 0 = Lens of Truth, 1 = Pictograph Box shown
    uint16_t ootUpgrades;     // 3 bits each: bulletBag(0) quiver(3) bombBag(6) strength(9) scale(12)
    uint16_t ootMasksOwned;   // OoT child-trade masks bitmask (wheel lands in the per-item pass)
    // Slingshot-cell wheel selection (Skijer's NEI slingshot pass): 0 = plain Fairy Slingshot,
    // 1..6 = SW97 elemental bullet fire/ice/light/dark/soul/wind (ITEM_SW97_BULLET_*). Appended
    // at the END so older shipSaveInfo blobs stay readable.
    uint8_t slingshotWheel;

    // --- Farore's Wind warp point (Skijer's NEI spells pass) — OoT's gSaveContext.fw, which MM's
    //     SaveContext lacks. Mirrors OoT FaroresWindData; restored into the transient
    //     gSaveContext.respawn[RESPAWN_MODE_TOP] on load (item_oot_spells.c). Appended at the END
    //     so older shipSaveInfo blobs stay readable. ---
    uint8_t fwSet;          // OoT fw.set (a warp point exists)
    float fwPosX;
    float fwPosY;
    float fwPosZ;
    int16_t fwYaw;
    int16_t fwPlayerParams; // OoT fw.playerParams (0x6FF)
    uint16_t fwEntrance;    // OoT fw.entranceIndex (MM combined entrance value)
    uint8_t fwRoomIndex;
    uint32_t fwTempSwitchFlags;
    uint32_t fwTempCollectFlags;
    // --- Spell identity (Skijer's NEI spell-variants pass): which shield the running
    //     gSaveContext.nayrusLoveTimer belongs to — 1 = Shadow Medallion (black diamond +
    //     Stone-Mask stealth, MagicDark params 0), 0 = vanilla Nayru's Love (OoT blue, params 1).
    //     Latched by MagicDark_Init; OotSpells_OnPlayerInit reads it so a scene load re-spawns
    //     the SAME diamond. Appended at the END so older shipSaveInfo blobs stay readable. ---
    uint8_t neiShadowStealthMode;

    // --- OoT quest-status page (Skijer's NEI kaleido-collect L-flip pass). MM's
    //     gSaveContext.inventory.questItems can't hold OoT's collect_register (its bit layout is
    //     MM's own remains/songs), so the OoT quest page reads a PARALLEL store with OoT's exact
    //     bit layout. Filled by Give-All + the SaveEditor; the OoT collect page reads it via
    //     OOT_CHECK_QUEST_ITEM. Appended at the END so older shipSaveInfo blobs stay readable. ---
    //   ootQuestItems bits (OoT z64save.h QUEST_*):
    //     0..5  medallions (Forest,Fire,Water,Spirit,Shadow,Light)
    //     6..17 songs (Minuet..Prelude, Lullaby..Storms)
    //     18..20 spiritual stones (Kokiri Emerald, Goron Ruby, Zora Sapphire)
    //     21    Stone of Agony
    //     22    Gerudo Membership Card
    //     23    Gold Skulltula token bit (count lives in ootGsCount)
    //     28..31 heart-piece count (unused by MM; kept for layout parity)
    uint32_t ootQuestItems;
    uint16_t ootGsCount; // Gold Skulltula tokens collected (0..100), shown on the OoT quest page

    // --- Hookshot family (Skijer's NEI hookshot overhaul). ootHookshotLevel now goes to 3:
    //     1 = Hookshot (dist 1), 2 = Longshot (dist 2), 3 = Ultrashot (Longshot icon + "Ultrashot"
    //     name + Light-medallion indicator; 4x reach and 2x speed). Clawshot is now its OWN
    //     selectable item on the hookshot cell (MM-native model; pulls non-boss enemies to Link,
    //     no surface grapple) — its ownership is clawshotOwned, its wheel-active flag is the
    //     existing clawshotModeActive. Appended at the END so older shipSaveInfo blobs stay readable. ---
    uint8_t clawshotOwned; // Clawshot owned (separate from the Hookshot/Longshot/Ultrashot chain)

    // --- Fleet Ship Combo (cross-game) fields — bit/index layouts in 2s2h/FleetShipCombo/
    //     FleetComboIds.h. Appended at the END so older shipSaveInfo blobs stay readable. ---
    uint16_t shieldOwned;       // unified 10-shield ownership bitmask (FC_SHIELD_*): MM Hero maps to
                                // FC_SHIELD_HYLIAN, MM Mirror maps to FC_SHIELD_IKANA
    uint8_t comboObtained[128]; // universal cross-game obtained registry (FC_* index; u8 VALUES:
                                // flags 0/1, counters raw). Souls/abilities bridge to randoInf here.
    uint16_t comboTriforce;     // shared Triforce-piece count (syncs vs foundTriforcePieces/OoT)
    uint8_t comboObtainedFc[FC_COMBO_OBTAINED_FC_SIZE]; // fcId(FcComboItemId)-indexed cross store (counts); synced
    uint8_t comboAppliedFc[FC_COMBO_OBTAINED_FC_SIZE];  // local: copies already materialized here (NOT synced)

    // --- OoT vanilla tunics/boots (Skijer's NEI equipment behaviors, MM port). MM's Link has NO
    //     native Goron/Zora tunic or Iron/Hover boots slots, so the equipped vanilla tunic/boots
    //     live here. Effects are REINTERPRETED for MM's context (user decision 2026-07-11):
    //       Goron tunic  -> fireproof (no burning in hot rooms / from fire) + fire-damage immunity
    //       Zora tunic   -> immunity to gas/poison-swamp damage
    //       Iron boots   -> anti-knockback + anti-wind (not blown / pushed)
    //       Hover boots  -> brief air-float (reduced gravity for a moment after the jump apex)
    //     Appended at the END so older shipSaveInfo blobs stay readable. ---
    uint8_t vanillaTunic; // 0 = Kokiri (green), 1 = Goron (red, fireproof), 2 = Zora (blue, gas-immune)
    uint8_t vanillaBoots; // 0 = Kokiri, 1 = Iron (anti-knockback/wind), 2 = Hover (air-float)
    // Deku shield "skin": MM has only Hero(1)/Mirror(2) shield equip values. The Deku shield is
    // equipped AS Hero (same IA / raise gate / collider) but draws OoT's smaller Deku shield model,
    // so it needs this flag to distinguish "Hero value + Deku skin" from a real Hero shield. Cleared
    // whenever a non-Deku shield (Hero/Mirror/ext) is equipped.
    uint8_t vanillaShieldSkin; // 0 = native model by equip value, 1 = Deku shield, 2 = OoT Mirror
    // Upgrade-column passives (Skijer 2026-07-16, mirror of the OoT rework): Magic Cape + Pendant of
    // Memories moved OUT of the ext-equipment grid onto the equipment kaleido's upgrade column,
    // A-toggled there (solid = on, half-transparent = off). Ownership stays in extEquipOwnedBits
    // (TUNIC 1 / BOOTS 2). Appended at the END so older blobs stay readable.
    uint8_t capeHidden;       // 1 = don't draw the cape cloth on Link (half-cost passive stays active)
    uint8_t pendantEffectOff; // 1 = Pendant of Memories moveset disabled
    // Time Gate "adult mode" (Skijer's NEI): the Time Gate item (item_time_gate.c) swaps MM's human
    // Link model to OoT's ADULT Link (DL + colliders only, no mechanic change). MM has no real age
    // system (gSaveContext.save.linkAge is always 0), so this is our own persistent toggle. When set,
    // adult_link_model.cpp forces the OoT-adult skeleton/textures over gLinkHumanSkel and grows the
    // body collider; it re-asserts on scene load. Appended at the END so older blobs stay readable.
    uint8_t timeGateAdultMode; // 1 = OoT adult Link model + adult collider forced
} NeiSaveData;

// Hookshot-cell variant ids (which item currently fires from SLOT_HOOKSHOT). Returned by
// Nei_HookshotVariant(); consumed by z_arms_hook.c (reach/speed) and the in-hand draw.
#define NEI_HOOK_VARIANT_HOOKSHOT 0
#define NEI_HOOK_VARIANT_LONGSHOT 1
#define NEI_HOOK_VARIANT_ULTRASHOT 2
#define NEI_HOOK_VARIANT_CLAWSHOT 3

// OoT quest-item bit indices (mirror soh z64save.h QUEST_*), for the parallel ootQuestItems store.
#define OOT_QUEST_MEDALLION_FOREST 0
#define OOT_QUEST_MEDALLION_FIRE 1
#define OOT_QUEST_MEDALLION_WATER 2
#define OOT_QUEST_MEDALLION_SPIRIT 3
#define OOT_QUEST_MEDALLION_SHADOW 4
#define OOT_QUEST_MEDALLION_LIGHT 5
#define OOT_QUEST_SONG_MINUET 6
#define OOT_QUEST_SONG_BOLERO 7
#define OOT_QUEST_SONG_SERENADE 8
#define OOT_QUEST_SONG_REQUIEM 9
#define OOT_QUEST_SONG_NOCTURNE 10
#define OOT_QUEST_SONG_PRELUDE 11
// CANONICAL song rows 12..17 (verified against the reader: z_kaleido_collect.c draws song quads
// 6..17 straight off these bit indices, and sOotSongToMmOcarina maps each row's playback):
//   12 Zelda's Lullaby, 14 Saria's Song, 15 Sun's Song — melodies MM also owns; their MM-native
//   gives (GiveItem.cpp) light these bits so the OoT page mirrors the pickup.
//   13 / 16 / 17 are the NEI CUSTOM songs. In OoT's own z64save.h those bits are Epona / Time /
//   Storms, but the mirrored page REPLACES those 3 truly-doubled rows (MM grants Epona's Song,
//   Song of Time and Song of Storms natively, so they never need OoT-page bits): here the bits
//   canonically mean Fugue of Home / Command Melody / Ballad of the Hero. No collision.
#define OOT_QUEST_SONG_LULLABY 12
#define OOT_QUEST_SONG_FUGUE_OF_HOME 13 // NEI custom (Epona's row in stock OoT)
#define OOT_QUEST_SONG_SARIA 14
#define OOT_QUEST_SONG_SUN 15
#define OOT_QUEST_SONG_COMMAND_MELODY 16 // NEI custom (Song of Time's row in stock OoT)
#define OOT_QUEST_SONG_BALLAD_OF_HERO 17 // NEI custom (Song of Storms' row in stock OoT)
#define OOT_QUEST_KOKIRI_EMERALD 18
#define OOT_QUEST_GORON_RUBY 19
#define OOT_QUEST_ZORA_SAPPHIRE 20
#define OOT_QUEST_STONE_OF_AGONY 21
#define OOT_QUEST_GERUDO_CARD 22
#define OOT_QUEST_SKULL_TOKEN 23

// Skijer's NEI slingshot pass — bullet-bag helpers (implemented in nei_save.cpp, C-callable):
// level = ootUpgrades bulletBag bits clamped to 0..3; capacity = OoT gUpgradeCapacities row
// { 0, 30, 40, 50 }. Used by z_player.c (ammo gate), z_parameter.c (HUD) and the SaveEditor.
uint8_t Nei_BulletBagLevel(void);
uint8_t Nei_SlingshotCapacity(void);
uint8_t Nei_SlingshotSeeds(void);

// Which hookshot-cell variant currently fires (NEI_HOOK_VARIANT_*). Clawshot when its wheel mode
// is active + owned; otherwise the OoT chain by ootHookshotLevel (1/2/3). Used by z_arms_hook.c.
uint8_t Nei_HookshotVariant(void);

// OoT hookshot-chain level (1 Hookshot / 2 Longshot / 3 Ultrashot) — for TUs that don't include the
// NEI headers (z_parameter.c HUD medallion marker).
uint8_t Nei_HookshotLevel(void);

// Single accessor — returns the live per-save state (never NULL).
NeiSaveData* Nei_Save(void);

// Custom inventory slot helpers (slot 24..71 -> ownedItems[slot-24]).
uint8_t Nei_GetOwnedItem(uint8_t slot);
void Nei_SetOwnedItem(uint8_t slot, uint8_t v);

// Reset NEI state for a brand-new save (empty custom slots = 0xFF = ITEM_NONE).
// 2ship port: call from the new-file init path (gSaveContext.save.shipSaveInfo.nei).
void Nei_InitNewSave(void);

#ifdef __cplusplus
}
#endif

#endif // NEI_SAVE_H
