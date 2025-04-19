#pragma once

// Standard Library Includes
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>

// Forward Declarations
namespace Ship {
class GuiWindow;
}

/**
 * @enum AchievementState
 * @brief Represents the state of an achievement
 */
enum class AchievementState { LOCKED, UNLOCKED };

/**
 * @enum AchievementCategory
 * @brief Categorizes achievements by game mode
 */
enum class AchievementCategory { VANILLA, RANDOMIZER, BOTH };

/**
 * @enum AchievementNotificationType
 * @brief Determines the visual style of achievement notifications
 */
enum class AchievementNotificationType { SIMPLE, ENHANCED };

/**
 * @struct Achievement
 * @brief Data structure representing a single achievement
 */
struct Achievement {
  public:
    /**
     * @brief Constructs an Achievement
     * @param id Unique identifier for the achievement
     * @param name Display name of the achievement
     * @param description Text description of the achievement
     * @param iconPath Path to the achievement's icon
     * @param isSecret Whether the achievement is hidden until unlocked
     * @param gamerscore Point value associated with the achievement
     * @param category Category determining which game mode the achievement belongs to
     */
    Achievement(std::string id, std::string name, std::string description, std::string iconPath, bool isSecret = false,
                int gamerscore = 0, AchievementCategory category = AchievementCategory::BOTH);

    // Public Getters
    const std::string& getId() const {
        return mId;
    }
    const std::string& getName() const {
        return mName;
    }
    const std::string& getDescription() const {
        return mDescription;
    }
    const std::string& getIconPath() const {
        return mIconPath;
    }
    bool isSecret() const {
        return mIsSecret;
    }
    int getGamerscore() const {
        return mGamerscore;
    }
    AchievementCategory getCategory() const {
        return mCategory;
    }

    // Allow AchievementSystem to modify state directly (friend or internal setter)
    // Option 1: Friend class (simple for this case)
    // friend class AchievementSystem;
    // Option 2: Internal setter (if friend is undesirable)
    // void setStateInternal(AchievementState newState) { mState = newState; }

  private:
    std::string mId;
    std::string mName;
    std::string mDescription;
    std::string mIconPath;
    bool mIsSecret;
    int mGamerscore;               // Optional gamerscore value
    AchievementCategory mCategory; // Category for game mode specific achievements
};

/**
 * @class AchievementSystem
 * @brief Manages the registration, unlocking, and persistence of achievements
 */
class AchievementSystem {
  public:
    static AchievementSystem& Instance();

    /**
     * @brief Registers a single achievement with the system
     * @param achievement The achievement to register
     */
    void RegisterAchievement(std::shared_ptr<Achievement> achievement);

    /**
     * @brief Gets an achievement by its ID
     * @param id Unique identifier of the achievement
     * @return Pointer to the achievement if found, nullptr otherwise
     */
    std::shared_ptr<Achievement> GetAchievement(const std::string& id) const;

    /**
     * @brief Unlocks an achievement and shows a notification
     * @param id Unique identifier of the achievement to unlock
     */
    void UnlockAchievement(const std::string& id);

    /**
     * @brief Locks an achievement (primarily for testing/debugging)
     * @param id Unique identifier of the achievement to lock
     */
    void LockAchievement(const std::string& id);

    /**
     * @brief Queues an achievement to be unlocked during gameplay
     * @param id Unique identifier of the achievement to queue for unlock
     */
    void QueueAchievementUnlock(const std::string& id);

    /**
     * @brief Processes any queued achievement unlocks during gameplay
     * This ensures notifications only appear when the player is in a playable state
     */
    void ProcessQueuedAchievements();

    /**
     * @brief Checks if an achievement is unlocked
     * @param id Unique identifier of the achievement to check
     * @return true if the achievement is unlocked, false otherwise
     */
    bool IsAchievementUnlocked(const std::string& id);

    /**
     * @brief Gets all registered achievements
     * @return Vector of all achievements
     */
    const std::vector<std::shared_ptr<Achievement>>& GetAchievements() const;

    /**
     * @brief Gets all achievements for a specific category
     * @param category The category to filter by
     * @return Vector of achievements matching the category
     */
    std::vector<std::shared_ptr<Achievement>> GetAchievementsByCategory(AchievementCategory category) const;

    /**
     * @brief Checks if an achievement is relevant for the current game mode
     * @param id Unique identifier of the achievement to check
     * @param isRandomizer Whether the current game mode is randomizer
     * @return true if the achievement is applicable to the current game mode
     */
    bool IsAchievementRelevantForGameMode(const std::string& id, bool isRandomizer) const;

    /**
     * @brief Gets the count of unlocked achievements
     * @return Number of unlocked achievements
     */
    size_t GetUnlockedAchievementsCount() const;

    /**
     * @brief Gets the current runtime state of all achievements
     * @return A map of achievement ID to unlocked status (true/false)
     */
    const std::unordered_map<std::string, bool>& GetCurrentStates() const;

    /**
     * @brief Shows a simple achievement notification
     * @param achievementName Name of the achievement to display
     */
    void ShowNotification(const std::string& achievementName);

    /**
     * @brief Shows an enhanced achievement notification with icon and animation
     * @param achievement The achievement to display
     */
    void ShowEnhancedNotification(const std::shared_ptr<Achievement>& achievement);

    /**
     * @brief Creates a window to display achievements
     * @return Shared pointer to the created window
     */
    std::shared_ptr<Ship::GuiWindow> CreateAchievementsWindow();

    /**
     * @brief Loads achievement states from deserialized data map
     * @param loadedStates Map of achievement ID to unlocked status from save file
     */
    void LoadFromSaveData(const std::unordered_map<std::string, bool>& loadedStates);

    /**
     * @brief Resets all achievement states to LOCKED for a new game.
     */
    void ResetStatesForNewGame();

  private:
    AchievementSystem();
    ~AchievementSystem();

    // Achievement Storage
    std::vector<std::shared_ptr<Achievement>> mAchievements;
    std::unordered_map<std::string, std::shared_ptr<Achievement>> mAchievementsMap;
    std::unordered_map<std::string, bool> mCurrentAchievementStates;

    // Queue for pending achievement unlocks
    std::queue<std::string> mPendingAchievements;
    bool mProcessingEnabled;
};

/**
 * @brief Initializes the achievement system singleton
 */
void InitializeAchievementSystem();