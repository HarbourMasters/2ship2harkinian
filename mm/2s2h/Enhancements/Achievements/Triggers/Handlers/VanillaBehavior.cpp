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
        // Check common conditions (unlocked, relevant, loading, additional lambda)
        // Note: Lambdas here need to be careful about accessing va_list or assuming gPlayState is valid
        if (CheckCommonAchievementConditions(achData)) {
            SPDLOG_DEBUG("[Achievements] VanillaBehavior Triggered: HookID={}, Queuing Achievement: {}",
                         (int)hookEnumId, achData.name);
            AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
            // IMPORTANT: We never modify *should for achievements
            // Do not break; multiple achievements could trigger on the same hook ID
        }
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