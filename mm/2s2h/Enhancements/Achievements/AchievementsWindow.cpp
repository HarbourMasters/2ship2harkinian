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
    : Ship::GuiWindow(consoleVariable, name, ImVec2(500, 600)),
      mAchievements(AchievementSystem::Instance().GetAchievements()) {

    // Set initial visibility based on CVar value
    bool shouldBeVisible = CVarGetInteger(consoleVariable.c_str(), 0) != 0;
    if (shouldBeVisible) {
        Show();
    } else {
        Hide();
    }
}

void AchievementsWindow::InitElement() {
    // No initialization needed
}

void AchievementsWindow::UpdateElement() {
    mIsRandomizerMode = IsRandomizerMode();

    bool shouldBeVisible = CVarGetInteger("gOpenWindows.Achievements", 0) != 0;
    if (shouldBeVisible && !IsVisible()) {
        Show();
    } else if (!shouldBeVisible && IsVisible()) {
        Hide();
    }

    bool achievementsEnabled = CVarGetInteger("gEnhancements.Achievements.Enabled", 1) != 0;
}

void AchievementsWindow::DrawElement() {
    bool achievementsEnabled = CVarGetInteger("gEnhancements.Achievements.Enabled", 1) != 0;
    if (!achievementsEnabled) {
        DrawDisabledMessage();
        return;
    }

    extern PlayState* gPlayState;
    if (gPlayState == nullptr) {
        DrawNotInGameMessage();
        return;
    }

    DrawHeader();
    DrawProgressBar();
    DrawFilters();

    DrawAchievementList();
}

void AchievementsWindow::DrawHeader() {
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

void AchievementsWindow::DrawProgressBar() {
    size_t unlockedCount = 0;
    int totalGamerscore = 0;
    int unlockedGamerscore = 0;
    size_t relevantTotalCount = 0;

    for (const auto& achievement : mAchievements) {
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

void AchievementsWindow::DrawAchievementList() {
    ImGui::BeginChild("AchievementsScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    for (const auto& achievement : mAchievements) {
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
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ImGui::GetStyle().WindowPadding.x, 4.0f)); // Reduce child window vertical padding

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
                       (isSecret && !isUnlocked) ? "Keep playing to discover this achievement!" : achievement->getDescription().c_str());

    // Third line: Progress (if applicable)
    const auto& staticDataIt = AllAchievementData.find(achievement->getId());
    if (staticDataIt != AllAchievementData.end()) {
        const AchievementStaticData& staticData = staticDataIt->second;
        bool isSecretAndLocked = isSecret && !isUnlocked; // Combine check
        if (staticData.hasProgressTracking && !isSecretAndLocked) { // Only show progress if not secret+locked
            s32 currentProgress = gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)achievement->getId()].currentProgress;
            ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "Progress: %d / %d", 
                             currentProgress, staticData.targetProgress);
        }
    }

    ImGui::EndGroup(); // End details group
    ImGui::PopStyleVar(); // Restore original ItemSpacing
}

void AchievementsWindow::DrawDisabledMessage() {
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPos(ImVec2((windowSize.x - ImGui::CalcTextSize("Achievements are disabled").x) * 0.5f,
                               windowSize.y * 0.5f - ImGui::GetTextLineHeight() * 0.5f));
    ImGui::TextColored(GRAY_COLOR, "Achievements are disabled");
}

void AchievementsWindow::DrawNotInGameMessage() {
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPos(ImVec2((windowSize.x - ImGui::CalcTextSize("Achievements can only be viewed in-game").x) * 0.5f,
                               windowSize.y * 0.5f - ImGui::GetTextLineHeight() * 0.5f));
    ImGui::TextColored(GRAY_COLOR, "Achievements can only be viewed in-game");
}

bool AchievementsWindow::IsRandomizerMode() const {
    return IS_RANDO;
}