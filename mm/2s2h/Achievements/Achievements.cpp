#include "Core.h"
#include "AchievementIntegration.h"
#include "StaticData/Registry.h"
#include "2s2h/BenGui/Notification.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "public/bridge/consolevariablebridge.h"

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <functional>

extern "C" {
#include "variables.h"
#include "functions.h"
}

namespace Achievements {

// Internal types and data structures
enum AchievementQueueEventType { QUEUE_ACHIEVEMENT, SHOW_UNLOCK, SHOW_PROGRESS };

struct AchievementQueueEvent {
    AchievementQueueEventType type;
    AchievementEvent eventId;           // for QUEUE_ACHIEVEMENT
    AchievementId achievementId;        // for SHOW_UNLOCK and SHOW_PROGRESS
    uint32_t currentProgress = 0;       // for SHOW_PROGRESS
    uint32_t maxProgress = 0;           // for SHOW_PROGRESS
    AchievementEvent triggeringEventId; // for SHOW_PROGRESS
};

// Static data
static std::vector<AchievementQueueEvent> achievementQueue;
static bool processing = false; // Prevents re-entry

// Lifecycle functions
void Init() {
    StaticData::Init();
    Integration::Init();
}

void EnableAchievements() {
    // Initialize and enable achievements for the current save
    memset(&gSaveContext.save.shipSaveInfo.achievements, 0, sizeof(AchievementSaveData));
    gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled = true;

    // Re-register achievement tracker now that they're enabled
    RegisterAchievementTracker();
}

// Core API functions
bool IsUnlocked(AchievementId id) {
    if (!IS_ACHIEVEMENTS)
        return false;

    int idIndex = static_cast<int>(id);
    if (idIndex < 0 || idIndex >= static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX)) {
        return false;
    }

    return gSaveContext.save.shipSaveInfo.achievements.unlocked[idIndex];
}

bool IsEventTriggered(AchievementEvent eventId) {
    if (!IS_ACHIEVEMENTS)
        return false;

    int eventIndex = static_cast<int>(eventId);
    if (eventIndex < 0 || eventIndex >= static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX)) {
        return false;
    }

    return gSaveContext.save.shipSaveInfo.achievements.events[eventIndex];
}

void GetProgress(AchievementId id, uint32_t& current, uint32_t& max) {
    current = 0;
    max = 1;

    const Achievement* achievement = StaticData::GetAchievement(id);
    if (!achievement)
        return;

    max = static_cast<uint32_t>(achievement->requiredEvents.size());
    if (max == 0)
        max = 1; // Prevent division by zero

    // Count how many required events have been triggered
    for (AchievementEvent requiredEvent : achievement->requiredEvents) {
        if (IsEventTriggered(requiredEvent)) {
            current++;
        }
    }
}

// Event queue system
void QueueEvent(AchievementEvent eventId) {
    if (!IS_ACHIEVEMENTS)
        return;

    // Avoid duplicate events in queue
    for (const auto& queuedEvent : achievementQueue) {
        if (queuedEvent.type == QUEUE_ACHIEVEMENT && queuedEvent.eventId == eventId) {
            return;
        }
    }

    achievementQueue.push_back({ QUEUE_ACHIEVEMENT, eventId, AchievementId::ACHIEVEMENT_ID_MAX, 0, 0,
                                 AchievementEvent::ACHIEVEMENT_EVENT_MAX });
}

void ProcessQueuedEvents() {
    // If already processing, return
    if (processing) {
        return;
    }

    // If no events queued, return
    if (achievementQueue.empty()) {
        return;
    }

    // Basic safety checks
    if (!IS_ACHIEVEMENTS)
        return;

    Player* player = GET_PLAYER(gPlayState);
    if (!player)
        return;

    // If the player has a message active, stop
    if (gPlayState->msgCtx.msgMode != 0) {
        return;
    }

    // If the player is in a blocking cutscene, stop
    if (Player_InBlockingCsMode(gPlayState, player)) {
        return;
    }

    // If player is dead, stop
    if (player->stateFlags1 & PLAYER_STATE1_80) {
        return;
    }

    // For notifications, check if another notification is already active
    AchievementQueueEvent& nextEvent = achievementQueue.front();
    if ((nextEvent.type == SHOW_UNLOCK || nextEvent.type == SHOW_PROGRESS) && Notification::IsNotificationActive()) {
        return;
    }

    // Process one event at a time
    processing = true;

    AchievementQueueEvent event = achievementQueue.front();
    achievementQueue.erase(achievementQueue.begin());

    switch (event.type) {
        case QUEUE_ACHIEVEMENT:
            TriggerEvent(event.eventId);
            break;

        case SHOW_UNLOCK: {
            const Achievement* achievement = StaticData::GetAchievement(event.achievementId);
            if (achievement) {
                Notification::EmitAchievement(achievement->iconPath ? achievement->iconPath : "",
                                              std::string(achievement->name), achievement->gamerscore);
            }
            break;
        }

        case SHOW_PROGRESS: {
            const Achievement* achievement = StaticData::GetAchievement(event.achievementId);
            if (achievement) {
                const auto* triggeringEvent = StaticData::GetEvent(event.triggeringEventId);
                const char* eventName = triggeringEvent ? triggeringEvent->name : "Unknown Event";
                Notification::EmitAchievementProgressWithEvent(achievement->iconPath ? achievement->iconPath : "",
                                                               eventName, achievement->name, event.currentProgress,
                                                               event.maxProgress);
            }
            break;
        }
    }

    processing = false;
}

void TriggerEvent(AchievementEvent eventId, bool fromEditor) {
    if (!IS_ACHIEVEMENTS)
        return;

    // Mark the event as triggered (only if not already triggered)
    bool wasAlreadyTriggered = IsEventTriggered(eventId);
    if (!wasAlreadyTriggered) {
        int eventIndex = static_cast<int>(eventId);
        if (eventIndex >= 0 && eventIndex < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX)) {
            gSaveContext.save.shipSaveInfo.achievements.events[eventIndex] = true;
        }
    }

    // Always check achievement completion, regardless of whether event was already triggered
    // This is crucial for cases where achievements were manually locked but events are already triggered
    const Event* event = StaticData::GetEvent(eventId);
    if (!event)
        return;

    // Check each affected achievement for completion
    for (AchievementId achId : event->dependentAchievements) {
        if (!IsUnlocked(achId)) {
            const Achievement* achievement = StaticData::GetAchievement(achId);
            if (!achievement)
                continue;

            // Check if achievement is now complete (all required events triggered)
            bool isComplete = true;
            for (AchievementEvent requiredEvent : achievement->requiredEvents) {
                if (!IsEventTriggered(requiredEvent)) {
                    isComplete = false;
                    break;
                }
            }

            if (isComplete) {
                // Unlock the achievement
                int idIndex = static_cast<int>(achId);
                if (idIndex >= 0 && idIndex < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX)) {
                    gSaveContext.save.shipSaveInfo.achievements.unlocked[idIndex] = true;

                    // Check if we should bypass queue for editor mode
                    if (fromEditor) {
                        // Editor mode - instant notification
                        Notification::EmitAchievement(achievement->iconPath ? achievement->iconPath : "",
                                                      std::string(achievement->name), achievement->gamerscore);
                    } else {
                        // Player mode - use queue with timing restrictions
                        achievementQueue.push_back({ SHOW_UNLOCK, AchievementEvent::ACHIEVEMENT_EVENT_MAX, achId, 0, 0,
                                                     AchievementEvent::ACHIEVEMENT_EVENT_MAX });
                    }
                }
            } else if (!wasAlreadyTriggered) {
                // Only show progress notification for newly triggered events, not re-evaluated ones
                // Achievement is not complete yet, queue progress notification for multi-event achievements
                uint32_t current, max;
                GetProgress(achId, current, max);

                if (max > 1 && !achievement->secret) {
                    // Check if we should bypass queue for editor mode
                    if (fromEditor) {
                        // Editor mode - instant notification
                        const auto* triggeringEvent = StaticData::GetEvent(eventId);
                        const char* eventName = triggeringEvent ? triggeringEvent->name : "Unknown Event";
                        Notification::EmitAchievementProgressWithEvent(achievement->iconPath ? achievement->iconPath
                                                                                             : "",
                                                                       eventName, achievement->name, current, max);
                    } else {
                        // Player mode - use queue with timing restrictions
                        achievementQueue.push_back(
                            { SHOW_PROGRESS, AchievementEvent::ACHIEVEMENT_EVENT_MAX, achId, current, max, eventId });
                    }
                }
            }
        }
    }
}

// Developer tools functions
void Lock(AchievementId id) {
    if (!IS_ACHIEVEMENTS)
        return;

    int idIndex = static_cast<int>(id);
    if (idIndex >= 0 && idIndex < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX)) {
        gSaveContext.save.shipSaveInfo.achievements.unlocked[idIndex] = false;
    }
}

void ResetEvent(AchievementEvent eventId) {
    if (!IS_ACHIEVEMENTS)
        return;

    int eventIndex = static_cast<int>(eventId);
    if (eventIndex >= 0 && eventIndex < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX)) {
        gSaveContext.save.shipSaveInfo.achievements.events[eventIndex] = false;
    }

    // Lock all achievements that depend on this event for consistent testing behavior
    const Event* event = StaticData::GetEvent(eventId);
    if (!event)
        return;

    for (AchievementId achId : event->dependentAchievements) {
        Lock(achId);
    }
}

// Runtime check for achievements
bool AreAchievementsActive() {
    return IS_ACHIEVEMENTS;
}

void RegisterAchievementTracker() {
    // Only register achievement tracking hooks if achievements are active
    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, IS_ACHIEVEMENTS,
                 [](Actor* actor) { Achievements::ProcessQueuedEvents(); });

    COND_HOOK(OnFlagSet, IS_ACHIEVEMENTS,
              [](FlagType flagType, u32 flag) { Achievements::Integration::OnFlagSet(flagType, flag); });

    COND_HOOK(OnSceneFlagSet, IS_ACHIEVEMENTS, [](s16 sceneId, FlagType flagType, u32 flag) {
        Achievements::Integration::OnSceneFlagSet(sceneId, flagType, flag);
    });

    // Register COND_VB_SHOULD hooks for each vanilla behavior in the map
    for (const auto& [vbFlag, conditions] : Integration::vanillaBehaviorMap) {
        COND_VB_SHOULD(vbFlag, true, { Integration::OnVanillaBehavior(_, should, args); });
    }
}

void RegisterAchievementCore() {
    // Register save lifecycle hooks
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveInit>([](s16 fileNum) {
        // Auto-enable achievements if the global cvar is set
        if (CVarGetInteger("gEnhancements.Achievements.Enabled", 0)) {
            // Initialize and enable achievements for this save
            memset(&gSaveContext.save.shipSaveInfo.achievements, 0, sizeof(AchievementSaveData));
            gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled = true;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>([](s16 fileNum) {
        // Register achievement tracker when save is loaded (COND* macros handle conditional registration)
        RegisterAchievementTracker();
    });
}

// Register with ShipInit system
static RegisterShipInitFunc initFunc(RegisterAchievementCore, {});

} // namespace Achievements