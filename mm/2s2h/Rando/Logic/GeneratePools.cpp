#include "Logic.h"
#include "Rando/MiscBehavior/ClockShuffle.h"
#include "2s2h/FleetShipCombo/FleetShipCombo.h" // FleetShipCombo_GetActiveGame (combo-only items)
#include <libultraship/bridge/consolevariablebridge.h>

extern "C" {
#include "variables.h"
#include "ShipUtils.h"
}

namespace Rando {

namespace Logic {

void GeneratePools(RandoSaveInfo& saveInfo, std::vector<RandoCheckId>& checkPool, std::vector<RandoItemId>& itemPool) {
    std::vector<RandoItemId> startingItems = Rando::GetStartingItemsFromSave(saveInfo);
    std::vector<RandoItemId> computedStartingItems = Rando::GetComputedStartingItems(saveInfo);
    startingItems.insert(startingItems.end(), computedStartingItems.begin(), computedStartingItems.end());

    std::vector<RandoCheckId> excludedChecks = Rando::GetExcludedChecksFromConfig();

    std::set<RandoCheckId> vanillaSkulltulas;
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_GOLD_SKULLTULAS] == RO_GENERIC_YES) {
        std::map<SceneId, std::vector<RandoCheckId>> skulltulasByScene;
        for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
            if (randoStaticCheck.randoCheckType == RCTYPE_SKULL_TOKEN) {
                skulltulasByScene[randoStaticCheck.sceneId].push_back(randoCheckId);
            }
        }

        size_t shuffledCount = saveInfo.randoSaveOptions[RO_SKULLTULA_SHUFFLED];
        for (auto& [sceneId, sceneSkulltulas] : skulltulasByScene) {
            if (shuffledCount >= sceneSkulltulas.size()) {
                continue;
            }

            for (size_t i = 0; i < sceneSkulltulas.size(); i++) {
                std::swap(sceneSkulltulas[i], sceneSkulltulas[Ship_Random(0, sceneSkulltulas.size())]);
            }
            vanillaSkulltulas.insert(sceneSkulltulas.begin() + shuffledCount, sceneSkulltulas.end());
        }
    }

    // First loop through all regions and add checks/items to the pool
    for (auto& [randoRegionId, randoRegion] : Rando::Logic::Regions) {
        for (auto& [randoCheckId, _] : randoRegion.checks) {
            auto& randoStaticCheck = Rando::StaticData::Checks[randoCheckId];

            // Initialize the check with it's vanilla item
            if (randoStaticCheck.randoCheckId != RC_UNKNOWN) {
                saveInfo.randoSaveChecks[randoCheckId].randoItemId = randoStaticCheck.randoItemId;
            }

            // Skip checks that are already in the pool
            if (std::find(checkPool.begin(), checkPool.end(), randoCheckId) != checkPool.end()) {
                continue;
            }

            // TODO: We may never shuffle these 2 pots, leaving this decision for later
            if (randoStaticCheck.sceneId == SCENE_LAST_BS) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_SKULL_TOKEN) {
                if (saveInfo.randoSaveOptions[RO_SHUFFLE_GOLD_SKULLTULAS] == RO_GENERIC_NO) {
                    continue;
                }

                if (vanillaSkulltulas.contains(randoCheckId)) {
                    saveInfo.randoSaveChecks[randoCheckId].shuffled = true;
                    continue;
                }
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_OWL &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_OWL_STATUES] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_POT &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_POT_DROPS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_CRATE &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_CRATE_DROPS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_BARREL &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_BARREL_DROPS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_GRASS &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_GRASS_DROPS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_BEEHIVE &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_HIVE_DROPS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_TREE &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_TREE_DROPS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_BUTTERFLY &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_BUTTERFLIES] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_FREESTANDING &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_FREESTANDING_ITEMS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_SNOWBALL &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_SNOWBALL_DROPS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_FROG &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_FROGS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_REMAINS &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_BOSS_REMAINS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_COW &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_COWS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_ENEMY_DROP &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_ENEMY_DROPS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_WONDER_ITEM &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_WONDER_ITEMS] == RO_GENERIC_NO) {
                continue;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_TINGLE_SHOP &&
                saveInfo.randoSaveOptions[RO_SHUFFLE_TINGLE_SHOPS] == RO_GENERIC_NO) {
                continue;
            } else {
                int price = Ship_Random(0, 200);
                saveInfo.randoSaveChecks[randoCheckId].price = price;
            }

            if (randoStaticCheck.randoCheckType == RCTYPE_SHOP) {
                // We always want shuffle RC_CURIOSITY_SHOP_SPECIAL_ITEM, RC_BOMB_SHOP_ITEM_03 &
                // RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM
                if (saveInfo.randoSaveOptions[RO_SHUFFLE_SHOPS] == RO_GENERIC_NO &&
                    randoCheckId != RC_CURIOSITY_SHOP_SPECIAL_ITEM && randoCheckId != RC_BOMB_SHOP_ITEM_03 &&
                    randoCheckId != RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM) {
                    continue;
                } else {
                    // We may come up with a better solution for this in the future, but for now we choose a
                    // random price ahead of time, logic will account for whatever price we choose
                    int price = Ship_Random(0, 200);
                    saveInfo.randoSaveChecks[randoCheckId].price = price;
                }
            }

            // Excluded checks are always left out of the check pool, but what happens to them depends on what their
            // vanilla item is worth. A check whose vanilla item is junk (grass, pots, snowballs...) has nothing worth
            // preserving, so it stays unshuffled and behaves exactly like it does in vanilla, and its item never enters
            // the item pool. Every other check keeps its item in the pool and is marked as skipped with junk in its
            // place. That leaves an inbalance in the pools that will get sorted automatically if there is enough space.
            if (saveInfo.randoSaveOptions[RO_LOGIC] != RO_LOGIC_VANILLA) {
                if (std::binary_search(excludedChecks.begin(), excludedChecks.end(), randoCheckId)) {
                    if (Rando::StaticData::Items[randoStaticCheck.randoItemId].randoItemType != RITYPE_JUNK) {
                        itemPool.push_back(randoStaticCheck.randoItemId);

                        saveInfo.randoSaveChecks[randoCheckId].shuffled = true;
                        saveInfo.randoSaveChecks[randoCheckId].randoItemId = RI_JUNK;
                        saveInfo.randoSaveChecks[randoCheckId].skipped = true;
                    }
                    continue;
                }
            }

            checkPool.emplace_back(randoCheckId);
            itemPool.push_back(randoStaticCheck.randoItemId);
        }
    }

    // Add sword and shield to the pool because they don't have a vanilla location, if you are starting with
    // them they will be removed from the pool in the next step
    itemPool.push_back(RI_PROGRESSIVE_SWORD);
    itemPool.push_back(RI_SHIELD_HERO);

    // BOTTLES: 8, one per slot of the bottle system (NeiSaveData.bottleSlots[8] = two kaleido cells,
    // each a wheel over 4 slots). MM's own checks only supply 6, so two more are added here.
    //
    // Empty ones on purpose: MM's logic never asks for a CONTENT, only for HAS_BOTTLE — the
    // HAS_BOTTLE_ITEM(item) macro exists but is used nowhere — and an empty bottle can be filled from
    // any source in the world. In a combo this block is skipped: there the 8 come from the shared
    // pool and are split across both worlds (bottleSlots is synced by FleetSync, so the two games
    // share ONE 8-slot inventory and adding more here would overflow it and silently lose bottles).
    if (FleetShipCombo_GetActiveGame() < 0) {
        itemPool.push_back(RI_BOTTLE_EMPTY);
        itemPool.push_back(RI_BOTTLE_EMPTY);
    }

    // Combo (OoT+MM): the Progressive Master Sword is fully implemented on this side already (item
    // table, GiveItem, DrawItem) but nothing ever put it in MM's pool, so the combo's per-game filter
    // saw it in NEITHER pool and dropped it from cross-placement entirely — it never appeared in a
    // single seed. Two copies = the chain's two levels (Master -> True Master).
    //
    // Gated on the combo actually being active so a solo-MM randomizer never offers an OoT-only
    // sword. This is also true while the host drives the oracle (manifest / fillTurn), which is
    // exactly when the pool has to contain it. Skijer's NEI
    if (FleetShipCombo_GetActiveGame() >= 0) {
        itemPool.push_back(RI_OOT_PROGRESSIVE_MASTER_SWORD); // L1 Master
        itemPool.push_back(RI_OOT_PROGRESSIVE_MASTER_SWORD); // L2 True Master
        // BGS chain. The standalone Great Fairy's Sword (vanilla item of RC_IKANA_GREAT_FAIRY) is a
        // SECOND source of the very thing the chain's level 2 grants, so with both in the pool the
        // sword can be obtained without ever touching the chain — and it showed up in the combo's
        // unshared report as an item with no FC row. Swap that one entry for the chain's L1 instead of
        // deleting it, so the pool keeps exactly as many items as there are checks.
        auto gfs = std::find(itemPool.begin(), itemPool.end(), RI_GREAT_FAIRY_SWORD);
        if (gfs != itemPool.end()) {
            *gfs = RI_OOT_PROGRESSIVE_BGS;                   // L1 Biggoron Sword (replaces the loose GFS)
        } else {
            itemPool.push_back(RI_OOT_PROGRESSIVE_BGS);      // GFS check not shuffled: supply L1 ourselves
        }
        itemPool.push_back(RI_OOT_PROGRESSIVE_BGS);          // L2 Great Fairy's Sword

        // The only OoT song with no MM counterpart, so it could never cross until now.
        itemPool.push_back(RI_OOT_SONG_ZELDAS_LULLABY);

        // SoH's ship-vanilla Roc's Feather (Nayru's Love slot). Distinct check from the progressive
        // Skijer Roc — that one is a NEI custom item and is pooled in the NEI block further down,
        // NOT here. (An FC row only maps the item for cross-game transport; it never puts anything in
        // a pool, so having one was never enough to make the Skijer Roc placeable.)
        itemPool.push_back(RI_OOT_ROCS_FEATHER);

        // THE 3 DUPLICATE SONGS BECOME THE 3 NEI CUSTOMS. Epona's Song, the Song of Time and the Song
        // of Storms exist in BOTH games, so in a combo two of each is a wasted item. NEI's answer is
        // to give MM's side three songs of its own instead, and the MM quest page in OoT already draws
        // them in exactly those three rows (sMmPageSongs, z_kaleido_collect.c).
        //
        // Done as a POOL SWAP, not by editing the checks table: a check's randoItemId is its VANILLA
        // item (where the copy comes from), not what it will hand out, and rewriting it would change
        // solo-MM randos too. Swapping keeps the pool exactly as long as the check list. OoT keeps its
        // own Epona/Time/Storms — they are only dropped from MM's side. Skijer's NEI
        const std::pair<RandoItemId, RandoItemId> kComboSongSwaps[] = {
            { RI_SONG_EPONA, RI_OOT_SONG_FUGUE_OF_HOME },
            { RI_SONG_TIME, RI_OOT_SONG_COMMAND_MELODY },
            { RI_SONG_STORMS, RI_OOT_SONG_BALLAD_OF_THE_HERO },
        };
        for (auto& [dup, custom] : kComboSongSwaps) {
            auto it = std::find(itemPool.begin(), itemPool.end(), dup);
            if (it != itemPool.end()) {
                *it = custom;
            }
        }

        // Deku stick / nut capacity: 2 levels each, matching the FC chain (v1 has no bag-gate).
        // MM has the upgrades natively, they just had no item.
        itemPool.push_back(RI_OOT_PROGRESSIVE_STICK_CAPACITY);
        itemPool.push_back(RI_OOT_PROGRESSIVE_STICK_CAPACITY);
        itemPool.push_back(RI_OOT_PROGRESSIVE_NUT_CAPACITY);
        itemPool.push_back(RI_OOT_PROGRESSIVE_NUT_CAPACITY);

        // OoT's "can open chests" skill. Termina has no such gate, so it does nothing here — it just
        // needs to be findable in MM, the same way Climb and Crawl are. 2 copies = OoT's pool count.
        itemPool.push_back(RI_OOT_ABILITY_CHESTS);
        itemPool.push_back(RI_OOT_ABILITY_CHESTS);
        itemPool.push_back(RI_OOT_PROGRESSIVE_STRENGTH);     // L1 Goron Bracelet
        itemPool.push_back(RI_OOT_PROGRESSIVE_STRENGTH);     // L2 Silver Gauntlets
        itemPool.push_back(RI_OOT_PROGRESSIVE_STRENGTH);     // L3 Gold Gauntlets
    }

    // ── 2026-08-06: cross-game categories for SOLO MM — user requirement #1 ──────────────────────
    // Each game must be able to offer the other's items WITHOUT the combo. These blocks are the
    // standalone half: they only run when the combo is INACTIVE (in combo the block above plus the
    // FC delegation already supply these items — running both would double-supply the pool, which is
    // exactly the "expected=2 got=4" class of bug the combo fill fought before). No swaps here, only
    // additions: the swap idiom above (songs, GFS) encodes COMBO decisions and must not leak into a
    // standalone MM seed. None of these are referenced by MM logic, so they place as extras — a seed
    // can never become unbeatable through them. All four options default OFF. Skijer's NEI
    if (FleetShipCombo_GetActiveGame() < 0) {
        if (saveInfo.randoSaveOptions[RO_SHUFFLE_OOT_GEAR] == RO_GENERIC_YES) {
            itemPool.push_back(RI_OOT_PROGRESSIVE_MASTER_SWORD); // L1 Master
            itemPool.push_back(RI_OOT_PROGRESSIVE_MASTER_SWORD); // L2 True Master
            itemPool.push_back(RI_OOT_PROGRESSIVE_BGS);          // L1 Biggoron Sword
            itemPool.push_back(RI_OOT_PROGRESSIVE_BGS);          // L2 Great Fairy's Sword upgrade
            itemPool.push_back(RI_OOT_PROGRESSIVE_STICK_CAPACITY);
            itemPool.push_back(RI_OOT_PROGRESSIVE_STICK_CAPACITY);
            itemPool.push_back(RI_OOT_PROGRESSIVE_NUT_CAPACITY);
            itemPool.push_back(RI_OOT_PROGRESSIVE_NUT_CAPACITY);
            itemPool.push_back(RI_OOT_ABILITY_CHESTS);
            itemPool.push_back(RI_OOT_PROGRESSIVE_STRENGTH); // L1 Goron Bracelet
            itemPool.push_back(RI_OOT_PROGRESSIVE_STRENGTH); // L2 Silver Gauntlets
            itemPool.push_back(RI_OOT_PROGRESSIVE_STRENGTH); // L3 Gold Gauntlets
            itemPool.push_back(RI_OOT_ROCS_FEATHER);         // ship-vanilla feather (Nayru's slot)
        }
        if (saveInfo.randoSaveOptions[RO_SHUFFLE_OOT_EQUIPMENT] == RO_GENERIC_YES) {
            itemPool.push_back(RI_OOT_EXT_CANE_OF_BYRNA);
            itemPool.push_back(RI_OOT_EXT_FOUR_SWORD);
            itemPool.push_back(RI_OOT_EXT_TRIDENT);
            itemPool.push_back(RI_OOT_EXT_DIVINE_SHIELD);
            itemPool.push_back(RI_OOT_EXT_SHEIKAH_SHIELD); // Kite Shield
            itemPool.push_back(RI_OOT_EXT_CHAMPIONS_TUNIC);
            itemPool.push_back(RI_OOT_EXT_SPIRIT_BREASTPLATE); // Magic Tunic
            itemPool.push_back(RI_OOT_EXT_WATER_DRAGON_SCALE); // Sage's Tunic
            itemPool.push_back(RI_OOT_EXT_PEGASUS_ANKLET);
            itemPool.push_back(RI_OOT_EXT_CLIMB_BOOTS);
            itemPool.push_back(RI_OOT_EXT_ROC_BOOTS);
            itemPool.push_back(RI_OOT_EXT_MAGIC_CAPE);
            itemPool.push_back(RI_OOT_PROGRESSIVE_ROC); // Skijer Roc: L1 Feather
            itemPool.push_back(RI_OOT_PROGRESSIVE_ROC); // L2 Cape
        }
        if (saveInfo.randoSaveOptions[RO_SHUFFLE_OOT_QUEST] == RO_GENERIC_YES) {
            itemPool.push_back(RI_OOT_MEDALLION_FOREST);
            itemPool.push_back(RI_OOT_MEDALLION_FIRE);
            itemPool.push_back(RI_OOT_MEDALLION_WATER);
            itemPool.push_back(RI_OOT_MEDALLION_SPIRIT);
            itemPool.push_back(RI_OOT_MEDALLION_SHADOW);
            itemPool.push_back(RI_OOT_MEDALLION_LIGHT);
            itemPool.push_back(RI_OOT_STONE_KOKIRI_EMERALD);
            itemPool.push_back(RI_OOT_STONE_GORON_RUBY);
            itemPool.push_back(RI_OOT_STONE_ZORA_SAPPHIRE);
            itemPool.push_back(RI_OOT_STONE_OF_AGONY);
            itemPool.push_back(RI_OOT_SONG_ZELDAS_LULLABY);
            itemPool.push_back(RI_OOT_SONG_MINUET_OF_FOREST);
            itemPool.push_back(RI_OOT_SONG_BOLERO_OF_FIRE);
            itemPool.push_back(RI_OOT_SONG_SERENADE_OF_WATER);
            itemPool.push_back(RI_OOT_SONG_REQUIEM_OF_SPIRIT);
            itemPool.push_back(RI_OOT_SONG_NOCTURNE_OF_SHADOW);
            itemPool.push_back(RI_OOT_SONG_PRELUDE_OF_LIGHT);
            itemPool.push_back(RI_OOT_SONG_FUGUE_OF_HOME);
            itemPool.push_back(RI_OOT_SONG_COMMAND_MELODY);
            itemPool.push_back(RI_OOT_SONG_BALLAD_OF_THE_HERO);
        }
        if (saveInfo.randoSaveOptions[RO_SHUFFLE_OOT_MASKS] == RO_GENERIC_YES) {
            itemPool.push_back(RI_OOT_MASK_SKULL);
            itemPool.push_back(RI_OOT_MASK_SPOOKY);
            itemPool.push_back(RI_OOT_MASK_GERUDO);
        }
    }

    // Add other items that don't have a vanilla location like Sun's Song or Song of Double Time

    // Boss Souls
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_BOSS_SOULS] == RO_GENERIC_YES) {
        for (int i = RI_SOUL_BOSS_GOHT; i <= RI_SOUL_BOSS_TWINMOLD; i++) {
            if (i == RI_SOUL_BOSS_MAJORA && saveInfo.randoSaveOptions[RO_SHUFFLE_TRIFORCE_PIECES] == RO_GENERIC_YES) {
                continue;
            }
            itemPool.push_back((RandoItemId)i);
        }
    }

    // Enemy Souls
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_ENEMY_SOULS] == RO_GENERIC_YES) {
        for (int i = RI_SOUL_ENEMY_ALIEN; i <= RI_SOUL_ENEMY_WOLFOS; i++) {
            itemPool.push_back((RandoItemId)i);
        }
    }

    // Shuffle Time
    if (saveInfo.randoSaveOptions[RO_CLOCK_SHUFFLE] == RO_GENERIC_YES) {
        auto clockShuffleMode = saveInfo.randoSaveOptions[RO_CLOCK_SHUFFLE_PROGRESSIVE];

        if (clockShuffleMode == RO_CLOCK_SHUFFLE_RANDOM) {
            itemPool.push_back(RI_TIME_DAY_1);
            itemPool.push_back(RI_TIME_NIGHT_1);
            itemPool.push_back(RI_TIME_DAY_2);
            itemPool.push_back(RI_TIME_NIGHT_2);
            itemPool.push_back(RI_TIME_DAY_3);
            itemPool.push_back(RI_TIME_NIGHT_3);
        } else {
            for (int i = 0; i < ClockItems::HALF_COUNT; ++i) {
                itemPool.push_back(RI_TIME_PROGRESSIVE);
            }
        }
    }

    // Abilities
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_SWIM] == RO_GENERIC_YES) {
        itemPool.push_back(RI_ABILITY_SWIM);
    }

    // Ocarina Buttons
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_OCARINA_BUTTONS] == RO_GENERIC_YES) {
        for (int i = RI_OCARINA_BUTTON_A; i <= RI_OCARINA_BUTTON_C_UP; i++) {
            itemPool.push_back((RandoItemId)i);
        }
    }

    // Songs
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_SONG_SUN] == RO_GENERIC_YES) {
        itemPool.push_back(RI_SONG_SUN);
    }
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_SONG_DOUBLE_TIME] == RO_GENERIC_YES) {
        itemPool.push_back(RI_SONG_DOUBLE_TIME);
    }
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_SONG_INVERTED_TIME] == RO_GENERIC_YES) {
        itemPool.push_back(RI_SONG_INVERTED_TIME);
    }
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_SONG_SARIA] == RO_GENERIC_YES) {
        itemPool.push_back(RI_SONG_SARIA);
    }

    // Tycoon's Wallet
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_TYCOON_WALLET] == RO_GENERIC_YES) {
        itemPool.push_back(RI_PROGRESSIVE_WALLET);
    }

    // Shuffle Triforce Pieces into the Pool
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_TRIFORCE_PIECES] == RO_GENERIC_YES) {
        int piecesToShuffle = saveInfo.randoSaveOptions[RO_TRIFORCE_PIECES_MAX];
        while (piecesToShuffle) {
            itemPool.push_back(RI_TRIFORCE_PIECE);
            piecesToShuffle--;
        }
    }

    // Shuffle the Skeleton Key into the Pool, unless starting with small keys
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_SKELETON_KEY] == RO_GENERIC_YES &&
        saveInfo.randoSaveOptions[RO_PLACEMENT_SMALL_KEYS] != RO_DUNGEON_ITEM_START_WITH) {
        itemPool.push_back(RI_SKELETON_KEY);
    }

    // Remove extra stray fairies from the pool.
    std::map<RandoItemId, int> removeAbleItemsInPool = {
        { RI_STONE_TOWER_STRAY_FAIRY, 0 },
        { RI_GREAT_BAY_STRAY_FAIRY, 0 },
        { RI_SNOWHEAD_STRAY_FAIRY, 0 },
        { RI_WOODFALL_STRAY_FAIRY, 0 },
    };
    for (RandoItemId itemId : itemPool) {
        if (removeAbleItemsInPool.find(itemId) != removeAbleItemsInPool.end()) {
            removeAbleItemsInPool[itemId]++;
        }
    }
    for (auto& [itemId, count] : removeAbleItemsInPool) {
        int max = 0;
        switch (itemId) {
            case RI_STONE_TOWER_STRAY_FAIRY:
            case RI_GREAT_BAY_STRAY_FAIRY:
            case RI_SNOWHEAD_STRAY_FAIRY:
            case RI_WOODFALL_STRAY_FAIRY:
                max = saveInfo.randoSaveOptions[RO_STRAY_FAIRIES_MAX];
                break;
            default:
                break;
        }

        while (count > max) {
            auto it = std::find(itemPool.begin(), itemPool.end(), itemId);
            if (it != itemPool.end()) {
                itemPool.erase(it);
                count--;
            } else {
                break;
            }
        }
    }

    // Remove starting items from the pool (but only one per entry in startingItems)
    for (RandoItemId startingItem : startingItems) {
        auto it = std::find(itemPool.begin(), itemPool.end(), startingItem);
        if (it != itemPool.end()) {
            itemPool.erase(it);
        }
    }

    // Adjust Heart Pieces based on starting health
    if (saveInfo.randoSaveOptions[RO_STARTING_HEALTH] < 3) {
        // Add up to 8 Heart Pieces
        int piecesToAdd = 4 * (3 - saveInfo.randoSaveOptions[RO_STARTING_HEALTH]);
        while (piecesToAdd) {
            itemPool.emplace_back(RI_HEART_PIECE);
            piecesToAdd--;
        }
    } else if (saveInfo.randoSaveOptions[RO_STARTING_HEALTH] > 3) {
        // Remove up to 52 Heart Pieces
        int piecesToRemove = 4 * (saveInfo.randoSaveOptions[RO_STARTING_HEALTH] - 3);
        while (piecesToRemove) {
            auto it = std::find(itemPool.begin(), itemPool.end(), RI_HEART_PIECE);
            if (it != itemPool.end()) {
                itemPool.erase(it);
            } else {
                break;
            }
            piecesToRemove--;
        }
    }

    // Plentiful
    if (saveInfo.randoSaveOptions[RO_PLENTIFUL_ITEMS] == RO_GENERIC_YES) {
        std::vector<RandoItemId> plentifulItems;
        for (size_t i = 0; i < itemPool.size(); i++) {
            // The user can specify exactly how many pieces they want to shuffle, so skip those
            if (itemPool[i] == RI_TRIFORCE_PIECE) {
                continue;
            }
            if (itemPool[i] == RI_SKELETON_KEY) {
                continue;
            }

            switch (Rando::StaticData::Items[itemPool[i]].randoItemType) {
                case RITYPE_BOSS_KEY:
                case RITYPE_SMALL_KEY:
                case RITYPE_MASK:
                case RITYPE_MAJOR:
                    plentifulItems.push_back(itemPool[i]);
                    break;
                case RITYPE_LESSER:
                    if (Rando::StaticData::Items[itemPool[i]].itemId != ITEM_TINGLE_MAP &&
                        Rando::StaticData::Items[itemPool[i]].itemId != ITEM_DUNGEON_MAP &&
                        Rando::StaticData::Items[itemPool[i]].itemId != ITEM_COMPASS) {
                        plentifulItems.push_back(itemPool[i]);
                    }
                    break;
                case RITYPE_HEALTH:
                case RITYPE_JUNK:
                default:
                    break;
            }
        }

        for (RandoItemId plentifulItem : plentifulItems) {
            itemPool.push_back(plentifulItem);
        }
    }

    // Traps
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_TRAPS] == RO_GENERIC_YES) {
        int trapsToShuffle = saveInfo.randoSaveOptions[RO_TRAP_AMOUNT];
        while (trapsToShuffle) {
            itemPool.push_back(RI_TRAP);
            trapsToShuffle--;
        }
    }

    // ── Skijer's NEI custom items ────────────────────────────────────────────────────────────────
    // These have had give / draw / icon plumbing for a long time but were NEVER reachable: nothing
    // in Checks.cpp points a location's randoItemId at an RI_OOT_NEI_*, and this file had no block
    // for them, so generating a seed could not place a single one. This is that missing block —
    // the mirror of soh's `if (ctx->GetOption(RSK_SKIJER_CUSTOM_ITEMS))` in item_pool.cpp.
    //
    // Off by default: turning it on by default would silently change every existing seed.
    if (saveInfo.randoSaveOptions[RO_SHUFFLE_NEI_ITEMS] == RO_GENERIC_YES) {
        // The page-2 items, one copy each.
        // 2026-08-06 re-layout: Hylia's Grace is OUT (item retired outright — its RI gives nothing
        // now); the four new page-2 cells are IN as behaviorless-but-real items.
        static const RandoItemId sNeiPoolItems[] = {
            RI_OOT_NEI_BALL_AND_CHAIN, RI_OOT_NEI_BEETLE,        RI_OOT_NEI_DEKU_LEAF,
            RI_OOT_NEI_DEMISE_DESTRUCTION, RI_OOT_NEI_DESIRE_SENSOR, RI_OOT_NEI_DOMINION_ROD,
            RI_OOT_NEI_FIRE_ROD,       RI_OOT_NEI_GUST_JAR,      RI_OOT_NEI_ICE_ROD,
            RI_OOT_NEI_LANTERN,        RI_OOT_NEI_LIGHT_ROD,     RI_OOT_NEI_MINISH_CAP,
            RI_OOT_NEI_MOGMA_MITTS,    RI_OOT_NEI_POKE_BALL,     RI_OOT_NEI_SHOVEL,
            RI_OOT_NEI_SPINNER,        RI_OOT_NEI_SWITCH_HOOK,   RI_OOT_NEI_TIME_GATE,
            RI_OOT_NEI_WHIP,           RI_OOT_NEI_ZONAI_PERMAFROST,
            RI_OOT_NEI_PHANTOM_HOURGLASS,
            RI_OOT_NEI_SHADOW_CRYSTAL, RI_OOT_NEI_ROD_OF_SEASONS,
            // Sheikah Slate: the pool item is gone — the FOUR RUNES are the placeable siblings now
            // (wand idiom: any order, each with its own textbox; the first found hands over the slate).
            RI_OOT_NEI_SLATE_RUNE_BOMB,   RI_OOT_NEI_SLATE_RUNE_MASTER_CYCLE,
            RI_OOT_NEI_SLATE_RUNE_STASIS, RI_OOT_NEI_SLATE_RUNE_CRYONIS,
        };
        for (RandoItemId neiItem : sNeiPoolItems) {
            itemPool.push_back(neiItem);
        }

        // BOTH Roc's Feathers. They are DIFFERENT items sharing nothing but a name, and each has to
        // be findable on its own:
        //
        //   RI_OOT_PROGRESSIVE_ROC -> Skijer's feather, page 2 / SLOT_ROCS, 2 levels (feather -> cape)
        //   RI_OOT_ROCS_FEATHER    -> the ship-vanilla one that shares the Nayru's Love cell
        //
        // Neither could be placed before this. The Skijer Roc was fully plumbed (item table, GiveItem,
        // DrawItem, an FC row) but appeared in no pool at all, so no seed ever contained it. The
        // vanilla one was pooled only inside the combo-only branch above, so a solo-MM seed never had
        // it either. Two copies for the Skijer Roc, matching its two chain levels — same shape as the
        // Dual Cane below. Skijer's NEI
        itemPool.push_back(RI_OOT_PROGRESSIVE_ROC); // L1 Roc's Feather (Skijer)
        itemPool.push_back(RI_OOT_PROGRESSIVE_ROC); // L2 Roc's Cape

        // The combo branch above already puts the ship-vanilla feather in MM's pool (it is an OoT-side
        // item that has to be there for cross-placement), so only add it here when running solo —
        // otherwise a combo seed would place two of them.
        if (FleetShipCombo_GetActiveGame() < 0) {
            itemPool.push_back(RI_OOT_ROCS_FEATHER);
        }

        // Dual Cane: SIX copies, because the cane is six separate skills sharing one slot and each
        // pickup unlocks the next (see the RI_OOT_NEI_CANE_* arms in GiveItem.cpp).
        itemPool.push_back(RI_OOT_NEI_CANE_OF_SOMARIA);
        itemPool.push_back(RI_OOT_NEI_CANE_SOMARIA_BLOCK);
        itemPool.push_back(RI_OOT_NEI_CANE_SOMARIA_PLATFORM);
        itemPool.push_back(RI_OOT_NEI_CANE_PACCI_FLIP);
        itemPool.push_back(RI_OOT_NEI_CANE_PACCI_STONE);
        itemPool.push_back(RI_OOT_NEI_CANE_PACCI_ULTRAHAND);

        // Bomb Arrows only enters the pool in "Shuffled" mode — the other two hand it out for free
        // (Off = Twilight Upgrade only, Bomb Bag = the moment you own a bomb bag), and placing a
        // check for something you already have would waste a location.
        if (saveInfo.randoSaveOptions[RO_SHUFFLE_BOMB_ARROWS] == RO_BOMB_ARROWS_SHUFFLED) {
            itemPool.push_back(RI_OOT_NEI_BOMB_ARROWS);
        }

        // Elemental Wand — same slot flag in all three modes, different pool shape:
        //   Medallions / Single item -> ONE item (the wand); the medallions or that single pickup
        //                               decide which rods work.
        //   Elemental shuffle        -> SIX items, one per rod; the first found grants the slot.
        if (saveInfo.randoSaveOptions[RO_ELEMENTAL_WAND_SHUFFLE] == RO_WAND_ELEMENTAL_SHUFFLE) {
            itemPool.push_back(RI_OOT_NEI_WAND_SAND_ROD);
            itemPool.push_back(RI_OOT_NEI_WAND_TORNADO_ROD);
            itemPool.push_back(RI_OOT_NEI_WAND_WATER_ROD);
            itemPool.push_back(RI_OOT_NEI_WAND_METEOR_ROD);
            itemPool.push_back(RI_OOT_NEI_WAND_STORM_ROD);
            itemPool.push_back(RI_OOT_NEI_WAND_SHADOW_SCEPTER);
        } else {
            itemPool.push_back(RI_OOT_NEI_ELEMENTAL_WAND);
        }
    }
}

} // namespace Logic

} // namespace Rando
