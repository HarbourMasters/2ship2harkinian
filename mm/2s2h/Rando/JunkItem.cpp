#include "Rando/Rando.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/ShipUtils.h"

#define FAIL_OVER_MAX 5 // Number of rand() attempts before defaulting to RI_NONE

// clang-format off
std::vector<std::tuple<RandoItemId, const char*, const char*>> Rando::junkCvarMap = {
    { RI_RECOVERY_HEART,    "Recovery Heart",       "gRando.Junk.Heart." },
    { RI_MAGIC_JAR_SMALL,   "Small Magic Jar",      "gRando.Junk.MagicSmall." },
    { RI_DEKU_STICKS_5,     "Deku Sticks (5)",      "gRando.Junk.DekuSticks." },
    { RI_DEKU_NUTS_5,       "Deku Nuts (5)",        "gRando.Junk.DekuNuts." },
    { RI_BOMBS_5,           "Bombs (5)",            "gRando.Junk.Bombs." },
    { RI_ARROWS_10,         "Arrows (10)",          "gRando.Junk.Arrows." },
    { RI_BOMBCHU_5,         "Bombchus (5)",         "gRando.Junk.Bombchus." },
    { RI_RUPEE_GREEN,       "Green Rupee",          "gRando.Junk.RupeeGreen." },
    { RI_RUPEE_BLUE,        "Blue Rupee",           "gRando.Junk.RupeeBlue." },
    { RI_RUPEE_RED,         "Red Rupee",            "gRando.Junk.RupeeRed." },
    { RI_RUPEE_PURPLE,      "Purple Rupee",         "gRando.Junk.RupeePurple." },
    { RI_NONE,              "Literally Nothing",    "gRando.Junk.Nothing." },
};

static std::vector<RandoItemId> rupeeList = {
    RI_RUPEE_GREEN,
    RI_RUPEE_BLUE,
    RI_RUPEE_RED,
    RI_RUPEE_PURPLE,
};

static std::map<RandoItemId, uint16_t> maxJunkThresholdMap = {
    { RI_RECOVERY_HEART,    320 },
    { RI_MAGIC_JAR_SMALL,   96 },
    { RI_DEKU_STICKS_5,     10 },
    { RI_DEKU_NUTS_5,       20 },
    { RI_BOMBS_5,           40 },
    { RI_ARROWS_10,         50 },
    { RI_BOMBCHU_5,         40 },
};
// clang-format on

std::vector<std::tuple<RandoItemId, uint16_t, uint16_t>> junkSelectionList;

inline const std::tuple<RandoItemId, const char*, const char*>& Rando::GetJunkTuple(RandoItemId id) {
    for (const auto& t : Rando::junkCvarMap) {
        if (std::get<0>(t) == id) {
            return t;
        }
    }
}

RandoItemId Rando::CurrentJunkItem() {
    static RandoItemId lastJunkItem = RI_UNKNOWN;
    RandoItemId currentJunkItem = RI_UNKNOWN;
    static u32 lastChosenAt = 0;
    static int32_t lastRupee = -1;
    uint32_t totalWeight = 0;
    uint32_t weightedValue = 0;
    uint32_t currentWeight = 0;
    uint16_t failOver = 0;

    switch (CVarGetInteger("gRando.Junk.ItemType", (uint32_t)RO_JUNK_TYPE_DEFAULT)) {
        if (junkSelectionList.size() == 0) {
            Rando::UpdateJunkOptions();
        }

        case RO_JUNK_TYPE_DEFAULT:
            if (gPlayState != NULL && ABS(gPlayState->gameplayFrames - lastChosenAt) > 20) {
                lastChosenAt = gPlayState->gameplayFrames;
                lastJunkItem = RI_UNKNOWN;
            }

            while (lastJunkItem == RI_UNKNOWN) {
                RandoItemId randJunkItem = std::get<0>(junkSelectionList[rand() % junkSelectionList.size()]);
                if (Rando::IsItemObtainable(randJunkItem)) {
                    lastJunkItem = randJunkItem;
                }
                if (failOver >= FAIL_OVER_MAX) {
                    randJunkItem = RI_NONE;
                    lastJunkItem = randJunkItem;
                }
                failOver++;
            }
            break;
        case RO_JUNK_TYPE_WEIGHTED:
            for (auto& [itemId, weight, threshold] : junkSelectionList) {
                if (weight == 0) {
                    continue;
                }
                totalWeight += weight;
            }
            if (totalWeight == 0) {
                lastJunkItem = RI_NONE;
                break;
            }

            weightedValue = rand() % totalWeight;
            for (auto& [itemId, weight, threshold] : junkSelectionList) {
                currentWeight += weight;
                if (weightedValue < currentWeight) {
                    lastJunkItem = itemId;
                    break;
                }
            }
            break;
        case RO_JUNK_TYPE_SUPPLY:
            for (auto& [itemId, weight, threshold] : junkSelectionList) {
                if (threshold == 0) {
                    continue;
                }

                switch (itemId) {
                    case RI_RECOVERY_HEART:
                        if (gSaveContext.save.saveInfo.playerData.health <=
                            CVarGetInteger(JUNK_CVAR(RI_RECOVERY_HEART, "Threshold"), 10) * 16) {
                            lastJunkItem = RI_RECOVERY_HEART;
                            break;
                        }
                    case RI_MAGIC_JAR_SMALL:
                        if (gSaveContext.save.saveInfo.playerData.isMagicAcquired == true &&
                            gSaveContext.save.saveInfo.playerData.magic <=
                                CVarGetInteger(JUNK_CVAR(RI_MAGIC_JAR_SMALL, "Threshold"), 10)) {
                            lastJunkItem = RI_MAGIC_JAR_SMALL;
                            break;
                        }
                    case RI_DEKU_STICKS_5:
                        if (AMMO(ITEM_DEKU_STICK) <= CVarGetInteger(JUNK_CVAR(RI_DEKU_STICKS_5, "Threshold"), 10)) {
                            lastJunkItem = RI_DEKU_STICKS_5;
                            break;
                        }
                    case RI_DEKU_NUTS_5:
                        if (AMMO(ITEM_DEKU_NUT) <= CVarGetInteger(JUNK_CVAR(RI_DEKU_NUTS_5, "Threshold"), 10)) {
                            lastJunkItem = RI_DEKU_NUTS_5;
                            break;
                        }
                    case RI_BOMBS_5:
                        if (INV_CONTENT(ITEM_BOMB) == ITEM_BOMB) {
                            if (AMMO(ITEM_BOMB) <= CVarGetInteger(JUNK_CVAR(RI_BOMBS_5, "Threshold"), 10)) {
                                lastJunkItem = RI_BOMBS_5;
                                break;
                            }
                        }
                    case RI_ARROWS_10:
                        if (INV_CONTENT(ITEM_BOW) == ITEM_BOW) {
                            if (AMMO(ITEM_BOW) <= CVarGetInteger(JUNK_CVAR(RI_ARROWS_10, "Threshold"), 10)) {
                                lastJunkItem = RI_ARROWS_10;
                                break;
                            }
                        }
                    case RI_BOMBCHU_5:
                        if (INV_CONTENT(ITEM_BOMBCHU) == ITEM_BOMBCHU) {
                            if (AMMO(ITEM_BOMBCHU) <= CVarGetInteger(JUNK_CVAR(RI_BOMBCHU_5, "Threshold"), 10)) {
                                lastJunkItem = RI_BOMBCHU_5;
                                break;
                            }
                        }
                    default:
                        if (lastRupee != gSaveContext.save.saveInfo.playerData.rupees) {
                            lastJunkItem = rupeeList[rand() % rupeeList.size()];
                            lastRupee = gSaveContext.save.saveInfo.playerData.rupees;
                        }
                        break;
                }
                if (lastJunkItem <= RI_RECOVERY_HEART) {
                    lastRupee = -1;
                }
            }
            break;
        default:
            break;
    }

    if (lastJunkItem == RI_UNKNOWN) {
        lastJunkItem = RI_NONE;
    }

    return lastJunkItem;
}

void Rando::UpdateJunkOptions() {
    junkSelectionList.clear();

    for (const auto& [itemId, itemName, cvar] : junkCvarMap) {
        if (!CVarGetInteger(JUNK_CVAR(itemId, "Enabled"), 0)) {
            continue;
        }
        std::tuple<RandoItemId, uint16_t, uint16_t> junkItem;

        junkItem = std::make_tuple(itemId, CVarGetInteger(JUNK_CVAR(itemId, "Weight"), 10),
                                   CVarGetInteger(JUNK_CVAR(itemId, "Threshold"), 10));

        junkSelectionList.push_back(junkItem);
    }

    if (junkSelectionList.size() == 0) {
        junkSelectionList.push_back({ RI_NONE, 100, 0 });
    }
}

uint32_t Rando::GetJunkThresholdMax(RandoItemId randoItemId) {
    auto findMax = maxJunkThresholdMap.find(randoItemId);
    if (findMax != maxJunkThresholdMap.end()) {
        uint16_t max = findMax->second;
        if (randoItemId == RI_RECOVERY_HEART) {
            max = max / 16;
        }
        return max;
    }
    return 500;
}
