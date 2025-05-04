#pragma once

// Forward declarations or definitions needed by AchievementTriggers.cpp
// Mirrors Rando/ActorBehavior/ActorBehavior.h structure

namespace AchievementTriggers {

    // Performs one-time setup for the trigger module.
    void Init();

    // Registers/unregisters hooks based on CVar state.
    // Called via the OnSaveLoad hook.
    void OnFileLoad();

} // namespace AchievementTriggers 