#include "Rando/Rando.h"
#include "Rando/MiscBehavior/ClockShuffle.h"
#include "Spoiler.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

namespace Rando {

namespace Spoiler {

// Helper function to check if an item is a clock item
static bool IsClockItem(RandoItemId itemId) {
    return itemId == RI_CLOCK_DAY_1 || itemId == RI_CLOCK_NIGHT_1 || itemId == RI_CLOCK_DAY_2 ||
           itemId == RI_CLOCK_NIGHT_2 || itemId == RI_CLOCK_DAY_3 || itemId == RI_CLOCK_NIGHT_3 ||
           itemId == RI_CLOCK_PROGRESSIVE;
}

nlohmann::json GenerateFromSaveContext() {
    nlohmann::json spoiler;
    spoiler["type"] = "2S2H_RANDO_SPOILER";
    spoiler["commitHash"] = gSaveContext.save.shipSaveInfo.commitHash;
    spoiler["finalSeed"] = gSaveContext.save.shipSaveInfo.rando.finalSeed;

    spoiler["options"] = nlohmann::json::object();
    for (auto& [randoOptionId, randoStaticOption] : Rando::StaticData::Options) {
        spoiler["options"][randoStaticOption.name] = RANDO_SAVE_OPTIONS[randoOptionId];
    }

    spoiler["startingItems"] = RANDO_STARTING_ITEMS;

    spoiler["checks"] = nlohmann::json::object();
    for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
        if (randoStaticCheck.randoCheckId == RC_UNKNOWN) {
            continue;
        }

        if (!RANDO_SAVE_CHECKS[randoCheckId].shuffled) {
            continue;
        }

        // Skip clock items when clock shuffle is enabled - they go in clockShuffle section
        RandoItemId itemId = RANDO_SAVE_CHECKS[randoCheckId].randoItemId;
        if (RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE] && IsClockItem(itemId)) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_SHOP || randoStaticCheck.randoCheckType == RCTYPE_TINGLE_SHOP) {
            spoiler["checks"][randoStaticCheck.name] = nlohmann::json::object();
            spoiler["checks"][randoStaticCheck.name]["randoItemId"] =
                Rando::StaticData::Items[RANDO_SAVE_CHECKS[randoCheckId].randoItemId].spoilerName;
            spoiler["checks"][randoStaticCheck.name]["price"] = RANDO_SAVE_CHECKS[randoCheckId].price;
        } else {
            spoiler["checks"][randoStaticCheck.name] =
                Rando::StaticData::Items[RANDO_SAVE_CHECKS[randoCheckId].randoItemId].spoilerName;
        }
    }

    // Store clock shuffle information
    if (RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE]) {
        spoiler["clockShuffle"] = nlohmann::json::object();
        spoiler["clockShuffle"]["enabled"] = true;

        // Convert progressive mode enum to readable string
        int progressiveMode = RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE_PROGRESSIVE];
        switch (progressiveMode) {
            case RO_CLOCK_SHUFFLE_RANDOM:
                spoiler["clockShuffle"]["progressive"] = "Random";
                break;
            case RO_CLOCK_SHUFFLE_ASCENDING:
                spoiler["clockShuffle"]["progressive"] = "Progressive: Ascending";
                break;
            case RO_CLOCK_SHUFFLE_DESCENDING:
                spoiler["clockShuffle"]["progressive"] = "Progressive: Descending";
                break;
            default:
                spoiler["clockShuffle"]["progressive"] = "Unknown";
                break;
        }

        // Find which clock half was granted initially and convert to readable name
        for (int halfDayIndex = 0; halfDayIndex < 6; ++halfDayIndex) {
            if (Flags_GetRandoInf(static_cast<RandoInf>(RANDO_INF_OBTAINED_CLOCK_DAY_1 + halfDayIndex))) {
                RandoItemId clockItem = Rando::ClockItems::GetClockItemFromHalfDayIndex(halfDayIndex);
                if (clockItem != RI_UNKNOWN) {
                    spoiler["clockShuffle"]["grantedHalf"] = Rando::StaticData::Items[clockItem].spoilerName;
                }
                break;
            }
        }

        // Add all clock checks to the clockShuffle section
        spoiler["clockShuffle"]["checks"] = nlohmann::json::object();
        for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
            if (randoStaticCheck.randoCheckId == RC_UNKNOWN) {
                continue;
            }

            if (!RANDO_SAVE_CHECKS[randoCheckId].shuffled) {
                continue;
            }

            RandoItemId itemId = RANDO_SAVE_CHECKS[randoCheckId].randoItemId;
            if (IsClockItem(itemId)) {
                // Handle shop items with prices
                if (randoStaticCheck.randoCheckType == RCTYPE_SHOP ||
                    randoStaticCheck.randoCheckType == RCTYPE_TINGLE_SHOP) {
                    spoiler["clockShuffle"]["checks"][randoStaticCheck.name] = nlohmann::json::object();
                    spoiler["clockShuffle"]["checks"][randoStaticCheck.name]["randoItemId"] =
                        Rando::StaticData::Items[itemId].spoilerName;
                    spoiler["clockShuffle"]["checks"][randoStaticCheck.name]["price"] =
                        RANDO_SAVE_CHECKS[randoCheckId].price;
                } else {
                    spoiler["clockShuffle"]["checks"][randoStaticCheck.name] =
                        Rando::StaticData::Items[itemId].spoilerName;
                }
            }
        }
    }

    return spoiler;
}

} // namespace Spoiler

} // namespace Rando
