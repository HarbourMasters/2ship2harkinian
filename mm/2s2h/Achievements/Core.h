#pragma once

#include <cstdint>
#include <vector>
#include "StaticData/Registry.h"

extern "C" {
#include "variables.h"
#include "z64save.h"
}

// Core Achievement System API
namespace Achievements {
// Lifecycle functions
void Init();        // Called on game boot

// Hook registration functions
void RegisterAchievementTracker(); // Register achievement tracking hooks (conditional)
void RegisterAchievementCore(); // Register all achievement system hooks

// Core API functions
bool IsUnlocked(AchievementId id);
bool IsEventTriggered(AchievementEvent eventId);
void GetProgress(AchievementId id, uint32_t& current, uint32_t& max);

// Primary entry point - the heart of the event-driven system
void TriggerEvent(AchievementEvent eventId, bool fromEditor = false);

// UI functions
void EnableAchievements(); // Called when manually enabling via UI button

// Developer tools functions
void Lock(AchievementId id);
void ResetEvent(AchievementEvent eventId);

// Event queue system
void QueueEvent(AchievementEvent eventId);
void ProcessQueuedEvents();

} // namespace Achievements

// Primary control macro
#define IS_ACHIEVEMENTS (gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled)

// Macro for triggering an event
#define QUEUE_ACHIEVEMENT(eventId)                 \
    do {                                       \
        if (IS_ACHIEVEMENTS) {         \
            Achievements::QueueEvent(eventId); \
        }                                      \
    } while (0)

// Tracking macros
#define IS_ACH_TRIGGERED(eventId) Achievements::IsEventTriggered(eventId)
#define IS_ACH_UNLOCKED(id) Achievements::IsUnlocked(id)
