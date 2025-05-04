#include "Handlers.h"
#include "../../AchievementData.h"       // For AllAchievementData
#include "../../Achievements.h"          // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"       // For IS_RANDO
#include "libultraship/libultraship.h" // For CVarGetInteger
#include <spdlog/spdlog.h>            // For logging

namespace Handlers {

// Handler function for the OnFlagSet hook
void HandleFlagSetTrigger(FlagType flagType, u32 flag) {
    // Iterate ALL achievements, as one flag might trigger multiple checks (especially FLAG_RANDO_INF)
    for (auto const& [key, achData] : AllAchievementData) {
        if (achData.triggerType == AchievementTriggerType::OnFlagSet) {
            // Determine if the specific flag trigger matches this achievement's requirements
            bool triggerMatches = false;
            if (achData.checkFlagType == FLAG_RANDO_INF && flagType == FLAG_RANDO_INF) {
                // Rando checks are triggered by any RANDO_INF flag; the lambda handles specifics.
                triggerMatches = true;
            } else if (achData.checkFlagType == flagType && achData.checkFlag == flag) {
                // Standard check for specific flag type and value.
                triggerMatches = true;
            }

            // If the trigger matches, check common conditions (unlocked, relevant, loading, additional lambda)
            if (triggerMatches && CheckCommonAchievementConditions(achData)) {
                SPDLOG_DEBUG("[Achievements] FlagSet Triggered: Type={}, Flag={}, Queuing Achievement: {}", (int)flagType, flag, achData.name);
                AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
                // Note: Don't break here, one flag could unlock multiple achievements
            }
        }
    }
}

// Registers the OnFlagSet hook
void InitFlagSetHandlers() {
    COND_HOOK(OnFlagSet, CVAR_ACHIEVEMENTS, HandleFlagSetTrigger);
}

} // namespace Handlers 