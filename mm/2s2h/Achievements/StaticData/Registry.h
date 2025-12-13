#ifndef ACHIEVEMENT_STATIC_DATA_REGISTRY_H
#define ACHIEVEMENT_STATIC_DATA_REGISTRY_H

// Local includes
#include "Types.h"

// Standard library
#include <map>
#include <vector>

enum class AchievementCategory { GENERAL, VANILLA, RANDO };

struct Achievement {
    AchievementId id;
    const char* name;
    const char* description;
    const char* iconPath;
    bool secret;
    AchievementCategory category;
    int harbourMastery;
    std::vector<AchievementEvent> requiredEvents;
};

struct Event {
    AchievementEvent id;
    const char* name;
    const char* description;
    std::vector<AchievementId> dependentAchievements;
};

namespace Achievements {

namespace StaticData {

extern std::map<AchievementId, Achievement> Data;
extern std::map<AchievementEvent, Event> EventData;

void Init();
const Achievement* GetAchievement(AchievementId achievementId);
const Event* GetEvent(AchievementEvent achievementEventId);

} // namespace StaticData

} // namespace Achievements

#endif // ACHIEVEMENT_STATIC_DATA_REGISTRY_H