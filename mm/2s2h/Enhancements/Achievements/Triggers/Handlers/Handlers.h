#pragma once

#include "libultraship/libultraship.h" // For CVarGetInteger

// Central header to declare initializer functions for all trigger handlers.
// Mirrors the pattern of Rando::ActorBehavior calling Init*Behavior functions.

// CVar Definitions (Centralized)
#define CVAR_ACHIEVEMENTS                         \
    (CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1) && \
     (gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled))

// Includes needed for the helper function
#include "../../Achievements.h"    // For AchievementSystem singleton
#include "../../AchievementData.h" // For AchievementStaticData
#include "2s2h/Rando/Rando.h"      // For IS_RANDO
#include <spdlog/spdlog.h>         // Potentially needed if logging is added here later

namespace Handlers {

// Helper function to check common conditions for queuing an achievement
// Returns true if the achievement is not already unlocked, relevant for the game mode,
// not currently loading, AND satisfies its additionalCondition (if any).
static inline bool CheckCommonAchievementConditions(const AchievementStaticData& achData) {
    // Early exit if loading/initializing
    if (AchievementSystem::Instance().IsLoadingOrInitializing()) {
        return false;
    }

    // Check if already unlocked or not relevant for the current game mode
    if (AchievementSystem::Instance().IsAchievementUnlocked(achData.id) ||
        !AchievementSystem::Instance().IsAchievementRelevantForGameMode(achData.id, IS_RANDO)) {
        return false;
    }

    // Check additional condition ONLY if it exists
    // If it exists and returns false, the conditions are not met.
    if (achData.additionalCondition && !achData.additionalCondition()) {
        return false;
    }

    // All checks passed
    return true;
}

void InitFlagSetHandlers();
void InitItemGiveHandlers();
void InitSceneInitHandlers();
void InitActorInitHandlers();
void InitEndOfCycleSaveHandlers();
void InitVanillaBehaviorHandlers();

// Add declarations for new handlers here

} // namespace Handlers