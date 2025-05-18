#include "Handlers.h"
#include "../../AchievementData.h"              // For AllAchievementData -> GetAchievementsFromFlagSet
#include "../../Achievements.h"                 // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"                   // For IS_RANDO
#include "libultraship/libultraship.h"          // For CVarGetInteger
#include <spdlog/spdlog.h>                      // For logging

namespace Handlers {

// Handler function for the OnFlagSet hook
void HandleFlagSetTrigger(FlagType flagType, u32 flag) {
    // Get all achievements that might be triggered by this specific flag event.
    std::vector<AchievementStaticData> potentialAchievements = GetAchievementsFromFlagSet(flagType, flag);

    for (const auto& achData : potentialAchievements) {
        // Basic pre-checks (loading, unlocked, relevant mode)
        if (AchievementSystem::Instance().IsLoadingOrInitializing() ||
            AchievementSystem::Instance().IsAchievementUnlocked(achData.id) ||
            !AchievementSystem::Instance().IsAchievementRelevantForGameMode(achData.id, IS_RANDO)) {
            continue; // Skip if basic conditions not met
        }

        // If we reach here, basic pre-conditions are met.
        if (achData.hasProgressTracking) {
            SPDLOG_DEBUG("[Achievements] FlagSet: Updating progress for {}. Type={}, Flag={}", achData.name,
                         (int)flagType, flag);
            AchievementSystem::Instance().UpdateAchievementProgress(achData.id);
        } else {
            // Not progress-tracked, so check additionalCondition directly for unlock
            if (achData.additionalCondition == nullptr || achData.additionalCondition()) {
                SPDLOG_DEBUG("[Achievements] FlagSet: Queuing achievement {}. Type={}, Flag={}", achData.name,
                             (int)flagType, flag);
                AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
            }
        }
    }
}

// Registers the OnFlagSet hook
void InitFlagSetHandlers() {
    COND_HOOK(OnFlagSet, CVAR_ACHIEVEMENTS, HandleFlagSetTrigger);
}

} // namespace Handlers