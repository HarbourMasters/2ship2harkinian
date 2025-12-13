// Local includes
#include "Achievements.h"
#include "AchievementIntegration.h"
#include "StaticData/Registry.h"

// Standard library
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <vector>

// Third-party
#include <spdlog/spdlog.h>

// Ship/libultraship
#include <libultraship/bridge/consolevariablebridge.h>

extern "C" {
#include "functions.h"
#include "variables.h"
}

// 2s2h
#include "2s2h/BenGui/Notification.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

namespace Achievements {

namespace {
enum AchievementQueueEventType { QUEUE_ACHIEVEMENT, SHOW_UNLOCK, SHOW_PROGRESS };

struct AchievementQueueEvent {
    AchievementQueueEventType type;
    AchievementEvent achievementEventId;
    AchievementId achievementId;
    uint32_t currentProgress = 0;
    uint32_t maxProgress = 0;
    AchievementEvent triggeringEventId;
};

std::vector<AchievementQueueEvent> achievementQueue;
bool processing = false;

inline bool IsValidAchievementId(AchievementId achievementId) {
    const int idIndex = static_cast<int>(achievementId);
    return idIndex >= 0 && idIndex < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX);
}

inline bool IsValidEventId(AchievementEvent achievementEventId) {
    const int eventIndex = static_cast<int>(achievementEventId);
    return eventIndex >= 0 && eventIndex < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX);
}

bool IsAchievementComplete(const Achievement* achievement) {
    if (!achievement) {
        return false;
    }

    for (const AchievementEvent requiredEvent : achievement->requiredEvents) {
        if (!IsEventTriggered(requiredEvent)) {
            return false;
        }
    }
    return true;
}

void UnlockAchievement(AchievementId achievementId, bool fromEditor) {
    if (!IsValidAchievementId(achievementId)) {
        return;
    }

    const Achievement* achievement = StaticData::GetAchievement(achievementId);
    if (!achievement) {
        return;
    }

    const int idIndex = static_cast<int>(achievementId);
    gSaveContext.save.shipSaveInfo.achievements.unlocked[idIndex] = true;

    if (fromEditor) {
        Notification::EmitAchievement(achievement->iconPath ? achievement->iconPath : "",
                                      std::string(achievement->name), achievement->harbourMastery);
    } else {
        achievementQueue.push_back({ SHOW_UNLOCK, AchievementEvent::ACHIEVEMENT_EVENT_MAX, achievementId, 0, 0,
                                     AchievementEvent::ACHIEVEMENT_EVENT_MAX });
    }
}

void ShowAchievementProgress(AchievementId achievementId, AchievementEvent triggeringEventId, bool fromEditor) {
    const Achievement* achievement = StaticData::GetAchievement(achievementId);
    if (!achievement) {
        return;
    }

    uint32_t current = 0;
    uint32_t max = 0;
    GetProgress(achievementId, current, max);

    if (max <= 1 || achievement->secret) {
        return;
    }

    if (fromEditor) {
        const Event* triggeringEvent = StaticData::GetEvent(triggeringEventId);
        const char* eventName = triggeringEvent ? triggeringEvent->name : "Unknown Event";
        Notification::EmitAchievementProgressWithEvent(achievement->iconPath ? achievement->iconPath : "", eventName,
                                                       achievement->name, current, max);
    } else {
        achievementQueue.push_back(
            { SHOW_PROGRESS, AchievementEvent::ACHIEVEMENT_EVENT_MAX, achievementId, current, max, triggeringEventId });
    }
}
} // namespace

void Init() {
    StaticData::Init();
    Integration::Init();
}

void EnableAchievements() {
    memset(&gSaveContext.save.shipSaveInfo.achievements, 0, sizeof(AchievementSaveData));
    gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled = true;
    RegisterAchievementTracker();
}

bool IsUnlocked(AchievementId achievementId) {
    if (!IS_ACHIEVEMENTS || !IsValidAchievementId(achievementId)) {
        return false;
    }

    const int idIndex = static_cast<int>(achievementId);
    return gSaveContext.save.shipSaveInfo.achievements.unlocked[idIndex];
}

bool IsEventTriggered(AchievementEvent achievementEventId) {
    if (!IS_ACHIEVEMENTS || !IsValidEventId(achievementEventId)) {
        return false;
    }

    const int eventIndex = static_cast<int>(achievementEventId);
    return gSaveContext.save.shipSaveInfo.achievements.events[eventIndex];
}

void GetProgress(AchievementId achievementId, uint32_t& current, uint32_t& max) {
    current = 0;
    max = 1;

    const Achievement* achievement = StaticData::GetAchievement(achievementId);
    if (!achievement) {
        return;
    }

    max = static_cast<uint32_t>(achievement->requiredEvents.size());
    if (max == 0) {
        max = 1;
    }

    for (const AchievementEvent requiredEvent : achievement->requiredEvents) {
        if (IsEventTriggered(requiredEvent)) {
            current++;
        }
    }
}

void QueueEvent(AchievementEvent achievementEventId) {
    if (!IS_ACHIEVEMENTS) {
        return;
    }

    for (const auto& queuedEvent : achievementQueue) {
        if (queuedEvent.type == QUEUE_ACHIEVEMENT && queuedEvent.achievementEventId == achievementEventId) {
            return;
        }
    }

    achievementQueue.push_back({ QUEUE_ACHIEVEMENT, achievementEventId, AchievementId::ACHIEVEMENT_ID_MAX, 0, 0,
                                 AchievementEvent::ACHIEVEMENT_EVENT_MAX });
}

void ProcessQueuedEvents() {
    if (processing || achievementQueue.empty() || !IS_ACHIEVEMENTS)
        return;

    Player* player = GET_PLAYER(gPlayState);
    if (!player || gPlayState->msgCtx.msgMode != 0 || Player_InBlockingCsMode(gPlayState, player) ||
        (player->stateFlags1 & PLAYER_STATE1_DEAD))
        return;

    AchievementQueueEvent& nextEvent = achievementQueue.front();
    if ((nextEvent.type == SHOW_UNLOCK || nextEvent.type == SHOW_PROGRESS) && Notification::IsNotificationActive())
        return;

    processing = true;
    AchievementQueueEvent event = achievementQueue.front();
    achievementQueue.erase(achievementQueue.begin());

    switch (event.type) {
        case QUEUE_ACHIEVEMENT:
            TriggerEvent(event.achievementEventId);
            break;

        case SHOW_UNLOCK: {
            const Achievement* achievement = StaticData::GetAchievement(event.achievementId);
            if (achievement) {
                Notification::EmitAchievement(achievement->iconPath ? achievement->iconPath : "",
                                              std::string(achievement->name), achievement->harbourMastery);
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

        default:
            SPDLOG_WARN("Unknown achievement queue event {}", static_cast<int>(event.type));
            break;
    }

    processing = false;
}

void TriggerEvent(AchievementEvent achievementEventId, bool fromEditor) {
    if (!IS_ACHIEVEMENTS) {
        return;
    }

    const bool wasAlreadyTriggered = IsEventTriggered(achievementEventId);
    if (!wasAlreadyTriggered && IsValidEventId(achievementEventId)) {
        const int eventIndex = static_cast<int>(achievementEventId);
        gSaveContext.save.shipSaveInfo.achievements.events[eventIndex] = true;
    }

    const Event* event = StaticData::GetEvent(achievementEventId);
    if (!event) {
        return;
    }

    for (const AchievementId achievementId : event->dependentAchievements) {
        if (IsUnlocked(achievementId)) {
            continue;
        }

        const Achievement* achievement = StaticData::GetAchievement(achievementId);
        if (!achievement) {
            continue;
        }

        if (IsAchievementComplete(achievement)) {
            UnlockAchievement(achievementId, fromEditor);
        } else if (!wasAlreadyTriggered) {
            ShowAchievementProgress(achievementId, achievementEventId, fromEditor);
        }
    }
}

void Lock(AchievementId achievementId) {
    if (!IS_ACHIEVEMENTS || !IsValidAchievementId(achievementId)) {
        return;
    }

    const int idIndex = static_cast<int>(achievementId);
    gSaveContext.save.shipSaveInfo.achievements.unlocked[idIndex] = false;
}

void ResetEvent(AchievementEvent achievementEventId) {
    if (!IS_ACHIEVEMENTS) {
        return;
    }

    if (IsValidEventId(achievementEventId)) {
        const int eventIndex = static_cast<int>(achievementEventId);
        gSaveContext.save.shipSaveInfo.achievements.events[eventIndex] = false;
    }

    const Event* event = StaticData::GetEvent(achievementEventId);
    if (!event) {
        return;
    }

    for (const AchievementId achievementId : event->dependentAchievements) {
        Lock(achievementId);
    }
}

void RegisterAchievementTracker() {
    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, IS_ACHIEVEMENTS,
                 [](Actor* actor) { Achievements::ProcessQueuedEvents(); });

    COND_HOOK(OnFlagSet, IS_ACHIEVEMENTS,
              [](FlagType flagType, u32 flag) { Achievements::Integration::OnFlagSet(flagType, flag); });

    COND_HOOK(OnSceneFlagSet, IS_ACHIEVEMENTS, [](s16 sceneId, FlagType flagType, u32 flag) {
        Achievements::Integration::OnSceneFlagSet(sceneId, flagType, flag);
    });

    for (const auto& [vbFlag, conditions] : Achievements::Integration::vanillaBehaviorMap) {
        COND_VB_SHOULD(vbFlag, IS_ACHIEVEMENTS, { Achievements::Integration::OnVanillaBehavior(_, should, args); });
    }
}

void RegisterAchievementCore() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveInit>([](s16 fileNum) {
        if (CVarGetInteger("gEnhancements.Achievements.Enabled", 0)) {
            memset(&gSaveContext.save.shipSaveInfo.achievements, 0, sizeof(AchievementSaveData));
            gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled = true;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>(
        [](s16 fileNum) { RegisterAchievementTracker(); });
}

static RegisterShipInitFunc initFunc(RegisterAchievementCore, {});

} // namespace Achievements
