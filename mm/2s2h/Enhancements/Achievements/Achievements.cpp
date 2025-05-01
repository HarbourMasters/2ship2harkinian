#include "Achievements.h"
#include "AchievementsWindow.h"
#include "AchievementDefinitions.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenGui/Notification.h"
#include "AchievementTypes.h"
#include <spdlog/spdlog.h>
#include <libultraship/libultraship.h>
#include "2s2h/BenPort.h"
#include <z64save.h>
#include "2s2h/Rando/Rando.h"

// Forward declaration
void RegisterAllAchievementTriggers();

extern "C" {
#include <variables.h>
#include <functions.h>
#include <z64item.h>
#include <z64.h>
}

#define CVAR_NAME_ACHIEVEMENTS "gEnhancements.Achievements.Enabled"
#define CVAR_ACHIEVEMENTS CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1)

// --- Singleton Implementation ---
AchievementSystem& AchievementSystem::Instance() {
    static AchievementSystem instance;
    SPDLOG_DEBUG("AchievementSystem::Instance() accessed."); // Log instance access
    return instance;
}

AchievementSystem::~AchievementSystem() {
    SPDLOG_INFO("AchievementSystem destroyed.");
}

// --- Achievement Registration & Info ---
Achievement::Achievement(AchievementId id, std::string name, std::string description, std::string iconPath,
                         bool isSecret, int gamerscore, AchievementCategory category)
    : mId(id), mName(std::move(name)), mDescription(std::move(description)), mIconPath(std::move(iconPath)),
      mIsSecret(isSecret), mGamerscore(gamerscore), mCategory(category) {
    SPDLOG_TRACE("Achievement constructor: ID={}, Name={}", (int)id, name);
}

void AchievementSystem::RegisterAchievement(std::shared_ptr<Achievement> achievement) {
    if (!achievement) {
        SPDLOG_ERROR("Attempted to register a null achievement pointer.");
        return;
    }
    // Check if achievement ID already exists to prevent duplicates
    if (mAchievementsMap.count(achievement->getId())) {
        SPDLOG_TRACE("Skipping registration for already registered achievement: ID={}, Name={}",
                     (int)achievement->getId(), achievement->getName());
        return; // Already registered
    }

    mAchievements.push_back(achievement);
    mAchievementsMap[achievement->getId()] = achievement;
    SPDLOG_DEBUG("Registered achievement: ID={}, Name={}", (int)achievement->getId(), achievement->getName());
}

std::shared_ptr<Achievement> AchievementSystem::GetAchievement(AchievementId id) const {
    auto it = mAchievementsMap.find(id);
    if (it != mAchievementsMap.end()) {
        return it->second;
    }
    SPDLOG_WARN("GetAchievement called for unknown ID: {}", (int)id);
    return nullptr;
}

// --- Core Unlock Logic ---
void AchievementSystem::QueueAchievementUnlock(AchievementId id) {
    SPDLOG_DEBUG("QueueAchievementUnlock called for ID: {}", (int)id);

    // Check if currently loading/initializing - THIS IS THE KEY CHECK
    if (mIsLoadingOrInitializing) {
        SPDLOG_INFO(
            "Skipping achievement queue for ID {} during loading/initialization phase (OnSaveInit or OnSaveLoad).",
            (int)id);
        return; // Don't queue anything during this phase
    }

    auto achievement = GetAchievement(id);
    if (!achievement) {
        SPDLOG_ERROR("QueueAchievementUnlock: Could not find achievement for ID: {}", (int)id);
        return;
    }

    // Check if already unlocked in the current save context
    if (gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked) {
        SPDLOG_TRACE("QueueAchievementUnlock: Achievement ID {} already unlocked.", (int)id);
        return;
    }

    // Check if achievement system is enabled
    if (!CVAR_ACHIEVEMENTS) {
        SPDLOG_DEBUG("Achievement system disabled, skipping queue for: {}", achievement->getName());
        return;
    }

    // Check if the achievement is relevant for the current game mode
    bool isRandomizer = IS_RANDO;
    if (!IsAchievementRelevantForGameMode(id, isRandomizer)) {
        SPDLOG_DEBUG("Achievement {} not relevant for current game mode (Rando: {}), skipping queue.",
                     achievement->getName(), isRandomizer);
        return;
    }

    // All checks passed, mark as unlocked in save and add to pending queue
    gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked = true;
    mPendingAchievements.push(id);
    SPDLOG_INFO("Achievement queued for unlock: Name={}, ID={}, Queue Size={}", achievement->getName(), (int)id,
                mPendingAchievements.size());

    // Rely on OnGameStateUpdate hook to call TryProcessQueueNow
}

// --- Queue Processing & Notifications ---
void AchievementSystem::ProcessQueuedAchievements() {
    if (!mPendingAchievements.empty()) {
        AchievementId id = mPendingAchievements.front();
        mPendingAchievements.pop();
        SPDLOG_DEBUG("ProcessQueuedAchievements: Processing ID={}, New Queue Size={}", (int)id,
                     mPendingAchievements.size());

        auto achievement = GetAchievement(id);
        if (achievement) {
            // Double-check it's still marked unlocked in save context before showing notification
            if (gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked) {
                SPDLOG_INFO("Showing notification for achievement: {}", achievement->getName());
                ShowEnhancedNotification(achievement);
                // Immediately try processing next after showing this one
                TryProcessQueueNow();
            } else {
                SPDLOG_WARN("ProcessQueuedAchievements: Achievement ID {} was in queue but is no longer marked "
                            "unlocked in save context. Notification skipped.",
                            (int)id);
            }
        } else {
            SPDLOG_ERROR("ProcessQueuedAchievements: Could not find achievement for queued ID: {}", (int)id);
        }
    } else {
        SPDLOG_TRACE("ProcessQueuedAchievements called, but queue is empty.");
    }
}

void AchievementSystem::TryProcessQueueNow() {
    SPDLOG_TRACE("TryProcessQueueNow called. Pending Queue Size: {}. Is Notification Active: {}",
                 mPendingAchievements.size(), Notification::IsAchievementNotificationActive());
    if (!mPendingAchievements.empty() && !Notification::IsAchievementNotificationActive()) {
        ProcessQueuedAchievements();
    }
}

void AchievementSystem::ShowEnhancedNotification(const std::shared_ptr<Achievement>& achievement) {
    if (!achievement) {
        SPDLOG_ERROR("ShowEnhancedNotification called with null achievement.");
        return;
    }
    const char* iconPath = (const char*)gItemIcons[ITEM_SKULL_TOKEN]; // Default icon

    if (!achievement->getIconPath().empty()) {
        iconPath = achievement->getIconPath().c_str();
    }
    SPDLOG_DEBUG("ShowEnhancedNotification: Name={}, Icon={}, Score={}", achievement->getName(), iconPath,
                 achievement->getGamerscore());
    Notification::EmitAchievement(iconPath, achievement->getName(), achievement->getGamerscore());
}

// --- State Management & Debug ---

// New methods to manage loading flag
void AchievementSystem::StartLoadingOrInitializing() {
    SPDLOG_DEBUG("StartLoadingOrInitializing: Setting mIsLoadingOrInitializing to TRUE.");
    mIsLoadingOrInitializing = true;
}

void AchievementSystem::FinishLoadingOrInitializing() {
    SPDLOG_DEBUG("FinishLoadingOrInitializing: Setting mIsLoadingOrInitializing to FALSE.");
    mIsLoadingOrInitializing = false;
}

void AchievementSystem::ResetStateAndQueueForLoadedSave() {
    SPDLOG_INFO("Resetting achievement queue..."); // Simplified log
    SPDLOG_DEBUG("Queue size before reset: {}", mPendingAchievements.size());

    // State is assumed to be loaded correctly from gSaveContext.save.shipSaveInfo.achievements by SaveManager

    // Clear any pending achievements
    std::queue<AchievementId> emptyQueue;
    std::swap(mPendingAchievements, emptyQueue);

    SPDLOG_INFO("Achievement queue cleared. New queue size: {}.", mPendingAchievements.size());
}

void AchievementSystem::DebugUnlockAchievement(AchievementId id) {
    SPDLOG_INFO("DebugUnlockAchievement called for ID: {}", (int)id);
    auto achievement = GetAchievement(id);

    if (achievement && !gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked) {
        gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked = true;
        SPDLOG_INFO("Achievement unlocked (debug): {}", achievement->getName());
        ShowEnhancedNotification(achievement); // Show immediately for debug
    } else if (achievement) {
        SPDLOG_INFO("DebugUnlockAchievement: Achievement {} already unlocked.", achievement->getName());
    } else {
        SPDLOG_ERROR("DebugUnlockAchievement: Achievement ID {} not found.", (int)id);
    }
}

void AchievementSystem::LockAchievement(AchievementId id) {
    SPDLOG_INFO("LockAchievement called for ID: {}", (int)id);
    auto achievement = GetAchievement(id);

    if (achievement && gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked) {
        gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked = false;
        SPDLOG_INFO("Achievement locked (debug): {}", achievement->getName());
    } else if (achievement) {
        SPDLOG_INFO("LockAchievement: Achievement {} already locked.", achievement->getName());
    } else {
        SPDLOG_ERROR("LockAchievement: Achievement ID {} not found.", (int)id);
    }
}

bool AchievementSystem::IsAchievementUnlocked(AchievementId id) {
    if (id < 0 || id >= AID_MAX) {
        SPDLOG_WARN("IsAchievementUnlocked called with invalid ID: {}", (int)id);
        return false;
    }
    bool unlocked = gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked;
    SPDLOG_TRACE("IsAchievementUnlocked: ID={}, Unlocked={}", (int)id, unlocked);
    return unlocked;
}

// --- Getters & Window Creation ---
const std::vector<std::shared_ptr<Achievement>>& AchievementSystem::GetAchievements() const {
    return mAchievements;
}

std::vector<std::shared_ptr<Achievement>>
AchievementSystem::GetAchievementsByCategory(AchievementCategory category) const {
    std::vector<std::shared_ptr<Achievement>> filteredAchievements;
    for (const auto& achievement : mAchievements) {
        if (achievement &&
            (achievement->getCategory() == category || achievement->getCategory() == AchievementCategory::BOTH)) {
            filteredAchievements.push_back(achievement);
        }
    }
    return filteredAchievements;
}

bool AchievementSystem::IsAchievementRelevantForGameMode(AchievementId id, bool isRandomizer) const {
    auto achievement = GetAchievement(id);
    if (!achievement) {
        return false; // GetAchievement already logged warning
    }
    bool relevant = (achievement->getCategory() == AchievementCategory::BOTH) ||
                    (isRandomizer && achievement->getCategory() == AchievementCategory::RANDOMIZER) ||
                    (!isRandomizer && achievement->getCategory() == AchievementCategory::VANILLA);
    SPDLOG_TRACE("IsAchievementRelevantForGameMode: ID={}, IsRando={}, Category={}, Relevant={}", (int)id, isRandomizer,
                 (int)achievement->getCategory(), relevant);
    return relevant;
}

size_t AchievementSystem::GetUnlockedAchievementsCount() const {
    size_t count = 0;
    for (int i = 0; i < AID_MAX; ++i) {
        if (gSaveContext.save.shipSaveInfo.achievements.achievementData[i].unlocked) {
            count++;
        }
    }
    SPDLOG_TRACE("GetUnlockedAchievementsCount: Count={}", count);
    return count;
}

std::shared_ptr<Ship::GuiWindow> AchievementSystem::CreateAchievementsWindow() {
    SPDLOG_DEBUG("Creating AchievementsWindow.");
    return std::static_pointer_cast<Ship::GuiWindow>(
        std::make_shared<AchievementsWindow>("gOpenWindows.Achievements", "Achievements"));
}

// --- Initialization & Hooks ---

// Handler for when a save is loaded into gSaveContext (Load Game from File Select or Owl Load)
static void OnSaveLoadHandler(s16 fileNum) {
    AchievementSystem& system = AchievementSystem::Instance();
    SPDLOG_INFO("=============== ACHIEVEMENT SYSTEM OnSaveLoad (File: {}) ==============", fileNum);
    system.StartLoadingOrInitializing();      // SET FLAG before any post-load processing
    system.ResetStateAndQueueForLoadedSave(); // Ensure queue reflects the loaded save state
    SPDLOG_INFO("=============== ACHIEVEMENT SYSTEM OnSaveLoad FINISH (File: {}) ==============", fileNum);
}

// Handler for GameState Main Start (Called AFTER state transition)
// Responsible for clearing the loading flag.
static void OnGameStateMainStartHandler() {
    AchievementSystem& system = AchievementSystem::Instance();
    bool isInPlayState = (gPlayState != nullptr);

    SPDLOG_DEBUG("=============== ACHIEVEMENT SYSTEM OnGameStateMainStart (IsInPlay: {}) ==============",
                 isInPlayState);

    if (isInPlayState) {
        if (system.IsLoadingOrInitializing()) {
            // Just entered Play state after a normal load/select
            SPDLOG_INFO(
                "Entering GAMESTATE_PLAY after load/select. Clearing loading flag and re-registering triggers.");
            system.FinishLoadingOrInitializing(); // CLEAR LOADING FLAG

            // Re-register hooks now that the save is loaded and game state is active
            if (CVAR_ACHIEVEMENTS) {
                RegisterAllAchievementTriggers();
            }
        }
    }
    SPDLOG_DEBUG("=============== ACHIEVEMENT SYSTEM OnGameStateMainStart FINISH (IsInPlay: {}) ==============",
                 isInPlayState);
}

void AchievementSystem::Init() {
    SPDLOG_INFO("=============== Initializing Achievement System ==============");

    // Set flag and reset queue when a save is loaded
    COND_HOOK(OnSaveLoad, CVAR_ACHIEVEMENTS, OnSaveLoadHandler);

    // Clear flag when entering the fully playable state
    COND_HOOK(OnGameStateMainStart, CVAR_ACHIEVEMENTS, OnGameStateMainStartHandler);

    // Hook to process the queue during normal gameplay
    COND_HOOK(OnGameStateUpdate, CVAR_ACHIEVEMENTS, []() {
        // Only try processing if NOT loading/initializing
        if (!AchievementSystem::Instance().IsLoadingOrInitializing()) {
            AchievementSystem::Instance().TryProcessQueueNow();
        } else {
            SPDLOG_TRACE("Skipping TryProcessQueueNow during loading/initialization phase.");
        }
    });

    SPDLOG_INFO("Core Achievement System initialized. Using FileSelect/Load hooks for flag start, GameState hook for "
                "flag end.");
}
