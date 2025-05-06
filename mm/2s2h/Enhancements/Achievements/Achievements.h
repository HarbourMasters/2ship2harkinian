#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <cstdint>
#include "AchievementTypes.h"
#include <map>
#include "libultraship/libultraship.h"
#include <functional>

// Forward declare handlers
// void OnSaveInitHandler(int16_t fileNum);
// void OnSaveLoadHandler(int16_t fileNum);
void OnGameStateMainStartHandler();
void OnGameStateMainFinishHandler();

namespace Ship {
class GuiWindow;
}

enum class AchievementCategory { VANILLA, RANDOMIZER, BOTH };

struct Achievement {
  public:
    Achievement(AchievementId id, std::string name, std::string description, std::string iconPath,
                bool isSecret = false, int gamerscore = 0, AchievementCategory category = AchievementCategory::BOTH);

    AchievementId getId() const {
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

  private:
    AchievementId mId;
    std::string mName;
    std::string mDescription;
    std::string mIconPath;
    bool mIsSecret;
    int mGamerscore;
    AchievementCategory mCategory;
};

// Enum to differentiate notification types in the queue
enum class PendingNotificationType {
    UNLOCK,
    PROGRESS
};

// Struct to hold information for any pending notification
struct PendingNotificationInfo {
    PendingNotificationType type;
    AchievementId id;
    s32 currentProgress; // Relevant for PROGRESS type
    s32 targetProgress;  // Relevant for PROGRESS type

    // Constructor for UNLOCK type
    PendingNotificationInfo(PendingNotificationType t, AchievementId aid)
        : type(t), id(aid), currentProgress(0), targetProgress(0) {}

    // Constructor for PROGRESS type
    PendingNotificationInfo(PendingNotificationType t, AchievementId aid, s32 current, s32 target)
        : type(t), id(aid), currentProgress(current), targetProgress(target) {}
};

class AchievementSystem {
  public:
    static AchievementSystem& Instance();
    static void Init();

    void RegisterAchievement(std::shared_ptr<Achievement> achievement);

    std::shared_ptr<Achievement> GetAchievement(AchievementId id) const;

    void DebugUnlockAchievement(AchievementId id);

    void LockAchievement(AchievementId id);

    void QueueAchievementUnlock(AchievementId id);

    bool IsAchievementUnlocked(AchievementId id);

    const std::vector<std::shared_ptr<Achievement>>& GetAchievements() const;

    std::vector<std::shared_ptr<Achievement>> GetAchievementsByCategory(AchievementCategory category) const;

    bool IsAchievementRelevantForGameMode(AchievementId id, bool isRandomizer) const;

    size_t GetUnlockedAchievementsCount() const;

    // Progress Tracking
    void UpdateAchievementProgress(AchievementId id);

    std::shared_ptr<Ship::GuiWindow> CreateAchievementsWindow();

    void ResetStateAndQueueForLoadedSave();

    bool IsLoadingOrInitializing() const {
        return mIsLoadingOrInitializing;
    }

    // New methods to manage the loading flag
    void StartLoadingOrInitializing();
    void FinishLoadingOrInitializing();

    // Make new handlers friends if they need direct access (they will need to set the flag)
    friend void OnGameStateMainStartHandler();

    void TryProcessQueueNow();

  private:
    AchievementSystem() = default;
    ~AchievementSystem();
    AchievementSystem(const AchievementSystem&) = delete;
    AchievementSystem& operator=(const AchievementSystem&) = delete;

    std::vector<std::shared_ptr<Achievement>> mAchievements;
    std::map<AchievementId, std::shared_ptr<Achievement>> mAchievementsMap;
    std::queue<PendingNotificationInfo> mPendingAchievements;
    bool mIsLoadingOrInitializing = false;

    void ProcessQueuedAchievements();
    void ShowEnhancedNotification(const std::shared_ptr<Achievement>& achievement);

    // Friend declaration removed
    // friend void OnSaveLoadHandler(int16_t fileNum);
};