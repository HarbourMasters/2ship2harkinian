#include "Handlers.h"
#include "../../AchievementData.h"              // For AllAchievementData
#include "../../Achievements.h"                 // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"                   // For IS_RANDO
#include "libultraship/libultraship.h"          // For CVarGetInteger
#include <spdlog/spdlog.h>

namespace Handlers {

// Handler function for the OnActorInit hook
void HandleActorInitTrigger(Actor* actor) {
    // Iterate through all achievements, looking for those specifically triggered by OnActorInit
    for (auto const& [key, achData] : AllAchievementData) {
        if (achData.triggerType == AchievementTriggerType::OnActorInit) {
            // Basic pre-checks (moved these out of CheckCommonAchievementConditions for clarity here)
            if (AchievementSystem::Instance().IsLoadingOrInitializing() ||
                AchievementSystem::Instance().IsAchievementUnlocked(achData.id) ||
                !AchievementSystem::Instance().IsAchievementRelevantForGameMode(achData.id, IS_RANDO)) {
                continue; // Skip if basic conditions not met
            }

            // Process based on tracking type
            if (achData.hasProgressTracking) {
                SPDLOG_DEBUG("[Achievements] ActorInit: Updating progress for {}. Actor: {}", achData.name,
                             (void*)actor);
                AchievementSystem::Instance().UpdateAchievementProgress(achData.id);
            } else {
                // Not progress-based: check additionalCondition directly (which CheckCommonAchievementConditions does
                // part of), then queue unlock. CheckCommonAchievementConditions effectively checks the remaining
                // conditions. We've already done the loading, unlocked, and relevance checks above.
                if (achData.additionalCondition == nullptr || achData.additionalCondition()) {
                    SPDLOG_DEBUG("[Achievements] ActorInit: Queuing achievement {}. Actor: {}", achData.name,
                                 (void*)actor);
                    AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
                }
            }
            // Note: No break; multiple achievements could potentially trigger on the same actor init event
        }
    }
}

// Registers the OnActorInit hook
void InitActorInitHandlers() {
    // Hook runs fairly often, handler needs to be efficient
    COND_HOOK(OnActorInit, CVAR_ACHIEVEMENTS, HandleActorInitTrigger);
}

} // namespace Handlers