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
AchievementSystem* AchievementSystem::Instance = nullptr;

// Achievement implementation
Achievement::Achievement(std::string id, std::string name, std::string description, std::string iconPath, bool isSecret,
                         int gamerscore, AchievementCategory category)
    : id(id), name(name), description(description), iconPath(iconPath), state(AchievementState::LOCKED),
      isSecret(isSecret), gamerscore(gamerscore), category(category) {
}

// Achievement System implementation
AchievementSystem::AchievementSystem() {
    Instance = this;
    mProcessingEnabled = false;
}

AchievementSystem::~AchievementSystem() {
    if (Instance == this) {
        Instance = nullptr;
    }
}

void AchievementSystem::Initialize() {
    SPDLOG_CRITICAL("=== ACHIEVEMENT SYSTEM INITIALIZATION STARTED ===");
    SPDLOG_CRITICAL("Current instance pointer: {}", (void*)Instance);
    SPDLOG_CRITICAL("This instance pointer: {}", (void*)this);

    // Register all achievements
    RegisterAchievements();

    // Check if the number of registered achievements exceeds the max defined in save structure
    if (mAchievements.size() > MAX_ACHIEVEMENTS) {
        SPDLOG_ERROR("Number of registered achievements ({}) exceeds MAX_ACHIEVEMENTS ({})!", mAchievements.size(), MAX_ACHIEVEMENTS);
        // Handle error appropriately, e.g., disable achievements or assert
    }

    SPDLOG_CRITICAL("Achievement System initialized with {} achievements", mAchievements.size());
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
    // Find the index of the achievement in our achievements list
    for (size_t i = 0; i < mAchievements.size(); i++) {
        if (mAchievements[i]->id == id) {
            // Check bounds before returning
            if (i >= MAX_ACHIEVEMENTS) {
                 SPDLOG_ERROR("Achievement index {} is out of bounds for ID {} (Max: {})", i, id, MAX_ACHIEVEMENTS -1);
                 return MAX_ACHIEVEMENTS; // Return an invalid index
            }
            return static_cast<unsigned int>(i);
        }
    }
    // Achievement not found
     SPDLOG_WARN("Achievement ID {} not found in registered list", id);
    return MAX_ACHIEVEMENTS; // Return an invalid index
}

void AchievementSystem::LoadFromSaveContext() {
    if (!&gSaveContext) {
        SPDLOG_ERROR("Save context not available for achievement state loading");
        return;
    }

    SPDLOG_INFO("Loading achievement states from save context");

    // For each achievement, load its state from the save context's achievementData array
    for (const auto& achievement : mAchievements) {
        unsigned int index = GetAchievementIndex(achievement->id);

        // Check if index is valid before accessing array
        if (index < MAX_ACHIEVEMENTS) {
            bool isUnlocked = gSaveContext.save.shipSaveInfo.achievementData[index].unlocked;

            // Update the in-memory achievement state
            if (isUnlocked) {
                achievement->state = AchievementState::UNLOCKED;
                SPDLOG_DEBUG("Loaded achievement {} ({}) as UNLOCKED", achievement->id, index);
            } else {
                achievement->state = AchievementState::LOCKED;
                 SPDLOG_DEBUG("Loaded achievement {} ({}) as LOCKED", achievement->id, index);
            }
        } else {
            SPDLOG_ERROR("Failed to load state for achievement {}: Invalid index {}", achievement->id, index);
        }
    }
}

// Initialize achievement system
void InitializeAchievementSystem() {
    static bool isInitialized = false;

    SPDLOG_CRITICAL("=== ACHIEVEMENT SYSTEM GLOBAL INITIALIZATION STARTED ===");
    SPDLOG_CRITICAL("Current initialization state: {}", isInitialized);
    SPDLOG_CRITICAL("Current instance pointer: {}", (void*)AchievementSystem::Instance);

    // Prevent double initialization
    if (isInitialized) {
        SPDLOG_CRITICAL("Achievement System already initialized, skipping");
        SPDLOG_CRITICAL("=== ACHIEVEMENT SYSTEM GLOBAL INITIALIZATION SKIPPED ===");
        return;
    }

    // Create console variable for achievements configuration
    CVarRegisterInteger(CVAR_NAME_ACHIEVEMENTS, 1);

    // Create and initialize the achievement system only
    static AchievementSystem achievementSystem;
    achievementSystem.Initialize();

    // Load achievements from save context if available (this now reads the new structure)
    if (&gSaveContext) {
        achievementSystem.LoadFromSaveContext();
        SPDLOG_INFO("Loaded initial achievement states from save context");
    }

    // Register event hook for save loading
    COND_HOOK(OnSaveLoad, CVAR_ACHIEVEMENTS, [](s16 fileNum) {
        if (AchievementSystem::Instance) {
            AchievementSystem::Instance->LoadFromSaveContext();
            SPDLOG_INFO("Loaded achievement states after save file load");
        }
    });

    isInitialized = true;
    SPDLOG_CRITICAL("Achievement System initialization complete");
    SPDLOG_CRITICAL("=== ACHIEVEMENT SYSTEM GLOBAL INITIALIZATION COMPLETED ===");
}

// Register initialization function
static RegisterShipInitFunc initFunc(InitializeAchievementSystem, { CVAR_NAME_ACHIEVEMENTS });
