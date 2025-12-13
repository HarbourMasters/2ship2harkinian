#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

// Local includes
#include "StaticData/Registry.h"

// Standard library
#include <cstdint>
#include <vector>

extern "C" {
#include "variables.h"
#include "z64save.h"
}

namespace Achievements {

void Init();
void RegisterAchievementTracker();
void RegisterAchievementCore();

bool IsUnlocked(AchievementId achievementId);
bool IsEventTriggered(AchievementEvent achievementEventId);

void GetProgress(AchievementId achievementId, uint32_t& current, uint32_t& max);

void TriggerEvent(AchievementEvent achievementEventId, bool fromEditor = false);
void EnableAchievements();
void Lock(AchievementId achievementId);
void ResetEvent(AchievementEvent achievementEventId);
void QueueEvent(AchievementEvent achievementEventId);
void ProcessQueuedEvents();

} // namespace Achievements

#define IS_ACHIEVEMENTS (gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled)

#define QUEUE_ACHIEVEMENT(eventId)             \
    do {                                       \
        if (IS_ACHIEVEMENTS) {                 \
            Achievements::QueueEvent(eventId); \
        }                                      \
    } while (0)

#define IS_ACH_TRIGGERED(eventId) Achievements::IsEventTriggered(eventId)
#define IS_ACH_UNLOCKED(id) Achievements::IsUnlocked(id)

#endif // ACHIEVEMENTS_H
