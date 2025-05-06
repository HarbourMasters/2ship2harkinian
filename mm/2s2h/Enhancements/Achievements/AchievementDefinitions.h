SPDLOG_INFO("Achievement System Initialized (Refactored).");
}

// --- SaveLoad Handler --- Registered via ShipInit below
// Calls the trigger module's OnFileLoad function to manage hooks.
static void OnAchievementSaveLoadHandler(s16 fileNum) {
    SPDLOG_DEBUG("OnAchievementSaveLoadHandler called for file {}. Calling AchievementTriggers::OnFileLoad().",
                 fileNum);
    // ... existing code ...
}

// --- Register the main initialization function with ShipInit --- Runs once at boot.
static RegisterShipInitFunc initFunc(InitializeAchievementSystem); // No longer depends directly on CVar

// TODO: Consider if the SaveLoad handler needs to be conditional based on CVAR_ACHIEVEMENTS
// Currently, AchievementTriggers::OnFileLoad() checks the CVar internally.

void AchievementSystem::StartLoadingOrInitializing() {
    isLoadingOrInitializing = true;
}