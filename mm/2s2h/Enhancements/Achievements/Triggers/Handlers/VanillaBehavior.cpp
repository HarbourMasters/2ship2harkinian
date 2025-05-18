#include "Handlers.h"
#include "../../AchievementData.h"              // For GetAchievementsFromVanillaBehavior, AllAchievementData
#include "../../Achievements.h"                 // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_ID_HOOK
#include "2s2h/Rando/Rando.h"                   // For IS_RANDO
#include "libultraship/libultraship.h"          // For CVarGetInteger
#include <spdlog/spdlog.h>
#include <set> // For finding unique VB hook IDs

namespace Handlers {

// Handler function for the ShouldVanillaBehavior hook
// This function is registered for specific GIVanillaBehavior IDs used by achievements.
void HandleVanillaBehaviorTrigger(GIVanillaBehavior hookEnumId, bool* should, va_list args) {
    // Use helper to get all achievements associated with this specific VB Hook ID
    std::vector<AchievementStaticData> relevantAchievements = GetAchievementsFromVanillaBehavior(hookEnumId);

    for (const auto& achData : relevantAchievements) {
        // Basic pre-checks
        if (AchievementSystem::Instance().IsLoadingOrInitializing() ||
            AchievementSystem::Instance().IsAchievementUnlocked(achData.id) ||
            !AchievementSystem::Instance().IsAchievementRelevantForGameMode(achData.id, IS_RANDO)) {
            continue; // Skip if basic conditions not met
        }

        // If we reach here, basic pre-conditions are met.
        // Note: additionalCondition lambdas (type std::function<bool()>) for VB hooks do not receive the hook's va_list
        // arguments. They are intended for checking general game state (e.g., via gPlayState, gSaveContext) or other
        // conditions not directly part of the hook's parameters.
        if (achData.hasProgressTracking) {
            SPDLOG_DEBUG("[Achievements] VanillaBehavior: Updating progress for {}. HookID: {}", achData.name,
                         (int)hookEnumId);
            AchievementSystem::Instance().UpdateAchievementProgress(achData.id);
        } else {
            // Not progress-tracked, check additionalCondition directly
            if (achData.additionalCondition == nullptr || achData.additionalCondition()) {
                SPDLOG_DEBUG("[Achievements] VanillaBehavior: Queuing achievement {}. HookID: {}", achData.name,
                             (int)hookEnumId);
                AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
            }
        }
        // IMPORTANT: We never modify *should for achievements
        // Do not break; multiple achievements could trigger on the same hook ID
    }
}

// Registers ShouldVanillaBehavior hooks for each unique ID used by achievements
void InitVanillaBehaviorHandlers() {
    // Find all unique GIVanillaBehavior IDs used by achievements
    std::set<GIVanillaBehavior> usedVbHookIds;
    for (const auto& [key, achData] : AllAchievementData) {
        if (achData.triggerType == AchievementTriggerType::ShouldVanillaBehavior &&
            achData.checkVbHookId != (GIVanillaBehavior)-1) {
            usedVbHookIds.insert(achData.checkVbHookId);
        }
    }

    // Register a specific COND_ID_HOOK for each used ID
    for (GIVanillaBehavior vbId : usedVbHookIds) {
        SPDLOG_TRACE("[Achievements] Registering VB Hook for ID: {}", (int)vbId);
        // The HandleVanillaBehaviorTrigger function will be called only when this specific vbId hook runs
        COND_ID_HOOK(ShouldVanillaBehavior, vbId, CVAR_ACHIEVEMENTS, HandleVanillaBehaviorTrigger);
    }
}

} // namespace Handlers