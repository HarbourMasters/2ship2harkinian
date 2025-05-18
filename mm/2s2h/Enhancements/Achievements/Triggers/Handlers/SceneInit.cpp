#include "Handlers.h"
#include "../../AchievementData.h"              // For GetAchievementsFromSceneInit
#include "../../Achievements.h"                 // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"                   // For IS_RANDO
#include "libultraship/libultraship.h"          // For CVarGetInteger
#include <spdlog/spdlog.h>

namespace Handlers {

// Handler function for the OnSceneInit hook
void HandleSceneInitTrigger(s16 sceneId, s8 spawnNum) { // spawnNum currently unused by achievements
    // Get all achievements that might be triggered by this specific scene init event.
    std::vector<AchievementStaticData> potentialAchievements = GetAchievementsFromSceneInit(sceneId);

    for (const auto& achData : potentialAchievements) {
        // Basic pre-checks
        if (AchievementSystem::Instance().IsLoadingOrInitializing() ||
            AchievementSystem::Instance().IsAchievementUnlocked(achData.id) ||
            !AchievementSystem::Instance().IsAchievementRelevantForGameMode(achData.id, IS_RANDO)) {
            continue; // Skip if basic conditions not met
        }

        // If we reach here, basic pre-conditions are met.
        if (achData.hasProgressTracking) {
            SPDLOG_DEBUG("[Achievements] SceneInit: Updating progress for {}. Scene: {}", achData.name, sceneId);
            AchievementSystem::Instance().UpdateAchievementProgress(achData.id);
        } else {
            // Not progress-tracked, check additionalCondition directly
            if (achData.additionalCondition == nullptr || achData.additionalCondition()) {
                SPDLOG_DEBUG("[Achievements] SceneInit: Queuing achievement {}. Scene: {}", achData.name, sceneId);
                AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
            }
        }
        // Note: Don't break, other achievements might trigger on the same scene init
    }
}

// Registers the OnSceneInit hook
void InitSceneInitHandlers() {
    COND_HOOK(OnSceneInit, CVAR_ACHIEVEMENTS, HandleSceneInitTrigger);
}

} // namespace Handlers