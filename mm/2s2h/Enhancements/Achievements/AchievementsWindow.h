#pragma once

// Standard Library Includes
#include <vector>
#include <memory>
#include <string>

// Ship Includes
#include <window/gui/GuiWindow.h>
#include "../../BenGui/UIWidgets.hpp"

// Local Includes
#include "Achievements.h"

/**
 * @class AchievementsWindow
 * @brief GUI window for displaying and managing achievements
 *
 * This window provides a user interface for viewing achievement progress,
 * filtering achievements by state, and displaying achievement details.
 * Features include:
 * - Achievement progress tracking
 * - Filtering by locked/unlocked state
 * - Search functionality
 * - Visual indicators for achievement status
 */
class AchievementsWindow : public Ship::GuiWindow {
  public:
    /**
     * @brief Constructs an AchievementsWindow
     * @param consoleVariable The console variable controlling window visibility
     * @param name The name of the window
     */
    AchievementsWindow(const std::string& consoleVariable, const std::string& name);

    /**
     * @brief Destructor that clears achievement references
     */
    ~AchievementsWindow() {
        mAchievements.clear();
    }

  protected:
    /**
     * @brief Initializes the window elements
     */
    void InitElement() override;

    /**
     * @brief Draws the window contents
     */
    void DrawElement() override;

    /**
     * @brief Updates the window state based on CVars and achievement system
     */
    void UpdateElement() override;

  private:
    // Achievement Data
    std::vector<std::shared_ptr<Achievement>> mAchievements;

    // UI State
    bool mShowLockedOnly = false;
    bool mShowUnlockedOnly = false;
    bool mIsRandomizerMode = false; // Current game mode
    static ImGuiTextFilter sAchievementFilter;

    /**
     * @brief Draws the window header with title
     */
    void DrawHeader();

    /**
     * @brief Draws the achievement progress bar and statistics
     */
    void DrawProgressBar();

    /**
     * @brief Draws the filter controls for achievements
     */
    void DrawFilters();

    /**
     * @brief Draws the scrollable list of achievements
     */
    void DrawAchievementList();

    /**
     * @brief Draws a single achievement item in the list
     * @param achievement The achievement to draw
     */
    void DrawAchievementItem(const std::shared_ptr<Achievement>& achievement);

    /**
     * @brief Draws the icon for an achievement
     * @param achievement The achievement whose icon to draw
     */
    void DrawAchievementIcon(const std::shared_ptr<Achievement>& achievement);

    /**
     * @brief Draws the detailed information for an achievement
     * @param achievement The achievement to display details for
     */
    void DrawAchievementDetails(const std::shared_ptr<Achievement>& achievement);

    /**
     * @brief Draws a message indicating achievements are disabled
     */
    void DrawDisabledMessage();

    /**
     * @brief Draws a message indicating the player is not in game
     */
    void DrawNotInGameMessage();

    /**
     * @brief Checks if the current game is in randomizer mode
     * @return true if the game is in randomizer mode
     */
    bool IsRandomizerMode() const;

    // Constants
    static constexpr float PROGRESS_BAR_HEIGHT = 24.0f;
    static constexpr ImVec4 GOLD_COLOR = ImVec4(1.0f, 0.85f, 0.0f, 1.0f);
    static constexpr ImVec4 GRAY_COLOR = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    static constexpr ImVec4 DARK_GRAY_COLOR = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
};