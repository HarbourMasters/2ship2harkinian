#include "Handlers.h"
#include "../../AchievementData.h"              // For GetAchievementsFromEndOfCycleSave
#include "../../Achievements.h"                 // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"                   // For IS_RANDO
#include "libultraship/libultraship.h"          // For CVarGetInteger
#include <spdlog/spdlog.h>

namespace Handlers {

// Handler function for the AfterEndOfCycleSave hook
void HandleEndOfCycleSaveTrigger() {
    // Get all achievements that might be triggered by this event.
    std::vector<AchievementStaticData> potentialAchievements = GetAchievementsFromEndOfCycleSave();

    for (const auto& achData : potentialAchievements) {
        // Basic pre-checks
        if (AchievementSystem::Instance().IsLoadingOrInitializing() ||
            AchievementSystem::Instance().IsAchievementUnlocked(achData.id) ||
            !AchievementSystem::Instance().IsAchievementRelevantForGameMode(achData.id, IS_RANDO)) {
            continue; // Skip if basic conditions not met
        }

        // If we reach here, basic pre-conditions are met.
        if (achData.hasProgressTracking) {
            SPDLOG_DEBUG("[Achievements] EndOfCycleSave: Updating progress for {}", achData.name);
            AchievementSystem::Instance().UpdateAchievementProgress(achData.id);
        } else {
            // Not progress-tracked, check additionalCondition directly
            if (achData.additionalCondition == nullptr || achData.additionalCondition()) {
                SPDLOG_DEBUG("[Achievements] EndOfCycleSave: Queuing achievement {}", achData.name);
                AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
            }
        }
        // Note: No break; multiple achievements could potentially trigger on the same hook
    }
}

// Registers the AfterEndOfCycleSave hook
void InitEndOfCycleSaveHandlers() {
    COND_HOOK(AfterEndOfCycleSave, CVAR_ACHIEVEMENTS, HandleEndOfCycleSaveTrigger);
}

} // namespace Handlers