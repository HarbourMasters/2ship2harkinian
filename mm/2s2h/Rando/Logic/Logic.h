#ifndef RANDO_LOGIC_H
#define RANDO_LOGIC_H

#include "Rando/Rando.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipUtils.h"

#include <unordered_map>
#include <set>
#include <cassert>

extern "C" {
#include "functions.h"
#include "variables.h"
}

namespace Rando {

namespace Logic {

// Time slice enum - 45 granular time points throughout MM's 3-day cycle
enum TimeSlice {
    TIME_DAY1_AM_06_00 = 0,
    TIME_DAY1_AM_07_00,
    TIME_DAY1_AM_08_00,
    TIME_DAY1_AM_10_00,
    TIME_DAY1_PM_01_45,
    TIME_DAY1_PM_03_00,
    TIME_DAY1_PM_04_00,
    TIME_NIGHT1_PM_06_00,
    TIME_NIGHT1_PM_08_00,
    TIME_NIGHT1_PM_09_00,
    TIME_NIGHT1_PM_10_00,
    TIME_NIGHT1_PM_11_00,
    TIME_NIGHT1_AM_12_00,
    TIME_NIGHT1_AM_02_30,
    TIME_NIGHT1_AM_04_00,
    TIME_NIGHT1_AM_05_00,
    TIME_DAY2_AM_06_00,
    TIME_DAY2_AM_07_00,
    TIME_DAY2_AM_08_00,
    TIME_DAY2_AM_10_00,
    TIME_DAY2_AM_11_30,
    TIME_DAY2_PM_02_00,
    TIME_DAY2_PM_04_00,
    TIME_NIGHT2_PM_06_00,
    TIME_NIGHT2_PM_08_00,
    TIME_NIGHT2_PM_09_00,
    TIME_NIGHT2_PM_10_00,
    TIME_NIGHT2_PM_11_00,
    TIME_NIGHT2_AM_12_00,
    TIME_NIGHT2_AM_04_00,
    TIME_NIGHT2_AM_05_00,
    TIME_DAY3_AM_06_00,
    TIME_DAY3_AM_07_00,
    TIME_DAY3_AM_08_00,
    TIME_DAY3_AM_10_00,
    TIME_DAY3_AM_11_30,
    TIME_DAY3_PM_01_00,
    TIME_NIGHT3_PM_06_00,
    TIME_NIGHT3_PM_08_00,
    TIME_NIGHT3_PM_09_00,
    TIME_NIGHT3_PM_10_00,
    TIME_NIGHT3_PM_11_00,
    TIME_NIGHT3_AM_12_00,
    TIME_NIGHT3_AM_04_00,
    TIME_NIGHT3_AM_05_00 // = 44
};

// Time slice count and bitmask constants
// Derived from enum - update if last enum value changes
constexpr int TIME_SLICE_COUNT = TIME_NIGHT3_AM_05_00 + 1;
constexpr uint64_t TIME_BIT_ONE = 1ULL;              // Base value for bit shifting
constexpr uint64_t TIME_ALL_SLICES = 0x1FFFFFFFFFFF; // All 45 time bits set

// Time state for tracking time accessibility during logic solving
struct RegionTimeState {
    uint64_t timeSlices;
    bool canStayOverTime;
};

// Thread-local current region time for check evaluation
extern thread_local uint64_t gCurrentRegionTime;

void FindReachableRegions(RandoRegionId currentRegion, std::set<RandoRegionId>& reachableRegions,
                          std::unordered_map<RandoRegionId, RegionTimeState>& regionTimeStates);
RandoRegionId GetRegionIdFromEntrance(s32 entrance);
void ApplyFrenchVanillaLogicToSaveContext(std::unordered_map<RandoCheckId, bool>& checkPool,
                                          std::vector<RandoItemId>& itemPool);
void ApplyGlitchlessLogicToSaveContext(std::unordered_map<RandoCheckId, bool>& checkPool,
                                       std::vector<RandoItemId>& itemPool);
void ApplyNearlyNoLogicToSaveContext(std::unordered_map<RandoCheckId, bool>& checkPool,
                                     std::vector<RandoItemId>& itemPool);
void ApplyNoLogicToSaveContext(std::unordered_map<RandoCheckId, bool>& checkPool, std::vector<RandoItemId>& itemPool);

struct RandoRegionExit {
    s32 returnEntrance;
    std::function<bool()> condition;
    std::string conditionString;
};

struct RandoRegion {
    const char* name = "";
    SceneId sceneId;
    std::unordered_map<RandoCheckId, std::pair<std::function<bool()>, std::string>> checks;
    std::unordered_map<s32, RandoRegionExit> exits;
    std::unordered_map<RandoRegionId, std::pair<std::function<bool()>, std::string>> connections;
    std::vector<std::pair<RandoEvent, std::function<bool()>>> events;
    std::set<s32> oneWayEntrances;
    uint64_t timeSlices = 0;     // Bitfield: accessible time slices (bits 0-44)
    bool canStayOverTime = true; // Can player wait for time to pass? (default true for most regions)
    std::unordered_map<TimeSlice, std::function<bool()>>
        timeStayRestrictions; // Time slices where staying is restricted
};

extern std::unordered_map<RandoRegionId, RandoRegion> Regions;

// TODO: This may not stay here
#define IS_DEKU (GET_PLAYER_FORM == PLAYER_FORM_DEKU)
#define IS_ZORA (GET_PLAYER_FORM == PLAYER_FORM_ZORA)
#define IS_DEITY (GET_PLAYER_FORM == PLAYER_FORM_FIERCE_DEITY)
#define IS_GORON (GET_PLAYER_FORM == PLAYER_FORM_GORON)
#define IS_HUMAN (GET_PLAYER_FORM == PLAYER_FORM_HUMAN)
#define HAS_ITEM(item) (INV_CONTENT(item) == item)
#define CAN_BE_DEKU (IS_DEKU || HAS_ITEM(ITEM_MASK_DEKU))
#define CAN_BE_ZORA (IS_ZORA || HAS_ITEM(ITEM_MASK_ZORA))
#define CAN_BE_DEITY (IS_DEITY || HAS_ITEM(ITEM_MASK_FIERCE_DEITY))
#define CAN_BE_GORON (IS_GORON || HAS_ITEM(ITEM_MASK_GORON))
#define CAN_BE_HUMAN                                                                                        \
    (IS_HUMAN || (IS_DEITY && HAS_ITEM(ITEM_MASK_FIERCE_DEITY)) || (IS_ZORA && HAS_ITEM(ITEM_MASK_ZORA)) || \
     (IS_DEKU && HAS_ITEM(ITEM_MASK_DEKU)) || (IS_GORON && HAS_ITEM(ITEM_MASK_GORON)))
#define CHECK_MAX_HP(TARGET_HP) ((TARGET_HP * 16) <= gSaveContext.save.saveInfo.playerData.healthCapacity)
#define HAS_MAGIC (gSaveContext.save.saveInfo.playerData.isMagicAcquired)
#define CAN_HOOK_SCARECROW (HAS_ITEM(ITEM_OCARINA_OF_TIME) && HAS_ITEM(ITEM_HOOKSHOT))
#define CAN_USE_EXPLOSIVE ((HAS_ITEM(ITEM_BOMB) || HAS_ITEM(ITEM_BOMBCHU) || HAS_ITEM(ITEM_MASK_BLAST)))
#define CAN_USE_HUMAN_SWORD (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) >= EQUIP_VALUE_SWORD_KOKIRI)
#define CAN_USE_SWORD (CAN_USE_HUMAN_SWORD || HAS_ITEM(ITEM_SWORD_GREAT_FAIRY) || CAN_BE_DEITY)
// Be careful here, as some checks require you to play the song as a specific form
#define CAN_PLAY_SONG(song) (HAS_ITEM(ITEM_OCARINA_OF_TIME) && CHECK_QUEST_ITEM(QUEST_SONG_##song))
#define CAN_RIDE_EPONA (CAN_PLAY_SONG(EPONA))
#define GBT_CAN_REVERSE_WATER_FLOW                                                         \
    (RANDO_EVENTS[RE_GREAT_BAY_RED_SWITCH_1] && RANDO_EVENTS[RE_GREAT_BAY_RED_SWITCH_2] && \
     HAS_ITEM(ITEM_HOOKSHOT)) // Keeping for the sake of check tracker clarity
#define GBT_GREEN_SWITCH_FLOW                                                                  \
    (RANDO_EVENTS[RE_GREAT_BAY_GREEN_SWITCH_1] && RANDO_EVENTS[RE_GREAT_BAY_GREEN_SWITCH_2] && \
     RANDO_EVENTS[RE_GREAT_BAY_GREEN_SWITCH_3])
#define ONE_WAY_EXIT -1
#define CAN_OWL_WARP(owlId) ((gSaveContext.save.saveInfo.playerData.owlActivationFlags >> owlId) & 1)
#define SET_OWL_WARP(owlId) (gSaveContext.save.saveInfo.playerData.owlActivationFlags |= (1 << owlId))
#define CLEAR_OWL_WARP(owlId) (gSaveContext.save.saveInfo.playerData.owlActivationFlags &= ~(1 << owlId))
#define HAS_BOTTLE_ITEM(item) (Inventory_HasItemInBottle(item))
// TODO: Maybe not reliable because of theif bird stealing bottle
#define HAS_BOTTLE (INV_CONTENT(ITEM_BOTTLE) != ITEM_NONE)
#define CAN_USE_PROJECTILE (HAS_ITEM(ITEM_BOW) || HAS_ITEM(ITEM_HOOKSHOT) || (CAN_BE_DEKU && HAS_MAGIC) || CAN_BE_ZORA)
#define CAN_ACCESS(access) (RANDO_EVENTS[RE_ACCESS_##access])
#define CAN_GROW_BEAN_PLANT        \
    (HAS_ITEM(ITEM_MAGIC_BEANS) && \
     (CAN_PLAY_SONG(STORMS) || (HAS_BOTTLE && (CAN_ACCESS(SPRING_WATER) || CAN_ACCESS(HOT_SPRING_WATER)))))
#define CAN_USE_MAGIC_ARROW(arrowType) (HAS_ITEM(ITEM_BOW) && HAS_ITEM(ITEM_ARROW_##arrowType) && HAS_MAGIC)
#define CAN_LIGHT_TORCH_NEAR_ANOTHER (HAS_ITEM(ITEM_DEKU_STICK) || CAN_USE_MAGIC_ARROW(FIRE))
#define KEY_COUNT(dungeon) (gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys[DUNGEON_SCENE_INDEX_##dungeon])
#define CAN_AFFORD(rc)                                                                                                \
    ((RANDO_SAVE_CHECKS[rc].price < 100) || (RANDO_SAVE_CHECKS[rc].price <= 200 && CUR_UPG_VALUE(UPG_WALLET) >= 1) || \
     (CUR_UPG_VALUE(UPG_WALLET) >= 2))
#define HAS_ENOUGH_STRAY_FAIRIES(dungeonIndex) \
    (gSaveContext.save.saveInfo.inventory.strayFairies[dungeonIndex] >= RANDO_SAVE_OPTIONS[RO_MINIMUM_STRAY_FAIRIES])
#define FOUND_ALL_FROGS                                                                  \
    (CHECK_WEEKEVENTREG(WEEKEVENTREG_33_01) && CHECK_WEEKEVENTREG(WEEKEVENTREG_32_40) && \
     CHECK_WEEKEVENTREG(WEEKEVENTREG_32_80) && CHECK_WEEKEVENTREG(WEEKEVENTREG_33_02))
#define CAN_USE_ABILITY(ability) (Flags_GetRandoInf(RI_ABILITY_##ability - RI_ABILITY_SWIM + RANDO_INF_OBTAINED_SWIM))

#define EVENT(randoEvent, condition)         \
    {                                        \
        randoEvent, [] { return condition; } \
    }
#define EXIT(toEntrance, fromEntrance, condition)                           \
    {                                                                       \
        toEntrance, {                                                       \
            fromEntrance, [] { return condition; }, LogicString(#condition) \
        }                                                                   \
    }
#define CONNECTION(region, condition)                         \
    {                                                         \
        region, {                                             \
            [] { return condition; }, LogicString(#condition) \
        }                                                     \
    }
#define CHECK(check, condition)                               \
    {                                                         \
        check, {                                              \
            [] { return condition; }, LogicString(#condition) \
        }                                                     \
    }
#define STAY(timeSlice, condition)          \
    {                                       \
        timeSlice, [] { return condition; } \
    }

inline std::string LogicString(std::string condition) {
    if (condition == "true")
        return "";

    return condition;
}

inline bool CanAccessDungeon(DungeonSceneIndex dungeonIndex) {
    bool hasSongAccess = false;
    bool hasFormAccess = false;
    switch (dungeonIndex) {
        case DUNGEON_SCENE_INDEX_WOODFALL_TEMPLE:
            hasSongAccess = CAN_PLAY_SONG(SONATA);
            hasFormAccess = CAN_BE_DEKU && HAS_ITEM(ITEM_OCARINA_OF_TIME);
            break;
        case DUNGEON_SCENE_INDEX_SNOWHEAD_TEMPLE:
            hasSongAccess = CAN_PLAY_SONG(LULLABY);
            hasFormAccess = CAN_BE_GORON && HAS_ITEM(ITEM_OCARINA_OF_TIME);
            break;
        case DUNGEON_SCENE_INDEX_GREAT_BAY_TEMPLE:
            hasSongAccess = CAN_PLAY_SONG(BOSSA_NOVA);
            hasFormAccess = CAN_BE_ZORA && HAS_ITEM(ITEM_OCARINA_OF_TIME);
            break;
        default:
            break;
    }
    switch (RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS]) {
        case RO_ACCESS_DUNGEONS_FORM_OR_SONG:
            return hasSongAccess || hasFormAccess;
        case RO_ACCESS_DUNGEONS_FORM_ONLY:
            return hasFormAccess;
        case RO_ACCESS_DUNGEONS_SONG_ONLY:
            return hasSongAccess;
        case RO_ACCESS_DUNGEONS_OPEN:
            return true;
        case RO_ACCESS_DUNGEONS_FORM_AND_SONG:
        default:
            return hasSongAccess && hasFormAccess;
    }
}

inline uint32_t MoonMaskCount() {
    uint32_t count = 0;
    for (int i = ITEM_MASK_TRUTH; i <= ITEM_MASK_GIANT; i++) {
        if (INV_CONTENT(i) == i) {
            count++;
        }
    }
    return count;
}

inline uint32_t RemainsCount() {
    uint32_t count = 0;
    for (int i = QUEST_REMAINS_ODOLWA; i <= QUEST_REMAINS_TWINMOLD; i++) {
        if (CHECK_QUEST_ITEM(i)) {
            count++;
        }
    }
    return count;
}

inline bool MeetsMoonRequirements() {
    return RemainsCount() >= RANDO_SAVE_OPTIONS[RO_ACCESS_MOON_REMAINS_COUNT];
}

// ============================================================================
// CLOCK OWNERSHIP HELPERS
// ============================================================================

inline uint32_t ClockCount() {
    uint32_t count = 0;
    for (int i = 0; i < 6; ++i) {
        if (Flags_GetRandoInf(static_cast<RandoInf>(RANDO_INF_OBTAINED_CLOCK_DAY_1 + i))) {
            count++;
        }
    }
    return count;
}

inline bool SettingClocks() {
    return RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE] != 0;
}

inline bool ClockSeparate(int halfDayIndex) {
    if (halfDayIndex < 0 || halfDayIndex >= 6)
        return false;
    return Flags_GetRandoInf(static_cast<RandoInf>(RANDO_INF_OBTAINED_CLOCK_DAY_1 + halfDayIndex));
}

inline bool ClockAscending(int count) {
    // Only works in ascending mode
    if (RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE_PROGRESSIVE] != RO_CLOCK_SHUFFLE_ASCENDING) {
        return false;
    }
    return ClockCount() >= count;
}

inline bool ClockDescending(int count) {
    // Only works in descending mode
    if (RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE_PROGRESSIVE] != RO_CLOCK_SHUFFLE_DESCENDING) {
        return false;
    }
    return ClockCount() >= count;
}

// ============================================================================
// TIME OPERATOR FUNCTIONS
// ============================================================================

inline bool RawAt(TimeSlice slice) {
    return (gCurrentRegionTime & (TIME_BIT_ONE << slice)) != 0;
}

inline bool RawBefore(TimeSlice slice) {
    if (slice == 0)
        return false;
    uint64_t mask = (TIME_BIT_ONE << slice) - 1;
    return (gCurrentRegionTime & mask) != 0;
}

inline bool RawAfter(TimeSlice slice) {
    uint64_t mask = ~((TIME_BIT_ONE << slice) - 1) & TIME_ALL_SLICES;
    return (gCurrentRegionTime & mask) != 0;
}

inline bool RawBetween(TimeSlice start, TimeSlice end) {
    uint64_t mask = ((TIME_BIT_ONE << end) - 1) & ~((TIME_BIT_ONE << start) - 1);
    return (gCurrentRegionTime & mask) != 0;
}

// ============================================================================
// CLOCK ITEM MACROS
// ============================================================================

#define CLOCK_DAY1() (!SettingClocks() || ClockSeparate(0) || ClockAscending(1) || ClockDescending(6))
#define CLOCK_NIGHT1() (!SettingClocks() || ClockSeparate(1) || ClockAscending(2) || ClockDescending(5))
#define CLOCK_DAY2() (!SettingClocks() || ClockSeparate(2) || ClockAscending(3) || ClockDescending(4))
#define CLOCK_NIGHT2() (!SettingClocks() || ClockSeparate(3) || ClockAscending(4) || ClockDescending(3))
#define CLOCK_DAY3() (!SettingClocks() || ClockSeparate(4) || ClockAscending(5) || ClockDescending(2))
#define CLOCK_NIGHT3() (!SettingClocks() || ClockSeparate(5) || ClockAscending(6) || ClockDescending(1))

// ============================================================================
// COMPOSITE TIME CHECKS
// ============================================================================

#define IS_DAY1() (RawBefore(TIME_NIGHT1_PM_06_00) && CLOCK_DAY1())
#define IS_NIGHT1() (RawBetween(TIME_NIGHT1_PM_06_00, TIME_DAY2_AM_06_00) && CLOCK_NIGHT1())
#define IS_DAY2() (RawBetween(TIME_DAY2_AM_06_00, TIME_NIGHT2_PM_06_00) && CLOCK_DAY2())
#define IS_NIGHT2() (RawBetween(TIME_NIGHT2_PM_06_00, TIME_DAY3_AM_06_00) && CLOCK_NIGHT2())
#define IS_DAY3() (RawBetween(TIME_DAY3_AM_06_00, TIME_NIGHT3_PM_06_00) && CLOCK_DAY3())
#define IS_NIGHT3() (RawAfter(TIME_NIGHT3_PM_06_00) && CLOCK_NIGHT3())

#define IS_DAY() (IS_DAY1() || IS_DAY2() || IS_DAY3())
#define IS_NIGHT() (IS_NIGHT1() || IS_NIGHT2() || IS_NIGHT3())
#define FIRST_DAY() (IS_DAY1() || IS_NIGHT1())
#define SECOND_DAY() (IS_DAY2() || IS_NIGHT2())
#define FINAL_DAY() (IS_DAY3() || IS_NIGHT3())

// ============================================================================
// PUBLIC TIME API
// ============================================================================

#define AT(slice)    \
    (RawAt(slice) && \
     (!SettingClocks() || IS_DAY1() || IS_NIGHT1() || IS_DAY2() || IS_NIGHT2() || IS_DAY3() || IS_NIGHT3()))
#define BEFORE(slice)    \
    (RawBefore(slice) && \
     (!SettingClocks() || IS_DAY1() || IS_NIGHT1() || IS_DAY2() || IS_NIGHT2() || IS_DAY3() || IS_NIGHT3()))
#define AFTER(slice)    \
    (RawAfter(slice) && \
     (!SettingClocks() || IS_DAY1() || IS_NIGHT1() || IS_DAY2() || IS_NIGHT2() || IS_DAY3() || IS_NIGHT3()))
#define BETWEEN(s, e)    \
    (RawBetween(s, e) && \
     (!SettingClocks() || IS_DAY1() || IS_NIGHT1() || IS_DAY2() || IS_NIGHT2() || IS_DAY3() || IS_NIGHT3()))

#define MIDNIGHT()                                                                                             \
    (BETWEEN(TIME_NIGHT1_AM_12_00, TIME_DAY2_AM_06_00) || BETWEEN(TIME_NIGHT2_AM_12_00, TIME_DAY3_AM_06_00) || \
     AFTER(TIME_NIGHT3_AM_12_00))

// ============================================================================
// COMPLEX TIME MACROS
// ============================================================================

#define GRANDMA_STORY_1()                                                                                   \
    (BEFORE(TIME_DAY1_PM_04_00) || BETWEEN(TIME_DAY2_AM_06_00, TIME_DAY2_PM_04_00) ||                       \
     (IS_DAY1() && (CLOCK_NIGHT1() || CLOCK_DAY2() || CLOCK_NIGHT2() || CLOCK_DAY3() || CLOCK_NIGHT3())) || \
     (IS_DAY2() && (CLOCK_NIGHT2() || CLOCK_DAY3() || CLOCK_NIGHT3())))

#define GRANDMA_STORY_2()                                                                 \
    ((IS_DAY1() && (CLOCK_DAY2() || CLOCK_NIGHT2() || CLOCK_DAY3() || CLOCK_NIGHT3())) || \
     (IS_DAY2() && (CLOCK_DAY3() || CLOCK_NIGHT3())))

inline bool CanKillEnemy(ActorId EnemyId) {
    switch (EnemyId) {
        case ACTOR_BOSS_01: // Odolwa
            return (CAN_USE_SWORD || CAN_BE_GORON || CAN_BE_ZORA || CAN_USE_EXPLOSIVE || CAN_USE_MAGIC_ARROW(FIRE) ||
                    CAN_USE_MAGIC_ARROW(LIGHT)) &&
                   (Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_ODOLWA) ||
                    RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_SOULS] == RO_GENERIC_NO);
        case ACTOR_BOSS_02: // Twinmold
            return (HAS_ITEM(ITEM_BOW) || (HAS_ITEM(ITEM_MASK_GIANT) && HAS_MAGIC && CAN_USE_HUMAN_SWORD)) &&
                   (Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_TWINMOLD) ||
                    RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_SOULS] == RO_GENERIC_NO);
        case ACTOR_BOSS_03: // Gyorg
            return ((CAN_BE_DEITY && HAS_MAGIC) || (CAN_BE_ZORA && HAS_MAGIC)) &&
                   (Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_GYORG) ||
                    RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_SOULS] == RO_GENERIC_NO);
        case ACTOR_BOSS_04: // Wart
            return (HAS_ITEM(ITEM_BOW) || HAS_ITEM(ITEM_HOOKSHOT) || CAN_BE_ZORA);
        case ACTOR_BOSS_HAKUGIN: // Goht
            return (CAN_USE_MAGIC_ARROW(FIRE)) && (Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_GOHT) ||
                                                   RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_SOULS] == RO_GENERIC_NO);
        case ACTOR_EN_KNIGHT: // Igos du Ikana/IdI Lackey
            return (CAN_USE_MAGIC_ARROW(FIRE) &&
                    (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) >= EQUIP_VALUE_SHIELD_MIRROR) &&
                    (CAN_USE_HUMAN_SWORD || CAN_BE_DEKU || CAN_BE_GORON || CAN_BE_ZORA));
        case ACTOR_EN_KAIZOKU: // Fighter Pirate
            return (CAN_USE_SWORD || CAN_BE_ZORA);
        case ACTOR_EN_PAMETFROG: // Swamp Gekko
            return (HAS_ITEM(ITEM_BOW) && (CAN_BE_DEKU || CAN_USE_EXPLOSIVE || CAN_BE_GORON));
        case ACTOR_EN_BIGSLIME: // Great Bay Gekko
            return (CAN_USE_MAGIC_ARROW(ICE));
        case ACTOR_EN_SW: // Gold Skulltula
            return (CAN_USE_PROJECTILE || CAN_BE_DEKU || CAN_BE_GORON || CAN_USE_HUMAN_SWORD || CAN_USE_EXPLOSIVE);
        case ACTOR_EN_DINOFOS: // Dinofos
            return (CAN_USE_SWORD || CAN_BE_GORON || HAS_ITEM(ITEM_BOW) || (CAN_BE_DEKU && HAS_MAGIC));
        case ACTOR_EN_WIZ: // Wizrobe
            return (HAS_ITEM(ITEM_BOW) || HAS_ITEM(ITEM_HOOKSHOT) || CAN_USE_SWORD || CAN_BE_GORON);
        case ACTOR_EN_WF: // Wolfos
            return (CAN_USE_HUMAN_SWORD || (CAN_BE_DEKU && HAS_MAGIC) || CAN_BE_GORON || CAN_BE_ZORA);
        case ACTOR_EN_JSO2: // Garo Master
            return (HAS_ITEM(ITEM_BOW) || CAN_BE_GORON || CAN_USE_SWORD);
        case ACTOR_EN_IK: // Iron Knuckle
            return (CAN_USE_HUMAN_SWORD || CAN_BE_GORON);
        case ACTOR_EN_GRASSHOPPER: // Dragonfly
            return ((CAN_BE_DEKU && HAS_MAGIC) || CAN_USE_EXPLOSIVE || HAS_ITEM(ITEM_DEKU_NUT) || CAN_USE_SWORD ||
                    CAN_BE_ZORA);
        case ACTOR_EN_MKK: // Boe
            return ((CAN_BE_DEKU && HAS_MAGIC) || CAN_USE_EXPLOSIVE || HAS_ITEM(ITEM_DEKU_NUT) || CAN_USE_SWORD ||
                    CAN_BE_ZORA || CAN_BE_GORON);
        case ACTOR_EN_BIGPAMET: // Snapper
            return (CAN_BE_DEKU || CAN_USE_EXPLOSIVE || CAN_BE_GORON);
        case ACTOR_EN_ST: // Large Skulltula
            return (CAN_USE_SWORD || CAN_USE_PROJECTILE || CAN_BE_GORON || CAN_USE_EXPLOSIVE);
        case ACTOR_EN_BAT: // Bat Bat
            return (CAN_USE_SWORD || HAS_ITEM(ITEM_HOOKSHOT) || HAS_ITEM(ITEM_BOW) || CAN_USE_EXPLOSIVE ||
                    CAN_BE_GORON || CAN_BE_ZORA);
        case ACTOR_EN_DEKUBABA: // Neck bending Deku Baba
            return (CAN_USE_HUMAN_SWORD || CAN_BE_DEKU || CAN_BE_GORON || CAN_BE_ZORA || HAS_ITEM(ITEM_BOW) ||
                    CAN_USE_EXPLOSIVE || HAS_ITEM(ITEM_DEKU_STICK));
        case ACTOR_OBJ_SNOWBALL: // Large Snowball
            return (CAN_USE_EXPLOSIVE || CAN_BE_GORON || CAN_USE_MAGIC_ARROW(FIRE));
        default: // Incorrect actor ID inputed.
            assert(false);
            return false;
    }
}

} // namespace Logic

} // namespace Rando

#endif // RANDO_LOGIC_H
