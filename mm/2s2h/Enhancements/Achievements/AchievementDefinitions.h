#pragma once

#include "Achievements.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <spdlog/spdlog.h>

/**
 * @brief Helper macro to create a hook function for OnSceneInit
 * @param condition The condition to check for unlocking the achievement
 */
#define CREATE_SCENE_HOOK(condition)                   \
    [this](s8 sceneId, s8 spawnNum) {                  \
        if (condition && !IsAchievementUnlocked(id)) { \
            UnlockAchievement(id);                     \
        }                                              \
    }

/**
 * @brief Helper macro to create a hook function for OnActorInit
 * @param condition The condition to check for unlocking the achievement
 */
#define CREATE_ACTOR_HOOK(condition)                   \
    [this](Actor* actor) {                             \
        if (condition && !IsAchievementUnlocked(id)) { \
            UnlockAchievement(id);                     \
        }                                              \
    }

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
        auto achievement =                                                                                         \
            std::make_shared<Achievement>(id, name, description, iconPath, isSecret, gamerscore, category);        \
        AchievementSystem::Instance->RegisterAchievement(achievement);                                             \
                                                                                                                   \
        COND_HOOK(hookType, CVAR_ACHIEVEMENTS, [this](auto... args) {                                              \
            if (IsAchievementRelevantForGameMode(id, IS_RANDO) && condition && !IsAchievementUnlocked(id)) {       \
                QueueAchievementUnlock(id);                                                                        \
            }                                                                                                      \
        });                                                                                                        \
    }

// Example usage:
/*
REGISTER_ACHIEVEMENT(
    "first_steps",                    // id
    "First Steps",                    // name
    "Begin your adventure in Termina", // description
    (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], // icon
    false,                           // isSecret
    10,                              // gamerscore
    AchievementCategory::BOTH,      // category
    OnSceneInit,                     // hook type
    (std::get<0>(std::forward_as_tuple(args...)) != 0x00 &&
     std::get<0>(std::forward_as_tuple(args...)) != 0x02) // hook condition
);
*/