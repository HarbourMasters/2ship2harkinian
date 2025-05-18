#include "Achievements.h"
#include "AchievementsWindow.h"
#include "AchievementData.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenGui/Notification.h"
#include "AchievementTypes.h"
#include <spdlog/spdlog.h>
#include <libultraship/libultraship.h>
#include "2s2h/BenPort.h"
#include <z64save.h>
#include "2s2h/Rando/Rando.h"
#include "Triggers/AchievementTriggers.h"

// Forward declaration
void RegisterAllAchievementTriggers();

extern "C" {
#include <variables.h>
#include <functions.h>
#include <z64item.h>
#include <z64.h>
}

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
                         bool isSecret, int gamerscore, AchievementCategory category, bool isInternal)
    : mId(id), mName(std::move(name)), mDescription(std::move(description)), mIconPath(std::move(iconPath)),
      mIsSecret(isSecret), mGamerscore(gamerscore), mCategory(category), mIsInternal(isInternal) {
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

    // NEW CHECKS: System enabled in save AND global CVar is ON
    if (!gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled) {
        SPDLOG_DEBUG("Achievement system disabled for this save file, skipping queue for ID: {}.", (int)id);
        return;
    }
    if (!CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1)) {
        SPDLOG_DEBUG("Achievement system globally disabled (CVar), skipping queue for ID: {}.", (int)id);
        return;
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

    // Check if the achievement is relevant for the current game mode
    bool isRandomizer = IS_RANDO;
    if (!IsAchievementRelevantForGameMode(id, isRandomizer)) {
        SPDLOG_DEBUG("Achievement {} not relevant for current game mode (Rando: {}), skipping queue.",
                     achievement->getName(), isRandomizer);
        return;
    }

    // All checks passed, mark as unlocked in save and add to pending queue
    gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked = true;
    mPendingAchievements.push(PendingNotificationInfo(PendingNotificationType::UNLOCK, id));
    SPDLOG_INFO("Achievement queued for unlock: Name={}, ID={}, Queue Size={}", achievement->getName(), (int)id,
                mPendingAchievements.size());

    // NEW: Check if this unlock should trigger a progress update for any MANUAL achievements
    for (const auto& pair : AllAchievementData) {
        const AchievementId& dependentAchId = pair.first;
        const AchievementStaticData& dependentAchStaticData = pair.second;

        if (dependentAchId != id && // Don't update the achievement that just got unlocked by itself
            dependentAchStaticData.triggerType == AchievementTriggerType::MANUAL &&
            dependentAchStaticData.hasProgressTracking) {
            SPDLOG_DEBUG("Unlocked achievement {} might affect MANUAL achievement {}. Updating progress.", (int)id,
                         (int)dependentAchId);
            UpdateAchievementProgress(dependentAchId);
        }
    }

    // Rely on OnGameStateUpdate hook to call TryProcessQueueNow
}

// --- Queue Processing & Notifications ---
void AchievementSystem::ProcessQueuedAchievements() {
    if (!mPendingAchievements.empty()) {
        PendingNotificationInfo info = mPendingAchievements.front();
        mPendingAchievements.pop();
        SPDLOG_DEBUG("ProcessQueuedAchievements: Processing ID={}, Type={}, New Queue Size={}", (int)info.id,
                     (info.type == PendingNotificationType::UNLOCK ? "UNLOCK" : "PROGRESS"),
                     mPendingAchievements.size());

        auto achievement = GetAchievement(info.id);
        if (achievement) {
            if (info.type == PendingNotificationType::UNLOCK) {
                // Double-check it's still marked unlocked in save context before showing notification
                if (gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)info.id].unlocked) {
                    SPDLOG_INFO("Showing UNLOCK notification for achievement: {}", achievement->getName());
                    ShowEnhancedNotification(achievement);
                } else {
                    SPDLOG_WARN("ProcessQueuedAchievements: UNLOCK for ID {} was in queue but is no longer marked "
                                "unlocked. Notification skipped.",
                                (int)info.id);
                }
            } else if (info.type == PendingNotificationType::PROGRESS) {
                // Don't show progress notifications for secret achievements that are still locked
                if (achievement->isInternal()) {
                    SPDLOG_DEBUG("Skipping PROGRESS notification for internal achievement: ID={}", (int)info.id);
                } else if (achievement->isSecret() &&
                           !gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)info.id].unlocked) {
                    SPDLOG_DEBUG("Skipping PROGRESS notification for secret locked achievement: ID={}", (int)info.id);
                } else {
                    SPDLOG_INFO("Showing PROGRESS notification for achievement: {}: {}/{} ", achievement->getName(),
                                info.currentProgress, info.targetProgress);
                    const char* iconPath = achievement->getIconPath().empty()
                                               ? (const char*)gItemIcons[ITEM_SKULL_TOKEN]
                                               : achievement->getIconPath().c_str();
                    Notification::EmitAchievementProgress(iconPath, achievement->getName().c_str(),
                                                          info.currentProgress, info.targetProgress);
                }
            }
            // Immediately try processing next after showing this one, regardless of type
            TryProcessQueueNow();
        } else {
            SPDLOG_ERROR("ProcessQueuedAchievements: Could not find achievement for queued ID: {}", (int)info.id);
        }
    } else {
        SPDLOG_TRACE("ProcessQueuedAchievements called, but queue is empty.");
    }
}

void AchievementSystem::TryProcessQueueNow() {
    SPDLOG_TRACE("TryProcessQueueNow called. Pending Queue Size: {}. Is *ANY* Notification Active: {}",
                 mPendingAchievements.size(), Notification::IsNotificationActive());
    if (!mPendingAchievements.empty() && !Notification::IsNotificationActive()) { // Changed to IsNotificationActive()
        ProcessQueuedAchievements();
    }
}

void AchievementSystem::ShowEnhancedNotification(const std::shared_ptr<Achievement>& achievement) {
    if (!achievement) {
        SPDLOG_ERROR("ShowEnhancedNotification called with null achievement.");
        return;
    }

    // Don't show notifications for internal achievements
    if (achievement->isInternal()) {
        SPDLOG_DEBUG("ShowEnhancedNotification: Skipping notification for internal achievement: {}",
                     achievement->getName());
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
    // Attempt to process queue immediately after loading finishes, if game is in a suitable state
    if (gPlayState != nullptr) { // Ensure we are in a playable state
        TryProcessQueueNow();
    }
}

void AchievementSystem::ResetStateAndQueueForLoadedSave() {
    SPDLOG_INFO("Resetting achievement queue..."); // Simplified log
    SPDLOG_DEBUG("Queue size before reset: {}", mPendingAchievements.size());

    // State is assumed to be loaded correctly from gSaveContext.save.shipSaveInfo.achievements by SaveManager

    // Clear any pending achievements
    std::queue<PendingNotificationInfo> emptyQueue; // Updated type
    std::swap(mPendingAchievements, emptyQueue);

    SPDLOG_INFO("Achievement queue cleared. New queue size: {}.", mPendingAchievements.size());
}

void AchievementSystem::DebugUnlockAchievement(AchievementId id) {
    SPDLOG_INFO("DebugUnlockAchievement called for ID: {}", (int)id);
    auto achievement = GetAchievement(id);

    if (achievement && !gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked) {
        // ADDED: Call QueueAchievementUnlock to trigger progress updates for MANUAL achievements
        SPDLOG_DEBUG("Calling QueueAchievementUnlock for ID {} to trigger progress updates.", (int)id);
        QueueAchievementUnlock(id);

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

bool AchievementSystem::IsAchievementUnlocked(AchievementId id) const {
    if (id < 0 || id >= AID_MAX) {
        SPDLOG_WARN("IsAchievementUnlocked called with invalid ID: {}", (int)id);
        return false;
    }
    bool unlocked = gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked;
    SPDLOG_TRACE("IsAchievementUnlocked: ID={}, Unlocked={}", (int)id, unlocked);
    return unlocked;
}

// --- Getters & Window Creation ---
std::vector<std::shared_ptr<Achievement>> AchievementSystem::GetAchievements() const {
    std::vector<std::shared_ptr<Achievement>> filteredAchievements;
    for (const auto& achievement : mAchievements) {
        if (achievement && !achievement->isInternal()) {
            filteredAchievements.push_back(achievement);
        }
    }
    return filteredAchievements;
}

std::vector<std::shared_ptr<Achievement>>
AchievementSystem::GetAchievementsByCategory(AchievementCategory category) const {
    std::vector<std::shared_ptr<Achievement>> filteredAchievements;
    for (const auto& achievement : mAchievements) {
        if (achievement && !achievement->isInternal() &&
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

// --- Progress Tracking --- NEW ---
void AchievementSystem::UpdateAchievementProgress(AchievementId id) {
    if (mIsLoadingOrInitializing) {
        SPDLOG_TRACE("[Achievements] UpdateAchievementProgress: Skipping update for ID {} during loading/init phase.",
                     (int)id);
        return;
    }

    // NEW CHECKS: System enabled in save AND global CVar is ON
    if (!gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled) {
        SPDLOG_TRACE(
            "[Achievements] UpdateAchievementProgress: System disabled for this save file, skipping update for ID: {}.",
            (int)id);
        return;
    }
    if (!CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1)) {
        SPDLOG_TRACE(
            "[Achievements] UpdateAchievementProgress: System globally disabled (CVar), skipping update for ID: {}.",
            (int)id);
        return;
    }

    auto achievement = GetAchievement(id); // Gets the std::shared_ptr<Achievement>
    if (!achievement) {
        SPDLOG_ERROR("[Achievements] UpdateAchievementProgress: Could not find achievement object for ID: {}", (int)id);
        return;
    }

    // Get the static data from AllAchievementData
    auto itStaticData = AllAchievementData.find(id);
    if (itStaticData == AllAchievementData.end()) {
        SPDLOG_ERROR("[Achievements] UpdateAchievementProgress: Could not find static data for ID: {}", (int)id);
        return;
    }
    const AchievementStaticData& staticData = itStaticData->second;

    if (!staticData.hasProgressTracking || staticData.getCurrentProgress == nullptr) {
        SPDLOG_TRACE("[Achievements] UpdateAchievementProgress: Achievement ID {} does not support progress tracking "
                     "or has no getCurrentProgress fn.",
                     (int)id);
        return;
    }

    // Check if already unlocked
    if (gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].unlocked) {
        SPDLOG_TRACE(
            "[Achievements] UpdateAchievementProgress: Achievement ID {} already unlocked. Progress not updated.",
            (int)id);
        return;
    }

    // Check if relevant for game mode before updating progress
    if (!IsAchievementRelevantForGameMode(id, IS_RANDO)) {
        SPDLOG_DEBUG("[Achievements] UpdateAchievementProgress: Achievement {} not relevant for current game mode "
                     "(Rando: {}). Progress not updated.",
                     achievement->getName(), IS_RANDO);
        return;
    }

    s32 liveProgress = staticData.getCurrentProgress();
    s32 oldProgress = gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].currentProgress;
    s32 target = (staticData.getTargetProgress != nullptr) ? staticData.getTargetProgress() : staticData.targetProgress;

    if (liveProgress != oldProgress) {
        gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)id].currentProgress = liveProgress;
        SPDLOG_DEBUG("[Achievements] UpdateAchievementProgress: ID {} progress updated from {} to {}. Target: {}",
                     (int)id, oldProgress, liveProgress, target);

        // Queue progress notification if progress increased but target not yet met
        if (liveProgress < target && target > 0) { // Check target > 0 to avoid notification for invalid targets
            mPendingAchievements.push(
                PendingNotificationInfo(PendingNotificationType::PROGRESS, id, liveProgress, target));
            SPDLOG_DEBUG(
                "[Achievements] UpdateAchievementProgress: Queued PROGRESS notification for ID {}. Queue size: {}",
                (int)id, mPendingAchievements.size());
            // Rely on OnGameStateUpdate hook to call TryProcessQueueNow for this new queued item
        }
    }

    // Check for unlock condition
    if (target <= 0) { // Avoid division by zero or nonsensical progress checks if target is invalid
        SPDLOG_TRACE("[Achievements] UpdateAchievementProgress: ID {} has invalid or zero target progress ({}). "
                     "Skipping unlock check.",
                     (int)id, target);
        return;
    }

    if (staticData.unlockOnTargetMet && liveProgress >= target) {
        SPDLOG_DEBUG("[Achievements] UpdateAchievementProgress: ID {} met progress target ({} / {}). Checking "
                     "additional conditions.",
                     (int)id, liveProgress, target);
        // Perform final additionalCondition check if it exists
        if (staticData.additionalCondition == nullptr || staticData.additionalCondition()) {
            SPDLOG_INFO("[Achievements] UpdateAchievementProgress: ID {} met all conditions. Queuing for unlock.",
                        (int)id);
            QueueAchievementUnlock(
                id); // QueueAchievementUnlock already checks for CVAR_ACHIEVEMENTS and if it's relevant
        } else {
            SPDLOG_DEBUG(
                "[Achievements] UpdateAchievementProgress: ID {} met progress target but failed additionalCondition.",
                (int)id);
        }
    }
}

size_t AchievementSystem::GetUnlockedAchievementsCount() const {
    size_t count = 0;
    for (const auto& pair : mAchievementsMap) { // Iterate mAchievementsMap to easily access achievement objects
        const auto& achievement = pair.second;
        if (achievement && !achievement->isInternal() && IsAchievementUnlocked(achievement->getId())) {
            count++;
        }
    }
    SPDLOG_TRACE("GetUnlockedAchievementsCount (Player-Facing): Count={}", count);
    return count;
}

std::shared_ptr<Ship::GuiWindow> AchievementSystem::CreateAchievementsWindow() {
    SPDLOG_DEBUG("Creating AchievementsWindow.");
    return std::static_pointer_cast<Ship::GuiWindow>(
        std::make_shared<AchievementsWindow>("gOpenWindows.Achievements", "Achievements"));
}

// --- Initialization & Hooks ---

// Handler for when the main game state begins, needed for initializing achievements at startup
void OnGameStateMainStartHandler() {
    AchievementSystem& system = AchievementSystem::Instance();
    bool isInPlayState = (gPlayState != nullptr);

    SPDLOG_DEBUG("=============== ACHIEVEMENT SYSTEM OnGameStateMainStart (IsInPlay: {}) ==============",
                 isInPlayState);

    if (isInPlayState) {
        if (system.IsLoadingOrInitializing()) {
            // Just entered Play state after a normal load/select
            SPDLOG_INFO("Entering GAMESTATE_PLAY after load/select. Clearing loading flag.");
            system.FinishLoadingOrInitializing(); // CLEAR LOADING FLAG
        }
    }
    SPDLOG_DEBUG("=============== ACHIEVEMENT SYSTEM OnGameStateMainStart FINISH (IsInPlay: {}) ==============",
                 isInPlayState);
}

void AchievementSystem::Init() {
    SPDLOG_INFO("=============== Initializing Core Achievement System Hooks ==============");

    // RETAINED - Hook to clear loading flag when entering playable state
    COND_HOOK(OnGameStateMainStart, CVAR_ACHIEVEMENTS, OnGameStateMainStartHandler);

    // RETAINED - Hook to process the queue during normal gameplay
    COND_HOOK(OnGameStateUpdate, CVAR_ACHIEVEMENTS, []() {
        // Only try processing if NOT loading/initializing
        if (!AchievementSystem::Instance().IsLoadingOrInitializing()) {
            AchievementSystem::Instance().TryProcessQueueNow();
        } else {
            SPDLOG_TRACE("Skipping TryProcessQueueNow during loading/initialization phase.");
        }
    });

    SPDLOG_INFO("Core Achievement System hooks initialized. Using GameState hook for flag end and queue processing.");
}

// IMPLEMENT EnableAchievementsForCurrentSave
EnableSaveStatus AchievementSystem::EnableAchievementsForCurrentSave() {
    // Ensure gPlayState is valid as we'll be accessing gSaveContext through it.
    // This also implicitly checks if a save is loaded.
    if (gPlayState == nullptr) {
        SPDLOG_WARN("EnableAchievementsForCurrentSave: Attempted but no save file (gPlayState is null).");
        return EnableSaveStatus::NO_SAVE_LOADED;
    }

    if (!CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1)) {
        SPDLOG_INFO("EnableAchievementsForCurrentSave: Global achievement system (CVar) is disabled.");
        return EnableSaveStatus::GLOBALLY_DISABLED;
    }

    if (gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled) {
        SPDLOG_INFO("EnableAchievementsForCurrentSave: Achievements are already enabled for this save file.");
        return EnableSaveStatus::ALREADY_ENABLED;
    }

    SPDLOG_INFO("Enabling achievements for current save and resetting progress...");

    StartLoadingOrInitializing(); // Prevent immediate processing during state change

    // Set the flag for the current save
    gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled = true;

    // Reset all achievement progress and unlock states for the current save
    for (int i = 0; i < AID_MAX; ++i) {
        gSaveContext.save.shipSaveInfo.achievements.achievementData[i].unlocked = false;
        gSaveContext.save.shipSaveInfo.achievements.achievementData[i].currentProgress = 0;
    }
    SPDLOG_DEBUG("All achievement data reset for the current save.");

    // Re-evaluate and register/unregister achievement trigger hooks based on the new state.
    // OnFileLoad uses CVAR_ACHIEVEMENTS which now checks the save-specific flag too.
    AchievementTriggers::OnFileLoad();
    SPDLOG_DEBUG("AchievementTriggers::OnFileLoad() called to update hooks.");

    FinishLoadingOrInitializing(); // Allow processing again

    SPDLOG_INFO("Achievements successfully enabled for current save. Progress reset.");
    return EnableSaveStatus::SUCCESS;
}

// ADDED: Implementation for GetAllAchievementsIncludingInternals
std::vector<std::shared_ptr<Achievement>> AchievementSystem::GetAllAchievementsIncludingInternals() const {
    // mAchievements already contains all registered achievements, including internal ones.
    // Return a copy so the caller has their own list instance.
    return mAchievements;
}
