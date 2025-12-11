#pragma once

#include <string>
#include <vector>

#include "ship/window/gui/GuiWindow.h"
#include "2s2h/Achievements/StaticData/Registry.h"

namespace Achievements {

namespace DeveloperTools {

// UI Layout Constants
namespace AchievementEditorUI {
namespace Layout {
constexpr float CARD_HEIGHT = 80.0f;
constexpr float ICON_SIZE = 64.0f;
constexpr float SECTION_SPACING = 8.0f;
} // namespace Layout
} // namespace AchievementEditorUI

class AchievementEditor : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

  protected:
    // GuiElement overrides
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;

  private:
    // Tab enumeration
    enum class Tab { BROWSER, EVENTS, DIAGNOSTICS, BULK_OPS };

    // Main Drawing Methods
    void DrawSystemStatus();
    void DrawAchievementBrowser();
    void DrawEventTesting();
    void DrawSystemDiagnostics();
    void DrawBulkOperations();

    // Browser Section Methods
    void DrawFilterControls();
    void DrawAchievementList();
    void DrawAchievementCard(const Achievement* achievement);
    void DrawAchievementDetails();
    void DrawSelectedAchievementInfo();
    void DrawEventProgressDisplay();
    void DrawAchievementActions();

    // Event Testing Section Methods
    void DrawEventTriggerControls();
    void DrawEventStatusGrid();

    // Diagnostic Section Methods
    void DrawSystemInfo();
    void DrawStatistics();
    void DrawValidationResults();

    // Utility Methods
    uint32_t GetUnlockedCount() const;
    uint32_t GetTotalCount() const;
    uint32_t GetTotalGamerscore() const;
    uint32_t GetUnlockedGamerscore() const;
    uint32_t GetTriggeredEventCount() const;

    // Filter and Validation Methods
    bool ShouldShowAchievement(const Achievement* achievement) const;
    void RefreshValidation();
    void FixCompletionIssues();
    void UnlockAllAchievements();
    void ClearAllProgress();

    // State Variables
    AchievementId mSelectedAchievementId = static_cast<AchievementId>(0);
    AchievementEvent mSelectedEventId = static_cast<AchievementEvent>(0);
    Tab mActiveTab = Tab::BROWSER;

    // Filter State
    char mSearchText[256] = {};
    AchievementCategory mCategoryFilter = static_cast<AchievementCategory>(-1);
    bool mShowUnlockedOnly = false;
    bool mShowLockedOnly = false;
    bool mShowSecretAchievements = true;

    // Validation State
    bool mValidationDirty = true;
    std::vector<std::string> mValidationErrors;
    std::vector<std::string> mValidationWarnings;
};

} // namespace DeveloperTools

} // namespace Achievements