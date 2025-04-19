#include "Achievements.h"
#include "AchievementsWindow.h"
#include "AchievementDefinitions.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenGui/Notification.h"
#include <spdlog/spdlog.h>
#include <libultraship/libultraship.h>
#include "2s2h/BenPort.h"
#include <z64save.h>

extern "C" {
#include <variables.h>
#include <functions.h>
#include <z64item.h>
}

#define CVAR_NAME_ACHIEVEMENTS "gEnhancements.Achievements.Enabled"
#define CVAR_ACHIEVEMENTS CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1)

// Singleton instance
// AchievementSystem* AchievementSystem::Instance = nullptr; // Removed old global instance

// Meyers' Singleton implementation
AchievementSystem& AchievementSystem::Instance() {
    static AchievementSystem instance; // Function-local static instance
    return instance;
}

// Achievement implementation
Achievement::Achievement(std::string id, std::string name, std::string description, std::string iconPath, bool isSecret,
                         int gamerscore, AchievementCategory category)
    : mId(std::move(id)), mName(std::move(name)), mDescription(std::move(description)), mIconPath(std::move(iconPath)),
      mIsSecret(isSecret), mGamerscore(gamerscore), mCategory(category) {
    // Use std::move for string parameters passed by value
}

// Achievement System implementation
AchievementSystem::AchievementSystem() {
    mProcessingEnabled = false;
}

AchievementSystem::~AchievementSystem() {
}

void AchievementSystem::RegisterAchievement(std::shared_ptr<Achievement> achievement) {
    mAchievements.push_back(achievement);
    mAchievementsMap[achievement->getId()] = achievement;
    // Initialize runtime state to locked upon registration
    mCurrentAchievementStates[achievement->getId()] = false;

    SPDLOG_DEBUG("Registered achievement: {}", achievement->getId());
}

std::shared_ptr<Achievement> AchievementSystem::GetAchievement(const std::string& id) const {
    auto it = mAchievementsMap.find(id);
    if (it != mAchievementsMap.end()) {
        return it->second;
    }
    return nullptr;
}

void AchievementSystem::QueueAchievementUnlock(const std::string& id) {
    auto achievement = GetAchievement(id);
    // Use mCurrentAchievementStates as the source of truth for unlock status
    if (achievement && !mCurrentAchievementStates[id]) {
        mCurrentAchievementStates[id] = true;
        SPDLOG_INFO("Achievement queued for unlock: {}", achievement->getName());

        // Save context interaction removed - state is saved via GetCurrentStates() during serialization

        mPendingAchievements.push(id);

        mProcessingEnabled = true;

        // Register hook to process queued achievements during the main game state update loop.
        COND_HOOK(OnGameStateUpdate, CVAR_ACHIEVEMENTS, [this]() {
            if (mProcessingEnabled && !mPendingAchievements.empty()) {
                this->ProcessQueuedAchievements();
            }
        });
    }
}

void AchievementSystem::ProcessQueuedAchievements() {
    // Don't process if an achievement notification is already being shown
    if (Notification::IsAchievementNotificationActive()) {
        return;
    }

    // Only process one achievement per frame (to avoid overwhelming the player)
    if (!mPendingAchievements.empty()) {
        std::string id = mPendingAchievements.front();
        mPendingAchievements.pop();

        auto achievement = GetAchievement(id);
        if (achievement) {
            SPDLOG_INFO("Processing queued achievement: {}", achievement->getName());

            ShowEnhancedNotification(
                achievement); // This will now display immediately if no other notification is active
        }

        // Disable processing if we've processed all pending achievements
        if (mPendingAchievements.empty()) {
            mProcessingEnabled = false;
        }
    }
}

void AchievementSystem::UnlockAchievement(const std::string& id) {
    auto achievement = GetAchievement(id);
    // Use mCurrentAchievementStates as the source of truth for unlock status
    if (achievement && !mCurrentAchievementStates[id]) {
        mCurrentAchievementStates[id] = true;
        SPDLOG_INFO("Achievement unlocked: {}", achievement->getName());

        // Save context interaction removed - state is saved via GetCurrentStates() during serialization

        ShowEnhancedNotification(achievement);
    }
}

void AchievementSystem::LockAchievement(const std::string& id) {
    auto achievement = GetAchievement(id);
    // Use mCurrentAchievementStates as the source of truth for unlock status
    if (achievement && mCurrentAchievementStates[id]) {
        mCurrentAchievementStates[id] = false;
        SPDLOG_INFO("Achievement locked (debug): {}", achievement->getName());

        // Save context interaction removed - state is saved via GetCurrentStates() during serialization
    }
}

bool AchievementSystem::IsAchievementUnlocked(const std::string& id) {
    return mCurrentAchievementStates.count(id) ? mCurrentAchievementStates.at(id) : false;
}

const std::vector<std::shared_ptr<Achievement>>& AchievementSystem::GetAchievements() const {
    return mAchievements;
}

std::vector<std::shared_ptr<Achievement>>
AchievementSystem::GetAchievementsByCategory(AchievementCategory category) const {
    std::vector<std::shared_ptr<Achievement>> filteredAchievements;

    for (const auto& achievement : mAchievements) {
        if (achievement->getCategory() == category || achievement->getCategory() == AchievementCategory::BOTH) {
            filteredAchievements.push_back(achievement);
        }
    }

    return filteredAchievements;
}

bool AchievementSystem::IsAchievementRelevantForGameMode(const std::string& id, bool isRandomizer) const {
    auto achievement = GetAchievement(id);
    if (!achievement) {
        return false;
    }

    // BOTH category is always relevant
    if (achievement->getCategory() == AchievementCategory::BOTH) {
        return true;
    }

    return (isRandomizer && achievement->getCategory() == AchievementCategory::RANDOMIZER) ||
           (!isRandomizer && achievement->getCategory() == AchievementCategory::VANILLA);
}

size_t AchievementSystem::GetUnlockedAchievementsCount() const {
    size_t count = 0;
    for (const auto& [id, unlocked] : mCurrentAchievementStates) {
        if (unlocked) {
            count++;
        }
    }
    return count;
}

// Added getter for serialization
const std::unordered_map<std::string, bool>& AchievementSystem::GetCurrentStates() const {
    return mCurrentAchievementStates;
}

void AchievementSystem::ShowNotification(const std::string& achievementName) {
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (gui) {
        gui->GetGameOverlay()->TextDrawNotification(10.0f, true, "Achievement Unlocked: %s", achievementName.c_str());
    }
}

void AchievementSystem::ShowEnhancedNotification(const std::shared_ptr<Achievement>& achievement) {
    // Default icon if none specified
    const char* iconPath = (const char*)gItemIcons[ITEM_SKULL_TOKEN];

    if (!achievement->getIconPath().empty()) {
        iconPath = achievement->getIconPath().c_str();
    }

    Notification::EmitAchievement(iconPath, achievement->getName(), achievement->getGamerscore());
}

std::shared_ptr<Ship::GuiWindow> AchievementSystem::CreateAchievementsWindow() {
    return std::static_pointer_cast<Ship::GuiWindow>(
        std::make_shared<AchievementsWindow>("gOpenWindows.Achievements", "Achievements"));
}

// Renamed and refactored to use the ID-based map from deserialization
void AchievementSystem::LoadFromSaveData(const std::unordered_map<std::string, bool>& loadedStates) {
    // Prevent loading if we are currently in the process of resetting for a new game

    SPDLOG_INFO("Loading achievement states from deserialized save data...");

    // 1. Clear current runtime states before loading
    mCurrentAchievementStates.clear();

    // 2. Populate runtime states ONLY from the loaded data
    for (const auto& [id, unlocked] : loadedStates) {
        // Only add entries that are actually registered, warn about unknown IDs
        if (mAchievementsMap.count(id)) {
            mCurrentAchievementStates[id] = unlocked;
        } else {
            SPDLOG_WARN("Achievement ID '{}' found in save data but is no longer registered. Ignoring.", id);
        }
    }

    // Log the state map JUST before synchronizing visual states
    SPDLOG_DEBUG("Achievement state map after loading from save:");
    for (const auto& [log_id, log_unlocked] : mCurrentAchievementStates) {
        SPDLOG_DEBUG("  - ID: {}, Unlocked: {}", log_id, log_unlocked);
    }

    // 3. Synchronize ALL registered achievements with the loaded states (or default to locked)
    for (auto& [id, achievement] : mAchievementsMap) {
        bool isUnlocked = false;
        if (mCurrentAchievementStates.count(id)) {
            // State was present in the loaded map
            isUnlocked = mCurrentAchievementStates.at(id);
            if (isUnlocked) {
                SPDLOG_DEBUG("Loaded unlocked state for: {}", id);
            }
        } else {
            // Achievement registered but not found in save data map (treat as locked)
            SPDLOG_INFO("Achievement '{}' not found in loaded save data map, forcing LOCKED state.",
                        id);                       // Changed to INFO
            mCurrentAchievementStates[id] = false; // Ensure it exists in the runtime map as locked
            isUnlocked = false;
        }
        // Update the visual/struct state based on the definitive loaded state
        // achievement->mState = isUnlocked ? AchievementState::UNLOCKED : AchievementState::LOCKED; // Removed: State
        // managed solely by mCurrentAchievementStates
    }

    SPDLOG_INFO("Achievement states loaded and synchronized.");
}

void AchievementSystem::ResetStatesForNewGame() {
    SPDLOG_INFO("Resetting achievement states for new game...");

    mCurrentAchievementStates.clear();

    for (auto& [id, achievement] : mAchievementsMap) {
        mCurrentAchievementStates[id] = false;
        // Reset visual/struct state to locked (using friend access)
        // State managed solely by mCurrentAchievementStates
    }

    while (!mPendingAchievements.empty()) {
        mPendingAchievements.pop();
    }
    mProcessingEnabled = false;

    SPDLOG_INFO("Achievement states reset.");
}

void InitializeAchievementSystem() {
    AchievementSystem& achievementSystem = AchievementSystem::Instance();

    // Register CVars and necessary hooks here (things that MUST happen once)
    CVarRegisterInteger(CVAR_NAME_ACHIEVEMENTS, 1);

    // Hook for loading achievements on save load
    // NOTE: The actual call to LoadFromSaveData(map) must now happen within the
    // save loading logic AFTER deserialization provides the map.
    // This hook might need removal or modification depending on where the call is moved.

    // Hook for resetting achievement state on new file initialization
    COND_HOOK(OnSaveInit, CVAR_ACHIEVEMENTS,
              [](s16 fileNum) { AchievementSystem::Instance().ResetStatesForNewGame(); });

    SPDLOG_INFO("Core Achievement System registered for initialization.");
}

// This RegisterShipInitFunc ensures InitializeAchievementSystem runs during ShipInit
static RegisterShipInitFunc initFunc(InitializeAchievementSystem, { CVAR_NAME_ACHIEVEMENTS });
