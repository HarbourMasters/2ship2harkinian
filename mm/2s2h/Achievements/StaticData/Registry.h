#pragma once

#include "Types.h"
#include <vector>
#include <map>

// Defines the context in which an achievement is available.
// Used for filtering achievements by game mode.
enum class AchievementCategory {
    GENERAL, // Available in all game modes
    VANILLA, // Primarily for vanilla-only goals (e.g., Bomber's Notebook)
    RANDO    // For achievements specific to randomizer mode
};

struct Achievement {
    AchievementId id;
    const char* name;
    const char* description;
    const char* iconPath;
    bool secret;
    AchievementCategory category;
    int gamerscore;
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

// External data declarations
extern std::map<AchievementId, Achievement> Data;
extern std::map<AchievementEvent, Event> EventData;

// Initialization function
void Init();

// Data accessor functions
const Achievement* GetAchievement(AchievementId id);

// Event data accessor functions
const Event* GetEvent(AchievementEvent eventId);

} // namespace StaticData

} // namespace Achievements