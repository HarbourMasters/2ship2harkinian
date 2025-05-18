#include "AchievementTriggers.h"
#include "Handlers/Handlers.h" // Include a central header for all handler initializers

// Performs one-time setup for the trigger module.
// Currently minimal, mirroring Rando::ActorBehavior::Init.
void AchievementTriggers::Init() {
    // Potential future setup: Create actor extensions if needed, etc.
}

// Registers/unregisters all achievement trigger hooks based on CVars.
// Called via OnSaveLoad hook. Mirrors Rando::MiscBehavior::OnFileLoad structure.
void AchievementTriggers::OnFileLoad() {
    // Call initializer functions for each trigger type handler.
    // These functions (defined in Handlers/*.cpp) will use COND_HOOK/COND_ID_HOOK.
    // The CVAR_ACHIEVEMENTS macro used by COND_HOOK now checks both the global CVar
    // and the save-specific gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled flag.
    // COND_HOOK will automatically unregister a hook if its condition (CVAR_ACHIEVEMENTS) is false,
    // and register it if the condition is true.

    bool achievementsEnabledForSave = false;
    if (gPlayState != nullptr) { // gPlayState might be null if called very early
        achievementsEnabledForSave = gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled;
    }

    // The Handlers::Init* functions are called regardless of the condition below.
    // The COND_HOOK macros within them will use the CVAR_ACHIEVEMENTS macro
    // (which checks both global and save-specific flags) to determine if hooks
    // should be active or inactive (unregistered).

    if (achievementsEnabledForSave && CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1)) {
        SPDLOG_DEBUG("[Achievements] OnFileLoad: System enabled for this save and global CVar is ON. Evaluating "
                     "trigger handlers for registration.");
    } else {
        SPDLOG_DEBUG("[Achievements] OnFileLoad: System disabled for this save (SaveFlag: {}, GlobalCVar: {}) OR "
                     "global CVar is OFF. Evaluating trigger handlers for unregistration.");
    }

    // Call all Init*Handlers. COND_HOOK within them will handle registration/unregistration
    // based on the evaluation of CVAR_ACHIEVEMENTS (which includes the save-specific flag).
    Handlers::InitFlagSetHandlers();
    Handlers::InitItemGiveHandlers();
    Handlers::InitSceneInitHandlers();
    Handlers::InitActorInitHandlers();
    Handlers::InitEndOfCycleSaveHandlers();
    Handlers::InitVanillaBehaviorHandlers();
    // Add calls for other handlers if new trigger types are added.
}