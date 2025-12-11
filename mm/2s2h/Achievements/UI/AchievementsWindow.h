#pragma once

// Standard library
#include <memory>
#include <string>

// Third-party libraries
#include <imgui.h>
#include "ship/window/gui/GuiWindow.h"

// Forward declarations
struct Achievement;

namespace Achievements {

namespace UI {

namespace AchievementsUI {
// UI Layout Constants
namespace Layout {
constexpr float STANDARD_SPACING = 6.0f;
constexpr float TIGHT_SPACING = 4.0f;
constexpr float PANEL_PADDING = 8.0f;
constexpr float FRAME_ROUNDING = 6.0f;
constexpr float BORDER_SIZE = 1.0f;
} // namespace Layout

namespace Card {
constexpr float HEIGHT = 80.0f;
constexpr float PADDING = 12.0f;
constexpr float ICON_SIZE = 60.0f;
constexpr float BORDER_ROUNDING = 6.0f;
} // namespace Card

namespace Icon {
constexpr float PROGRESS_SIZE = 20.0f;
constexpr float HEADER_SIZE = 24.0f;
constexpr float FONT_SCALE = 1.5f;
} // namespace Icon

namespace ProgressBar {
constexpr float MIN_WIDTH = 100.0f;
constexpr float HEIGHT_RATIO = 0.8f;
constexpr float ROUNDING = 4.0f;
} // namespace ProgressBar

// Theme enumeration for different card states
enum class CardTheme {
    UNLOCKED, // Gold theme for completed achievements
    PROGRESS, // Blue theme for achievements with partial progress
    LOCKED    // Gray theme for locked achievements
};
} // namespace AchievementsUI

class AchievementsWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;
    ~AchievementsWindow();

    // Cache management - public to allow external systems to invalidate when modifying achievement state
    void InvalidateCache();

  protected:
    // GuiElement overrides
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;

  private:
    // Data Structures
    struct ProgressStats {
        uint32_t unlockedCount = 0;
        uint32_t totalCount = 0;
        uint32_t unlockedScore = 0;
        uint32_t totalScore = 0;
    };

    struct CachedTextures {
        const char* bombersNotebook = nullptr;
        const char* heartPiece = nullptr;
        const char* ribbon = nullptr;
        const char* wallet = nullptr;
        bool initialized = false;
    };

    // Main UI Drawing Methods
    void DrawInGameInterface();
    void DrawNotInGameMessage();
    void DrawActivationPrompt();
    void DrawHeaderPanel();
    void DrawProgressSection();
    void DrawFilterSection();
    void DrawAchievementList();
    void DrawAchievementCard(const Achievement* achievement);

    // Card Drawing Helpers
    void DrawCardIcon(const Achievement* achievement, AchievementsUI::CardTheme theme, bool hasProgress);
    void DrawCardContent(const Achievement* achievement, AchievementsUI::CardTheme theme);
    void DrawCardScore(const Achievement* achievement, AchievementsUI::CardTheme theme, const ImVec2& cardPos,
                       float cardWidth);
    void DrawProgressIcons(const Achievement* achievement);

    // Panel and UI Helpers
    void BeginAchievementsPanel();
    void EndAchievementsPanel();
    void DrawFontIcon(const char* icon, const ImVec4& color, AchievementsUI::CardTheme theme);
    void DrawFilterButtons();
    void DrawActivationDisclaimer();
    void DrawProgressIcon();
    void DrawScoreIcon();
    void DrawProgressBar(float progress, float width, float height, const ImVec4& color);
    void DrawCardBackground(const ImVec2& pos, const ImVec2& size, AchievementsUI::CardTheme theme, bool hovered);

    // Utility Methods
    ProgressStats CalculateProgressStats() const;
    AchievementsUI::CardTheme DetermineCardTheme(const Achievement* achievement, bool& hasProgress) const;
    bool ShouldShowAchievement(const Achievement* achievement) const;
    std::string GetDisplayName(const Achievement* achievement, bool isUnlocked) const;
    std::string GetDisplayDescription(const Achievement* achievement, bool isUnlocked) const;
    bool IsRandomizerMode() const;

    // Resource Management
    void InitializeTextures();
    const char* GetBestIconTexture() const;

    // Styling Helpers
    ImVec4 GetCardColors(AchievementsUI::CardTheme theme, bool isBackground = false) const;
    ImVec4 GetTextColor(AchievementsUI::CardTheme theme, bool isPrimary = true) const;

    // Settings Management
    void LoadSettings();
    void SaveSettings();

    // State Management
    ImGuiTextFilter mAchievementFilter;
    bool mShowLockedOnly = false;
    bool mShowUnlockedOnly = false;
    bool mIsRandomizerMode = false;
    bool mIsInGame = false;
    bool mIsValidGameMode = false;

    // Cached Data
    mutable ProgressStats mCachedStats = {};
    mutable bool mStatsNeedUpdate = true;
    CachedTextures mTextures = {};
};

} // namespace UI

} // namespace Achievements