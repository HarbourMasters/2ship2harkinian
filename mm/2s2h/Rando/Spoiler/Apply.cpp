#include "Rando/Rando.h"
#include "Rando/MiscBehavior/ClockShuffle.h"
#include "Spoiler.h"
#include "public/bridge/consolevariablebridge.h"
#include "ShipUtils.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

namespace Rando {

namespace Spoiler {

void ApplyToSaveContext(nlohmann::json spoiler) {
    gSaveContext.save.shipSaveInfo.rando.finalSeed = spoiler["finalSeed"].get<uint32_t>();

    for (auto& [randoOptionId, randoStaticOption] : Rando::StaticData::Options) {
        RANDO_SAVE_OPTIONS[randoOptionId] = spoiler["options"][randoStaticOption.name].get<uint32_t>();
    }

    std::string startingItemsSave = spoiler["startingItems"].get<std::string>();
    strncpy(RANDO_STARTING_ITEMS, startingItemsSave.c_str(), startingItemsSave.size() + 1);

    if (RANDO_SAVE_OPTIONS[RO_STARTING_HEALTH] != 3) {
        gSaveContext.save.saveInfo.playerData.healthCapacity = gSaveContext.save.saveInfo.playerData.health =
            RANDO_SAVE_OPTIONS[RO_STARTING_HEALTH] * 0x10;
    }

    if (RANDO_SAVE_OPTIONS[RO_STARTING_CONSUMABLES]) {
        GiveItem(RI_DEKU_STICK);
        GiveItem(RI_DEKU_NUT);
        AMMO(ITEM_DEKU_STICK) = CUR_CAPACITY(UPG_DEKU_STICKS);
        AMMO(ITEM_DEKU_NUT) = CUR_CAPACITY(UPG_DEKU_NUTS);
    }

    std::vector<RandoItemId> startingItems = convertStartingItemsToRandoItemId(RANDO_STARTING_ITEMS, ",");

    if (RANDO_SAVE_OPTIONS[RO_STARTING_MAPS_AND_COMPASSES]) {
        std::vector<RandoItemId> MapsAndCompasses = {
            RI_GREAT_BAY_COMPASS,       RI_GREAT_BAY_MAP,       RI_SNOWHEAD_COMPASS,       RI_SNOWHEAD_MAP,
            RI_STONE_TOWER_COMPASS,     RI_STONE_TOWER_MAP,     RI_TINGLE_MAP_CLOCK_TOWN,  RI_TINGLE_MAP_GREAT_BAY,
            RI_TINGLE_MAP_ROMANI_RANCH, RI_TINGLE_MAP_SNOWHEAD, RI_TINGLE_MAP_STONE_TOWER, RI_TINGLE_MAP_WOODFALL,
            RI_WOODFALL_COMPASS,        RI_WOODFALL_MAP,
        };
        for (RandoItemId itemId : MapsAndCompasses) {
            startingItems.push_back(itemId);
        }
    }

    for (RandoItemId startingItem : startingItems) {
        GiveItem(ConvertItem(startingItem));
    }

    if (RANDO_SAVE_OPTIONS[RO_STARTING_RUPEES]) {
        gSaveContext.save.saveInfo.playerData.rupees = CUR_CAPACITY(UPG_WALLET);
    }

    // Process clock checks from clockShuffle section if present
    if (spoiler.contains("clockShuffle") && spoiler["clockShuffle"].contains("checks")) {
        for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
            if (randoStaticCheck.randoCheckId == RC_UNKNOWN) {
                continue;
            }

            if (!spoiler["clockShuffle"]["checks"].contains(randoStaticCheck.name)) {
                continue;
            }

            // Check if it's an object (shop with price) or a string
            if (spoiler["clockShuffle"]["checks"][randoStaticCheck.name].is_object()) {
                std::string itemName =
                    spoiler["clockShuffle"]["checks"][randoStaticCheck.name]["randoItemId"].get<std::string>();
                RandoItemId randoItemId = Rando::StaticData::GetItemIdFromName(itemName.c_str());

                RANDO_SAVE_CHECKS[randoCheckId].randoItemId = randoItemId;
                RANDO_SAVE_CHECKS[randoCheckId].shuffled = true;

                // If it has a price, set it
                if (spoiler["clockShuffle"]["checks"][randoStaticCheck.name].contains("price")) {
                    RANDO_SAVE_CHECKS[randoCheckId].price =
                        spoiler["clockShuffle"]["checks"][randoStaticCheck.name]["price"].get<uint16_t>();
                }
            } else {
                std::string itemName = spoiler["clockShuffle"]["checks"][randoStaticCheck.name].get<std::string>();
                RandoItemId randoItemId = Rando::StaticData::GetItemIdFromName(itemName.c_str());

                RANDO_SAVE_CHECKS[randoCheckId].randoItemId = randoItemId;
                RANDO_SAVE_CHECKS[randoCheckId].shuffled = true;
            }
        }
    }

    for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
        if (randoStaticCheck.randoCheckId == RC_UNKNOWN) {
            continue;
        }

        if (!spoiler["checks"].contains(randoStaticCheck.name)) {
            RANDO_SAVE_CHECKS[randoCheckId].randoItemId = randoStaticCheck.randoItemId;
            RANDO_SAVE_CHECKS[randoCheckId].shuffled = false;
            continue;
        }

        // Check if it's an object or a string
        if (spoiler["checks"][randoStaticCheck.name].is_object()) {
            std::string itemName = spoiler["checks"][randoStaticCheck.name]["randoItemId"].get<std::string>();
            RandoItemId randoItemId = Rando::StaticData::GetItemIdFromName(itemName.c_str());

            RANDO_SAVE_CHECKS[randoCheckId].randoItemId = randoItemId;
            RANDO_SAVE_CHECKS[randoCheckId].shuffled = true;

            // If it has a price, set it
            if (spoiler["checks"][randoStaticCheck.name].contains("price")) {
                RANDO_SAVE_CHECKS[randoCheckId].price =
                    spoiler["checks"][randoStaticCheck.name]["price"].get<uint16_t>();
            }
        } else {
            std::string itemName = spoiler["checks"][randoStaticCheck.name].get<std::string>();
            RandoItemId randoItemId = Rando::StaticData::GetItemIdFromName(itemName.c_str());

            RANDO_SAVE_CHECKS[randoCheckId].randoItemId = randoItemId;
            RANDO_SAVE_CHECKS[randoCheckId].shuffled = true;
        }
    }

    // Restore clock shuffle information
    if (spoiler.contains("clockShuffle") && spoiler["clockShuffle"]["enabled"].get<bool>()) {
        if (spoiler["clockShuffle"].contains("grantedHalf")) {
            int grantedHalfIndex = -1;

            // Handle both old format (int) and new format (string)
            if (spoiler["clockShuffle"]["grantedHalf"].is_string()) {
                // New format: clock item name as string
                std::string clockItemName = spoiler["clockShuffle"]["grantedHalf"].get<std::string>();
                RandoItemId clockItemId = Rando::StaticData::GetItemIdFromName(clockItemName.c_str());
                grantedHalfIndex = Rando::ClockItems::GetHalfDayIndexFromClockItem(clockItemId);
            } else if (spoiler["clockShuffle"]["grantedHalf"].is_number()) {
                // Old format: half-day index as int (backward compatibility)
                grantedHalfIndex = spoiler["clockShuffle"]["grantedHalf"].get<int>();
            }

            if (grantedHalfIndex >= 0 && grantedHalfIndex < 6) {
                RandoInf clockFlag = static_cast<RandoInf>(RANDO_INF_OBTAINED_CLOCK_DAY_1 + grantedHalfIndex);
                Flags_SetRandoInf(clockFlag);
            }
        }
    }
}

} // namespace Spoiler

} // namespace Rando
