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
enum class AchievementCategory {
    VANILLA,    // Standard game achievements
    RANDOMIZER, // Randomizer mode specific achievements
    BOTH        // Achievements applicable to both modes
};

/**
 * @enum AchievementNotificationType
 * @brief Determines the visual style of achievement notifications
 */
enum class AchievementNotificationType {
    SIMPLE,  // Simple notification
    ENHANCED // Enhanced notification with animations
};

/**
 * @struct Achievement
 * @brief Data structure representing a single achievement
 */
struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    std::string iconPath;
    AchievementState state;
    bool isSecret;
    int gamerscore;               // Optional gamerscore value
    AchievementCategory category; // Category for game mode specific achievements

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
};

/**
 * @class AchievementSystem
 * @brief Manages the registration, unlocking, and persistence of achievements
 */
class AchievementSystem {
  public:
    static AchievementSystem* Instance;

    AchievementSystem();
    ~AchievementSystem();

    /**
     * @brief Initializes the achievement system
     */
    void Initialize();

    /**
     * @brief Registers all available achievements
     */
    void RegisterAchievements();

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
     * @brief Loads achievement states from save context
     */
    void LoadFromSaveContext();

  private:
    // Achievement Storage
    std::vector<std::shared_ptr<Achievement>> mAchievements;
    std::unordered_map<std::string, std::shared_ptr<Achievement>> mAchievementsMap;

    // Queue for pending achievement unlocks
    std::queue<std::string> mPendingAchievements;
    bool mProcessingEnabled;

    /**
     * @brief Gets the index for an achievement in the save data array
     * @param id Unique identifier of the achievement
     * @return Index for the achievement, or MAX_ACHIEVEMENTS if not found/out of bounds
     */
    unsigned int GetAchievementIndex(const std::string& id) const;
};

/**
 * @brief Initializes the achievement system singleton
 */
void InitializeAchievementSystem();