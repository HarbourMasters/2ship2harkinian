#pragma once

#include "Achievements.h"                       // For AchievementCategory
#include "AchievementTypes.h"                   // For AchievementId
#include "2s2h/GameInteractor/GameInteractor.h" // For FlagType, GIVanillaBehavior, etc.
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <z64item.h> // For ITEM_*
#include <limits>    // For numeric_limits

// Define a constant for use with checkSceneId when an achievement should be checked on any scene init
const s16 SCENE_ID_CHECK_ALWAYS = -1; // Or use std::numeric_limits<s16>::min() etc.

// Enum to describe how an achievement is triggered
// Mirrors the GameInteractor hooks primarily used
enum class AchievementTriggerType {
    MANUAL, // Not triggered by standard hooks (maybe debug/special cases)
    OnFlagSet,
    OnItemGive,
    OnSceneInit,
    OnActorInit, // Use sparingly - potentially frequent checks
    AfterEndOfCycleSave,
    ShouldVanillaBehavior,
    // Add other GameInteractor hook types here if needed in the future
};

// Struct holding static definition data for an achievement
// Mirrors RandoStaticCheck/Item structure
struct AchievementStaticData {
    AchievementId id = AID_UNKNOWN;
    const char* name = "Unknown Achievement";
    const char* description = "";
    const char* iconPath = ""; // Can use gItemIcons[ITEM_ID]
    bool isSecret = false;
    int gamerscore = 0;
    AchievementCategory category = AchievementCategory::BOTH;
    bool isInternal = false; // True if this achievement is for internal logic and not player-facing

    // Triggering Info
    AchievementTriggerType triggerType = AchievementTriggerType::MANUAL;

    // Data specific to trigger types (use appropriate fields)
    FlagType checkFlagType = FLAG_NONE; // For OnFlagSet
    u32 checkFlag = 0;                  // For OnFlagSet
    u8 checkItemId = ITEM_NONE;         // For OnItemGive
    s16 checkSceneId = -1;              // For OnSceneInit
    // s8 checkSpawnNum = -1;                       // For OnSceneInit (if needed, currently unused)
    s16 checkActorId = -1; // For OnActorInit (if needing specific actor ID check)
    // u16 checkActorParams = 0xFFFF;               // For OnActorInit (if needing specific params check)
    GIVanillaBehavior checkVbHookId = (GIVanillaBehavior)-1; // For ShouldVanillaBehavior

    // Optional: Filter for item categories when checkItemId is ITEM_NONE for OnItemGive triggers.
    std::function<bool(u8 receivedItem)> itemCategoryFilter = nullptr;

    // Progress Tracking Fields
    bool hasProgressTracking = false;
    s32 targetProgress =
        0; // Value needed to unlock; 0 if not applicable or no progress. Used if getTargetProgress is null.
    std::function<s32()> getCurrentProgress = nullptr; // Lambda to calculate current progress from live game state
    std::function<s32()> getTargetProgress = nullptr;  // Optional lambda to calculate dynamic target progress.
    bool unlockOnTargetMet = true; // If true, meeting progress target (and additionalCondition if any) queues unlock

    // Optional additional condition lambda (use if trigger data alone isn't sufficient)
    // Can access global state like gSaveContext, gPlayState etc. but use with caution.
    // Should return true if the condition is met, false otherwise.
    std::function<bool()> additionalCondition = nullptr;
};

// Global map holding all static achievement definitions
// Mirrors Rando's extern std::map<...> StaticData::Checks;
extern std::map<AchievementId, AchievementStaticData> AllAchievementData;

// Helper functions to find achievement data based on trigger parameters
// These functions return a vector of all achievements that primarily match the event.
// The handler will then iterate this vector and apply common/secondary conditions.
std::vector<AchievementStaticData> GetAchievementsFromFlagSet(FlagType flagType, u32 flag);
std::vector<AchievementStaticData> GetAchievementsFromItemGive();
std::vector<AchievementStaticData> GetAchievementsFromSceneInit(s16 sceneId);
// Note: ActorInit check is done via iteration + additionalCondition due to complexity in its handler
// std::vector<AchievementStaticData> GetAchievementsFromActorInit(s16 actorId, u16 params); // Example if needed later
std::vector<AchievementStaticData> GetAchievementsFromEndOfCycleSave(); // Since this trigger has no params
std::vector<AchievementStaticData>
GetAchievementsFromVanillaBehavior(GIVanillaBehavior vbHookId); // Can return multiple