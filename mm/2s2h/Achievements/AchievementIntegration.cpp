#include "AchievementIntegration.h"
#include "Core.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
}

namespace Achievements {

namespace Integration {

// Non-static vanilla behavior map - populated at runtime in Init() to avoid static initialization order issues
std::map<GIVanillaBehavior, std::vector<std::pair<std::function<bool(va_list)>, AchievementEvent>>> vanillaBehaviorMap;

// Short macros for enum values
#define VB(flag) GIVanillaBehavior::flag
#define AE(event) AchievementEvent::event
#define FT(type) FlagType::type

// Achievement tracking macros
#define TRACK_FLAG(flagType, flag, event) \
    { { FT(flagType), flag }, AE(event) }

#define TRACK_SCENE_FLAG(sceneId, flagType, flag, event) \
    { { sceneId, FT(flagType), flag }, AE(event) }

// Usage: TRACK_VB(event, condition)
#define TRACK_VB(event, ...)                                 \
    {                                                        \
        [](va_list args) -> bool { __VA_ARGS__; }, AE(event) \
    }

// Flag to achievement event mapping
static std::map<std::pair<FlagType, u32>, AchievementEvent> flagToEventMap;

// Scene-specific flag to achievement event mapping
static std::map<std::tuple<s16, FlagType, u32>, AchievementEvent> sceneFlagToEventMap;

void Init() {
    // clang-format off
    flagToEventMap = {
        // Honorary Member events
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_LEARNED_SECRET_CODE, 
                   EVENT_LEARNED_SECRET_CODE),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BOMBERS_NOTEBOOK, 
                   EVENT_RECEIVED_BOMBERS_NOTEBOOK),
        
        // Faithful Bride events
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROOM_KEY, 
                   EVENT_RECEIVED_ROOM_KEY),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_MIDNIGHT_MEETING, 
                   EVENT_PROMISED_MIDNIGHT_MEETING),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_LETTER_TO_KAFEI, 
                   EVENT_RECEIVED_LETTER_TO_KAFEI),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DELIVERED_PENDANT_OF_MEMORIES, 
                   EVENT_DELIVERED_PENDANT_OF_MEMORIES),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_COUPLES_MASK, 
                   EVENT_RECEIVED_COUPLES_MASK),
        
        // Mini-He events
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_TO_MEET_KAFEI, 
                   EVENT_PROMISED_TO_MEET_KAFEI),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_PENDANT_OF_MEMORIES, 
                   EVENT_RECEIVED_PENDANT_OF_MEMORIES),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_ESCAPED_SAKONS_HIDEOUT, 
                   EVENT_ESCAPED_SAKONS_HIDEOUT),
        
        // Ambiguous Allegiance events
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KEATON_MASK, 
                   EVENT_RECEIVED_KEATON_MASK),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_PRIORITY_MAIL, 
                   EVENT_RECEIVED_PRIORITY_MAIL),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ALL_NIGHT_MASK, 
                   EVENT_RECEIVED_ALL_NIGHT_MASK),
        
        // Frail D.I.D. event
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BLAST_MASK, 
                   EVENT_RECEIVED_BLAST_MASK),
        
        // Brave Little Archer events
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_TO_HELP_WITH_THEM, 
                   EVENT_PROMISED_TO_HELP_WITH_THEM),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DEFENDED_AGAINST_THEM, 
                   EVENT_DEFENDED_AGAINST_THEM),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_MILK_BOTTLE, 
                   EVENT_RECEIVED_MILK_BOTTLE),
        
        // Mature Ranch Hand events
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_ESCORTED_CREMIA, 
                   EVENT_ESCORTED_CREMIA),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROMANIS_MASK, 
                   EVENT_RECEIVED_ROMANIS_MASK),
        
        // Single-event achievements
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_MAYOR_HP, 
                   EVENT_RECEIVED_MAYOR_HP),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KAFEIS_MASK, 
                   EVENT_RECEIVED_KAFEIS_MASK),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DELIVERED_PRIORITY_MAIL, 
                   EVENT_DELIVERED_PRIORITY_MAIL),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_CIRCUS_LEADERS_MASK, 
                   EVENT_RECEIVED_CIRCUS_LEADERS_MASK),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_GORMAN, 
                   EVENT_MET_GORMAN),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_POSTMAN_HP, 
                   EVENT_RECEIVED_POSTMAN_HP),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_POSTMANS_HAT, 
                   EVENT_RECEIVED_POSTMANS_HAT),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROSA_SISTERS_HP, 
                   EVENT_RECEIVED_ROSA_SISTERS_HP),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_TOILET_HAND_HP, 
                   EVENT_RECEIVED_TOILET_HAND_HP),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GRANDMA_SHORT_STORY_HP, 
                   EVENT_RECEIVED_GRANDMA_SHORT_STORY_HP),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GRANDMA_LONG_STORY_HP, 
                   EVENT_RECEIVED_GRANDMA_LONG_STORY_HP),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KAMAROS_MASK, 
                   EVENT_RECEIVED_KAMAROS_MASK),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BUNNY_HOOD, 
                   EVENT_RECEIVED_BUNNY_HOOD),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GAROS_MASK, 
                   EVENT_RECEIVED_GAROS_MASK),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_STONE_MASK, 
                   EVENT_RECEIVED_STONE_MASK),
        TRACK_FLAG(FLAG_WEEK_EVENT_REG, WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BREMEN_MASK, 
                   EVENT_RECEIVED_BREMEN_MASK),
    };
    // clang-format on

    // clang-format off
    sceneFlagToEventMap = {
        // Scene-specific achievements
    };
    // clang-format on

    // clang-format off
    vanillaBehaviorMap = {
        // Ocarina song achievements
        { VB(VB_START_CUTSCENE), {
            TRACK_VB(EVENT_PLAYED_SONG_OF_DOUBLE_TIME, 
                     return gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_APPLY_DOUBLE_SOT),
            
            TRACK_VB(EVENT_PLAYED_INVERTED_SONG_OF_TIME, 
                     u8 ocarinaMode = gPlayState->msgCtx.ocarinaMode;
                     return ocarinaMode == OCARINA_MODE_APPLY_INV_SOT_FAST || 
                            ocarinaMode == OCARINA_MODE_APPLY_INV_SOT_SLOW),
        }},
    };
    // clang-format on
}

void OnFlagSet(FlagType flagType, u32 flag) {
    auto it = flagToEventMap.find({ flagType, flag });
    if (it != flagToEventMap.end()) {
        QUEUE_ACHIEVEMENT(it->second);
    }
}

void OnSceneFlagSet(s16 sceneId, FlagType flagType, u32 flag) {
    auto it = sceneFlagToEventMap.find({ sceneId, flagType, flag });
    if (it != sceneFlagToEventMap.end()) {
        QUEUE_ACHIEVEMENT(it->second);
    }
}

void OnVanillaBehavior(GIVanillaBehavior flag, bool* should, va_list originalArgs) {
    auto it = vanillaBehaviorMap.find(flag);
    if (it != vanillaBehaviorMap.end()) {
        for (const auto& condition : it->second) {
            if (condition.first(originalArgs)) {
                QUEUE_ACHIEVEMENT(condition.second);
            }
        }
    }
}

} // namespace Integration

} // namespace Achievements