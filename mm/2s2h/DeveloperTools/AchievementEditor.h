#ifndef DEVELOPER_TOOLS_ACHIEVEMENT_EDITOR_H
#define DEVELOPER_TOOLS_ACHIEVEMENT_EDITOR_H

// Standard library
#include <string>
#include <vector>

// Ship/libultraship
#include <ship/window/gui/GuiWindow.h>

// 2s2h
#include "2s2h/Achievements/StaticData/Registry.h"

namespace DeveloperTools {

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
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;

  private:
    enum class Tab { BROWSER, EVENTS, DIAGNOSTICS, BULK_OPS };

    void DrawSystemStatus();
    void DrawAchievementBrowser();
    void DrawEventTesting();
    void DrawSystemDiagnostics();
    void DrawBulkOperations();
    void DrawFilterControls();
    void DrawAchievementList();
    void DrawAchievementCard(const Achievement* achievement);
    void DrawAchievementDetails();
    void DrawSelectedAchievementInfo();
    void DrawEventProgressDisplay();
    void DrawAchievementActions();
    void DrawEventTriggerControls();
    void DrawEventStatusGrid();
    void DrawSystemInfo();
    void DrawStatistics();
    void DrawValidationResults();
    uint32_t GetUnlockedCount() const;
    uint32_t GetTotalCount() const;
    uint32_t GetTotalHarbourMastery() const;
    uint32_t GetUnlockedHarbourMastery() const;
    uint32_t GetTriggeredEventCount() const;
    bool ShouldShowAchievement(const Achievement* achievement) const;
    void RefreshValidation();
    void FixCompletionIssues();
    void UnlockAllAchievements();
    void ClearAllProgress();

    AchievementId mSelectedAchievementId = static_cast<AchievementId>(0);
    AchievementEvent mSelectedEventId = static_cast<AchievementEvent>(0);
    Tab mActiveTab = Tab::BROWSER;
    char mSearchText[256] = {};
    AchievementCategory mCategoryFilter = static_cast<AchievementCategory>(-1);
    bool mShowUnlockedOnly = false;
    bool mShowLockedOnly = false;
    bool mShowSecretAchievements = true;
    bool mValidationDirty = true;
    std::vector<std::string> mValidationErrors;
    std::vector<std::string> mValidationWarnings;
};

} // namespace DeveloperTools

#endif // DEVELOPER_TOOLS_ACHIEVEMENT_EDITOR_H