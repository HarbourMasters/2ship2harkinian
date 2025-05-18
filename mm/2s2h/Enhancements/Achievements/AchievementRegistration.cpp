#include "Achievements.h"
// #include "AchievementDefinitions.h" // No longer needed
#include "AchievementData.h"              // Include the new static data definitions
#include "Triggers/AchievementTriggers.h" // Include the trigger module
#include "2s2h/BenPort.h"
#include "2s2h/Rando/Rando.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <libultraship/libultraship.h>
// #include <z64scene.h> // Likely not needed directly here anymore
// #include <z64item.h> // Likely not needed directly here anymore
#include <spdlog/spdlog.h>

#define CVAR_NAME_ACHIEVEMENTS "gEnhancements.Achievements.Enabled"
// #define CVAR_ACHIEVEMENTS CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1) // CVar checked in triggers module

// Forward declaration for the SaveLoad handler
static void OnAchievementSaveLoadHandler(s16 fileNum);
// ADD FORWARD DECLARATION FOR THE SAVE INIT HANDLER
static void OnAchievementSaveInitHandler(s16 fileNum);

// --- Main Initialization Function --- called once at boot by ShipInit
// Renamed from RegisterAllAchievementTriggers
void InitializeAchievementSystem() {

    SPDLOG_INFO("Initializing Achievement System (Refactored)...");

    // 1. Register Achievement Objects with the system
    //    Ensures the UI and state tracking know about all achievements.
    SPDLOG_DEBUG("Registering {} achievement definitions...", AllAchievementData.size());
    for (const auto& [id, data] : AllAchievementData) {
        // Use data directly from the static map entry
        auto achievement =
            std::make_shared<Achievement>(data.id, data.name, data.description, data.iconPath, data.isSecret,
                                          data.gamerscore, data.category, data.isInternal);
        // Ensure the AchievementSystem singleton is initialized if needed, though it should be by now
        AchievementSystem::Instance().RegisterAchievement(achievement);
    }
    SPDLOG_DEBUG("Finished registering achievement definitions.");

    // 2. Perform one-time setup for the trigger module
    AchievementTriggers::Init();

    // 3. Register the OnSaveLoad hook to handle CVar checks and hook registration/unregistration
    //    This hook will call AchievementTriggers::OnFileLoad()
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>(OnAchievementSaveLoadHandler);

    // ADD REGISTRATION FOR THE OnSaveInit HOOK
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveInit>(OnAchievementSaveInitHandler);

    SPDLOG_INFO("Achievement System Initialized (Refactored).");
}

// --- SaveLoad Handler --- Registered with GameInteractor
// Calls the trigger module's OnFileLoad function to manage hooks.
static void OnAchievementSaveLoadHandler(s16 fileNum) {
    // Check: Prevent running during title screen or file select
    if (gSaveContext.gameMode == GAMEMODE_TITLE_SCREEN || gSaveContext.gameMode == GAMEMODE_FILE_SELECT) {
        SPDLOG_DEBUG("OnAchievementSaveLoadHandler: Skipping during GameMode {} ", gSaveContext.gameMode);
        return;
    }

    AchievementSystem::Instance().StartLoadingOrInitializing();
    SPDLOG_DEBUG("OnAchievementSaveLoadHandler called for file {}. Calling AchievementTriggers::OnFileLoad().",
                 fileNum);
    // AchievementTriggers::OnFileLoad() will check CVAR_ACHIEVEMENTS and register/unregister hooks accordingly.
    AchievementTriggers::OnFileLoad();
}

// ADD THE OnAchievementSaveInitHandler IMPLEMENTATION
// --- SaveInit Handler --- Registered with GameInteractor
// Initializes achievement-specific data for a new save file.
static void OnAchievementSaveInitHandler(s16 fileNum) {
    // Check: Prevent running during title screen or file select
    if (gSaveContext.gameMode == GAMEMODE_TITLE_SCREEN) {
        SPDLOG_DEBUG("OnAchievementSaveInitHandler: Skipping during GameMode {}.", gSaveContext.gameMode);
        return;
    }

    AchievementSystem::Instance().StartLoadingOrInitializing();
    SPDLOG_DEBUG("OnAchievementSaveInitHandler called for new file {}. Initializing achievement save data.", fileNum);

    // Set whether the achievement system is enabled for this new save based on the global CVar
    bool systemEnabledByCvar = CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1);
    gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled = systemEnabledByCvar;
    SPDLOG_INFO("New save file: Achievement system will be {} for this save (based on current CVar value {}).",
                systemEnabledByCvar ? "ENABLED" : "DISABLED", systemEnabledByCvar);
    // ADD LOG: Verify the value written to gSaveContext immediately after setting
    SPDLOG_DEBUG("[AchievementRegistration] OnAchievementSaveInitHandler: Set "
                 "gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled to {} for fileNum {}.",
                 gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled, fileNum);

    // Initialize all achievement data for the new save
    for (int i = 0; i < AID_MAX; ++i) {
        gSaveContext.save.shipSaveInfo.achievements.achievementData[i].unlocked = false;
        gSaveContext.save.shipSaveInfo.achievements.achievementData[i].currentProgress = 0;
    }
    SPDLOG_DEBUG("All achievement progress and unlock states reset for new save file.");
}

// --- Register the main initialization function with ShipInit --- Runs once at boot.
static RegisterShipInitFunc initFunc(InitializeAchievementSystem); // No longer depends directly on CVar

// Placeholder for ShipInit registration update
