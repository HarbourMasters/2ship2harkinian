#include "Handlers.h"
#include "../../AchievementData.h"       // For GetAchievementsFromEndOfCycleSave
#include "../../Achievements.h"          // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"       // For IS_RANDO
#include "libultraship/libultraship.h" // For CVarGetInteger
#include <spdlog/spdlog.h>

namespace Handlers {

// Handler function for the AfterEndOfCycleSave hook
void HandleEndOfCycleSaveTrigger() {
    // Get all achievements that might be triggered by this event.
    std::vector<AchievementStaticData> potentialAchievements = GetAchievementsFromEndOfCycleSave();

    for (const auto& achData : potentialAchievements) {
        // Check common conditions (unlocked, relevant, loading, additional lambda)
        if (CheckCommonAchievementConditions(achData)) {
            SPDLOG_DEBUG("[Achievements] EndOfCycleSave Triggered: Queuing Achievement: {}", achData.name);
            AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
            // Note: No break; multiple achievements could potentially trigger on the same hook
        }
    }
}

// Registers the AfterEndOfCycleSave hook
void InitEndOfCycleSaveHandlers() {
    COND_HOOK(AfterEndOfCycleSave, CVAR_ACHIEVEMENTS, HandleEndOfCycleSaveTrigger);
}

} // namespace Handlers 