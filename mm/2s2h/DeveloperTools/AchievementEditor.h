#pragma once

#include <libultraship/libultraship.h>
#include "2s2h/Enhancements/Achievements/Achievements.h"
#include "2s2h/Rando/Rando.h" // Include for IS_RANDO and randomizer types

/**
 * @class AchievementEditorWindow
 * @brief Developer tool for editing and managing achievements
 *
 * Provides a GUI interface for:
 * - Viewing and editing achievements
 * - Testing achievement notifications
 * - Viewing achievement statistics
 * - Mass unlocking/locking achievements
 */
class AchievementEditorWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    /**
     * @brief Initializes the editor window
     */
    void InitElement() override;

    /**
     * @brief Draws the editor window contents
     */
    void DrawElement() override;

    /**
     * @brief Updates the editor window (unused)
     */
    void UpdateElement() override{};

  private:
    /**
     * @brief Draws the achievement list section
     */
    void DrawAchievementList();

    /**
     * @brief Draws the details for a specific achievement
     * @param achievement The achievement to display details for
     */
    void DrawAchievementDetails(std::shared_ptr<Achievement> achievement);

    /**
     * @brief Draws the achievement system control section
     */
    void DrawAchievementControls();

    /**
     * @brief Draws the achievement statistics section
     */
    void DrawAchievementStats();

    /**
     * @brief Draws diagnostic information about achievements and randomizer
     */
    void DrawDiagnostics();
};