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

static std::map<RandoItemId, std::pair<uint16_t, uint16_t>> junkThresholdMap = {
    { RI_RECOVERY_HEART,    { 48, 320 } },
    { RI_MAGIC_JAR_SMALL,   { 12, 96 } },
    { RI_DEKU_STICKS_5,     { 3, 10 } },
    { RI_DEKU_NUTS_5,       { 5, 20 } },
    { RI_BOMBS_5,           { 5, 40 } },
    { RI_ARROWS_10,         { 5, 50 } },
    { RI_BOMBCHU_5,         { 5, 40 } },
};
// clang-format on

std::vector<std::tuple<RandoItemId, uint16_t, uint16_t>> junkSelectionList;
Actor* currentActor;

static bool junkInit = false;
void InitJunkOptions() {
    COND_HOOK(OnSceneInit, IS_RANDO, [](s8 sceneId, s8 spawnNum) { currentActor = nullptr; });
    junkInit = true;
}

const std::tuple<RandoItemId, const char*, const char*>& Rando::GetJunkTuple(RandoItemId id) {
    for (const auto& t : Rando::junkCvarMap) {
        if (std::get<0>(t) == id) {
            return t;
        }
    }
}

RandoItemId Rando::CurrentJunkItem(Actor* actor) {
    static RandoItemId lastJunkItem = RI_UNKNOWN;
    static RandoItemId selectedRupee = RI_RUPEE_GREEN;
    RandoItemId currentJunkItem = RI_UNKNOWN;
    static u32 lastChosenAt = 0;
    static int32_t lastRupee = -1;

    uint32_t totalWeight = 0;
    uint32_t weightedValue = 0;
    uint32_t currentWeight = 0;
    uint16_t failOver = 0;

    if (junkSelectionList.size() == 0) {
        Rando::UpdateJunkOptions();
    }

    switch (CVarGetInteger("gRando.Junk.ItemType", (uint32_t)RO_JUNK_TYPE_DEFAULT)) {
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
            if (currentActor == actor) {
                break;
            }

            currentActor = actor;
            for (auto& [itemId, weight, threshold] : junkSelectionList) {
                if (weight == 0) {
                    continue;
                }
                if (!Rando::IsItemObtainable(itemId)) {
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
                lastJunkItem = RI_UNKNOWN;
                switch (itemId) {
                    case RI_RECOVERY_HEART:
                        if (gSaveContext.save.saveInfo.playerData.health <= threshold * 16) {
                            lastJunkItem = RI_RECOVERY_HEART;
                            break;
                        }
                        break;
                    case RI_MAGIC_JAR_SMALL:
                        if (Rando::IsItemObtainable(itemId)) {
                            if (gSaveContext.save.saveInfo.playerData.magic <= threshold) {
                                lastJunkItem = RI_MAGIC_JAR_SMALL;
                                break;
                            }
                        }
                        break;
                    case RI_DEKU_STICKS_5:
                        if (AMMO(ITEM_DEKU_STICK) <= threshold) {
                            lastJunkItem = RI_DEKU_STICKS_5;
                            break;
                        }
                        break;
                    case RI_DEKU_NUTS_5:
                        if (AMMO(ITEM_DEKU_NUT) <= threshold) {
                            lastJunkItem = RI_DEKU_NUTS_5;
                            break;
                        }
                        break;
                    case RI_BOMBS_5:
                        if (Rando::IsItemObtainable(itemId)) {
                            if (AMMO(ITEM_BOMB) <= threshold) {
                                lastJunkItem = RI_BOMBS_5;
                                break;
                            }
                        }
                        break;
                    case RI_ARROWS_10:
                        if (Rando::IsItemObtainable(itemId)) {
                            if (AMMO(ITEM_BOW) <= threshold) {
                                lastJunkItem = RI_ARROWS_10;
                                break;
                            }
                        }
                        break;
                    case RI_BOMBCHU_5:
                        if (Rando::IsItemObtainable(itemId)) {
                            if (AMMO(ITEM_BOMBCHU) <= threshold) {
                                lastJunkItem = RI_BOMBCHU_5;
                                break;
                            }
                        }
                        break;
                    default:
                        break;
                }
                if (lastJunkItem == RI_UNKNOWN) {
                    if (lastRupee != gSaveContext.save.saveInfo.playerData.rupees) {
                        selectedRupee = rupeeList[rand() % rupeeList.size()];
                        lastRupee = gSaveContext.save.saveInfo.playerData.rupees;
                    }
                    lastJunkItem = selectedRupee;
                    break;
                }
                break;
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
    if (!junkInit) {
        InitJunkOptions();
    }
    junkSelectionList.clear();

    for (const auto& [itemId, itemName, cvar] : junkCvarMap) {
        if (!CVarGetInteger(JUNK_CVAR(itemId, "Enabled"), 1)) {
            continue;
        }
        std::tuple<RandoItemId, uint16_t, uint16_t> junkItem;

        junkItem = std::make_tuple(
            itemId, CVarGetInteger(JUNK_CVAR(itemId, "Weight"), 10),
            CVarGetInteger(JUNK_CVAR(itemId, "Threshold"), Rando::GetJunkThresholds(itemId, "Default")));

        junkSelectionList.push_back(junkItem);
    }

    if (junkSelectionList.size() == 0) {
        junkSelectionList.push_back({ RI_NONE, 100, 0 });
    }
}

uint32_t Rando::GetJunkThresholds(RandoItemId randoItemId, std::string entry) {
    uint16_t value;
    auto findMax = junkThresholdMap.find(randoItemId);
    if (findMax != junkThresholdMap.end()) {
        if (entry == "Default") {
            value = findMax->second.first;
        } else {
            value = findMax->second.second;
        }
        if (randoItemId == RI_RECOVERY_HEART) {
            value = value / 16;
        }
        return value;
    }
    return 500;
}
