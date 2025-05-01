#pragma once

#include "Achievements.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "AchievementTypes.h"
#include <spdlog/spdlog.h>

/**
 * @brief Macro to define an achievement and its hook in one place
 * @param id Unique AchievementId enum value for the achievement
 * @param name Display name of the achievement
 * @param description Text description of the achievement
 * @param iconPath Path to the achievement's icon
 * @param isSecret Whether the achievement should be hidden until unlocked
 * @param gamerscore Point value associated with the achievement
 * @param category Category determining which game mode the achievement belongs to
 * @param hookType Event hook type to use (OnSceneInit, OnActorInit, etc.)
 * @param condition The condition to check for unlocking the achievement
 */
#define REGISTER_ACHIEVEMENT(id, name, description, iconPath, isSecret, gamerscore, category, hookType, condition)   \
    {                                                                                                                \
        const AchievementId ach_id = (id); /* Store the enum ID */                                                   \
        auto achievement =                                                                                           \
            std::make_shared<Achievement>(ach_id, name, description, iconPath, isSecret, gamerscore, category);      \
        /* Register the Achievement object FIRST */                                                                  \
        AchievementSystem::Instance().RegisterAchievement(achievement);                                              \
                                                                                                                     \
        /* Directly manage hook registration for this achievement */                                                 \
        static HOOK_ID hookId = 0;                                                                                   \
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::hookType>(hookId);                              \
        hookId = 0;                                                                                                  \
        /* Only register if the main CVar is enabled AND the achievement is currently locked */                      \
        if (CVAR_ACHIEVEMENTS && !AchievementSystem::Instance().IsAchievementUnlocked(ach_id)) {                     \
            hookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::hookType>(                           \
                [ach_id](auto... args) { /* Capture enum ach_id */                                                   \
                                         /* Double-check relevance and condition just before queuing */              \
                                         if (AchievementSystem::Instance().IsAchievementRelevantForGameMode(         \
                                                 ach_id, IS_RANDO) &&                                                \
                                             condition(args...)) {                                                   \
                                             AchievementSystem::Instance().QueueAchievementUnlock(ach_id);           \
                                             /* Unregister hook immediately upon unlock to prevent re-triggering */  \
                                             GameInteractor::Instance->UnregisterGameHook<GameInteractor::hookType>( \
                                                 hookId);                                                            \
                                         }                                                                           \
                });                                                                                                  \
        }                                                                                                            \
    }

/**
 * @brief Macro to define an achievement and its ID-specific hook in one place
 * @param id Unique AchievementId enum value for the achievement
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
#define ACHIEVEMENT_ID_HOOK(id, name, description, iconPath, isSecret, gamerscore, category, hookType, hookId_param, \
                            condition)                                                                               \
    {                                                                                                                \
        const AchievementId ach_id = (id); /* Store the enum ID */                                                   \
        auto achievement =                                                                                           \
            std::make_shared<Achievement>(ach_id, name, description, iconPath, isSecret, gamerscore, category);      \
        /* Register the Achievement object FIRST */                                                                  \
        AchievementSystem::Instance().RegisterAchievement(achievement);                                              \
                                                                                                                     \
        /* Directly manage ID-based hook registration for this achievement */                                        \
        static HOOK_ID hookId = 0;                                                                                   \
        GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::hookType>(hookId);                         \
        hookId = 0;                                                                                                  \
        /* Only register if the main CVar is enabled AND the achievement is currently locked */                      \
        if (CVAR_ACHIEVEMENTS && !AchievementSystem::Instance().IsAchievementUnlocked(ach_id)) {                     \
            hookId = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::hookType>(                      \
                hookId_param,                                                                                        \
                [ach_id](                                                                                            \
                    auto... args) { /* Capture enum ach_id */                                                        \
                                    /* Double-check relevance and condition just before queuing */                   \
                                    if (AchievementSystem::Instance().IsAchievementRelevantForGameMode(ach_id,       \
                                                                                                       IS_RANDO) &&  \
                                        condition(args...)) {                                                        \
                                        AchievementSystem::Instance().QueueAchievementUnlock(ach_id);                \
                                        /* Unregister hook immediately upon unlock */                                \
                                        GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::hookType>( \
                                            hookId);                                                                 \
                                    }                                                                                \
                });                                                                                                  \
        }                                                                                                            \
    }

/**
 * @brief Macro to define an achievement and its vanilla behavior hook in one place
 * @param id Unique AchievementId enum value for the achievement
 * @param name Display name of the achievement
 * @param description Text description of the achievement
 * @param iconPath Path to the achievement's icon
 * @param isSecret Whether the achievement should be hidden until unlocked
 * @param gamerscore Point value associated with the achievement
 * @param category Category determining which game mode the achievement belongs to
 * @param vbHookId The vanilla behavior hook ID to register
 * @param condition The condition to check for unlocking the achievement
 */
#define ACHIEVEMENT_VB_HOOK(id, name, description, iconPath, isSecret, gamerscore, category, vbHookId, condition)      \
    {                                                                                                                  \
        const AchievementId ach_id = (id); /* Store the enum ID */                                                     \
        auto achievement =                                                                                             \
            std::make_shared<Achievement>(ach_id, name, description, iconPath, isSecret, gamerscore, category);        \
        /* Register the Achievement object FIRST */                                                                    \
        AchievementSystem::Instance().RegisterAchievement(achievement);                                                \
                                                                                                                       \
        /* Directly manage ShouldVanillaBehavior hook registration */                                                  \
        static HOOK_ID hookId = 0;                                                                                     \
        GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(hookId);              \
        hookId = 0;                                                                                                    \
        /* Only register if the main CVar is enabled AND the achievement is currently locked */                        \
        if (CVAR_ACHIEVEMENTS && !AchievementSystem::Instance().IsAchievementUnlocked(ach_id)) {                       \
            hookId = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(           \
                vbHookId,                                                                                              \
                [ach_id](                                                                                              \
                    GIVanillaBehavior hookEnumId, bool* should,                                                        \
                    va_list args) { /* Capture enum ach_id */                                                          \
                                    /* Check if the achievement condition is met */                                    \
                                    /* Note: Condition check moved inside lambda for VB hook */                        \
                                    if (AchievementSystem::Instance().IsAchievementRelevantForGameMode(ach_id,         \
                                                                                                       IS_RANDO) &&    \
                                        (condition) && !AchievementSystem::Instance().IsAchievementUnlocked(ach_id)) { \
                                        AchievementSystem::Instance().QueueAchievementUnlock(ach_id);                  \
                                        /* Unregister hook immediately upon unlock */                                  \
                                        GameInteractor::Instance                                                       \
                                            ->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(hookId);  \
                                    }                                                                                  \
                                    /* IMPORTANT: We don't modify 'should' here */                                     \
                });                                                                                                    \
        }                                                                                                              \
    }
