#pragma once

#include <libultraship/libultraship.h>
#include "2s2h/Enhancements/Achievements/Achievements.h"
#include "2s2h/Rando/Rando.h" // Include for IS_RANDO and randomizer types
#include <string>
#include <vector>
#include <memory>
#include <window/gui/GuiWindow.h>

// Forward declare AchievementSystem and Achievement to avoid including the full header here if possible
// If methods need the full definition, we'll include Achievements.h in the .cpp file.
class AchievementSystem;
struct Achievement;

namespace Ship {

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
class AchievementEditor : public GuiWindow {
  public:
    AchievementEditor(const std::string& consoleVariable, const std::string& name);
    ~AchievementEditor() = default;

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
    void UpdateElement() override;

  private:
    /**
     * @brief Draws the achievement list section
     */
    void DrawAchievementList();

    /**
     * @brief Draws the details for a specific achievement
     * @param achievement The achievement to display details for
     */
    void DrawDetailsPane();

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

    AchievementSystem* mAchievementSystem = nullptr;
    std::string mSelectedAchievementId = "";
    std::vector<std::shared_ptr<Achievement>> mAchievementsList;
    char mFilterText[256] = { 0 };
};

} // namespace Ship