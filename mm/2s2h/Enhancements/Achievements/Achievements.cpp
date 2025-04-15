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
    : id(id), name(name), description(description), iconPath(iconPath), state(AchievementState::LOCKED),
      isSecret(isSecret), gamerscore(gamerscore), category(category) {
}

// Achievement System implementation
AchievementSystem::AchievementSystem() {
    // Constructor logic (if any) can go here.
    // Instance = this; // No longer needed
    mProcessingEnabled = false;
}

AchievementSystem::~AchievementSystem() {
    // Destructor logic (if any) can go here.
    // if (Instance == this) { // No longer needed
    //     Instance = nullptr;
    // }
}

void AchievementSystem::Initialize() {
    // This function's purpose might change slightly.
    // It's called by InitializeAchievementSystem, which is triggered by ShipInit.
    // It no longer needs to call RegisterAchievements.
    SPDLOG_CRITICAL("=== ACHIEVEMENT SYSTEM CORE INITIALIZATION STARTED ===");

    // Check if the number of registered achievements exceeds the max defined in save structure
    // Note: This check might run before all achievements are registered via ShipInit.
    // Consider moving this check to a post-initialization step if needed.
    if (mAchievements.size() > MAX_ACHIEVEMENTS) {
        SPDLOG_ERROR("Number of registered achievements ({}) exceeds MAX_ACHIEVEMENTS ({})!", mAchievements.size(), MAX_ACHIEVEMENTS);
        // Handle error appropriately, e.g., disable achievements or assert
    }

    SPDLOG_CRITICAL("Achievement System core initialized. Registered achievement count: {}", mAchievements.size());
    SPDLOG_CRITICAL("=== ACHIEVEMENT SYSTEM INITIALIZATION COMPLETED ===");
}

void AchievementSystem::RegisterAchievement(std::shared_ptr<Achievement> achievement) {
    mAchievements.push_back(achievement);
    mAchievementsMap[achievement->id] = achievement;

    SPDLOG_DEBUG("Registered achievement: {}", achievement->id);
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
    if (achievement && achievement->state != AchievementState::UNLOCKED) {
        achievement->state = AchievementState::UNLOCKED;
        SPDLOG_INFO("Achievement queued for unlock: {}", achievement->name);

        // Save achievement state to save context immediately
        unsigned int index = GetAchievementIndex(id);
        if (index < MAX_ACHIEVEMENTS && &gSaveContext) {
            gSaveContext.save.shipSaveInfo.achievementData[index].unlocked = true;
            SPDLOG_DEBUG("Saved achievement {} state directly to save context", id);
        } else {
             SPDLOG_ERROR("Failed to save achievement {} state: Invalid index {} or no save context", id, index);
        }

        // Queue for showing notification during gameplay
        mPendingAchievements.push(id);

        // Enable processing if it's not already
        if (!mProcessingEnabled) {
            mProcessingEnabled = true;

            // Register hook to process queued achievements during player updates
            COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, CVAR_ACHIEVEMENTS, [this](Actor* actor) {
                if (mProcessingEnabled && !mPendingAchievements.empty()) {
                    this->ProcessQueuedAchievements();
                }
            });
        }
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
            SPDLOG_INFO("Processing queued achievement: {}", achievement->name);

            // Show notification using the enhanced notification system
            ShowEnhancedNotification(achievement); // This will now display immediately if no other notification is active
        }

        // Disable processing if we've processed all pending achievements
        if (mPendingAchievements.empty()) {
            mProcessingEnabled = false;
        }
    }
}

void AchievementSystem::UnlockAchievement(const std::string& id) {
    auto achievement = GetAchievement(id);
    if (achievement && achievement->state != AchievementState::UNLOCKED) {
        achievement->state = AchievementState::UNLOCKED;
        SPDLOG_INFO("Achievement unlocked: {}", achievement->name);

        // Save achievement state to save context
        unsigned int index = GetAchievementIndex(id);
         if (index < MAX_ACHIEVEMENTS && &gSaveContext) {
            gSaveContext.save.shipSaveInfo.achievementData[index].unlocked = true;
            SPDLOG_DEBUG("Saved achievement {} state directly to save context", id);
        } else {
             SPDLOG_ERROR("Failed to save achievement {} state: Invalid index {} or no save context", id, index);
        }

        // Show enhanced notification by default
        ShowEnhancedNotification(achievement);
    }
}

void AchievementSystem::LockAchievement(const std::string& id) {
    auto achievement = GetAchievement(id);
    if (achievement && achievement->state != AchievementState::LOCKED) {
        achievement->state = AchievementState::LOCKED;
        SPDLOG_INFO("Achievement locked (debug): {}", achievement->name);

        // Update achievement state in save context
        unsigned int index = GetAchievementIndex(id);
        if (index < MAX_ACHIEVEMENTS && &gSaveContext) {
            gSaveContext.save.shipSaveInfo.achievementData[index].unlocked = false;
            SPDLOG_DEBUG("Saved achievement {} locked state directly to save context", id);
        } else {
             SPDLOG_WARN("Failed to save achievement {} locked state: Invalid index {} or no save context", id, index);
        }
    }
}

bool AchievementSystem::IsAchievementUnlocked(const std::string& id) {
    // Check in-memory state first (might be unlocked but not yet saved if game hasn't loaded yet)
    auto achievement = GetAchievement(id);
    if (achievement && achievement->state == AchievementState::UNLOCKED) {
        return true;
    }
    // If not found in memory or locked, check save context (authoritative source after load)
    if (&gSaveContext) {
         unsigned int index = GetAchievementIndex(id);
         if (index < MAX_ACHIEVEMENTS) {
             return gSaveContext.save.shipSaveInfo.achievementData[index].unlocked;
         }
    }
    return false;
}

const std::vector<std::shared_ptr<Achievement>>& AchievementSystem::GetAchievements() const {
    return mAchievements;
}

std::vector<std::shared_ptr<Achievement>>
AchievementSystem::GetAchievementsByCategory(AchievementCategory category) const {
    std::vector<std::shared_ptr<Achievement>> filteredAchievements;

    for (const auto& achievement : mAchievements) {
        if (achievement->category == category || achievement->category == AchievementCategory::BOTH) {
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
    if (achievement->category == AchievementCategory::BOTH) {
        return true;
    }

    // Otherwise, check if category matches game mode
    return (isRandomizer && achievement->category == AchievementCategory::RANDOMIZER) ||
           (!isRandomizer && achievement->category == AchievementCategory::VANILLA);
}

size_t AchievementSystem::GetUnlockedAchievementsCount() const {
    size_t count = 0;
     // Count based on the save context data as the authoritative source after load
    if (&gSaveContext) {
        for (size_t i = 0; i < mAchievements.size() && i < MAX_ACHIEVEMENTS; ++i) {
            if (gSaveContext.save.shipSaveInfo.achievementData[i].unlocked) {
                count++;
            }
        }
    } else {
        // Fallback to in-memory count if save context not available (e.g., before first load)
        for (const auto& achievement : mAchievements) {
            if (achievement->state == AchievementState::UNLOCKED) {
                count++;
            }
        }
    }
    return count;
}

void AchievementSystem::ShowNotification(const std::string& achievementName) {
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (gui) {
        gui->GetGameOverlay()->TextDrawNotification(10.0f, true, "Achievement Unlocked: %s", achievementName.c_str());
    }
}

void AchievementSystem::ShowEnhancedNotification(const std::shared_ptr<Achievement>& achievement) {
    // Default icon if none specified
    const char* iconPath = (const char*)gItemIcons[ITEM_SKULL_TOKEN]; // Gold skulltula token

    // Use achievement's icon if available
    if (!achievement->iconPath.empty()) {
        iconPath = achievement->iconPath.c_str();
    }

    // Emit enhanced style achievement notification
    Notification::EmitAchievement(iconPath, achievement->name, achievement->gamerscore);
}

std::shared_ptr<Ship::GuiWindow> AchievementSystem::CreateAchievementsWindow() {
    return std::static_pointer_cast<Ship::GuiWindow>(
        std::make_shared<AchievementsWindow>("gOpenWindows.Achievements", "Achievements"));
}

// Renamed from GetAchievementBitIndex
unsigned int AchievementSystem::GetAchievementIndex(const std::string& id) const {
     for (size_t i = 0; i < mAchievements.size(); ++i) {
        if (mAchievements[i]->id == id) {
            return i;
        }
    }
    SPDLOG_WARN("Achievement index not found for id: {}", id);
    return MAX_ACHIEVEMENTS; // Indicate not found/out of bounds
}

void AchievementSystem::LoadFromSaveContext() {
    if (&gSaveContext) {
        SPDLOG_INFO("Loading achievement states from save context...");
        for (size_t i = 0; i < mAchievements.size() && i < MAX_ACHIEVEMENTS; ++i) {
            if (gSaveContext.save.shipSaveInfo.achievementData[i].unlocked) {
                if (mAchievements[i]->state != AchievementState::UNLOCKED) {
                    mAchievements[i]->state = AchievementState::UNLOCKED;
                    SPDLOG_DEBUG("Loaded unlocked state for: {}", mAchievements[i]->id);
                }
            } else {
                if (mAchievements[i]->state != AchievementState::LOCKED) {
                    mAchievements[i]->state = AchievementState::LOCKED;
                     SPDLOG_DEBUG("Loaded locked state for: {}", mAchievements[i]->id);
                }
            }
        }
        SPDLOG_INFO("Achievement states loaded.");
    } else {
        SPDLOG_WARN("Attempted to load achievement states, but save context is not available.");
    }
}

void InitializeAchievementSystem() {
    // Ensure the singleton instance is created and initialized
    AchievementSystem& achievementSystem = AchievementSystem::Instance();

    // Register CVars and necessary hooks here (things that MUST happen once)
    CVarRegisterInteger(CVAR_NAME_ACHIEVEMENTS, 1);

    // Hook for loading achievements on save load
    COND_HOOK(OnSaveLoad, CVAR_ACHIEVEMENTS, [](s16 fileNum) {
        AchievementSystem::Instance().LoadFromSaveContext();
    });

    // Hook for potentially clearing achievement state on new file creation (Optional - uncomment if needed)
    /*
    COND_HOOK(OnNewFile, CVAR_ACHIEVEMENTS, []() {
        AchievementSystem::Instance().ClearAllStates(); // Assuming such a method exists
    });
    */

    // achievementSystem.Initialize(); // Removed: Core initialization logic moved/redundant
    SPDLOG_INFO("Core Achievement System registered for initialization.");
}

// This RegisterShipInitFunc ensures InitializeAchievementSystem runs during ShipInit
static RegisterShipInitFunc initFunc(InitializeAchievementSystem, { CVAR_NAME_ACHIEVEMENTS });
