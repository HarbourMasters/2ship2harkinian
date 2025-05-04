#include "Handlers.h"
#include "../../AchievementData.h"       // For GetAchievementFromEndOfCycleSave, AllAchievementData
#include "../../Achievements.h"          // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"       // For IS_RANDO
#include "libultraship/libultraship.h" // For CVarGetInteger
#include <spdlog/spdlog.h>

namespace Handlers {

// Handler function for the AfterEndOfCycleSave hook
void HandleEndOfCycleSaveTrigger() {
    // Iterate through all achievements, looking for those specifically triggered by AfterEndOfCycleSave
    for (auto const& [key, achData] : AllAchievementData) {
        if (achData.triggerType == AchievementTriggerType::AfterEndOfCycleSave) {
            // Check common conditions (unlocked, relevant, loading, additional lambda)
            if (CheckCommonAchievementConditions(achData)) {
                SPDLOG_DEBUG("[Achievements] EndOfCycleSave Triggered: Queuing Achievement: {}", achData.name);
                AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
                // Note: No break; multiple achievements could potentially trigger on the same hook
            }
        }
    }
}

// Registers the AfterEndOfCycleSave hook
void InitEndOfCycleSaveHandlers() {
    COND_HOOK(AfterEndOfCycleSave, CVAR_ACHIEVEMENTS, HandleEndOfCycleSaveTrigger);
}

} // namespace Handlers 