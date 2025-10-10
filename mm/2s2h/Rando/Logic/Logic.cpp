#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#include "Logic.h"

namespace Rando {

namespace Logic {

std::unordered_map<RandoRegionId, RandoRegion> Regions = {};

// Thread-local storage for current region time during check evaluation
thread_local uint64_t gCurrentRegionTime = 0;

// Time expansion function - expands accessible time forward with stay restrictions
// Implements OoTMM's sequential expansion: if a stay restriction fails, expansion stops permanently
inline uint64_t ExpandTimeForward(uint64_t timeSlices, const RandoRegion& region) {
    uint64_t expanded = timeSlices;
    bool canWait = false;

    for (int i = 0; i < TIME_SLICE_COUNT; ++i) {
        uint64_t mask = (TIME_BIT_ONE << i);

        if (timeSlices & mask) {
            // We can be at this time
            canWait = true;
            expanded |= mask;
        } else if (canWait) {
            // Check if we can wait to this time
            auto it = region.timeStayRestrictions.find(static_cast<TimeSlice>(i));
            if (it != region.timeStayRestrictions.end()) {
                // Explicit restriction exists
                if (it->second()) {
                    expanded |= mask; // Condition passed, add time
                } else {
                    canWait = false; // Kicked out, STOP expansion
                }
            } else {
                // No restriction = default true, can stay
                expanded |= mask;
            }
        }
    }

    return expanded;
}

RandoRegionId GetRegionIdFromEntrance(s32 entrance) {
    static std::unordered_map<s32, RandoRegionId> entranceToRegionId;
    if (entranceToRegionId.empty()) {
        for (auto& [randoRegionId, randoRegion] : Regions) {
            for (auto& [_, regionExit] : randoRegion.exits) {
                if (regionExit.returnEntrance == ONE_WAY_EXIT) {
                    continue;
                }
                entranceToRegionId[regionExit.returnEntrance] = randoRegionId;
            }
            for (auto& entrance : randoRegion.oneWayEntrances) {
                entranceToRegionId[entrance] = randoRegionId;
            }
        }
    }

    if (entranceToRegionId.contains(entrance)) {
        return entranceToRegionId[entrance];
    }

    return RR_MAX;
}

void FindReachableRegions(RandoRegionId currentRegion, std::set<RandoRegionId>& reachableRegions,
                          std::unordered_map<RandoRegionId, RegionTimeState>& regionTimeStates) {
    auto& sourceRegion = Regions[currentRegion];
    auto& sourceTimeState = regionTimeStates[currentRegion];

    // Expand time if player can wait in this region
    uint64_t currentTime = sourceTimeState.timeSlices;
    if (sourceTimeState.canStayOverTime) {
        currentTime = ExpandTimeForward(currentTime, sourceRegion);
        sourceTimeState.timeSlices = currentTime;
    }

    // Set global time for check evaluation
    gCurrentRegionTime = currentTime;

    // Explore connections
    for (auto& [connectedRegionId, condition] : sourceRegion.connections) {
        if (reachableRegions.count(connectedRegionId) == 0 && condition.first()) {
            reachableRegions.insert(connectedRegionId);

            auto& targetRegion = Regions[connectedRegionId];
            regionTimeStates[connectedRegionId] = { .timeSlices = currentTime,
                                                    .canStayOverTime = targetRegion.canStayOverTime };

            FindReachableRegions(connectedRegionId, reachableRegions, regionTimeStates);
        }
    }

    // Explore exits
    for (auto& [exitId, regionExit] : sourceRegion.exits) {
        RandoRegionId connectedRegionId = GetRegionIdFromEntrance(exitId);
        if (reachableRegions.count(connectedRegionId) == 0 && regionExit.condition()) {
            reachableRegions.insert(connectedRegionId);

            auto& targetRegion = Regions[connectedRegionId];
            regionTimeStates[connectedRegionId] = { .timeSlices = currentTime,
                                                    .canStayOverTime = targetRegion.canStayOverTime };

            FindReachableRegions(connectedRegionId, reachableRegions, regionTimeStates);
        }
    }
}

// clang-format off
static RegisterShipInitFunc initFunc([]() {
    Regions[RR_MAX] = RandoRegion{ .sceneId = SCENE_MAX,
        .checks = {
            CHECK(RC_STARTING_ITEM_DEKU_MASK, true),
            CHECK(RC_STARTING_ITEM_SONG_OF_HEALING, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 0),                      ONE_WAY_EXIT, true), // Save warp
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 9),                      ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_CLOCK_TOWN)),
            EXIT(ENTRANCE(SOUTHERN_SWAMP_POISONED, 10),              ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_SOUTHERN_SWAMP)),
            EXIT(ENTRANCE(MILK_ROAD, 4),                             ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_MILK_ROAD)),
            EXIT(ENTRANCE(MOUNTAIN_VILLAGE_WINTER, 8),               ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_MOUNTAIN_VILLAGE)),
            EXIT(ENTRANCE(SNOWHEAD, 3),                              ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_SNOWHEAD)),
            EXIT(ENTRANCE(GREAT_BAY_COAST, 11),                      ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_GREAT_BAY_COAST)),
            EXIT(ENTRANCE(ZORA_CAPE, 6),                             ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_ZORA_CAPE)),
            EXIT(ENTRANCE(IKANA_CANYON, 4),                          ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_IKANA_CANYON)),
            EXIT(ENTRANCE(STONE_TOWER, 3),                           ONE_WAY_EXIT, CAN_PLAY_SONG(SOARING) && CAN_OWL_WARP(OWL_WARP_STONE_TOWER)),
        },
    };
}, {});
// clang-format on

} // namespace Logic

} // namespace Rando
