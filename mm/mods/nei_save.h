// Skijer's NEI
#ifndef NEI_SAVE_H
#define NEI_SAVE_H

#include <stdint.h>
#include "2s2h/FleetShipCombo/FleetComboIds.h" // FC_COMBO_OBTAINED_FC_SIZE

#ifdef __cplusplus
extern "C" {
#endif

// ── SW97 primed element (Skijer's NEI) ───────────────────────────────────────────────────────────
// The bow's / slingshot's elemental shot used to BE the item id sitting on the C-button: six
// ITEM_SW97_ARROW_* plus six ITEM_SW97_BULLET_*. That burned twelve inventory ids to express three
// bits, and the bullets (0xA7-0xAC) collided head-on with ITEM_MAP_POINT_DEKU_PALACE..IKANA_CANYON.
// The element is a flag now (Gust Jar pattern): the button always holds the plain weapon and the
// medallion is composited over the icon.
//
// The 1..6 ordering is load-bearing: it keeps the existing offset expressions a one-liner
// (`ARROW_TYPE_SW97_FIRE + (e - SW97_ELEM_FIRE)`, `ARROW_TYPE_SEED_FIRE + (e - SW97_ELEM_FIRE)`) and
// it is bit-identical to the legacy `slingshotWheel` index, so that field migrates by plain copy.
#define SW97_ELEM_NONE 0  // plain bow / plain slingshot
#define SW97_ELEM_FIRE 1  // Fire Medallion
#define SW97_ELEM_ICE 2   // Water Medallion
#define SW97_ELEM_LIGHT 3 // Light Medallion
#define SW97_ELEM_DARK 4  // Shadow Medallion
#define SW97_ELEM_SOUL 5  // Spirit Medallion
#define SW97_ELEM_WIND 6  // Forest Medallion
#define SW97_ELEM_BOMB 7  // Bomb Arrows — BOW ONLY (never rides the slingshot flag)
#define SW97_ELEM_COUNT 8

// Elemental Wand modes (Skijer's NEI) — six rods in ONE page-2 cell, same wheel idiom. Index order
// IS the wheel order; the medallion column is what the wheel draws behind the rod icon.
//   0 Spirit -> Sand Rod      1 Forest -> Tornado Rod   2 Water  -> Water Rod
//   3 Fire   -> Meteor Rod    4 Light  -> Storm Rod     5 Shadow -> Shadow Scepter
#define WAND_MODE_SAND 0
#define WAND_MODE_TORNADO 1
#define WAND_MODE_WATER 2
#define WAND_MODE_METEOR 3
#define WAND_MODE_STORM 4
#define WAND_MODE_SCEPTER 5
#define WAND_MODE_COUNT 6

// Randomizer treatment of the wand (all three share the SAME slot flag).
#define WAND_RANDO_MEDALLIONS 0 // 1 pool item; mode N usable iff you own medallion N
#define WAND_RANDO_SINGLE 1     // 1 pool item; obtaining it lights all six modes
#define WAND_RANDO_ELEMENTAL 2  // 6 pool items; each lights its own mode

// Sheikah Slate runes (Skijer's NEI) — four runes in ONE page-2 cell, wand idiom: sibling
// obtainable items over one slot (each with its own textbox), gettable in any order, no levels.
// Index order IS the wheel order.
#define SLATE_RUNE_BOMB 0 // Remote Bomb
#define SLATE_RUNE_STASIS 1
#define SLATE_RUNE_CRYONIS 2
#define SLATE_RUNE_MASTER_CYCLE 3 // Master Cycle Zero
#define SLATE_RUNE_COUNT 4
// Future runes with art already staged in icon_item_custom: Magnesis, Camera
// (gItemIconSlateRuneMagnesisTex / gItemIconSlateRuneCameraTex).

// Randomizer treatment of Bomb Arrows.
#define BOMB_ARROWS_RANDO_OFF 0      // never granted on their own (Twilight Upgrade still works)
#define BOMB_ARROWS_RANDO_BOMB_BAG 1 // auto-granted the moment any bomb bag is owned
#define BOMB_ARROWS_RANDO_SHUFFLED 2 // a real randomizer item

// Skijer's NEI: per-save state, serialized via the "nei" SaveManager section
// (NOT in the vanilla SaveContext, which is kept 100% upstream).
typedef struct NeiSaveData {
    // Custom inventory slots 24..71 (page-2 items + MM masks).
    // u16, NOT u8: the vanilla u8 item-id space is exhausted — 5 free ids in all of 0x9C-0xFF, and
    // seven of the ported OoT items (Din's Fire..Roc's Feather, extended_inventory.h) are still
    // sitting on top of live MM ids (ITEM_MILK, ITEM_GOLD_DUST_2, and three owl-warp map points).
    // Widening this store is what lets a page-2 item carry an EXT id above 0xFF (the 0x02xx space
    // the Spiritual Stones already use) instead of competing for those last few bytes.
    // ITEM_NONE stays 0xFF — the empty marker did NOT become 0xFFFF. Skijer's NEI
    uint16_t ownedItems[48];
    // 2026-08-06 page-2 re-layout: Shovel and Dominion Rod share cell 46 behind a wheel, so the cell
    // value alone cannot say "both owned" — each carries its own flag (Power Keg idiom). The
    // Pokeball left page 2 for the Broken Items page; its ownership is a flag too. Skijer's NEI
    uint8_t shovelOwned;
    uint8_t dominionOwned;
    uint8_t pokeballOwned;
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
    // SLOT_TRADE_ADULT shows every owned entry. This bitmask is the Pendant's ONLY ownership flag
    // (2026-07-29: the ext BOOTS-2 bit it used to shadow belongs to the Climb Boots). See trade_items.c.
    uint32_t tradeAdultOwned;
    // (The NEI Pictograph Box block lived here — pictoboxOwned / pictoHasPhoto / pictoFlags0/1 /
    // pictoPhotoI5[11200]. REMOVED in 2ship: MM stores all of it natively, in this very layout, at
    // gSaveContext.save.saveInfo.pictoFlags0/1 + gSaveContext.pictoPhotoI5. Ownership is the vanilla
    // SLOT_PICTOGRAPH_BOX inventory slot. Old json saves may still carry the four keys; from_json
    // reads by name, so they are simply ignored.)

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
    uint16_t ootUpgrades;     // 3 bits each: bulletBag(0) quiver(3) bombBag(6) strength(9) scale(12)
    uint16_t ootMasksOwned;   // OoT child-trade masks bitmask (wheel lands in the per-item pass)
    // Slingshot-cell wheel selection (Skijer's NEI slingshot pass): 0 = plain Fairy Slingshot,
    // LEGACY (Skijer's NEI): superseded by sw97SlingElement, which uses the SAME 1..6 numbering.
    // Migrated once in Sw97_MigrateLayout and then written 0 forever; kept only because removing a
    // mid-struct field would shift every field after it. Do not read or write it in new code.
    // Was: 1..6 = SW97 elemental bullet fire/ice/light/dark/soul/wind. Appended
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
    uint8_t comboGoalFlags;     // FC_GOAL_*: which bosses are already down, across BOTH games.
                                // Beat Both Bosses ends only when both bits are set; the first win
                                // records its bit, saves, and returns you to play on.
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
    uint8_t capeOwned;
    uint8_t extTunicLayoutVersion; // 1 = Champion/Spirit/Sage's
    // Dual Cane (Somaria / Pacci) — Skijer's NEI. Six SEPARATE obtainable items share one
    // kaleido slot; each lights its own bit here and they may be obtained in any order.
    // Appended at the END so older blobs stay readable.
    uint8_t caneSkills;       // bitmask, CANE_SKILL_BIT(CANE_SKILL_*) — 0 = cane not owned at all
    uint8_t caneType;         // active cane: CANE_TYPE_SOMARIA / CANE_TYPE_PACCI
    uint8_t caneSkillSel[2];  // per-cane selected skill SLOT (0..2); index by caneType
    // Quartz of Motion (2nd level of the progressive Stone of Agony). The tracking
    // category is chosen by pressing A on the Stone of Agony quest slot in the
    // kaleido (Bombers'-Notebook-style list); activating spends one heart and runs
    // the sensor for 5 minutes. Only the SELECTION persists — the countdown itself
    // is session state in CustomItemState. Appended at the END so older blobs stay
    // readable.
    uint8_t quartzOwned;      // 1 = Quartz of Motion obtained (2nd Stone of Agony copy)
    uint8_t quartzCategory;   // DesireCompassCategory last selected in the kaleido
    uint8_t quartzSubcat;     // subcategory within that category (0 = any)
    // Ext BOOTS slots 2/3 became REAL boots (Climb Boots / Roc Boots) on 2026-07-29, so the Pendant
    // of Memories can no longer use the BOOTS-2 bit as its "combat owned" flag — its single source of
    // truth is the adult trade wheel (tradeAdultOwned, TRADE_ADULT_PENDANT). Old saves are migrated in
    // ExtEquip_Init, gated by this version. Appended at the END.
    uint8_t extBootsLayoutVersion; // 1 = Pegasus / Climb / Roc
    // Which trade index the unified wheel is currently showing (0..TRADE_ADULT_COUNT-1).
    // MM cannot identify these items by inventory id: the 14 OoT trade ids alias to ITEM_NONE here
    // (nei_oot_compat.h) and the u8 ItemId space has no room for 14 more. So the wheel is driven by
    // this index and the cell holds ITEM_TRADE_PLACEHOLDER; art comes from TradeAdult_Icon/NamePath().
    // APPENDED AT THE END — it first went in next to tradeAdultOwned, which shifted every field after
    // it (including pictoPhotoI5[11200]) and crashed on load. Skijer 2026-07-30
    uint8_t tradeAdultCursor;
    // Which of the 8 OoT child-trade masks the cell-4,6 wheel is showing (index into ootMasksOwned).
    // Same reasoning as tradeAdultCursor: those masks have no MM item id. APPENDED AT THE END.
    uint8_t ootMaskCursor;
    // Pendant of Memories — EQUIPMENT ownership (Skijer 2026-07-31). Distinct from the trade-wheel
    // item: holding the pendant in the adult trade slot GRANTS this flag, and it is then permanent —
    // the trade item can be handed away, the equipment piece cannot. This is the flag the kaleido
    // equipment upgrade column shows and lets you toggle; pendantEffectOff above is the separate
    // "moveset on/off" toggle. Mirrors soh's NeiSaveData::pendantOwned so FleetSync can carry it.
    // APPENDED AT THE END.
    uint8_t pendantOwned;
    // SW97 elemental shot + Bomb Arrows + Elemental Wand (Skijer's NEI). The bow and the slingshot
    // carry INDEPENDENT elements on purpose — you may prime spirit arrows and wind bullets at once.
    // sw97SlingElement supersedes the old slingshotWheel index (migrated once, see
    // Sw97_MigrateLayout). APPENDED AT THE END.
    uint8_t sw97BowElement;    // SW97_ELEM_*
    uint8_t sw97SlingElement;  // SW97_ELEM_* — never SW97_ELEM_BOMB (bombs are bow-only)
    uint8_t bombArrowsOwned;   // replaces the old page-2 SLOT_BOMB_ARROWS cell
    uint8_t wandMode;          // WAND_MODE_* — the rod the page-2 cell is showing
    uint8_t wandRodsOwned;     // WAND_MODE_* bitmask (six bits)
    uint8_t sw97LayoutVersion; // 1 = migrated off the per-element item ids
    // Sheikah Slate — Skijer's NEI. Four runes (Remote Bomb / Stasis / Cryonis / Master Cycle) share
    // the one SLOT_SHEIKAH_SLATE cell; each pickup grants a RANDOM unowned rune (like the wands,
    // no levels). APPENDED AT THE END.
    uint8_t slateMode;       // SLATE_RUNE_* — the rune the cell is showing / the button casts
    uint8_t slateRunesOwned; // SLATE_RUNE_* bitmask (four bits) — 0 = slate not owned at all
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

// ── OoT strength upgrade (Skijer's NEI) ──────────────────────────────────────────────────────────
// 0 none / 1 Goron's Bracelet / 2 Silver Gauntlets / 3 Golden Gauntlets. MM has no native strength
// stat, so the level lives in the NEI parallel OoT-upgrade store (ootUpgrades bits 9-11) — which is
// per-save (serialized with ShipSaveInfo by BenJsonConversions.hpp), is what the MM kaleido
// equipment page already draws (z_kaleido_collect.c, QUEST_SHIELD quad), and is what FleetSync
// publishes to OoT as upgrades["strength"], where it lands as a real Inventory_ChangeUpgrade(
// UPG_STRENGTH). This is THE strength variable on the MM side: anything that wants to gate on it
// later (lifting a silver rock, a heavier grab, an MM-side check) reads Nei_StrengthLevel() and
// gets the same number in both games. Do not add a second field for it. See also the native
// UPG_STRENGTH copy, which FleetSync keeps in step so its max() export can't walk backwards.
#define NEI_STRENGTH_SHIFT 9
#define NEI_STRENGTH_MAX 3
uint8_t Nei_StrengthLevel(void);
void Nei_SetStrengthLevel(uint8_t level);

// Which hookshot-cell variant currently fires (NEI_HOOK_VARIANT_*). Clawshot when its wheel mode
// is active + owned; otherwise the OoT chain by ootHookshotLevel (1/2/3). Used by z_arms_hook.c.
uint8_t Nei_HookshotVariant(void);

// OoT hookshot-chain level (1 Hookshot / 2 Longshot / 3 Ultrashot) — for TUs that don't include the
// NEI headers (z_parameter.c HUD medallion marker).
uint8_t Nei_HookshotLevel(void);

// ── Dual Cane (Somaria / Pacci) — Skijer's NEI ────────────────────────────────────────────────────
// Thin wrappers over NeiSaveData.caneSkills/caneType/caneSkillSel so the C item code and the C++ HUD
// share one source of truth. `skill` is a CANE_SKILL_* index (0..5), `type` a CANE_TYPE_*.
uint8_t Nei_CaneHasSkill(uint8_t skill);  // does the player own that skill?
void Nei_CaneGrantSkill(uint8_t skill);   // light its bit (idempotent)
uint8_t Nei_CaneSkillMask(void);          // the whole 6-bit mask (0 = cane not owned)
uint8_t Nei_CaneOwned(void);              // any skill owned -> the cane exists
uint8_t Nei_CaneTypeOwned(uint8_t type);  // is that wheel entry unlocked (4 entries)
uint8_t Nei_CaneTypeCount(void);          // how many of the four are owned
uint8_t Nei_CaneNextType(int8_t dir);     // next owned entry, wrapping
uint8_t Nei_CaneGetType(void);            // active cane (auto-corrected to one the player owns)
void Nei_CaneSetType(uint8_t type);
uint8_t Nei_CaneGetSkillSlot(uint8_t type); // selected slot 0..2 for that cane (auto-corrected)
void Nei_CaneSetSkillSlot(uint8_t type, uint8_t slot);
uint8_t Nei_CaneActiveSkill(void);        // CANE_SKILL_* the button would cast right now

// Single accessor — returns the live per-save state (never NULL).
NeiSaveData* Nei_Save(void);

// Custom inventory slot helpers (slot 24..71 -> ownedItems[slot-24]).
// u16 since the page-2 store was widened (see ownedItems above); empty is still 0xFF (ITEM_NONE).
uint16_t Nei_GetOwnedItem(uint8_t slot);
void Nei_SetOwnedItem(uint8_t slot, uint16_t v);

// Reset NEI state for a brand-new save (empty custom slots = 0xFF = ITEM_NONE).
// 2ship port: call from the new-file init path (gSaveContext.save.shipSaveInfo.nei).
void Nei_InitNewSave(void);

#ifdef __cplusplus
}
#endif

#endif // NEI_SAVE_H
