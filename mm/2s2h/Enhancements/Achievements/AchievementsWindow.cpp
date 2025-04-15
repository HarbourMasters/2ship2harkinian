#include "AchievementsWindow.h"
#include "Enhancements/Achievements/Achievements.h"
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

    mAchievements = AchievementSystem::Instance().GetAchievements();

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
    // Get the achievements from the achievement system
    mAchievements = AchievementSystem::Instance().GetAchievements();

    // Determine if we're in randomizer mode
    mIsRandomizerMode = IsRandomizerMode();

    // Check if the visibility CVar changed
    bool shouldBeVisible = CVarGetInteger("gOpenWindows.Achievements", 0) != 0;
    if (shouldBeVisible && !IsVisible()) {
        Show();
    } else if (!shouldBeVisible && IsVisible()) {
        Hide();
    }

    // Only update achievements if the system is enabled
    bool achievementsEnabled = CVarGetInteger("gEnhancements.Achievements.Enabled", 1) != 0;
    if (achievementsEnabled) {
        mAchievements = AchievementSystem::Instance().GetAchievements();
    }
}

void AchievementsWindow::DrawElement() {
    // Check if achievements are enabled
    bool achievementsEnabled = CVarGetInteger("gEnhancements.Achievements.Enabled", 1) != 0;
    if (!achievementsEnabled) {
        DrawDisabledMessage();
        return;
    }

    // Check if we're in actual gameplay
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
    ImGui::Spacing(); // Add spacing above the title
    ImGui::PushStyleColor(ImGuiCol_Text, GOLD_COLOR);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    ImGui::SetWindowFontScale(1.2f); // Make font size slightly bigger
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("ACHIEVEMENTS").x) * 0.5f);
    ImGui::Text("ACHIEVEMENTS");
    ImGui::SetWindowFontScale(1.0f); // Reset font scale
    ImGui::PopFont();
    ImGui::PopStyleColor();

    ImGui::Spacing();
}

void AchievementsWindow::DrawProgressBar() {
    size_t totalAchievements = mAchievements.size();
    size_t unlockedCount = 0;
    int totalGamerscore = 0;
    int unlockedGamerscore = 0;

    unlockedCount = AchievementSystem::Instance().GetUnlockedAchievementsCount();

    for (const auto& achievement : mAchievements) {
        totalGamerscore += achievement->gamerscore;
        if (achievement->state == AchievementState::UNLOCKED) {
            unlockedGamerscore += achievement->gamerscore;
        }
    }

    float progress = totalAchievements > 0 ? (float)unlockedCount / totalAchievements : 0.0f;
    char progressText[128];
    sprintf(progressText, "%zu / %zu (%d%%) - %d/%d HM", unlockedCount, totalAchievements, (int)(progress * 100),
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
        // Apply state filters
        if ((mShowLockedOnly && achievement->state == AchievementState::UNLOCKED) ||
            (mShowUnlockedOnly && achievement->state == AchievementState::LOCKED)) {
            continue;
        }

        // Automatically filter achievements based on game mode
        if (!AchievementSystem::Instance().IsAchievementRelevantForGameMode(achievement->id, mIsRandomizerMode)) {
            continue;
        }

        // Don't display secret achievements in search results unless they're unlocked
        if (achievement->isSecret && achievement->state == AchievementState::LOCKED && sAchievementFilter.IsActive()) {
            continue;
        }

        // Apply search filter
        if (!sAchievementFilter.PassFilter(achievement->name.c_str()) &&
            !sAchievementFilter.PassFilter(achievement->description.c_str())) {
            continue;
        }

        DrawAchievementItem(achievement);
    }

    ImGui::EndChild();
}

void AchievementsWindow::DrawAchievementItem(const std::shared_ptr<Achievement>& achievement) {
    ImGui::PushID(achievement->id.c_str());

    bool isUnlocked = (achievement->state == AchievementState::UNLOCKED);
    ImGui::PushStyleColor(ImGuiCol_ChildBg,
                          isUnlocked ? ImVec4(0.25f, 0.22f, 0.15f, 1.0f) : ImVec4(0.18f, 0.18f, 0.18f, 1.0f));

    ImGui::BeginChild(achievement->id.c_str(), ImVec2(0, 70), true);

    DrawAchievementIcon(achievement);
    DrawAchievementDetails(achievement);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::PopID();
}

void AchievementsWindow::DrawAchievementIcon(const std::shared_ptr<Achievement>& achievement) {
    bool isUnlocked = (achievement->state == AchievementState::UNLOCKED);
    bool isSecret = achievement->isSecret;

    if (isUnlocked) {
        if (!achievement->iconPath.empty()) {
            auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
            if (gui->HasTextureByName(achievement->iconPath)) {
                ImGui::Image(gui->GetTextureByName(achievement->iconPath), ImVec2(32, 32));
            } else {
                ImGui::TextColored(GOLD_COLOR, "%s", ICON_FA_TROPHY);
            }
        } else {
            ImGui::TextColored(GOLD_COLOR, "%s", ICON_FA_TROPHY);
        }
    } else {
        if (!achievement->iconPath.empty() && !isSecret) {
            auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
            if (gui->HasTextureByName(achievement->iconPath)) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                ImGui::Image(gui->GetTextureByName(achievement->iconPath), ImVec2(32, 32));
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
    bool isUnlocked = (achievement->state == AchievementState::UNLOCKED);
    bool isSecret = achievement->isSecret;

    if (isUnlocked || !isSecret) {
        ImGui::BeginGroup();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::TextColored(isUnlocked ? GOLD_COLOR : ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", achievement->name.c_str());
        ImGui::PopFont();

        ImGui::TextColored(isUnlocked ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : GRAY_COLOR, "%s",
                           achievement->description.c_str());
        ImGui::EndGroup();
    } else {
        ImGui::BeginGroup();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        ImGui::TextColored(GRAY_COLOR, "Secret Achievement");
        ImGui::PopFont();

        ImGui::TextColored(DARK_GRAY_COLOR, "Complete a hidden objective to unlock this achievement.");
        ImGui::EndGroup();
    }

    if (achievement->gamerscore > 0) {
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 40.0f);
        ImGui::TextColored(isUnlocked ? GOLD_COLOR : DARK_GRAY_COLOR, "%d HM", achievement->gamerscore);
    }
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
    // Check if the game is in randomizer mode using the IS_RANDO macro
    return IS_RANDO;
}