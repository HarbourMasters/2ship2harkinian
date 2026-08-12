#include "Rando/Rando.h"
#include "Rando/ActorBehavior/Souls.h"
#include "Rando/MiscBehavior/MiscBehavior.h"
#include "Rando/MiscBehavior/ClockShuffle.h"
#include "mods/nei_save.h" // NeiSaveData, Nei_Save, bullet-bag helpers (Skijer's NEI slingshot pass)
#include "2s2h/FleetShipCombo/FleetComboItemsGlue.h" // FcCombo_ItemForNative (native RI -> fcId)
#include "2s2h/FleetShipCombo/FleetComboItems.h"     // FCI_NO_ITEM, FCI_MAX
#include "2s2h/FleetShipCombo/FleetComboIds.h"       // FC_OOT_* registry indices + FC_SHIELD_* bits
#include "2s2h/BenGui/Notification.h"                // "You found X" corner toast (parity with SoH)

// MM's native EquipmentType (z64item.h) only has SWORD=0/SHIELD=1/TUNIC=2; BOOTS is the NEI accessory
// tag defined in mods/nei_oot_compat.h (=3, matches extended_equipment.c's sword/shield/tunic/boots
// indexing). That compat shim is too heavy to include in this C++ TU, so mirror the guarded define.
#ifndef EQUIP_TYPE_BOOTS
#define EQUIP_TYPE_BOOTS 3
#endif

extern "C" {
#include "variables.h"
#include "functions.h"
#include "mods/extended_equipment.h"
#include "mods/extended_inventory.h" // page-2 SLOT_*/custom ITEM_* ids + ExtInv_GiveItem (ownedItems store)
// Set by FleetSync's ApplyFcRegistryToNatives while it grants an FC deficit — suppress re-recording
// those native grants (they were already counted when obtained), else the registry feeds itself.
extern int gFcCombo_SuppressRecord;
// item_cane_of_somaria.c — Dual Cane. Six separate obtainable skills share ONE inventory
// slot; this lights the skill's bit and, on the FIRST one obtained (any of the six, in any
// order), also puts the cane into SLOT_CANE_OF_SOMARIA. CANE_SKILL_* live in
// mods/items/logic/item_cane_of_somaria.h, mirrored here so this TU stays free of the
// z_player-side item headers.
#define CANE_SKILL_SOMARIA_STATUE 0
#define CANE_SKILL_SOMARIA_BLOCK 1
#define CANE_SKILL_SOMARIA_PLATFORM 2
#define CANE_SKILL_PACCI_FLIP 3
#define CANE_SKILL_PACCI_STONE 4
#define CANE_SKILL_PACCI_ULTRAHAND 5
u8 Cane_GiveSkill(u8 skill);
// trade_items.c — sets the tradeAdultOwned bit for a NEI trade index (nei_save.h layout); index 19
// (Pendant of Memories) also sets its Ext Boots 2 combat-ownership bit (FCI_F_DUAL_GRANT).
void TradeAdult_GiveIndex(s32 index);
// weapon_upgrades.c — progressive L2 bits in nei->weaponUpgrades (weapon_upgrades.h layout).
void WeaponUpgrade_SetRazor(u8 on);       // Kokiri chain L1: Razor (kaleido cell tier via KokiriLevel)
void WeaponUpgrade_SetGilded(u8 on);      // Kokiri chain L2: Gilded
void WeaponUpgrade_SetHammerAxe(u8 on);   // bit 0: Hammer -> Iron Knuckle's Axe
void WeaponUpgrade_SetTrueMaster(u8 on);  // bit 3: Master -> True Master Sword
void WeaponUpgrade_SetGreatFairy(u8 on);  // bit 4: BGS -> Great Fairy's Sword
// custom_bottles.cpp — Net / Bottomless Bottle ownership (netEquipped / bottomlessBottleMode; the
// per-frame enforcer in mm_bottle_items.cpp projects them into SLOT_BOTTLE_3/4).
void Bottle_SetNetOwned(u8 owned);
void Bottle_SetBottomlessOwned(u8 owned);
void Bottle_BottomlessEmpty(void);
u8 Bottle_GiveBottle(u16 contentItem); // add a filled bottle to the first free NEI wheel slot
}

void Rando::GiveItem(RandoItemId randoItemId) {
    // FleetShipCombo cross-item record: on obtaining ANY FC-shared item, bump its fcId-indexed count
    // (comboObtainedFc, synced to OoT) and the local shadow (comboAppliedFc, since it's granted here).
    // Re-entrancy guard: GiveItem recurses for progressives/ConvertItem and RI_TRIFORCE_PIECE ->
    // RI_SOUL_BOSS_MAJORA — only the OUTERMOST call records so a single pickup counts once.
    static int sGiveDepth = 0;
    struct DepthGuard {
        DepthGuard() {
            sGiveDepth++;
        }
        ~DepthGuard() {
            sGiveDepth--;
        }
    } _dg;
    if (sGiveDepth == 1 && gFcCombo_SuppressRecord == 0) {
        // Dual Cane skills: MM's pool places SIX DISTINCT skill RIs, but the FC table pairs one RG
        // with one RI — soh's side is six copies of the single RG_CANE_OF_SOMARIA, all counting into
        // FCI_CANE_OF_SOMARIA (chainLen 6). Adding five more rows would need five fake RG peers and
        // collapse riToFc (documented FC-table rule), so instead the five skill RIs ALIAS to the
        // generic cane row here, at record time. Each obtained skill = +1 on the shared counter; the
        // peer materializes it through its own progressive give. This closes "the cane skills never
        // cross in combo". Skijer's NEI
        RandoItemId recordId = randoItemId;
        switch (randoItemId) {
            case RI_OOT_NEI_CANE_SOMARIA_BLOCK:
            case RI_OOT_NEI_CANE_SOMARIA_PLATFORM:
            case RI_OOT_NEI_CANE_PACCI_FLIP:
            case RI_OOT_NEI_CANE_PACCI_STONE:
            case RI_OOT_NEI_CANE_PACCI_ULTRAHAND:
                recordId = RI_OOT_NEI_CANE_OF_SOMARIA;
                break;
            default:
                break;
        }
        int fc = FcCombo_ItemForNative((int)recordId);
        if (fc != FCI_NO_ITEM) {
            NeiSaveData* nei = Nei_Save();
            if (fc >= 0 && fc < FC_COMBO_OBTAINED_FC_SIZE) {
                nei->comboObtainedFc[fc]++;
                nei->comboAppliedFc[fc]++;
            }
        }
        // "You found X" corner toast — parity with SoH's EnItem00 pickup notification
        // (soh hook_handlers.cpp ~1330). Outermost call only = one toast per pickup (recursion for
        // progressives/ConvertItem stays silent), and cross-sync deficit grants (SuppressRecord) stay
        // silent too, matching OoT where FC arrivals don't toast.
        if (Rando::StaticData::Items.contains(randoItemId)) {
            Notification::Emit({
                .message = "You found",
                .suffix = Rando::StaticData::GetItemName(randoItemId),
            });
        }
    }
    switch (randoItemId) {
        case RI_CLOCK_TOWN_STRAY_FAIRY:
            SET_WEEKEVENTREG(WEEKEVENTREG_08_80);
            break;
        case RI_WOODFALL_STRAY_FAIRY:
            gSaveContext.save.saveInfo.inventory.strayFairies[DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE]++;
            break;
        case RI_SNOWHEAD_STRAY_FAIRY:
            gSaveContext.save.saveInfo.inventory.strayFairies[DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE]++;
            break;
        case RI_GREAT_BAY_STRAY_FAIRY:
            gSaveContext.save.saveInfo.inventory.strayFairies[DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE]++;
            break;
        case RI_STONE_TOWER_STRAY_FAIRY:
            gSaveContext.save.saveInfo.inventory.strayFairies[DUNGEON_SCENE_INDEX_STONE_TOWER_TEMPLE]++;
            break;
        case RI_GREAT_SPIN_ATTACK:
            SET_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_GREAT_SPIN_ATTACK);
            break;
        case RI_DOUBLE_DEFENSE:
            gSaveContext.save.saveInfo.playerData.doubleDefense = true;
            gSaveContext.save.saveInfo.inventory.defenseHearts = 20;
            break;
        case RI_SINGLE_MAGIC:
            gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
            gSaveContext.save.saveInfo.playerData.magic = gSaveContext.magicFillTarget = MAGIC_NORMAL_METER;
            SET_WEEKEVENTREG(WEEKEVENTREG_12_80);
            break;
        case RI_DOUBLE_MAGIC:
            gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
            gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = true;
            gSaveContext.save.saveInfo.playerData.magic = gSaveContext.magicFillTarget = MAGIC_DOUBLE_METER;
            gSaveContext.save.saveInfo.playerData.magicLevel = 0;
            SET_WEEKEVENTREG(WEEKEVENTREG_12_80);
            break;
        // Don't love this because it doesn't hit GameInteractor_ExecuteOnItemGive()
        // but not sure how else to account for collecting outside of dungeon
        case RI_WOODFALL_BOSS_KEY:
        case RI_WOODFALL_MAP:
        case RI_WOODFALL_COMPASS:
            SET_DUNGEON_ITEM(Rando::StaticData::Items[randoItemId].itemId - ITEM_KEY_BOSS,
                             DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE);
            break;
        case RI_WOODFALL_SMALL_KEY:
            if (DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE) < 0) {
                DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE) = 1;
                gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE] = 1;
            } else {
                DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE)++;
                gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE]++;
            }
            break;
        case RI_SNOWHEAD_BOSS_KEY:
        case RI_SNOWHEAD_MAP:
        case RI_SNOWHEAD_COMPASS:
            SET_DUNGEON_ITEM(Rando::StaticData::Items[randoItemId].itemId - ITEM_KEY_BOSS,
                             DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE);
            break;
        case RI_SNOWHEAD_SMALL_KEY:
            if (DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE) < 0) {
                DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE) = 1;
                gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE] = 1;
            } else {
                DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE)++;
                gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE]++;
            }
            break;
        case RI_GREAT_BAY_BOSS_KEY:
        case RI_GREAT_BAY_MAP:
        case RI_GREAT_BAY_COMPASS:
            SET_DUNGEON_ITEM(Rando::StaticData::Items[randoItemId].itemId - ITEM_KEY_BOSS,
                             DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE);
            break;
        case RI_GREAT_BAY_SMALL_KEY:
            if (DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE) < 0) {
                DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE) = 1;
                gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE] = 1;
            } else {
                DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE)++;
                gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE]++;
            }
            break;
        case RI_STONE_TOWER_BOSS_KEY:
        case RI_STONE_TOWER_MAP:
        case RI_STONE_TOWER_COMPASS:
            SET_DUNGEON_ITEM(Rando::StaticData::Items[randoItemId].itemId - ITEM_KEY_BOSS,
                             DUNGEON_SCENE_INDEX_STONE_TOWER_TEMPLE);
            break;
        case RI_STONE_TOWER_SMALL_KEY:
            if (DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_STONE_TOWER_TEMPLE) < 0) {
                DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_STONE_TOWER_TEMPLE) = 1;
                gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_STONE_TOWER_TEMPLE] = 1;
            } else {
                DUNGEON_KEY_COUNT(DUNGEON_SCENE_INDEX_STONE_TOWER_TEMPLE)++;
                gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_STONE_TOWER_TEMPLE]++;
            }
            break;
        case RI_TRIFORCE_PIECE:
        case RI_TRIFORCE_PIECE_PREVIOUS:
            gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces++;
            if (gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces ==
                RANDO_SAVE_OPTIONS[RO_TRIFORCE_PIECES_REQUIRED]) {
                // Blocks the ability to beat the game through killing Majora until all Triforce Pieces are found.
                if (!Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_BOSS_MAJORA)) {
                    Rando::GiveItem(RI_SOUL_BOSS_MAJORA);
                }
                GameInteractor_ExecuteOnGameCompletion();
                GameInteractor::Instance->events.emplace_back(
                    GIEventTransition{ .entrance = ENTRANCE(TERMINA_FIELD, 0),
                                       .cutsceneIndex = 0xFFF7,
                                       .transitionTrigger = TRANS_TRIGGER_START,
                                       .transitionType = TRANS_TYPE_FADE_BLACK });
            }
            break;
        // Technically these should never be used, but leaving them here just in case
        case RI_PROGRESSIVE_MAGIC:
        case RI_PROGRESSIVE_BOW:
        case RI_PROGRESSIVE_BOMB_BAG:
        case RI_PROGRESSIVE_LULLABY:
        case RI_PROGRESSIVE_SWORD:
        case RI_PROGRESSIVE_WALLET:
            Rando::GiveItem(Rando::ConvertItem(randoItemId));
            break;
        case RI_BOMB_BAG_20:
        case RI_BOMB_BAG_30:
        case RI_BOMB_BAG_40:
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            INV_CONTENT(ITEM_BOMBCHU) = ITEM_BOMBCHU;
            AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
            break;
        case RI_WALLET_ADULT:
        case RI_WALLET_GIANT:
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            // Fill Rupees to max, this may be opt-in later
            // Use remaining space rather than full capacity to avoid excess in the accumulator.
            gSaveContext.rupeeAccumulator = CUR_CAPACITY(UPG_WALLET) - gSaveContext.save.saveInfo.playerData.rupees;
            break;
        case RI_WALLET_TYCOON:
            Inventory_ChangeUpgrade(UPG_WALLET, 3);
            gSaveContext.rupeeAccumulator = CUR_CAPACITY(UPG_WALLET) - gSaveContext.save.saveInfo.playerData.rupees;
            break;
        case RI_GS_TOKEN_SWAMP:
            // Set QUEST_QUIVER to match bug mentioned in z_parameter.c
            SET_QUEST_ITEM(QUEST_QUIVER);
            Inventory_IncrementSkullTokenCount(SCENE_KINSTA1);
            break;
        case RI_GS_TOKEN_OCEAN:
            SET_QUEST_ITEM(QUEST_QUIVER);
            Inventory_IncrementSkullTokenCount(SCENE_KINDAN2);
            break;
        case RI_MOONS_TEAR:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_MOONS_TEAR);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_DEED_LAND:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_DEED_LAND);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_DEED_SWAMP:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_DEED_SWAMP);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_DEED_MOUNTAIN:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_DEED_MOUNTAIN);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_DEED_OCEAN:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_DEED_OCEAN);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_ROOM_KEY:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_ROOM_KEY);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_LETTER_TO_MAMA:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_LETTER_TO_MAMA);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_LETTER_TO_KAFEI:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_LETTER_TO_KAFEI);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_PENDANT_OF_MEMORIES:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_PENDANT_OF_MEMORIES);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            // FCI_F_DUAL_GRANT: also the NEI trade-wheel bit (tradeAdultOwned bit 19), which itself
            // sets the Ext Boots 2 combat-ownership bit so the moveset is C-equippable (trade_items.c).
            TradeAdult_GiveIndex(19);
            break;
        case RI_POWDER_KEG:
            Flags_SetWeekEventReg(WEEKEVENTREG_HAS_POWDERKEG_PRIVILEGES);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_CLAWSHOT:
            // Skijer's NEI: Clawshot is its OWN rando item, independent of the base Hookshot chain.
            // It is NOT a native inventory item — ownership is the clawshotOwned flag, and the L-tap
            // wheel selector is gated by the TWILIGHT_UPGRADE_CLAWSHOT (0x1) bit of twilightUpgrade.
            // Do NOT grant ITEM_HOOKSHOT here: ExtInv_GetSlotItem synthesizes the hookshot cell from
            // clawshotOwned (see extended_inventory.h), so the clawshot shows/equips/fires on its own.
            // Mirrors how Nei_GiveAllOotItems / FleetSync grant the clawshot (both flags set together).
            Nei_Save()->clawshotOwned = 1;
            Nei_Save()->twilightUpgrade |= 0x1; // TWILIGHT_UPGRADE_CLAWSHOT
            break;
        case RI_HOOKSHOT: {
            // FC 3-level chain (FCI_HOOKSHOT, count 3) — OoT-parity progressive: each copy gives the
            // NEXT tier (1 Hookshot / 2 Longshot / 3 Ultrashot, nei->ootHookshotLevel — the level the
            // hookshot overhaul's reach/speed reads). The native inventory item is granted alongside
            // so the MM slot/HUD stay coherent (FleetSync derives level 1 from a bare native slot).
            NeiSaveData* nei = Nei_Save();
            int hookLvl = nei->ootHookshotLevel;
            if (hookLvl == 0 && INV_CONTENT(ITEM_HOOKSHOT) == ITEM_HOOKSHOT) {
                hookLvl = 1; // pre-chain native hookshot with no level recorded (FleetSync idiom)
            }
            if (hookLvl < 3) {
                hookLvl++;
            }
            nei->ootHookshotLevel = (uint8_t)hookLvl;
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        }
        case RI_SHIELD_HERO:
            // MM-native grant + canonical FC ownership bit (FC_SHIELD_HYLIAN — the exact bit
            // ExtractShared/ApplyShared carry, so OoT receives the Hylian Shield). Only equip it
            // when shieldless: Item_Give's shield branch (z_parameter.c) unconditionally SETS the
            // equip value, which would DOWNGRADE an owned Mirror Shield.
            if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) == EQUIP_VALUE_SHIELD_NONE) {
                Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            }
            Nei_Save()->shieldOwned |= FC_SHIELD_HYLIAN;
            break;
        case RI_SHIELD_MIRROR:
            // MM's own Mirror Shield (= Shield of Ikana in the unified mask). Straight upgrade —
            // the equip nibble only ever moves up here.
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            Nei_Save()->shieldOwned |= FC_SHIELD_IKANA;
            // ...and its page-2 equipment cell (SHIELD-3). Only the mask bit was set here, and the
            // kaleido cell reads extEquipOwnedBits, so in a solo-MM game the Shield of Ikana cell
            // stayed dark forever; it only lit up after a round trip through OoT, whose ApplyShared
            // turns the mask bit back into bit 21. OoT's own give already does both (randomizer.cpp).
            // Skijer's NEI
            ExtEquip_GiveItem(EQUIP_TYPE_SHIELD, 3); // bit 21
            break;
        case RI_SWORD_GILDED:
        case RI_SWORD_KOKIRI:
        case RI_SWORD_RAZOR:
            if (STOLEN_ITEM_1 == ITEM_SWORD_KOKIRI || STOLEN_ITEM_1 == ITEM_SWORD_RAZOR) {
                SET_STOLEN_ITEM_1(STOLEN_ITEM_NONE);
            }
            if (STOLEN_ITEM_2 == ITEM_SWORD_KOKIRI || STOLEN_ITEM_2 == ITEM_SWORD_RAZOR) {
                SET_STOLEN_ITEM_2(STOLEN_ITEM_NONE);
            }

            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            // The unified equipment kaleido draws the Kokiri-line cell tier from
            // WeaponUpgrade_KokiriLevel() (nei->weaponUpgrades bits), NOT from the native equip —
            // without these bits the cell never advances past Kokiri (z_kaleido_equipment.c:277).
            if (randoItemId == RI_SWORD_RAZOR) {
                WeaponUpgrade_SetRazor(1);
            } else if (randoItemId == RI_SWORD_GILDED) {
                WeaponUpgrade_SetRazor(1);
                WeaponUpgrade_SetGilded(1);
            }
            break;
        case RI_GREAT_FAIRY_SWORD: {
            // FC 2-level chain (FCI_BIGGORON_SWORD): L1 = Biggoron's Sword base ownership (the FC
            // registry cell the ext-equipment kaleido + FleetSync swordFlags read), L2 = the actual
            // Great Fairy's Sword — native MM inventory item + WEAPON_UPGRADE_BGS_GREAT_FAIRY
            // (weaponUpgrades bit 4), exactly the state ApplyShared materializes when syncing from OoT.
            NeiSaveData* nei = Nei_Save();
            if (nei->comboObtained[FC_OOT_SWORD_BIGGORON] == 0) {
                nei->comboObtained[FC_OOT_SWORD_BIGGORON] = 1;
            } else {
                WeaponUpgrade_SetGreatFairy(1); // weaponUpgrades |= (1 << 4)
                Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId); // native GFS
            }
            break;
        }
        case RI_TINGLE_MAP_CLOCK_TOWN:
            Inventory_SetWorldMapCloudVisibility(TINGLE_MAP_CLOCK_TOWN);
            SET_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_CLOCK_TOWN);
            break;
        case RI_TINGLE_MAP_WOODFALL:
            Inventory_SetWorldMapCloudVisibility(TINGLE_MAP_WOODFALL);
            SET_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_WOODFALL);
            break;
        case RI_TINGLE_MAP_SNOWHEAD:
            Inventory_SetWorldMapCloudVisibility(TINGLE_MAP_SNOWHEAD);
            SET_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_SNOWHEAD);
            break;
        case RI_TINGLE_MAP_ROMANI_RANCH:
            Inventory_SetWorldMapCloudVisibility(TINGLE_MAP_ROMANI_RANCH);
            SET_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_ROMANI_RANCH);
            break;
        case RI_TINGLE_MAP_GREAT_BAY:
            Inventory_SetWorldMapCloudVisibility(TINGLE_MAP_GREAT_BAY);
            SET_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_GREAT_BAY);
            break;
        case RI_TINGLE_MAP_STONE_TOWER:
            Inventory_SetWorldMapCloudVisibility(TINGLE_MAP_STONE_TOWER);
            SET_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_STONE_TOWER);
            break;
        case RI_OWL_CLOCK_TOWN_SOUTH:
            Sram_ActivateOwl(OWL_WARP_CLOCK_TOWN);
            break;
        case RI_OWL_GREAT_BAY_COAST:
            Sram_ActivateOwl(OWL_WARP_GREAT_BAY_COAST);
            break;
        case RI_OWL_IKANA_CANYON:
            Sram_ActivateOwl(OWL_WARP_IKANA_CANYON);
            break;
        case RI_OWL_MILK_ROAD:
            Sram_ActivateOwl(OWL_WARP_MILK_ROAD);
            break;
        case RI_OWL_MOUNTAIN_VILLAGE:
            Sram_ActivateOwl(OWL_WARP_MOUNTAIN_VILLAGE);
            break;
        case RI_OWL_SNOWHEAD:
            Sram_ActivateOwl(OWL_WARP_SNOWHEAD);
            break;
        case RI_OWL_SOUTHERN_SWAMP:
            Sram_ActivateOwl(OWL_WARP_SOUTHERN_SWAMP);
            break;
        case RI_OWL_STONE_TOWER:
            Sram_ActivateOwl(OWL_WARP_STONE_TOWER);
            break;
        case RI_OWL_WOODFALL:
            Sram_ActivateOwl(OWL_WARP_WOODFALL);
            break;
        case RI_OWL_ZORA_CAPE:
            Sram_ActivateOwl(OWL_WARP_ZORA_CAPE);
            break;
        case RI_TIME_DAY_1:
        case RI_TIME_NIGHT_1:
        case RI_TIME_DAY_2:
        case RI_TIME_NIGHT_2:
        case RI_TIME_DAY_3:
        case RI_TIME_NIGHT_3: {
            int index = Rando::ClockItems::GetHalfDayIndexFromClockItem(randoItemId);
            if (index != Rando::ClockItems::INVALID) {
                Flags_SetRandoInf(static_cast<RandoInf>(RANDO_INF_OBTAINED_CLOCK_DAY_1 + index));
            }
            break;
        }
        case RI_TIME_PROGRESSIVE: {
            // Convert to actual half-day per mode
            RandoItemId concrete = Rando::ConvertItem(RI_TIME_PROGRESSIVE);
            if (concrete != RI_JUNK) {
                Rando::GiveItem(concrete);
            }
            break;
        }
        case RI_HEART_CONTAINER:
        case RI_HEART_PIECE:
            gSaveContext.healthAccumulator = gSaveContext.save.saveInfo.playerData.healthCapacity + 0x10;
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_BOTTLE_RED_POTION:
            // ITEM_LONGSHOT will give a Red Potion bottle on the first available bottle slot
            // ITEM_POTION_RED will put a Red Potion bottle on the first bottle slot
            Item_Give(gPlayState, ITEM_LONGSHOT);
            break;
        // OoT bottled contents: add a NEW filled bottle to the NEI wheel (Bottle_GiveBottle -> first
        // free bottleSlots entry). The vanilla Item_Give(content) path only FILLS an empty bottle the
        // player is already holding and is silently LOST when they have none — which made several of
        // these checks look like they gave nothing. A wheel bottle is the deliverable; its content is
        // usable from the wheel, and the identity still crosses games via comboObtainedFc.
        case RI_OOT_BOTTLE_BIG_POE:
            Bottle_GiveBottle(ITEM_BIG_POE);
            break;
        case RI_OOT_BOTTLE_BLUE_POTION:
            Bottle_GiveBottle(ITEM_POTION_BLUE);
            break;
        case RI_OOT_BOTTLE_BUGS:
            Bottle_GiveBottle(ITEM_BUG);
            break;
        case RI_OOT_BOTTLE_FAIRY:
            Bottle_GiveBottle(ITEM_FAIRY);
            break;
        case RI_OOT_BOTTLE_FISH:
            Bottle_GiveBottle(ITEM_FISH);
            break;
        case RI_OOT_BOTTLE_GREEN_POTION:
            Bottle_GiveBottle(ITEM_POTION_GREEN);
            break;
        case RI_OOT_BOTTLE_MAGIC_MUSHROOM:
            Bottle_GiveBottle(ITEM_MUSHROOM);
            break;
        case RI_OOT_BOTTLE_POE:
            Bottle_GiveBottle(ITEM_POE);
            break;
        case RI_OOT_BOTTLE_BLUE_FIRE:
            // Blue Fire has no MM content branch — grant an empty bottle so the pickup still has bottle
            // value; identity crosses via comboObtainedFc.
            Bottle_GiveBottle(ITEM_BOTTLE);
            break;
        case RI_SOUL_BOSS_GOHT:
        case RI_SOUL_BOSS_GYORG:
        case RI_SOUL_BOSS_MAJORA:
        case RI_SOUL_BOSS_ODOLWA:
        case RI_SOUL_BOSS_TWINMOLD:
        case RI_SOUL_ENEMY_ALIEN:
        case RI_SOUL_ENEMY_ARMOS:
        case RI_SOUL_ENEMY_BAD_BAT:
        case RI_SOUL_ENEMY_BEAMOS:
        case RI_SOUL_ENEMY_BOE:
        case RI_SOUL_ENEMY_BUBBLE:
        case RI_SOUL_ENEMY_CAPTAIN_KEETA:
        case RI_SOUL_ENEMY_CHUCHU:
        case RI_SOUL_ENEMY_DEATH_ARMOS:
        case RI_SOUL_ENEMY_DEEP_PYTHON:
        case RI_SOUL_ENEMY_DEKU_BABA:
        case RI_SOUL_ENEMY_DEXIHAND:
        case RI_SOUL_ENEMY_DINOLFOS:
        case RI_SOUL_ENEMY_DODONGO:
        case RI_SOUL_ENEMY_DRAGONFLY:
        case RI_SOUL_ENEMY_EENO:
        case RI_SOUL_ENEMY_EYEGORE:
        case RI_SOUL_ENEMY_FREEZARD:
        case RI_SOUL_ENEMY_GARO:
        case RI_SOUL_ENEMY_GEKKO:
        case RI_SOUL_ENEMY_GIANT_BEE:
        case RI_SOUL_ENEMY_GOMESS:
        case RI_SOUL_ENEMY_GUAY:
        case RI_SOUL_ENEMY_HIPLOOP:
        case RI_SOUL_ENEMY_IGOS_DU_IKANA:
        case RI_SOUL_ENEMY_IRON_KNUCKLE:
        case RI_SOUL_ENEMY_KEESE:
        case RI_SOUL_ENEMY_LEEVER:
        case RI_SOUL_ENEMY_LIKE_LIKE:
        case RI_SOUL_ENEMY_MAD_SCRUB:
        case RI_SOUL_ENEMY_NEJIRON:
        case RI_SOUL_ENEMY_OCTOROK:
        case RI_SOUL_ENEMY_PEAHAT:
        case RI_SOUL_ENEMY_PIRATE:
        case RI_SOUL_ENEMY_POE:
        case RI_SOUL_ENEMY_REDEAD:
        case RI_SOUL_ENEMY_SHELLBLADE:
        case RI_SOUL_ENEMY_SKULLFISH:
        case RI_SOUL_ENEMY_SKULLTULA:
        case RI_SOUL_ENEMY_SNAPPER:
        case RI_SOUL_ENEMY_STALCHILD:
        case RI_SOUL_ENEMY_TAKKURI:
        case RI_SOUL_ENEMY_TEKTITE:
        case RI_SOUL_ENEMY_WALLMASTER:
        case RI_SOUL_ENEMY_WART:
        case RI_SOUL_ENEMY_WIZROBE:
        case RI_SOUL_ENEMY_WOLFOS:
            Flags_SetRandoInf(SOUL_RI_TO_RANDO_INF(randoItemId));
            break;
        case RI_FROG_BLUE:
            SET_WEEKEVENTREG(WEEKEVENTREG_33_01);
            break;
        case RI_FROG_CYAN:
            SET_WEEKEVENTREG(WEEKEVENTREG_32_40);
            break;
        case RI_FROG_PINK:
            SET_WEEKEVENTREG(WEEKEVENTREG_32_80);
            break;
        case RI_FROG_WHITE:
            SET_WEEKEVENTREG(WEEKEVENTREG_33_02);
            break;
        case RI_ABILITY_SWIM:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_SWIM);
            break;
        case RI_TRAP:
            Rando::MiscBehavior::OfferTrapItem();
            break;
        case RI_OCARINA_BUTTON_A:
        case RI_OCARINA_BUTTON_C_DOWN:
        case RI_OCARINA_BUTTON_C_LEFT:
        case RI_OCARINA_BUTTON_C_RIGHT:
        case RI_OCARINA_BUTTON_C_UP:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_OCARINA_BUTTON_A + (randoItemId - RI_OCARINA_BUTTON_A));
            break;
        case RI_SONG_DOUBLE_TIME:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_SONG_DOUBLE_TIME);
            break;
        case RI_SONG_INVERTED_TIME:
            Flags_SetRandoInf(RANDO_INF_OBTAINED_SONG_INVERTED_TIME);
            break;
        case RI_DEKU_SEEDS: {
            // Skijer's NEI — OoT slingshot ammo. Refill 30 seeds, clamped to the current bullet-bag
            // capacity (0 with no bag, exactly like OoT — a refill can't exceed your bag).
            NeiSaveData* nei = Nei_Save();
            u8 cap = Nei_SlingshotCapacity();
            u16 seeds = (u16)nei->slingshotSeeds + 30;
            nei->slingshotSeeds = (u8)((seeds > cap) ? cap : seeds);
            break;
        }
        case RI_FAIRY_SLINGSHOT: {
            // Skijer's NEI — OoT Progressive Slingshot (FCI_SLINGSHOT, count 3), OoT-parity:
            // copy 1 = the slingshot with a level-1 bullet bag (cap 30); copies 2/3 = the 40/50
            // bullet bags (ootUpgrades bulletBag bits 0-2). Every copy refills to the new capacity.
            // Ownership drives the virtual slingshot inventory slot.
            NeiSaveData* nei = Nei_Save();
            uint8_t bagLvl = Nei_BulletBagLevel();
            if (!nei->slingshotOwned) {
                nei->slingshotOwned = 1;
                if (bagLvl == 0) {
                    nei->ootUpgrades = (u16)((nei->ootUpgrades & ~0x7) | 0x1); // bullet bag level 1
                }
            } else if (bagLvl < 3) {
                nei->ootUpgrades = (u16)((nei->ootUpgrades & ~0x7) | (u16)(bagLvl + 1)); // next bag
            }
            nei->slingshotSeeds = Nei_SlingshotCapacity();
            break;
        }
        // Skijer's NEI — OoT (SoH) rando souls have no MM gameplay effect: give is a no-op. The get-item
        // cutscene/message/model still play (handled by DrawItem + the message system); the check is marked
        // collected by the rando check system independently. Not progressive → no ConvertItem handling.
        case RI_SOUL_OOT_BEAN_DEATH_MOUNTAIN_CRATER:
        case RI_SOUL_OOT_BEAN_DEATH_MOUNTAIN_TRAIL:
        case RI_SOUL_OOT_BEAN_DESERT_COLOSSUS:
        case RI_SOUL_OOT_BEAN_GERUDO_VALLEY:
        case RI_SOUL_OOT_BEAN_GRAVEYARD:
        case RI_SOUL_OOT_BEAN_KOKIRI_FOREST:
        case RI_SOUL_OOT_BEAN_LAKE_HYLIA:
        case RI_SOUL_OOT_BEAN_LOST_WOODS:
        case RI_SOUL_OOT_BEAN_LOST_WOODS_BRIDGE:
        case RI_SOUL_OOT_BEAN_ZORAS_RIVER:
        case RI_SOUL_OOT_BOSS_BARINADE:
        case RI_SOUL_OOT_BOSS_BONGO_BONGO:
        case RI_SOUL_OOT_BOSS_GANON:
        case RI_SOUL_OOT_BOSS_GOHMA:
        case RI_SOUL_OOT_BOSS_KING_DODONGO:
        case RI_SOUL_OOT_BOSS_MORPHA:
        case RI_SOUL_OOT_BOSS_PHANTOM_GANON:
        case RI_SOUL_OOT_BOSS_TWINROVA:
        case RI_SOUL_OOT_BOSS_VOLVAGIA:
        // Skijer's NEI — OoT (SoH) ADULT trade-chain items: real MM grant into the adult-trade wheel
        // ownership bitmask (nei->tradeAdultOwned, trade_items.c index layout 0..10 =
        // PocketEgg..ClaimCheck). The SLOT_TRADE_ADULT 2D-grid wheel shows every owned entry and
        // FleetSync ships tradeAdultOwned verbatim, so obtaining here == obtaining in OoT.
        case RI_OOT_TRADE_POCKET_EGG:
            TradeAdult_GiveIndex(0);
            break;
        case RI_OOT_TRADE_COJIRO:
            TradeAdult_GiveIndex(2);
            break;
        case RI_OOT_TRADE_ODD_MUSHROOM:
            TradeAdult_GiveIndex(3);
            break;
        case RI_OOT_TRADE_ODD_POTION:
            TradeAdult_GiveIndex(4);
            break;
        case RI_OOT_TRADE_POACHERS_SAW:
            TradeAdult_GiveIndex(5);
            break;
        case RI_OOT_TRADE_BROKEN_GORONS_SWORD:
            TradeAdult_GiveIndex(6);
            break;
        case RI_OOT_TRADE_PRESCRIPTION:
            TradeAdult_GiveIndex(7);
            break;
        case RI_OOT_TRADE_EYEBALL_FROG:
            TradeAdult_GiveIndex(8);
            break;
        case RI_OOT_TRADE_EYEDROPS:
            TradeAdult_GiveIndex(9);
            break;
        case RI_OOT_TRADE_CLAIM_CHECK:
            TradeAdult_GiveIndex(10);
            break;
        // OoT CHILD trade items. These USED to be a no-op ("no MM store"), but the trade wheel was
        // unified on 2026-07-30: the child chain's non-mask items now have their own bits in
        // tradeAdultOwned (20 = Weird Egg, 21 = Cucco, 22 = Zelda's Letter), so they belong in the
        // same wheel as everything else. Index 21 has no case because OoT does not randomize the
        // Cucco — the Weird Egg hatches into it (no RG_/RI_ id exists). The FC record hook at the top
        // still counts the pickup in comboObtainedFc for the cross-game registry. Skijer 2026-07-30
        case RI_OOT_TRADE_WEIRD_EGG:
            TradeAdult_GiveIndex(20);
            break;
        case RI_OOT_TRADE_ZELDAS_LETTER:
            TradeAdult_GiveIndex(22);
            break;
        // Skijer's NEI — OoT (SoH) medallions, warp songs, and spiritual stones: no MM gameplay
        // effect (no sage/warp systems), but ownership is recorded in the parallel OoT quest store
        // (NeiSaveData.ootQuestItems, OoT bit layout) so MM's mirrored OoT quest page lights up and
        // FleetSync carries the bit to OoT's native questItems. Not progressive.
        case RI_OOT_MEDALLION_FOREST:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_MEDALLION_FOREST);
            break;
        case RI_OOT_MEDALLION_FIRE:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_MEDALLION_FIRE);
            break;
        case RI_OOT_MEDALLION_WATER:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_MEDALLION_WATER);
            break;
        case RI_OOT_MEDALLION_SPIRIT:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_MEDALLION_SPIRIT);
            break;
        case RI_OOT_MEDALLION_SHADOW:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_MEDALLION_SHADOW);
            break;
        case RI_OOT_MEDALLION_LIGHT:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_MEDALLION_LIGHT);
            break;
        case RI_OOT_SONG_MINUET_OF_FOREST:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_MINUET);
            break;
        case RI_OOT_SONG_BOLERO_OF_FIRE:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_BOLERO);
            break;
        case RI_OOT_SONG_SERENADE_OF_WATER:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_SERENADE);
            break;
        case RI_OOT_SONG_REQUIEM_OF_SPIRIT:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_REQUIEM);
            break;
        case RI_OOT_SONG_NOCTURNE_OF_SHADOW:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_NOCTURNE);
            break;
        case RI_OOT_SONG_PRELUDE_OF_LIGHT:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_PRELUDE);
            break;
        case RI_OOT_SONG_ZELDAS_LULLABY:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_LULLABY);
            break;
        case RI_OOT_STONE_KOKIRI_EMERALD:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_KOKIRI_EMERALD);
            break;
        case RI_OOT_STONE_GORON_RUBY:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_GORON_RUBY);
            break;
        case RI_OOT_STONE_ZORA_SAPPHIRE:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_ZORA_SAPPHIRE);
            break;
        case RI_OOT_STONE_OF_AGONY:
            // Progressive (2 levels), per the Desire Compass design:
            //   lvl 1 - the stone itself: a passive "something is here" sense
            //           (quest icon + the L1 rumble).
            //   lvl 2 - a second copy upgrades it into the Desire Compass, the
            //           equippable 8-category dowsing wheel. Owning the compass
            //           item IS the level-2 gate, so the item logic needs no
            //           extra check.
            if (Nei_Save()->ootQuestItems & (1u << OOT_QUEST_STONE_OF_AGONY)) {
                // Level 2: Quartz of Motion. Not an equippable item — it is used
                // from the kaleido (A on the Stone of Agony slot opens the
                // category list). Ownership is a save bit, read by
                // Rando_DesireCompass_IsOwned().
                Nei_Save()->quartzOwned = 1;
            } else {
                Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_STONE_OF_AGONY); // bit 21
            }
            break;
        case RI_OOT_GERUDO_MEMBERSHIP_CARD:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_GERUDO_CARD); // bit 22
            break;
        // Skijer's NEI — the 3 NEI custom songs live in the SAME parallel OoT quest store, on the
        // rows of the 3 truly-doubled OoT songs (verified against the reader in z_message.c:
        // Fugue = Epona row (MINUET+7 = bit 13), Command = Song-of-Time row (MINUET+10 = bit 16),
        // Ballad = Storms row (MINUET+11 = bit 17)). Free-play recognition + the quest-page playback
        // (gNeiCustomSongsAvailable) light up from these bits.
        case RI_OOT_SONG_FUGUE_OF_HOME:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_FUGUE_OF_HOME); // bit 13
            break;
        case RI_OOT_SONG_COMMAND_MELODY:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_COMMAND_MELODY); // bit 16
            break;
        case RI_OOT_SONG_BALLAD_OF_THE_HERO:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_BALLAD_OF_HERO); // bit 17
            break;
        // Skijer's NEI — MM-native songs whose melodies ALSO have a row on the mirrored OoT quest
        // page (reader: z_kaleido_collect.c, rows = ootQuestItems bits 12/14/15). Item_Give still
        // sets MM's native quest bit (z_parameter.c song branch) — the extra bit lights the OoT-page
        // icon and FleetSync OR-merges ootQuestItems to OoT. Epona/Time/Storms need NO bits here:
        // their OoT-page rows are the NEI custom songs (bits 13/16/17, see nei_save.h layout).
        case RI_SONG_LULLABY:
            // RI_SONG_LULLABY = MM's Goron Lullaby (2ship has no separate Zelda's-Lullaby RI);
            // per the unified-Lullaby design its give lights the page's Lullaby row (bit 12).
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_LULLABY);
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_SONG_SARIA:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_SARIA); // bit 14
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        case RI_SONG_SUN:
            Nei_Save()->ootQuestItems |= (1u << OOT_QUEST_SONG_SUN); // bit 15
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
        // Skijer's NEI — OoT (SoH) per-dungeon items (small keys, boss keys, maps, compasses, key rings) have no
        // MM gameplay effect: give is a no-op (draw + message only). MM has no OoT dungeon-item systems. Distinct
        // per-dungeon tokens are kept for cross-collection identity. Not progressive → no ConvertItem handling.
        case RI_OOT_BOSS_KEY_FIRE_TEMPLE:
        case RI_OOT_BOSS_KEY_FOREST_TEMPLE:
        case RI_OOT_BOSS_KEY_GANONS_CASTLE:
        case RI_OOT_BOSS_KEY_SHADOW_TEMPLE:
        case RI_OOT_BOSS_KEY_SPIRIT_TEMPLE:
        case RI_OOT_BOSS_KEY_WATER_TEMPLE:
        case RI_OOT_COMPASS_BOTTOM_OF_THE_WELL:
        case RI_OOT_COMPASS_DEKU_TREE:
        case RI_OOT_COMPASS_DODONGOS_CAVERN:
        case RI_OOT_COMPASS_FIRE_TEMPLE:
        case RI_OOT_COMPASS_FOREST_TEMPLE:
        case RI_OOT_COMPASS_ICE_CAVERN:
        case RI_OOT_COMPASS_JABU_JABUS_BELLY:
        case RI_OOT_COMPASS_SHADOW_TEMPLE:
        case RI_OOT_COMPASS_SPIRIT_TEMPLE:
        case RI_OOT_COMPASS_WATER_TEMPLE:
        case RI_OOT_KEY_RING_BOTTOM_OF_THE_WELL:
        case RI_OOT_KEY_RING_FIRE_TEMPLE:
        case RI_OOT_KEY_RING_FOREST_TEMPLE:
        case RI_OOT_KEY_RING_GANONS_CASTLE:
        case RI_OOT_KEY_RING_GERUDO_FORTRESS:
        case RI_OOT_KEY_RING_GERUDO_TRAINING_GROUND:
        case RI_OOT_KEY_RING_SHADOW_TEMPLE:
        case RI_OOT_KEY_RING_SPIRIT_TEMPLE:
        case RI_OOT_KEY_RING_TREASURE_GAME:
        case RI_OOT_KEY_RING_WATER_TEMPLE:
        case RI_OOT_MAP_BOTTOM_OF_THE_WELL:
        case RI_OOT_MAP_DEKU_TREE:
        case RI_OOT_MAP_DODONGOS_CAVERN:
        case RI_OOT_MAP_FIRE_TEMPLE:
        case RI_OOT_MAP_FOREST_TEMPLE:
        case RI_OOT_MAP_ICE_CAVERN:
        case RI_OOT_MAP_JABU_JABUS_BELLY:
        case RI_OOT_MAP_SHADOW_TEMPLE:
        case RI_OOT_MAP_SPIRIT_TEMPLE:
        case RI_OOT_MAP_WATER_TEMPLE:
        case RI_OOT_SMALL_KEY_BOTTOM_OF_THE_WELL:
        case RI_OOT_SMALL_KEY_FIRE_TEMPLE:
        case RI_OOT_SMALL_KEY_FOREST_TEMPLE:
        case RI_OOT_SMALL_KEY_GANONS_CASTLE:
        case RI_OOT_SMALL_KEY_GERUDO_FORTRESS:
        case RI_OOT_SMALL_KEY_GERUDO_TRAINING_GROUND:
        case RI_OOT_SMALL_KEY_SHADOW_TEMPLE:
        case RI_OOT_SMALL_KEY_SPIRIT_TEMPLE:
        case RI_OOT_SMALL_KEY_TREASURE_GAME:
        case RI_OOT_SMALL_KEY_WATER_TEMPLE:
        // Skijer's NEI — OoT page-0 gear/spells: real grants into the NeiSaveData fields that back the
        // OoT-layout kaleido page 0 (extended_inventory.c VSLOT_* cells) and that FleetSync ships to
        // OoT (inv.dins/farores/nayrus/boomerang/hammer). Obtaining in MM == obtaining in OoT.
        case RI_OOT_DINS_FIRE:
            Nei_Save()->ootSpellsOwned |= (1 << 0); // VSLOT_DINS
            break;
        case RI_OOT_FARORES_WIND:
            Nei_Save()->ootSpellsOwned |= (1 << 1); // VSLOT_FARORES
            break;
        case RI_OOT_NAYRUS_LOVE:
            Nei_Save()->ootSpellsOwned |= (1 << 2); // VSLOT_NAYRUS
            break;
        case RI_OOT_BOOMERANG:
            Nei_Save()->ootBoomerangOwned = 1; // VSLOT_OOT_BOOMERANG
            break;
        case RI_OOT_PROGRESSIVE_HAMMER:
            // FC 2-level chain (FCI_HAMMER): L1 Megaton Hammer (VSLOT_HAMMER cell), L2 Iron Knuckle's
            // Axe (weaponUpgrades bit 0 — behavior-only upgrade, same icon).
            if (!Nei_Save()->ootHammerOwned) {
                Nei_Save()->ootHammerOwned = 1;
            } else {
                WeaponUpgrade_SetHammerAxe(1);
            }
            break;
        case RI_OOT_PROGRESSIVE_STRENGTH: {
            // FC 3-level chain (FCI_STRENGTH): Goron Bracelet -> Silver Gauntlets -> Gold Gauntlets.
            // The level belongs in ootUpgrades strength@9 (Nei_StrengthLevel) — that is the field
            // that persists with the save, that the kaleido equipment page draws, and that FleetSync
            // publishes as upgrades["strength"] for OoT to apply as a real UPG_STRENGTH. Until now
            // the ONLY thing written here was comboObtained[FC_OOT_STRENGTH], which nothing reads on
            // either side (soh defines the index and never consumes it), so a strength upgrade
            // obtained in MM was silently lost in both games. comboObtained is still written for the
            // combo registry/tracker, but it is no longer the source of truth. Skijer's NEI
            u8 lvl = Nei_StrengthLevel();
            // FleetSync imports into the native field as well and exports max(native, ootUpgrades),
            // so start from whichever is higher — otherwise a level that arrived from OoT would be
            // re-granted from zero here.
            u8 native = (u8)CUR_UPG_VALUE(UPG_STRENGTH);
            if (native > lvl) {
                lvl = native;
            }
            if (lvl < NEI_STRENGTH_MAX) {
                lvl++;
            }
            Nei_SetStrengthLevel(lvl);
            Inventory_ChangeUpgrade(UPG_STRENGTH, lvl); // keep the native copy FleetSync exports in step
            Nei_Save()->comboObtained[FC_OOT_STRENGTH] = lvl;
            break;
        }
        case RI_OOT_PROGRESSIVE_BGS:
            // FC 2-level chain (FCI_BIGGORON_SWORD): L1 Biggoron Sword ownership (comboObtained
            // registry, same cell the ext-equipment kaleido reads), L2 upgrades it to the Great
            // Fairy's Sword via the weaponUpgrades bit — mirrors how Shipwright's weapon_upgrades.c
            // models BGS -> GFS. Before this the two were unrelated items, so "2 copies of the
            // progressive BGS" meant nothing: there was no upgrade to hand out. Skijer's NEI
            if (Nei_Save()->comboObtained[FC_OOT_SWORD_BIGGORON] == 0) {
                Nei_Save()->comboObtained[FC_OOT_SWORD_BIGGORON] = 1;
            } else {
                WeaponUpgrade_SetGreatFairy(1);
            }
            break;
        case RI_OOT_PROGRESSIVE_MASTER_SWORD:
            // FC 2-level chain (FCI_MASTER_SWORD): L1 base ownership (comboObtained registry — the
            // ext-equipment kaleido reads exactly this cell), L2 True Master Sword (weaponUpgrades bit 3).
            if (Nei_Save()->comboObtained[FC_OOT_SWORD_MASTER] == 0) {
                Nei_Save()->comboObtained[FC_OOT_SWORD_MASTER] = 1;
            } else {
                WeaponUpgrade_SetTrueMaster(1);
            }
            break;
        // OoT tunics/boots: ownership lives in the FC registry (comboObtained) — the ext-equipment
        // kaleido reads these cells for the vanilla Goron/Zora tunic and Iron/Hover boots columns
        // (z_kaleido_equipment.c), and FleetSync merges the array with OoT wholesale.
        case RI_OOT_GORON_TUNIC:
            Nei_Save()->comboObtained[FC_OOT_TUNIC_GORON] = 1;
            break;
        case RI_OOT_ZORA_TUNIC:
            Nei_Save()->comboObtained[FC_OOT_TUNIC_ZORA] = 1;
            break;
        case RI_OOT_IRON_BOOTS:
            Nei_Save()->comboObtained[FC_OOT_BOOTS_IRON] = 1;
            break;
        case RI_OOT_HOVER_BOOTS:
            Nei_Save()->comboObtained[FC_OOT_BOOTS_HOVER] = 1;
            break;
        case RI_OOT_DEKU_SHIELD:
            // OoT's Deku Shield: canonical ownership bit in the unified shield mask
            // (FleetComboIds.h FC_SHIELD_DEKU — "OoT native only"), carried by FleetSync.
            Nei_Save()->shieldOwned |= FC_SHIELD_DEKU;
            break;
        case RI_OOT_MIRROR_SHIELD:
            // OoT's Mirror Shield (NOT MM's): canonical ownership bit in the unified shield mask —
            // the exact bit ExtractShared/ApplyShared carry (FleetComboIds.h FC_SHIELD_MIRROR_OOT).
            Nei_Save()->shieldOwned |= FC_SHIELD_MIRROR_OOT;
            break;
        case RI_OOT_FISHING_POLE:
            // No MM inventory slot — ownership rides its FC registry cell (FC_FISHING_ROD), which
            // FleetSync merges both ways (same relative the OoT side uses).
            Nei_Save()->comboObtained[FC_FISHING_ROD] = 1;
            break;
        case RI_OOT_BOMBCHU_BAG:
            // MM relative: carrying bombchus. Grant the bombchu inventory item + 20 chus (ammo only
            // topped up, never reduced).
            INV_CONTENT(ITEM_BOMBCHU) = ITEM_BOMBCHU;
            if (AMMO(ITEM_BOMBCHU) < 20) {
                AMMO(ITEM_BOMBCHU) = 20;
            }
            break;
        // OoT child-trade masks with no MM native item: ownership bitmask nei->ootMasksOwned. Bit =
        // OoT child-mask order (Keaton 0, Skull 1, Spooky 2, Bunny 3, Goron 4, Zora 5, Gerudo 6,
        // Truth 7) — MM authors this field and FleetSync ships it verbatim (OoT echoes).
        case RI_OOT_MASK_SKULL:
            Nei_Save()->ootMasksOwned |= (1 << 1);
            break;
        case RI_OOT_MASK_SPOOKY:
            Nei_Save()->ootMasksOwned |= (1 << 2);
            break;
        case RI_OOT_MASK_GERUDO:
            Nei_Save()->ootMasksOwned |= (1 << 6);
            break;
        // Skijer's NEI page-2 customs: grant the REAL custom-inventory slot (NeiSaveData.ownedItems
        // via ExtInv_GiveItem — the same store FleetSync's ApplyShared fills when syncing from OoT
        // and the kaleido page 2 reads via ExtInv_GetSlotItem). Slot/item pairs from
        // extended_inventory.h / z64item.h.
        case RI_OOT_NEI_WHIP:
            ExtInv_GiveItem(SLOT_WHIP, ITEM_WHIP);
            break;
        case RI_OOT_NEI_SPINNER:
            ExtInv_GiveItem(SLOT_SPINNER, ITEM_SPINNER);
            break;
        // Skijer's NEI — Bomb Arrows owns no inventory cell any more (it is the bow's element flag),
        // so ExtInv_GiveItem into slot 27 would land on the Elemental Wand's cell. Set the flag.
        case RI_OOT_NEI_BOMB_ARROWS:
            Nei_Save()->bombArrowsOwned = 1;
            break;
        case RI_OOT_NEI_FIRE_ROD:
            ExtInv_GiveItem(SLOT_FIRE_ROD, ITEM_ROD_FIRE);
            break;
        case RI_OOT_NEI_DEMISE_DESTRUCTION:
            ExtInv_GiveItem(SLOT_DEMISE_DESTRUCTION, ITEM_DEMISE_DESTRUCTION);
            break;
        case RI_OOT_NEI_DEKU_LEAF:
            ExtInv_GiveItem(SLOT_DEKU_LEAF, ITEM_DEKU_LEAF);
            break;
        case RI_OOT_NEI_TIME_GATE:
            ExtInv_GiveItem(SLOT_TIME_GATE, ITEM_TIME_GATE);
            break;
        case RI_OOT_NEI_BEETLE:
            ExtInv_GiveItem(SLOT_BEETLE, ITEM_BEETLE);
            break;
        case RI_OOT_NEI_SWITCH_HOOK:
            ExtInv_GiveItem(SLOT_SWITCH_HOOK, ITEM_SWITCH_HOOK);
            break;
        case RI_OOT_NEI_ICE_ROD:
            ExtInv_GiveItem(SLOT_ICE_ROD, ITEM_ROD_ICE);
            break;
        case RI_OOT_NEI_ZONAI_PERMAFROST:
            ExtInv_GiveItem(SLOT_ZONAI_PERMAFROST, ITEM_ZONAI_PERMAFROST);
            break;
        case RI_OOT_NEI_MOGMA_MITTS:
            ExtInv_GiveItem(SLOT_MOGMA_MITTS, ITEM_MOGMA_MITTS);
            break;
        case RI_OOT_NEI_GUST_JAR:
            ExtInv_GiveItem(SLOT_GUST_JAR, ITEM_GUST_JAR);
            break;
        case RI_OOT_NEI_BALL_AND_CHAIN:
            ExtInv_GiveItem(SLOT_BALL_AND_CHAIN, ITEM_BALL_AND_CHAIN);
            break;
        case RI_OOT_NEI_DESIRE_SENSOR:
            // The standalone Desire Sensor is retired — SLOT_DESIRE_SENSOR is no
            // longer equippable. Any seed that still places it grants the Quartz
            // of Motion instead, so it never becomes a dead item in the pool.
            Nei_Save()->quartzOwned = 1;
            break;
        case RI_OOT_NEI_LIGHT_ROD:
            ExtInv_GiveItem(SLOT_LIGHT_ROD, ITEM_ROD_LIGHT);
            break;
        case RI_OOT_NEI_HYLIAS_GRACE:
            // RETIRED (user 2026-08-06): Hylia's Grace is gone as an item — its cell (41) now belongs
            // to the Phantom Hourglass and its noclip capability is slated to move into the Soul
            // spell (TODO: wire that transfer when the Soul spell work lands). Old seeds that still
            // place this RI get nothing rather than a dead cell. The RI stays defined (append-only).
            break;
        case RI_OOT_NEI_LANTERN:
            ExtInv_GiveItem(SLOT_LANTERN, ITEM_LANTERN);
            break;
        case RI_OOT_NEI_MINISH_CAP:
            ExtInv_GiveItem(SLOT_MINISH_CAP, ITEM_MINISH_CAP);
            break;
        case RI_OOT_NEI_POKE_BALL:
            // 2026-08-06 re-layout: the Pokeball left page 2 (cell 44 is the Shadow Crystal now) and
            // lives on the Broken Items equipment page, where it is the PIKACHU MODE form — that page
            // already uses the Pokeball as that form's icon. Ownership is a flag; the form selector
            // will gate on it (TODO: BrokenItems_FormCount/EquipForm gating). Skijer's NEI
            Nei_Save()->pokeballOwned = 1;
            break;
        // Dual Cane: six separate skills on ONE slot. Cane_GiveSkill lights that
        // skill's bit and, if this is the first one found, drops the cane itself
        // into SLOT_CANE_OF_SOMARIA — so any of the six can be the "first" pickup.
        // The BASE check is progressive: each copy lights the next unowned skill, in
        // the same order SoH uses (randomizer.cpp), so a seed can simply place six
        // copies of one item. The five explicit skill ids below stay available for
        // anything that wants to hand out one specific skill.
        case RI_OOT_NEI_CANE_OF_SOMARIA: {
            static const u8 kCaneOrder[6] = { CANE_SKILL_SOMARIA_STATUE,   CANE_SKILL_PACCI_FLIP,
                                              CANE_SKILL_SOMARIA_BLOCK,    CANE_SKILL_PACCI_STONE,
                                              CANE_SKILL_SOMARIA_PLATFORM, CANE_SKILL_PACCI_ULTRAHAND };
            for (int i = 0; i < 6; i++) {
                if (Cane_GiveSkill(kCaneOrder[i])) {
                    break;
                }
            }
            break;
        }
        case RI_OOT_NEI_CANE_SOMARIA_BLOCK:
            Cane_GiveSkill(CANE_SKILL_SOMARIA_BLOCK);
            break;
        case RI_OOT_NEI_CANE_SOMARIA_PLATFORM:
            Cane_GiveSkill(CANE_SKILL_SOMARIA_PLATFORM);
            break;
        case RI_OOT_NEI_CANE_PACCI_FLIP:
            Cane_GiveSkill(CANE_SKILL_PACCI_FLIP);
            break;
        case RI_OOT_NEI_CANE_PACCI_STONE:
            Cane_GiveSkill(CANE_SKILL_PACCI_STONE);
            break;
        case RI_OOT_NEI_CANE_PACCI_ULTRAHAND:
            Cane_GiveSkill(CANE_SKILL_PACCI_ULTRAHAND);
            break;
        // 2026-08-06 re-layout: Shovel and Dominion Rod SHARE cell 46 behind a wheel (the user's
        // page-2 layout: "Shovel <-> Dominion Rod"), freeing cell 47 for the Rod of Seasons. The
        // cell holds whichever of the two is in hand; ownership of each is its own flag, exactly
        // like the Power Keg on the bomb cell — a cell value cannot say "both owned". The give only
        // seeds the cell when it is EMPTY, so obtaining the second one never kicks the first out of
        // hand. Skijer's NEI
        case RI_OOT_NEI_SHOVEL:
            Nei_Save()->shovelOwned = 1;
            if (ExtInv_GetSlotItem(SLOT_SHOVEL) == ITEM_NONE) {
                ExtInv_GiveItem(SLOT_SHOVEL, ITEM_SHOVEL);
            }
            break;
        case RI_OOT_NEI_DOMINION_ROD:
            Nei_Save()->dominionOwned = 1;
            if (ExtInv_GetSlotItem(SLOT_SHOVEL) == ITEM_NONE) {
                ExtInv_GiveItem(SLOT_SHOVEL, ITEM_DOMINION_ROD);
            }
            break;
        // The four cells the 2026-08-06 re-layout opened up (39 / 41 / 44 / 47). Real, owned,
        // equippable page-2 items with icon + description; their gameplay behaviour is pending
        // (behaviorless by design — NOT placeholders that vanish). EXT (u16) ids: the u8 space is
        // exhausted, which is what the ownedItems widening was for. Skijer's NEI
        case RI_OOT_NEI_SHEIKAH_SLATE:
            ExtInv_GiveItem(SLOT_SHEIKAH_SLATE, EXT_ITEM_SHEIKAH_SLATE);
            break;
        // Sheikah Slate runes — sibling items over the slate cell (wand idiom). Each lights its
        // slateRunesOwned bit; the first one also hands over the slate itself (Slate_GrantRune).
        case RI_OOT_NEI_SLATE_RUNE_BOMB:
            Slate_GrantRune(SLATE_RUNE_BOMB);
            break;
        case RI_OOT_NEI_SLATE_RUNE_MASTER_CYCLE:
            Slate_GrantRune(SLATE_RUNE_MASTER_CYCLE);
            break;
        case RI_OOT_NEI_SLATE_RUNE_STASIS:
            Slate_GrantRune(SLATE_RUNE_STASIS);
            break;
        case RI_OOT_NEI_SLATE_RUNE_CRYONIS:
            Slate_GrantRune(SLATE_RUNE_CRYONIS);
            break;
        case RI_OOT_NEI_PHANTOM_HOURGLASS:
            ExtInv_GiveItem(SLOT_PHANTOM_HOURGLASS, EXT_ITEM_PHANTOM_HOURGLASS);
            break;
        case RI_OOT_NEI_SHADOW_CRYSTAL:
            ExtInv_GiveItem(SLOT_SHADOW_CRYSTAL, EXT_ITEM_SHADOW_CRYSTAL);
            break;
        case RI_OOT_NEI_ROD_OF_SEASONS:
            ExtInv_GiveItem(SLOT_ROD_OF_SEASONS, EXT_ITEM_ROD_OF_SEASONS);
            break;
        // Elemental Wand: whichever rod lands grants that mode AND the slot. In "Single item" mode
        // one pickup lights all six; in "Elemental shuffle" each rod is its own check. Wand_GrantMode
        // handles both, so the arms are identical by design.
        case RI_OOT_NEI_ELEMENTAL_WAND:
        case RI_OOT_NEI_WAND_SAND_ROD:
            Wand_GrantMode(WAND_MODE_SAND);
            break;
        case RI_OOT_NEI_WAND_TORNADO_ROD:
            Wand_GrantMode(WAND_MODE_TORNADO);
            break;
        case RI_OOT_NEI_WAND_WATER_ROD:
            Wand_GrantMode(WAND_MODE_WATER);
            break;
        case RI_OOT_NEI_WAND_METEOR_ROD:
            Wand_GrantMode(WAND_MODE_METEOR);
            break;
        case RI_OOT_NEI_WAND_STORM_ROD:
            Wand_GrantMode(WAND_MODE_STORM);
            break;
        case RI_OOT_NEI_WAND_SHADOW_SCEPTER:
            Wand_GrantMode(WAND_MODE_SCEPTER);
            break;
        case RI_OOT_PROGRESSIVE_ROC: {
            // FC 2-level chain (FCI_SKIJER_ROC): Roc's Feather (Skijer) -> Roc's Cape, sharing
            // SLOT_ROCS (the cape replaces the feather in the same cell).
            uint16_t curRoc = Nei_GetOwnedItem(SLOT_ROCS);
            if (curRoc == ITEM_ROCS_FEATHER_SKIJER || curRoc == ITEM_ROCS_CAPE) {
                ExtInv_GiveItem(SLOT_ROCS, ITEM_ROCS_CAPE);
            } else {
                ExtInv_GiveItem(SLOT_ROCS, ITEM_ROCS_FEATHER_SKIJER);
            }
            break;
        }
        // Deku stick / nut capacity, the same shape as the bomb bag above: bump the native upgrade
        // level and top the ammo up to the new capacity. The FC row is a 2-level chain (v1 has no
        // bag-gate), so level 1 of the native upgrade is assumed and these take it to 2 and then 3 —
        // sticks 20 -> 30, nuts 30 -> 40. Also makes sure the slot itself is owned, since MM's logic
        // gates on HAS_ITEM(ITEM_DEKU_STICK/NUT) and a capacity with no stick is useless.
        case RI_OOT_PROGRESSIVE_STICK_CAPACITY: {
            uint8_t lvl = CUR_UPG_VALUE(UPG_DEKU_STICKS);
            Inventory_ChangeUpgrade(UPG_DEKU_STICKS, lvl < 3 ? lvl + 1 : 3);
            INV_CONTENT(ITEM_DEKU_STICK) = ITEM_DEKU_STICK;
            AMMO(ITEM_DEKU_STICK) = CUR_CAPACITY(UPG_DEKU_STICKS);
            break;
        }
        case RI_OOT_PROGRESSIVE_NUT_CAPACITY: {
            uint8_t lvl = CUR_UPG_VALUE(UPG_DEKU_NUTS);
            Inventory_ChangeUpgrade(UPG_DEKU_NUTS, lvl < 3 ? lvl + 1 : 3);
            INV_CONTENT(ITEM_DEKU_NUT) = ITEM_DEKU_NUT;
            AMMO(ITEM_DEKU_NUT) = CUR_CAPACITY(UPG_DEKU_NUTS);
            break;
        }
        case RI_OOT_ROCS_FEATHER:
            // SoH's OTHER feather (RG_ROCS_FEATHER): the SHIP-VANILLA one, which lives in the Nayru's
            // Love cell and cycles with it. Nothing to do with Skijer's feather in SLOT_ROCS — the two
            // are separate items and each is obtained on its own.
            //
            // Must be NayrusWheel_Grant, not ExtInv_Set*SlotItem: those are the wheel's selection
            // writes and confer no ownership (otherwise cycling the wheel would hand you the item).
            // ExtInv_GiveItem is no good either — it silently ignores anything outside slots 24..47,
            // and the OoT virtual slots live at 72+. Skijer's NEI
            NayrusWheel_Grant(ITEM_ROCS_FEATHER);
            break;
        // Extended equipment: the extEquipOwnedBits ownership bit (16 + type*3 + index-1) via the
        // canonical setter — the ext-equipment kaleido grid and FleetSync both read these bits.
        // Grid map (extended_equipment.c): SWORD Byrna/FourSword/Drillshaft, SHIELD Divine/Kite
        // (=Sheikah)/Ikana, TUNIC Champion/Spirit/Sage's, BOOTS Anklet/Pendant/Scale.
        // The Magic Cape now has dedicated ownership outside the tunic grid.
        case RI_OOT_EXT_CANE_OF_BYRNA:
            ExtEquip_GiveItem(EQUIP_TYPE_SWORD, 1); // bit 16
            break;
        case RI_OOT_EXT_FOUR_SWORD:
            ExtEquip_GiveItem(EQUIP_TYPE_SWORD, 2); // bit 17
            break;
        // The last three cells of the grid. They had playable behaviours in both games but no
        // randomizer identity at all, so the save editor was the only way to own them — and with no
        // id there was nothing for FleetSync to carry across either. Skijer's NEI
        case RI_OOT_EXT_TRIDENT:
            ExtEquip_GiveItem(EQUIP_TYPE_SWORD, 3); // bit 18
            break;
        case RI_OOT_EXT_CLIMB_BOOTS:
            ExtEquip_GiveItem(EQUIP_TYPE_BOOTS, 2); // bit 26
            break;
        case RI_OOT_EXT_ROC_BOOTS:
            ExtEquip_GiveItem(EQUIP_TYPE_BOOTS, 3); // bit 27
            break;
        case RI_OOT_EXT_DIVINE_SHIELD:
            ExtEquip_GiveItem(EQUIP_TYPE_SHIELD, 1);      // bit 19
            Nei_Save()->shieldOwned |= FC_SHIELD_DIVINE;  // unified shield mask stays coherent
            break;
        case RI_OOT_EXT_SHEIKAH_SHIELD:
            ExtEquip_GiveItem(EQUIP_TYPE_SHIELD, 2);    // bit 20 (Kite Shield slot)
            Nei_Save()->shieldOwned |= FC_SHIELD_KITE;  // unified shield mask stays coherent
            break;
        case RI_OOT_EXT_MAGIC_CAPE:
            ExtEquip_GiveCape();
            break;
        case RI_OOT_EXT_SPIRIT_BREASTPLATE:
            ExtEquip_GiveItem(EQUIP_TYPE_TUNIC, 2); // bit 23
            break;
        case RI_OOT_EXT_CHAMPIONS_TUNIC:
            ExtEquip_GiveItem(EQUIP_TYPE_TUNIC, 1);
            break;
        case RI_OOT_EXT_PEGASUS_ANKLET:
            ExtEquip_GiveItem(EQUIP_TYPE_BOOTS, 1); // bit 25
            break;
        case RI_OOT_EXT_WATER_DRAGON_SCALE:
            // Legacy RI name retained for serialized FleetCombo compatibility.
            ExtEquip_GiveItem(EQUIP_TYPE_TUNIC, 3); // Sage's
            break;
        // Skijer's NEI bottle randomizer — Net + Bottomless Bottle (Task: cross-plaçable natives).
        case RI_NET:
            // netEquipped: the per-frame enforcer (mm_bottle_items.cpp) projects ITEM_NET into
            // SLOT_BOTTLE_3; FleetSync ships it as inv["net"].
            Bottle_SetNetOwned(1);
            break;
        case RI_BOTTOMLESS_BOTTLE:
            // bottomlessBottleMode (+ start as an EMPTY Bottomless Bottle so the counter/content are
            // sane); enforcer projects SLOT_BOTTLE_4; FleetSync ships bottomlessMode/content/count.
            Bottle_SetBottomlessOwned(1);
            Bottle_BottomlessEmpty();
            break;
        // Genuinely storeless in MM (verified): give stays a no-op — draw + message only; the FC
        // record hook still counts the pickup in comboObtainedFc (the shared cross-game registry),
        // which IS their canonical ownership for the combo (Greg goal check / Skeleton Key pending).
        case RI_OOT_GREG:
        case RI_OOT_SKELETON_KEY:
        // Third wave storeless items (verified): Climb/Crawl have no MM movement-gate system, the
        // jabber nuts have no MM speak system, Ruto's Letter has no MM letter content (the custom
        // bottle system carries no letter), and the OoT GS Token has no MM store — its count crosses
        // via comboObtainedFc and OoT grants the real tokens on arrival.
        case RI_OOT_ABILITY_CHESTS:
        case RI_OOT_ABILITY_CLIMB:
        case RI_OOT_ABILITY_CRAWL:
        case RI_OOT_SPEAK_DEKU:
        case RI_OOT_SPEAK_GERUDO:
        case RI_OOT_SPEAK_GORON:
        case RI_OOT_SPEAK_HYLIAN:
        case RI_OOT_SPEAK_KOKIRI:
        case RI_OOT_SPEAK_ZORA:
        case RI_OOT_GS_TOKEN:
        case RI_OOT_RUTOS_LETTER:
        case RI_JUNK:
        case RI_NONE:
            break;
        default:
            Item_Give(gPlayState, Rando::StaticData::Items[randoItemId].itemId);
            break;
    }
}
