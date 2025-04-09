#include "Achievements.h"
#include "AchievementsWindow.h"
#include "AchievementDefinitions.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenGui/Notification.h"
#include <spdlog/spdlog.h>
#include <libultraship/libultraship.h>
#include "2s2h/BenPort.h"

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
                         int gamerscore)
    : id(id), name(name), description(description), iconPath(iconPath), state(AchievementState::LOCKED),
      isSecret(isSecret), gamerscore(gamerscore) {
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

    SPDLOG_CRITICAL("Achievement System initialized with {} achievements", mAchievements.size());
    SPDLOG_CRITICAL("=== ACHIEVEMENT SYSTEM INITIALIZATION COMPLETED ===");
}

void AchievementSystem::RegisterAchievement(std::shared_ptr<Achievement> achievement) {
    mAchievements.push_back(achievement);
    mAchievementsMap[achievement->id] = achievement;

    SPDLOG_DEBUG("Registered achievement: {}", achievement->id);
}

std::shared_ptr<Achievement> AchievementSystem::GetAchievement(const std::string& id) {
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
        unsigned int bitIndex = GetAchievementBitIndex(id);
        SetBitInSaveContext(bitIndex, true);

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
    // Only show one achievement notification per frame
    if (!mPendingAchievements.empty()) {
        std::string id = mPendingAchievements.front();
        mPendingAchievements.pop();

        auto achievement = GetAchievement(id);
        if (achievement) {
            SPDLOG_INFO("Processing queued achievement: {}", achievement->name);

            // Show notification
            ShowEnhancedNotification(achievement);
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
        unsigned int bitIndex = GetAchievementBitIndex(id);
        SetBitInSaveContext(bitIndex, true);

        // Show enhanced notification by default
        ShowEnhancedNotification(achievement);
    }
}

bool AchievementSystem::IsAchievementUnlocked(const std::string& id) {
    auto achievement = GetAchievement(id);
    return achievement && achievement->state == AchievementState::UNLOCKED;
}

const std::vector<std::shared_ptr<Achievement>>& AchievementSystem::GetAchievements() const {
    return mAchievements;
}

size_t AchievementSystem::GetUnlockedAchievementsCount() const {
    size_t count = 0;
    for (const auto& achievement : mAchievements) {
        if (achievement->state == AchievementState::UNLOCKED) {
            count++;
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

// Save Integration Methods
unsigned int AchievementSystem::GetAchievementBitIndex(const std::string& id) const {
    // Find the index of the achievement in our achievements list
    for (size_t i = 0; i < mAchievements.size(); i++) {
        if (mAchievements[i]->id == id) {
            return static_cast<unsigned int>(i);
        }
    }
    // Achievement not found
    return 0;
}

bool AchievementSystem::GetBitInSaveContext(unsigned int bitIndex) const {
    if (!&gSaveContext) {
        SPDLOG_ERROR("Save context not available for achievement state retrieval");
        return false;
    }

    // Calculate which array element and bit position
    unsigned int arrayIndex = bitIndex / 32;
    unsigned int bitPosition = bitIndex % 32;

    // Check if the index is valid
    if (arrayIndex >= 8) {
        SPDLOG_ERROR("Achievement bit index out of range: {}", bitIndex);
        return false;
    }

    // Get the bit from the save context
    return (gSaveContext.save.shipSaveInfo.achievements[arrayIndex] & (1 << bitPosition)) != 0;
}

void AchievementSystem::SetBitInSaveContext(unsigned int bitIndex, bool value) {
    if (!&gSaveContext) {
        SPDLOG_ERROR("Save context not available for achievement state storage");
        return;
    }

    // Calculate which array element and bit position
    unsigned int arrayIndex = bitIndex / 32;
    unsigned int bitPosition = bitIndex % 32;

    // Check if the index is valid
    if (arrayIndex >= 8) {
        SPDLOG_ERROR("Achievement bit index out of range: {}", bitIndex);
        return;
    }

    // Set or clear the bit
    if (value) {
        // Set the bit
        gSaveContext.save.shipSaveInfo.achievements[arrayIndex] |= (1 << bitPosition);
    } else {
        // Clear the bit
        gSaveContext.save.shipSaveInfo.achievements[arrayIndex] &= ~(1 << bitPosition);
    }
}

void AchievementSystem::LoadFromSaveContext() {
    if (!&gSaveContext) {
        SPDLOG_ERROR("Save context not available for achievement state loading");
        return;
    }

    SPDLOG_INFO("Loading achievement states from save context");

    // For each achievement, load its state from the save context
    for (const auto& achievement : mAchievements) {
        unsigned int bitIndex = GetAchievementBitIndex(achievement->id);
        bool isUnlocked = GetBitInSaveContext(bitIndex);

        // Update the achievement state
        if (isUnlocked) {
            achievement->state = AchievementState::UNLOCKED;
            SPDLOG_DEBUG("Loaded achievement {} as UNLOCKED", achievement->id);
        } else {
            achievement->state = AchievementState::LOCKED;
            SPDLOG_DEBUG("Loaded achievement {} as LOCKED", achievement->id);
        }
    }
}

void AchievementSystem::SaveToSaveContext() {
    if (!&gSaveContext) {
        SPDLOG_ERROR("Save context not available for achievement state saving");
        return;
    }

    SPDLOG_INFO("Saving achievement states to save context");

    // For each achievement, save its state to the save context
    for (const auto& achievement : mAchievements) {
        unsigned int bitIndex = GetAchievementBitIndex(achievement->id);
        bool isUnlocked = achievement->state == AchievementState::UNLOCKED;

        // Update the save context
        SetBitInSaveContext(bitIndex, isUnlocked);
        SPDLOG_DEBUG("Saved achievement {} as {}", achievement->id, isUnlocked ? "UNLOCKED" : "LOCKED");
    }
}

void AchievementSystem::SyncWithSaveContext() {
    if (!&gSaveContext) {
        SPDLOG_ERROR("Save context not available for achievement state synchronization");
        return;
    }

    bool achievementsChanged = false;

    // For each achievement, check if its state in memory differs from the save context
    for (const auto& achievement : mAchievements) {
        unsigned int bitIndex = GetAchievementBitIndex(achievement->id);
        bool isUnlockedInSave = GetBitInSaveContext(bitIndex);
        bool isUnlockedInMemory = achievement->state == AchievementState::UNLOCKED;

        // If the states differ, update the save context
        if (isUnlockedInMemory != isUnlockedInSave) {
            SetBitInSaveContext(bitIndex, isUnlockedInMemory);
            achievementsChanged = true;
            SPDLOG_DEBUG("Synced achievement {} state to {}", achievement->id,
                         isUnlockedInMemory ? "UNLOCKED" : "LOCKED");
        }
    }

    if (achievementsChanged) {
        SPDLOG_INFO("Achievement states synchronized with save context");
    }
}

// Define all achievements
void AchievementSystem::RegisterAchievements() {
    // Starting achievements
    REGISTER_ACHIEVEMENT(
        "first_steps", "First Steps", "Begin your adventure in Termina", (const char*)gItemIcons[ITEM_OCARINA_OF_TIME],
        false, 10, OnSceneInit,
        (std::get<0>(std::forward_as_tuple(args...)) != 0x00 && std::get<0>(std::forward_as_tuple(args...)) != 0x02));

    // Boss achievements
    REGISTER_ACHIEVEMENT("defeat_odolwa", "Jungle Warrior", "Defeat Odolwa, Masked Jungle Warrior",
                         (const char*)gItemIcons[ITEM_REMAINS_ODOLWA], false, 20, OnActorInit,
                         CHECK_QUEST_ITEM(QUEST_REMAINS_ODOLWA));

    REGISTER_ACHIEVEMENT("defeat_goht", "Mountain Racer", "Defeat Goht, Masked Mechanical Monster",
                         (const char*)gItemIcons[ITEM_REMAINS_GOHT], false, 20, OnActorInit,
                         CHECK_QUEST_ITEM(QUEST_REMAINS_GOHT));

    REGISTER_ACHIEVEMENT("defeat_gyorg", "Ocean Conqueror", "Defeat Gyorg, Gargantuan Masked Fish",
                         (const char*)gItemIcons[ITEM_REMAINS_GYORG], false, 20, OnActorInit,
                         CHECK_QUEST_ITEM(QUEST_REMAINS_GYORG));

    REGISTER_ACHIEVEMENT("defeat_twinmold", "Desert Exterminator", "Defeat Twinmold, Giant Masked Insects",
                         (const char*)gItemIcons[ITEM_REMAINS_TWINMOLD], false, 20, OnActorInit,
                         CHECK_QUEST_ITEM(QUEST_REMAINS_TWINMOLD));

    REGISTER_ACHIEVEMENT("defeat_majora", "Savior of Termina", "Defeat Majora and save Termina",
                         (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], false, 50, OnActorInit,
                         CHECK_WEEKEVENTREG(WEEKEVENTREG_77_01));

    // Mask achievements
    REGISTER_ACHIEVEMENT("collect_deku_mask", "Wooden Face", "Obtain the Deku Mask",
                         (const char*)gItemIcons[ITEM_MASK_DEKU], false, 10, OnActorInit,
                         INV_CONTENT(ITEM_MASK_DEKU) == ITEM_MASK_DEKU);

    REGISTER_ACHIEVEMENT("collect_goron_mask", "Stone Face", "Obtain the Goron Mask",
                         (const char*)gItemIcons[ITEM_MASK_GORON], false, 10, OnActorInit,
                         INV_CONTENT(ITEM_MASK_GORON) == ITEM_MASK_GORON);

    REGISTER_ACHIEVEMENT("collect_zora_mask", "Scale Face", "Obtain the Zora Mask",
                         (const char*)gItemIcons[ITEM_MASK_ZORA], false, 10, OnActorInit,
                         INV_CONTENT(ITEM_MASK_ZORA) == ITEM_MASK_ZORA);

    REGISTER_ACHIEVEMENT("collect_fierce_deity", "God of War", "Obtain the Fierce Deity Mask",
                         (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], true, 30, OnActorInit,
                         INV_CONTENT(ITEM_MASK_FIERCE_DEITY) == ITEM_MASK_FIERCE_DEITY);

    REGISTER_ACHIEVEMENT("collect_all_masks", "Mask Collector", "Collect all 24 masks",
                         (const char*)gItemIcons[ITEM_MASK_TRUTH], true, 50, OnActorInit, [this]() {
                             for (u8 i = ITEM_MASK_DEKU; i <= ITEM_MASK_GIANT; i++) {
                                 if (INV_CONTENT(i) == ITEM_NONE)
                                     return false;
                             }
                             return true;
                         }());

    // Heart piece achievements
    REGISTER_ACHIEVEMENT("collect_heart_container", "Heart of a Hero", "Collect your first Heart Container",
                         (const char*)gItemIcons[ITEM_HEART_CONTAINER], false, 10, OnActorInit,
                         gSaveContext.save.saveInfo.playerData.healthCapacity > 0x30);

    REGISTER_ACHIEVEMENT("max_health", "Full of Heart", "Obtain maximum health (20 hearts)",
                         (const char*)gItemIcons[ITEM_HEART_CONTAINER], true, 30, OnActorInit,
                         gSaveContext.save.saveInfo.playerData.healthCapacity >= 0x140);

    // Fairy achievements
    REGISTER_ACHIEVEMENT("collect_great_fairy_sword", "Fairy's Champion", "Obtain the Great Fairy's Sword",
                         (const char*)gItemIcons[ITEM_SWORD_GREAT_FAIRY], true, 20, OnActorInit,
                         INV_CONTENT(ITEM_SWORD_GREAT_FAIRY) == ITEM_SWORD_GREAT_FAIRY);

    // Song achievements
    REGISTER_ACHIEVEMENT("learn_song_of_time", "Time Traveler", "Learn the Song of Time",
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, OnActorInit,
                         CHECK_QUEST_ITEM(QUEST_SONG_TIME));

    REGISTER_ACHIEVEMENT("learn_song_of_healing", "Soul Healer", "Learn the Song of Healing",
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, OnActorInit,
                         CHECK_QUEST_ITEM(QUEST_SONG_HEALING));

    REGISTER_ACHIEVEMENT("learn_all_songs", "Musician of Termina", "Learn all the songs",
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 30, OnActorInit, [this]() {
                             u32 allSongsBits =
                                 (1 << QUEST_SONG_SONATA) | (1 << QUEST_SONG_LULLABY) | (1 << QUEST_SONG_BOSSA_NOVA) |
                                 (1 << QUEST_SONG_ELEGY) | (1 << QUEST_SONG_OATH) | (1 << QUEST_SONG_SARIA) |
                                 (1 << QUEST_SONG_TIME) | (1 << QUEST_SONG_HEALING) | (1 << QUEST_SONG_EPONA) |
                                 (1 << QUEST_SONG_SOARING) | (1 << QUEST_SONG_STORMS) | (1 << QUEST_SONG_SUN);
                             return (GET_SAVE_INVENTORY_QUEST_ITEMS & allSongsBits) == allSongsBits;
                         }());
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

    // Load achievements from save context if available
    if (&gSaveContext) {
        achievementSystem.LoadFromSaveContext();
        SPDLOG_INFO("Loaded achievement states from save context");
    }

    // Register event hooks for save integration using new COND_HOOK approach
    COND_HOOK(OnSaveLoad, CVAR_ACHIEVEMENTS, [](s16 fileNum) {
        if (AchievementSystem::Instance) {
            AchievementSystem::Instance->LoadFromSaveContext();
            SPDLOG_INFO("Loaded achievement states after save file load");
        }
    });

    // Use OnGameStateUpdate for file save detection instead of BeforeFileSave (which doesn't exist)
    COND_HOOK(OnGameStateUpdate, CVAR_ACHIEVEMENTS, []() {
        if (AchievementSystem::Instance && gSaveContext.save.entrance != -1) {
            // We don't have a direct "BeforeFileSave" hook, so we can update achievements
            // periodically during gameplay to ensure they're saved on the next save
            static uint64_t lastSyncTime = 0;
            uint64_t currentTime = GetUnixTimestamp();

            // Only sync achievements every 30 seconds to avoid unnecessary work
            if (currentTime - lastSyncTime >= 30) {
                AchievementSystem::Instance->SyncWithSaveContext();
                lastSyncTime = currentTime;
            }
        }
    });

    // Register cycle change hooks
    COND_HOOK(BeforeEndOfCycleSave, CVAR_ACHIEVEMENTS, []() {
        if (AchievementSystem::Instance) {
            // Save achievement state before cycle reset
            AchievementSystem::Instance->SaveToSaveContext();
            SPDLOG_INFO("Saved achievement states before cycle reset");
        }
    });

    COND_HOOK(AfterEndOfCycleSave, CVAR_ACHIEVEMENTS, []() {
        if (AchievementSystem::Instance) {
            // Achievements should persist after cycle resets
            AchievementSystem::Instance->SyncWithSaveContext();
            SPDLOG_INFO("Synchronized achievement states after cycle reset");
        }
    });

    // Handle moon crash reset
    COND_HOOK(BeforeMoonCrashSaveReset, CVAR_ACHIEVEMENTS, []() {
        if (AchievementSystem::Instance) {
            // Save achievement state before moon crash
            AchievementSystem::Instance->SaveToSaveContext();
            SPDLOG_INFO("Saved achievement states before moon crash");
        }
    });

    // We don't have direct hooks for owl statue or file creation, so we rely on other hooks
    // and existing functionalities in the game code.
    // We've added our SaveAchievementsToSaveContext C function that can be called from anywhere
    // in the game code.

    isInitialized = true;
    SPDLOG_CRITICAL("Achievement System initialization complete");
    SPDLOG_CRITICAL("=== ACHIEVEMENT SYSTEM GLOBAL INITIALIZATION COMPLETED ===");
}

// C-accessible function for saving achievements
extern "C" void SaveAchievementsToSaveContext() {
    if (AchievementSystem::Instance) {
        // Check if this is a new save file by looking at health capacity (3 hearts is default)
        bool isNewSaveFile = (gSaveContext.save.saveInfo.playerData.healthCapacity == 0x30 &&
                              gSaveContext.save.saveInfo.playerData.threeDayResetCount <= 1);

        if (isNewSaveFile) {
            // Reset achievements for new save file
            for (auto& achievement : AchievementSystem::Instance->GetAchievements()) {
                achievement->state = AchievementState::LOCKED;
            }
            SPDLOG_INFO("Reset achievements for new save file");
        }

        AchievementSystem::Instance->SaveToSaveContext();
        SPDLOG_INFO("Saved achievement states through C interface");
    }
}

// Register initialization function
static RegisterShipInitFunc initFunc(InitializeAchievementSystem, { CVAR_NAME_ACHIEVEMENTS });
