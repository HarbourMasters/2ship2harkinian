#pragma once

#include "Achievements.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <spdlog/spdlog.h>

/**
 * @brief Macro to define an achievement and its hook in one place
 * @param id Unique identifier for the achievement
 * @param name Display name of the achievement
 * @param description Text description of the achievement
 * @param iconPath Path to the achievement's icon
 * @param isSecret Whether the achievement should be hidden until unlocked
 * @param gamerscore Point value associated with the achievement
 * @param category Category determining which game mode the achievement belongs to
 * @param hookType Event hook type to use (OnSceneInit, OnActorInit, etc.)
 * @param condition The condition to check for unlocking the achievement
 */
#define REGISTER_ACHIEVEMENT(id, name, description, iconPath, isSecret, gamerscore, category, hookType, condition) \
    {                                                                                                              \
        const std::string ach_id = (id);                                                                           \
        auto achievement =                                                                                         \
            std::make_shared<Achievement>(ach_id, name, description, iconPath, isSecret, gamerscore, category);    \
        AchievementSystem::Instance().RegisterAchievement(achievement);                                            \
                                                                                                                   \
        COND_HOOK(hookType, CVAR_ACHIEVEMENTS, [ach_id](auto... args) {                                            \
            if (AchievementSystem::Instance().IsAchievementRelevantForGameMode(ach_id, IS_RANDO) && condition &&   \
                !AchievementSystem::Instance().IsAchievementUnlocked(ach_id)) {                                    \
                AchievementSystem::Instance().QueueAchievementUnlock(ach_id);                                      \
            }                                                                                                      \
        });                                                                                                        \
    }

/**
 * @brief Macro to define an achievement and its ID-specific hook in one place
 * @param id Unique identifier for the achievement
 * @param name Display name of the achievement
 * @param description Text description of the achievement
 * @param iconPath Path to the achievement's icon
 * @param isSecret Whether the achievement should be hidden until unlocked
 * @param gamerscore Point value associated with the achievement
 * @param category Category determining which game mode the achievement belongs to
 * @param hookType Event hook type to use (ShouldActorInit, OnOpenText, etc.)
 * @param hookId The ID to register the hook for (actor ID, text ID, etc.)
 * @param condition The condition to check for unlocking the achievement
 */
#define ACHIEVEMENT_ID_HOOK(id, name, description, iconPath, isSecret, gamerscore, category, hookType, hookId,   \
                            condition)                                                                           \
    {                                                                                                            \
        const std::string ach_id = (id);                                                                         \
        auto achievement =                                                                                       \
            std::make_shared<Achievement>(ach_id, name, description, iconPath, isSecret, gamerscore, category);  \
        AchievementSystem::Instance().RegisterAchievement(achievement);                                          \
                                                                                                                 \
        COND_ID_HOOK(hookType, hookId, CVAR_ACHIEVEMENTS, [ach_id](auto... args) {                               \
            if (AchievementSystem::Instance().IsAchievementRelevantForGameMode(ach_id, IS_RANDO) && condition && \
                !AchievementSystem::Instance().IsAchievementUnlocked(ach_id)) {                                  \
                AchievementSystem::Instance().QueueAchievementUnlock(ach_id);                                    \
            }                                                                                                    \
        });                                                                                                      \
    }

/**
 * @brief Macro to define an achievement and its vanilla behavior hook in one place
 * @param id Unique identifier for the achievement
 * @param name Display name of the achievement
 * @param description Text description of the achievement
 * @param iconPath Path to the achievement's icon
 * @param isSecret Whether the achievement should be hidden until unlocked
 * @param gamerscore Point value associated with the achievement
 * @param category Category determining which game mode the achievement belongs to
 * @param vbHookId The vanilla behavior hook ID to register
 * @param condition The condition to check for unlocking the achievement
 */
#define ACHIEVEMENT_VB_HOOK(id, name, description, iconPath, isSecret, gamerscore, category, vbHookId, condition) \
    {                                                                                                             \
        const std::string ach_id = (id);                                                                          \
        auto achievement =                                                                                        \
            std::make_shared<Achievement>(ach_id, name, description, iconPath, isSecret, gamerscore, category);   \
        AchievementSystem::Instance().RegisterAchievement(achievement);                                           \
                                                                                                                  \
        COND_VB_SHOULD(vbHookId, CVAR_ACHIEVEMENTS, {                                                             \
            if (AchievementSystem::Instance().IsAchievementRelevantForGameMode(ach_id, IS_RANDO) && condition &&  \
                !AchievementSystem::Instance().IsAchievementUnlocked(ach_id)) {                                   \
                AchievementSystem::Instance().QueueAchievementUnlock(ach_id);                                     \
            }                                                                                                     \
        });                                                                                                       \
    }
