#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <cstdint>
#include "AchievementTypes.h"

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

  private:
    AchievementSystem() = default;
    ~AchievementSystem();
    AchievementSystem(const AchievementSystem&) = delete;
    AchievementSystem& operator=(const AchievementSystem&) = delete;

    std::vector<std::shared_ptr<Achievement>> mAchievements;
    std::unordered_map<AchievementId, std::shared_ptr<Achievement>> mAchievementsMap;
    std::queue<AchievementId> mPendingAchievements;
    bool mIsLoadingOrInitializing = false;

    void ProcessQueuedAchievements();
    void ShowEnhancedNotification(const std::shared_ptr<Achievement>& achievement);
    void TryProcessQueueNow();

    // Friend declaration removed
    // friend void OnSaveLoadHandler(int16_t fileNum);
};