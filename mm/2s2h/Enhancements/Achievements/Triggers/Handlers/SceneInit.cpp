#include "Handlers.h"
#include "../../AchievementData.h"       // For AllAchievementData
#include "../../Achievements.h"          // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"       // For IS_RANDO
#include "libultraship/libultraship.h" // For CVarGetInteger
#include <spdlog/spdlog.h>

namespace Handlers {

// Handler function for the OnSceneInit hook
void HandleSceneInitTrigger(s16 sceneId, s8 spawnNum) { // spawnNum currently unused by achievements
    // Iterate through ALL achievements looking for OnSceneInit triggers
    for (auto const& [key, achData] : AllAchievementData) {
        if (achData.triggerType == AchievementTriggerType::OnSceneInit) {
            // Check if the trigger is for a specific scene OR if it's a general check (SCENE_ID_CHECK_ALWAYS)
            bool sceneMatches = (achData.checkSceneId == sceneId || achData.checkSceneId == SCENE_ID_CHECK_ALWAYS);

            // If the scene matches, check common conditions (unlocked, relevant, loading, additional lambda)
            if (sceneMatches && CheckCommonAchievementConditions(achData)) {
                SPDLOG_DEBUG("[Achievements] SceneInit Triggered: Scene={}, Queuing Achievement: {}", sceneId, achData.name);
                AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
                // Note: Don't break, other achievements might trigger on the same scene init
            }
        }
    }
}

// Registers the OnSceneInit hook
void InitSceneInitHandlers() {
    COND_HOOK(OnSceneInit, CVAR_ACHIEVEMENTS, HandleSceneInitTrigger);
}

} // namespace Handlers 