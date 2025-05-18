#include "AchievementsWindow.h"
#include "Enhancements/Achievements/Achievements.h"
#include "Enhancements/Achievements/AchievementData.h"
#include <libultraship/libultraship.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome4.h>
#include <string>
#include <vector>
#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h" // Include for game state access
#include "../../BenGui/UIWidgets.hpp"           // Include for UIWidgets
#include "2s2h/Rando/Rando.h"                   // Include for IS_RANDO

extern "C" {
#include <variables.h> // Include for gSaveContext
}

// Define static member
ImGuiTextFilter AchievementsWindow::sAchievementFilter;

AchievementsWindow::AchievementsWindow(const std::string& consoleVariable, const std::string& name)
    : Ship::GuiWindow(consoleVariable, name, ImVec2(500, 600)) {

    // Set initial visibility based on CVar value
    bool shouldBeVisible = CVarGetInteger(consoleVariable.c_str(), 0) != 0;
    if (shouldBeVisible) {
        Show();
    } else {
        Hide();
    }
    SPDLOG_DEBUG("[AchievementsWindow] Constructor: this = {}", (void*)this);
    // mAchievements member is removed, no need to log its size here or loop.
}

void AchievementsWindow::InitElement() {
    // No initialization needed
}

void AchievementsWindow::UpdateElement() {
    if (gPlayState != nullptr) {
        mIsRandomizerMode = IsRandomizerMode();
    } else {
        mIsRandomizerMode = false; // Default to false if not in a loaded game state
    }

    bool shouldBeVisible = CVarGetInteger("gOpenWindows.Achievements", 0) != 0;
    if (shouldBeVisible && !IsVisible()) {
        Show();
    } else if (!shouldBeVisible && IsVisible()) {
        Hide();
    }

    // bool achievementsEnabled = CVarGetInteger("gEnhancements.Achievements.Enabled", 1) != 0; // This line seems
    // unused here, can be removed.
}

void AchievementsWindow::DrawElement() {
    SPDLOG_TRACE("[AchievementsWindow] DrawElement: this = {}, about to draw progress bar.", (void*)this);
    bool globalAchievementsEnabled = CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1) != 0; // Use defined CVAR_NAME

    if (!globalAchievementsEnabled) {
        DrawDisabledMessage(); // Global CVar is off
        return;
    }

    if (gPlayState == nullptr) {
        DrawNotInGameMessage("Achievements can only be viewed in-game"); // Pass specific message
        return;
    }

    // Check: Prevent drawing save-specific UI during title screen or file select
    if (gSaveContext.gameMode == GAMEMODE_TITLE_SCREEN || gSaveContext.gameMode == GAMEMODE_FILE_SELECT) {
        DrawNotInGameMessage("Achievements are not available on this screen."); // Updated message
        return;
    }

    // NEW: Check if achievements are enabled for THIS save file
    // ADD LOG: Verify the value read by the UI
    SPDLOG_DEBUG("[AchievementsWindow] DrawElement: Reading "
                 "gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled as: {}",
                 gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled);
    bool currentSaveAchievementsEnabled = gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled;

    if (!currentSaveAchievementsEnabled) {
        // Global is ON, Save is Loaded, but THIS save has them OFF. Show button to enable.
        ImGui::Spacing();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 10.0f));
        ImGui::BeginChild("EnableAchievementsSection", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 6.0f), true,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NavFlattened);

        ImGui::TextWrapped("Achievements are currently OFF for this save file.");
        ImGui::TextWrapped("To start tracking progress, activate achievements for this save. This will reset any prior "
                           "(hidden) progress.");
        ImGui::Spacing();

        // Attempt to make the button more prominent
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 1.0f)); // Darker Green
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.6f, 0.25f, 1.0f));

        if (ImGui::Button("Activate Achievements for this Save File",
                          ImVec2(-1, ImGui::GetTextLineHeightWithSpacing() * 1.8f))) {
            EnableSaveStatus status = AchievementSystem::Instance().EnableAchievementsForCurrentSave();
            if (status == EnableSaveStatus::SUCCESS) {
                ImGui::OpenPopup("AchievementEnableDisclaimer");
            } else {
                SPDLOG_ERROR("[AchievementsWindow] EnableAchievementsForCurrentSave returned unexpected status: {}. "
                             "Save may be in an inconsistent state or there's a logic error.",
                             static_cast<int>(status));
            }
        }
        ImGui::PopStyleColor(3); // Pop 3 button colors
        ImGui::EndChild();
        ImGui::PopStyleVar();
        // No ImGui::Separator(); needed here as the child window provides separation.
    }

    // Disclaimer Popup for successful enabling
    ImVec2 popupSize(480, 0); // Define desired width, height auto-calculated by ImGuiWindowFlags_AlwaysAutoResize
    ImGui::SetNextWindowSizeConstraints(ImVec2(popupSize.x, 0), ImVec2(popupSize.x, FLT_MAX)); // Fix width, auto height

    if (ImGui::BeginPopupModal("AchievementEnableDisclaimer", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings)) {
        // Centering Title Text
        const char* titleText = "Achievements Successfully Enabled!";
        float titleWidth = ImGui::CalcTextSize(titleText).x;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleWidth) * 0.5f);
        ImGui::TextUnformatted(titleText);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped(
            "Progress tracking and unlocks will now consider actions from this point forward for this save file.");
        ImGui::TextWrapped("Previously completed objectives on this save will not retroactively grant achievements.");
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f)); // Orange-yellow for emphasis
        ImGui::TextWrapped("IMPORTANT: This change is active for your current play session. To make it permanent for "
                           "this save file, you MUST save your game (e.g., using an Owl Statue or the Song of Time).");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Spacing();

        // Center the OK button
        float buttonWidth = 120.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) * 0.5f);
        if (ImGui::Button("OK", ImVec2(buttonWidth, ImGui::GetTextLineHeightWithSpacing() * 1.5f))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Only draw the rest of the UI if achievements are currently active for this save.
    // This check correctly uses the current state of gSaveContext, which would have been updated
    // if the "Enable Achievements" button was pressed and the operation was successful.
    if (!gSaveContext.save.shipSaveInfo.achievements.achievementsSystemEnabled) {
        // This means they are still off for this save (e.g., button not pressed yet, or failed to enable).
        // The message and button to enable are already shown above if applicable.
        // Return here to avoid drawing the rest of the UI elements.
        return;
    }

    // Fetch achievements fresh each time DrawElement is called
    std::vector<std::shared_ptr<Achievement>> currentAchievements = AchievementSystem::Instance().GetAchievements();

    DrawHeader(currentAchievements);      // Pass the fresh list
    DrawProgressBar(currentAchievements); // Pass the fresh list
    DrawFilters();

    DrawAchievementList(currentAchievements); // Pass the fresh list
}

void AchievementsWindow::DrawHeader(const std::vector<std::shared_ptr<Achievement>>& achievements) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, GOLD_COLOR);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    ImGui::SetWindowFontScale(1.2f);
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("ACHIEVEMENTS").x) * 0.5f);
    ImGui::Text("ACHIEVEMENTS");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();
    ImGui::PopStyleColor();

    ImGui::Spacing();
}

void AchievementsWindow::DrawProgressBar(const std::vector<std::shared_ptr<Achievement>>& achievements) {
    size_t unlockedCount = 0;
    int totalGamerscore = 0;
    int unlockedGamerscore = 0;
    size_t relevantTotalCount = 0;

    SPDLOG_DEBUG("[AchievementsWindow] DrawProgressBar: achievements.size() = {}", achievements.size());
    for (size_t i = 0; i < achievements.size(); ++i) {
        if (!achievements[i]) {
            SPDLOG_DEBUG("[AchievementsWindow] DrawProgressBar: achievements[{}] is a null shared_ptr!", i);
        }
    }

    for (const auto& achievement : achievements) {
        if (!achievement) {
            SPDLOG_WARN("[AchievementsWindow] DrawProgressBar: Encountered a null shared_ptr in loop, skipping.");
            continue;
        }
        if (AchievementSystem::Instance().IsAchievementRelevantForGameMode(achievement->getId(), mIsRandomizerMode)) {
            relevantTotalCount++;
            totalGamerscore += achievement->getGamerscore();

            if (AchievementSystem::Instance().IsAchievementUnlocked(achievement->getId())) {
                unlockedCount++;
                unlockedGamerscore += achievement->getGamerscore();
            }
        }
    }

    float progress = relevantTotalCount > 0 ? (float)unlockedCount / relevantTotalCount : 0.0f;
    char progressText[128];
    sprintf(progressText, "%zu / %zu (%d%%) - %d/%d HM", unlockedCount, relevantTotalCount, (int)(progress * 100),
            unlockedGamerscore, totalGamerscore);

    float textWidth = ImGui::CalcTextSize(progressText).x;
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - textWidth) * 0.5f);
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", progressText);

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.6f, 0.0f, 1.0f));
    ImGui::ProgressBar(progress, ImVec2(-1, PROGRESS_BAR_HEIGHT), "");
    ImGui::PopStyleColor(2);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void AchievementsWindow::DrawFilters() {
    // Search bar
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    sAchievementFilter.Draw("##search");

    if (!sAchievementFilter.IsActive()) {
        ImGui::SameLine(18.0f);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Search achievements...");
    }

    ImGui::PopStyleColor(5);

    ImGui::Spacing();

    // Filter buttons
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

    const float buttonWidth = ImGui::GetContentRegionAvail().x / 3.0f - 4.0f;

    if (ImGui::Button("All", ImVec2(buttonWidth, 0))) {
        mShowLockedOnly = mShowUnlockedOnly = false;
    }

    ImGui::SameLine();
    if (ImGui::Button("Unlocked", ImVec2(buttonWidth, 0))) {
        mShowLockedOnly = false;
        mShowUnlockedOnly = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Locked", ImVec2(buttonWidth, 0))) {
        mShowLockedOnly = true;
        mShowUnlockedOnly = false;
    }

    ImGui::PopStyleColor(3);
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void AchievementsWindow::DrawAchievementList(const std::vector<std::shared_ptr<Achievement>>& achievements) {
    ImGui::BeginChild("AchievementsScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    for (const auto& achievement : achievements) {
        bool isUnlocked = AchievementSystem::Instance().IsAchievementUnlocked(achievement->getId());

        if ((mShowLockedOnly && isUnlocked) || (mShowUnlockedOnly && !isUnlocked)) {
            continue;
        }

        if (!AchievementSystem::Instance().IsAchievementRelevantForGameMode(achievement->getId(), mIsRandomizerMode)) {
            continue;
        }

        // Text search filter: Check against displayed text
        bool isSecret = achievement->isSecret();
        bool checkAgainstPlaceholders = isSecret && !isUnlocked;

        const char* nameToCheck = checkAgainstPlaceholders ? "Secret Achievement" : achievement->getName().c_str();
        const char* descToCheck = checkAgainstPlaceholders ? "Complete a hidden objective to unlock this achievement."
                                                           : achievement->getDescription().c_str();

        if (!sAchievementFilter.PassFilter(nameToCheck) && !sAchievementFilter.PassFilter(descToCheck)) {
            continue;
        }

        DrawAchievementItem(achievement);
    }

    ImGui::EndChild();
}

void AchievementsWindow::DrawAchievementItem(const std::shared_ptr<Achievement>& achievement) {
    ImGui::PushID(achievement->getName().c_str());

    bool isUnlocked = AchievementSystem::Instance().IsAchievementUnlocked(achievement->getId());
    // Log the state the UI sees
    SPDLOG_TRACE("UI Draw: Achievement '{}', ID: {}, IsUnlocked Check: {}", achievement->getName(),
                 (int)achievement->getId(), isUnlocked);

    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          isUnlocked ? ImVec4(0.25f, 0.22f, 0.15f, 1.0f) : ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(ImGui::GetStyle().WindowPadding.x, 4.0f)); // Reduce child window vertical padding

    ImGui::BeginChild(achievement->getName().c_str(), ImVec2(0, 70), true);

    DrawAchievementIcon(achievement);
    ImGui::SameLine(); // Keep icon and details on the same line
    DrawAchievementDetails(achievement);

    ImGui::EndChild();
    ImGui::PopStyleVar(); // Restore original WindowPadding
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::PopID();
}

void AchievementsWindow::DrawAchievementIcon(const std::shared_ptr<Achievement>& achievement) {
    bool isUnlocked = AchievementSystem::Instance().IsAchievementUnlocked(achievement->getId());
    bool isSecret = achievement->isSecret();

    if (isUnlocked) {
        if (!achievement->getIconPath().empty()) {
            auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
            if (gui->HasTextureByName(achievement->getIconPath())) {
                ImGui::Image(gui->GetTextureByName(achievement->getIconPath()), ImVec2(32, 32));
            } else {
                ImGui::TextColored(GOLD_COLOR, "%s", ICON_FA_TROPHY);
            }
        } else {
            ImGui::TextColored(GOLD_COLOR, "%s", ICON_FA_TROPHY);
        }
    } else {
        if (!achievement->getIconPath().empty() && !isSecret) {
            auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
            if (gui->HasTextureByName(achievement->getIconPath())) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                ImGui::Image(gui->GetTextureByName(achievement->getIconPath()), ImVec2(32, 32));
                ImGui::PopStyleVar();
            } else {
                ImGui::TextColored(DARK_GRAY_COLOR, "%s", ICON_FA_LOCK);
            }
        } else {
            ImGui::TextColored(DARK_GRAY_COLOR, "%s", ICON_FA_LOCK);
        }
    }

    ImGui::SameLine();
}

void AchievementsWindow::DrawAchievementDetails(const std::shared_ptr<Achievement>& achievement) {
    bool isUnlocked = AchievementSystem::Instance().IsAchievementUnlocked(achievement->getId());
    bool isSecret = achievement->isSecret();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 1.0f)); // Pack lines tighter
    ImGui::BeginGroup(); // Group for icon + details column

    // First line: Name [Secret] ---aligned right--> Gamerscore
    std::string displayName = achievement->getName();
    if (isSecret && isUnlocked) { // Only show [Secret] tag if unlocked
        displayName += " [Secret]";
    }
    ImGui::TextColored(isUnlocked ? GOLD_COLOR : ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s",
                       (isSecret && !isUnlocked) ? "Secret Achievement" : displayName.c_str());

    // Gamerscore aligned to the right on the same line
    if (achievement->getGamerscore() > 0) {
        char scoreText[16];
        snprintf(scoreText, sizeof(scoreText), "%d HM", achievement->getGamerscore());
        float scoreWidth = ImGui::CalcTextSize(scoreText).x;
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - scoreWidth);
        ImGui::TextColored(isUnlocked ? GOLD_COLOR : DARK_GRAY_COLOR, "%s", scoreText);
    }

    // Second line: Description (or placeholder)
    ImGui::TextColored(isUnlocked ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : GRAY_COLOR, "%s",
                       (isSecret && !isUnlocked) ? "Keep playing to discover this achievement!"
                                                 : achievement->getDescription().c_str());

    // Third line: Progress (if applicable)
    const auto& staticDataIt = AllAchievementData.find(achievement->getId());
    if (staticDataIt != AllAchievementData.end()) {
        const AchievementStaticData& staticData = staticDataIt->second;
        bool isSecretAndLocked = isSecret && !isUnlocked;           // Combine check
        if (staticData.hasProgressTracking && !isSecretAndLocked) { // Only show progress if not secret+locked
            s32 currentProgress =
                gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)achievement->getId()].currentProgress;
            ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "Progress: %d / %d", currentProgress,
                               staticData.targetProgress);
        }
    }

    ImGui::EndGroup();    // End details group
    ImGui::PopStyleVar(); // Restore original ItemSpacing
}

void AchievementsWindow::DrawDisabledMessage() {
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPos(ImVec2((windowSize.x - ImGui::CalcTextSize("Achievements are disabled").x) * 0.5f,
                               windowSize.y * 0.5f - ImGui::GetTextLineHeight() * 0.5f));
    ImGui::TextColored(GRAY_COLOR, "Achievements are disabled");
}

void AchievementsWindow::DrawNotInGameMessage(const char* message) {
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImVec2 textSize = ImGui::CalcTextSize(message);
    ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) * 0.5f, windowSize.y * 0.5f - textSize.y * 0.5f));
    ImGui::TextColored(GRAY_COLOR, "%s", message);
}

bool AchievementsWindow::IsRandomizerMode() const {
    // This function is now only called from UpdateElement when gPlayState is confirmed to be non-null.
    // Therefore, IS_RANDO (which accesses gSaveContext) can be used directly.
    return IS_RANDO; // IS_RANDO is (gSaveContext.save.shipSaveInfo.saveType == SAVETYPE_RANDO)
}
