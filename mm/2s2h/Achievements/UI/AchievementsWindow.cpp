#include "AchievementsWindow.h"

#include <algorithm>
#include <string>
#include <vector>

#include <imgui.h>
#include <IconsFontAwesome4.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <libultraship/libultraship.h>
#include <public/bridge/consolevariablebridge.h>

#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/Rando/Rando.h"
#include "../Core.h"
#include "../StaticData/Registry.h"

extern "C" {
#include <variables.h>
#include "assets/archives/icon_item_static/icon_item_static_yar.h"
#include "assets/interface/schedule_static/schedule_static.h"
#include "assets/archives/schedule_dma_static/schedule_dma_static_yar.h"
}

namespace {
// CVAR Constants
constexpr const char* CVAR_ACHIEVEMENTS_ENABLED = CVAR_PREFIX_ENHANCEMENT ".Achievements.Enabled";
constexpr const char* CVAR_FILTER_LOCKED_ONLY = CVAR_PREFIX_ENHANCEMENT ".Achievements.Filter.LockedOnly";
constexpr const char* CVAR_FILTER_UNLOCKED_ONLY = CVAR_PREFIX_ENHANCEMENT ".Achievements.Filter.UnlockedOnly";
} // namespace

// Lifecycle Methods

AchievementsWindow::~AchievementsWindow() {
    SaveSettings();
    SPDLOG_TRACE("destruct achievements window");
}

void AchievementsWindow::InitElement() {
    // Initialize member variables
    mShowLockedOnly = false;
    mShowUnlockedOnly = false;
    mIsRandomizerMode = false;
    mIsInGame = false;
    mIsValidGameMode = false;
    mStatsNeedUpdate = true;

    // Load settings and initialize resources
    LoadSettings();
    InitializeTextures();

    SPDLOG_DEBUG("AchievementsWindow initialized");
}

void AchievementsWindow::UpdateElement() {
    bool wasInGame = mIsInGame;
    mIsInGame = (gPlayState != nullptr);
    mIsValidGameMode =
        (gSaveContext.gameMode != GAMEMODE_TITLE_SCREEN && gSaveContext.gameMode != GAMEMODE_FILE_SELECT);

    if (mIsInGame) {
        mIsRandomizerMode = IsRandomizerMode();
    } else {
        mIsRandomizerMode = false;
    }

    if (wasInGame != mIsInGame) {
        InvalidateCache();
    }
}

void AchievementsWindow::DrawElement() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Dummy(ImVec2(0.0f, AchievementsUI::Layout::TIGHT_SPACING));

    BeginAchievementsPanel();

    if (!mIsInGame || !mIsValidGameMode) {
        DrawNotInGameMessage();
    } else if (!IS_ACHIEVEMENTS) {
        DrawActivationPrompt();
    } else {
        DrawInGameInterface();
    }

    EndAchievementsPanel();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

// Main UI Sections

void AchievementsWindow::DrawInGameInterface() {
    DrawHeaderPanel();
    ImGui::Dummy(ImVec2(0, AchievementsUI::Layout::TIGHT_SPACING));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, AchievementsUI::Layout::TIGHT_SPACING));
    if (ImGui::BeginChild("##AchievementsListScrollArea")) {
        DrawAchievementList();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void AchievementsWindow::DrawNotInGameMessage() {
    const char* message = "Achievements can only be viewed in-game";
    float windowWidth = ImGui::GetContentRegionAvail().x;
    float textWidth = ImGui::CalcTextSize(message).x;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::LightGray), "%s", message);
}

void AchievementsWindow::DrawActivationPrompt() {
    const ImVec4 primaryText = UIWidgets::ColorValues.at(UIWidgets::Colors::White);
    const ImVec4 secondaryText = UIWidgets::ColorValues.at(UIWidgets::Colors::LightGray);

    ImGui::TextColored(primaryText, "Achievements are currently OFF for this save file.");
    ImGui::Dummy(ImVec2(0, AchievementsUI::Layout::TIGHT_SPACING));
    ImGui::TextColored(secondaryText, "To start tracking progress, activate achievements for this save.");
    ImGui::TextColored(secondaryText, "This will reset any prior (hidden) progress.");

    ImGui::Dummy(ImVec2(0, AchievementsUI::Layout::STANDARD_SPACING));

    if (UIWidgets::Button("Activate Achievements for this Save File",
                          UIWidgets::ButtonOptions()
                              .Color(UIWidgets::Colors::Green)
                              .Size(ImVec2(-AchievementsUI::Layout::STANDARD_SPACING * 2,
                                           ImGui::GetFrameHeight() + AchievementsUI::Layout::TIGHT_SPACING)))) {
        Achievements::EnableAchievements();
        ImGui::OpenPopup("AchievementEnableDisclaimer");
        SPDLOG_INFO("Achievements enabled for current save");
    }

    DrawActivationDisclaimer();
}

// Header Section

void AchievementsWindow::DrawHeaderPanel() {
    DrawProgressSection();
    ImGui::Dummy(ImVec2(0, AchievementsUI::Layout::TIGHT_SPACING));
    DrawFilterSection();
}

void AchievementsWindow::DrawProgressSection() {
    auto stats = CalculateProgressStats();

    float progress = stats.totalCount > 0 ? static_cast<float>(stats.unlockedCount) / stats.totalCount : 0.0f;
    float frameHeight = ImGui::GetFrameHeight();
    float availableWidth = ImGui::GetContentRegionAvail().x;
    const ImVec4 progressColor = UIWidgets::ColorValues.at(UIWidgets::Colors::Orange);

    // Calculate text strings
    const float fontScale = 1.2f;
    int progressPercent = (stats.totalCount > 0)
                              ? static_cast<int>((static_cast<float>(stats.unlockedCount) / stats.totalCount) * 100)
                              : 0;

    std::string unlockedCountStr = fmt::format("{}", stats.unlockedCount);
    std::string totalCountStr = fmt::format("/{}", stats.totalCount);
    std::string percentStr = fmt::format(" ({}%)", progressPercent);
    std::string unlockedScoreStr = fmt::format("{}", stats.unlockedScore);
    std::string totalScoreStr = fmt::format("/{}", stats.totalScore);
    std::string hmStr = " HM";

    // Calculate layout
    ImGui::SetWindowFontScale(fontScale);
    ImVec2 unlockedCountSize = ImGui::CalcTextSize(unlockedCountStr.c_str());
    ImVec2 totalCountSize = ImGui::CalcTextSize(totalCountStr.c_str());
    ImVec2 percentSize = ImGui::CalcTextSize(percentStr.c_str());
    ImVec2 unlockedScoreSize = ImGui::CalcTextSize(unlockedScoreStr.c_str());
    ImVec2 totalScoreSize = ImGui::CalcTextSize(totalScoreStr.c_str());
    ImVec2 hmSize = ImGui::CalcTextSize(hmStr.c_str());
    float largeTextHeight = unlockedCountSize.y;
    ImGui::SetWindowFontScale(1.0f);

    float iconWidth = AchievementsUI::Icon::HEADER_SIZE + ImGui::GetStyle().ItemSpacing.x;
    float leftContentWidth = iconWidth + unlockedCountSize.x + totalCountSize.x + percentSize.x;
    float rightContentWidth = iconWidth + unlockedScoreSize.x + totalScoreSize.x + hmSize.x;
    float progressBarWidth =
        std::max(AchievementsUI::ProgressBar::MIN_WIDTH, availableWidth - leftContentWidth - rightContentWidth -
                                                             (AchievementsUI::Layout::STANDARD_SPACING * 2));

    // Draw progress section
    ImVec2 baseScreenPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImU32 goldColor = ImGui::ColorConvertFloat4ToU32(UIWidgets::ColorValues.at(UIWidgets::Colors::Orange));
    const ImU32 subtleColor = ImGui::ColorConvertFloat4ToU32(UIWidgets::ColorValues.at(UIWidgets::Colors::LightGray));
    const ImU32 dimColor = ImGui::ColorConvertFloat4ToU32(UIWidgets::ColorValues.at(UIWidgets::Colors::Gray));

    float iconOffsetY = (frameHeight - AchievementsUI::Icon::HEADER_SIZE) * 0.5f;
    float textOffsetY = (frameHeight - largeTextHeight) * 0.5f;

    // Left side
    ImGui::SetCursorScreenPos(ImVec2(baseScreenPos.x, baseScreenPos.y + iconOffsetY));
    DrawProgressIcon();

    ImVec2 textPos = ImVec2(baseScreenPos.x + iconWidth, baseScreenPos.y + textOffsetY);
    ImGui::SetWindowFontScale(fontScale);
    drawList->AddText(textPos, goldColor, unlockedCountStr.c_str());
    textPos.x += unlockedCountSize.x;
    drawList->AddText(textPos, subtleColor, totalCountStr.c_str());
    textPos.x += totalCountSize.x;
    drawList->AddText(textPos, dimColor, percentStr.c_str());
    ImGui::SetWindowFontScale(1.0f);

    // Center
    float progressStartX = baseScreenPos.x + leftContentWidth + AchievementsUI::Layout::STANDARD_SPACING;
    float progressBarHeight = frameHeight * AchievementsUI::ProgressBar::HEIGHT_RATIO;
    float progressBarOffsetY = (frameHeight - progressBarHeight) * 0.5f;
    ImGui::SetCursorScreenPos(ImVec2(progressStartX, baseScreenPos.y + progressBarOffsetY));
    DrawProgressBar(progress, progressBarWidth, progressBarHeight, progressColor);

    // Right side
    float rightStartX = baseScreenPos.x + availableWidth - rightContentWidth;
    ImGui::SetCursorScreenPos(ImVec2(rightStartX, baseScreenPos.y + iconOffsetY));
    DrawScoreIcon();

    textPos = ImVec2(rightStartX + iconWidth, baseScreenPos.y + textOffsetY);
    ImGui::SetWindowFontScale(fontScale);
    drawList->AddText(textPos, goldColor, unlockedScoreStr.c_str());
    textPos.x += unlockedScoreSize.x;
    drawList->AddText(textPos, subtleColor, totalScoreStr.c_str());
    textPos.x += totalScoreSize.x;
    drawList->AddText(textPos, dimColor, hmStr.c_str());
    ImGui::SetWindowFontScale(1.0f);

    ImGui::SetCursorScreenPos(ImVec2(baseScreenPos.x, baseScreenPos.y + frameHeight));
}

void AchievementsWindow::DrawFilterSection() {
    UIWidgets::PushStyleCombobox();
    mAchievementFilter.Draw("##AchievementSearch", -1.0f);
    UIWidgets::PopStyleCombobox();

    if (!mAchievementFilter.IsActive()) {
        ImGui::SameLine(18.0f); // Search offset
        ImGui::Text("Search achievements...");
    }

    ImGui::Dummy(ImVec2(0, AchievementsUI::Layout::STANDARD_SPACING));

    DrawFilterButtons();
}

// Achievement List

void AchievementsWindow::DrawAchievementList() {
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::BeginTable("##ListLayout", 2, ImGuiTableFlags_None)) {
        ImGui::PopStyleVar();

        ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Padding", ImGuiTableColumnFlags_WidthFixed, AchievementsUI::Layout::TIGHT_SPACING);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);

        ImGui::Dummy(ImVec2(0, AchievementsUI::Layout::TIGHT_SPACING));

        bool isFirstCard = true;
        for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
            AchievementId id = static_cast<AchievementId>(i);
            const Achievement* achievement = Achievements::StaticData::GetAchievement(id);

            if (!achievement || !ShouldShowAchievement(achievement)) {
                continue;
            }

            if (!isFirstCard) {
                ImGui::Dummy(ImVec2(0, AchievementsUI::Layout::STANDARD_SPACING));
            }
            isFirstCard = false;

            DrawAchievementCard(achievement);
        }

        ImGui::Dummy(ImVec2(0, AchievementsUI::Layout::TIGHT_SPACING));

        ImGui::EndTable();
    } else {
        ImGui::PopStyleVar();
    }
}

void AchievementsWindow::DrawAchievementCard(const Achievement* achievement) {
    if (!achievement) {
        SPDLOG_ERROR("Attempted to draw null achievement card");
        return;
    }

    ImGui::PushID(achievement->name);

    bool hasProgress = false;
    AchievementsUI::CardTheme theme = DetermineCardTheme(achievement, hasProgress);

    float cardWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 cardPos = ImGui::GetCursorScreenPos();
    ImVec2 cardSize = ImVec2(cardWidth, AchievementsUI::Card::HEIGHT);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImGuiID cardId = window->GetID("achievement_card");
    ImRect cardBounds(cardPos, cardPos + cardSize);

    ImGui::ItemSize(cardBounds);
    if (!ImGui::ItemAdd(cardBounds, cardId)) {
        ImGui::PopID();
        return;
    }

    bool hovered, held;
    ImGui::ButtonBehavior(cardBounds, cardId, &hovered, &held);

    DrawCardBackground(cardPos, cardSize, theme, hovered);

    float iconVerticalCenter = cardPos.y + (AchievementsUI::Card::HEIGHT - AchievementsUI::Card::ICON_SIZE) * 0.5f;
    ImGui::SetCursorScreenPos(ImVec2(cardPos.x + AchievementsUI::Card::PADDING, iconVerticalCenter));

    DrawCardIcon(achievement, theme, hasProgress);

    ImGui::SameLine(0, AchievementsUI::Card::PADDING);
    ImGui::BeginGroup();
    DrawCardContent(achievement, theme);
    ImGui::EndGroup();

    DrawCardScore(achievement, theme, cardPos, cardWidth);

    ImGui::SetCursorScreenPos(ImVec2(cardPos.x, cardPos.y + cardSize.y));

    ImGui::PopID();
}

// Card Components

void AchievementsWindow::DrawCardIcon(const Achievement* achievement, AchievementsUI::CardTheme theme,
                                      bool hasProgress) {
    bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);
    bool isSecret = achievement->secret;

    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (!gui) {
        SPDLOG_ERROR("Failed to get GUI context for achievement card icon");
        return;
    }

    if (isUnlocked && achievement->iconPath && strlen(achievement->iconPath) > 0) {
        if (gui->HasTextureByName(achievement->iconPath)) {
            ImGui::Image(gui->GetTextureByName(achievement->iconPath),
                         ImVec2(AchievementsUI::Card::ICON_SIZE, AchievementsUI::Card::ICON_SIZE));
        } else {
            DrawFontIcon(ICON_FA_TROPHY, UIWidgets::ColorValues.at(UIWidgets::Colors::Orange), theme);
        }
    } else if (!isSecret && achievement->iconPath && strlen(achievement->iconPath) > 0) {
        if (gui->HasTextureByName(achievement->iconPath)) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, hasProgress ? 0.45f : 0.4f);
            ImGui::Image(gui->GetTextureByName(achievement->iconPath),
                         ImVec2(AchievementsUI::Card::ICON_SIZE, AchievementsUI::Card::ICON_SIZE));
            ImGui::PopStyleVar();
        } else {
            ImVec4 lockColor = GetTextColor(theme, false);
            DrawFontIcon(ICON_FA_LOCK, lockColor, theme);
        }
    } else {
        ImVec4 lockColor = GetTextColor(theme, false);
        DrawFontIcon(ICON_FA_LOCK, lockColor, theme);
    }
}

void AchievementsWindow::DrawCardContent(const Achievement* achievement, AchievementsUI::CardTheme theme) {
    bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);
    bool isSecret = achievement->secret;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 2.0f));

    std::string displayName = GetDisplayName(achievement, isUnlocked);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    ImGui::SetWindowFontScale(1.1f);
    ImGui::TextColored(GetTextColor(theme, true), "%s", displayName.c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    std::string displayDescription = GetDisplayDescription(achievement, isUnlocked);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
    ImGui::TextColored(GetTextColor(theme, false), "%s", displayDescription.c_str());
    ImGui::PopTextWrapPos();

    bool isSecretAndLocked = isSecret && !isUnlocked;
    if (!isSecretAndLocked && achievement->requiredEvents.size() > 1) {
        ImGui::Spacing();
        DrawProgressIcons(achievement);
    }

    ImGui::PopStyleVar();
}

void AchievementsWindow::DrawCardScore(const Achievement* achievement, AchievementsUI::CardTheme theme,
                                       const ImVec2& cardPos, float cardWidth) {
    if (achievement->gamerscore <= 0)
        return;

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    std::string scoreText = fmt::format("{} HM", achievement->gamerscore);
    float scoreTextWidth = ImGui::CalcTextSize(scoreText.c_str()).x;

    float scoreX = cardPos.x + cardWidth - AchievementsUI::Card::PADDING - scoreTextWidth;
    float scoreY = cardPos.y + AchievementsUI::Card::PADDING;

    ImGui::SetCursorScreenPos(ImVec2(scoreX, scoreY));
    ImGui::TextColored(GetTextColor(theme, true), "%s", scoreText.c_str());
    ImGui::PopFont();
}

void AchievementsWindow::DrawProgressIcons(const Achievement* achievement) {
    if (!mTextures.bombersNotebook) {
        SPDLOG_DEBUG("Bombers notebook texture not available for progress icons");
        return;
    }

    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (!gui || !gui->HasTextureByName(mTextures.bombersNotebook)) {
        SPDLOG_DEBUG("GUI context or bombers notebook texture not available");
        return;
    }

    std::vector<std::pair<AchievementEvent, bool>> sortedEvents;
    for (AchievementEvent eventId : achievement->requiredEvents) {
        bool isEventTriggered = IS_ACH_TRIGGERED(eventId);
        sortedEvents.emplace_back(eventId, isEventTriggered);
    }

    std::sort(sortedEvents.begin(), sortedEvents.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    ImGui::BeginGroup();

    for (size_t i = 0; i < sortedEvents.size(); i++) {
        bool isEventTriggered = sortedEvents[i].second;

        if (i > 0) {
            ImGui::SameLine(0, 3.0f); // Icon spacing
        }

        if (!isEventTriggered) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f);
        }

        ImGui::Image(gui->GetTextureByName(mTextures.bombersNotebook),
                     ImVec2(AchievementsUI::Icon::PROGRESS_SIZE, AchievementsUI::Icon::PROGRESS_SIZE));

        if (!isEventTriggered) {
            ImGui::PopStyleVar();
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            const auto* event = Achievements::StaticData::GetEvent(sortedEvents[i].first);
            const char* eventName = event ? event->name : "Unknown Event";
            ImGui::TextColored(
                UIWidgets::ColorValues.at(isEventTriggered ? UIWidgets::Colors::Green : UIWidgets::Colors::Gray),
                "%s: %s", eventName, isEventTriggered ? "Complete" : "Incomplete");
            ImGui::EndTooltip();
        }
    }

    ImGui::EndGroup();
}

// Helper Functions

void AchievementsWindow::DrawFontIcon(const char* icon, const ImVec4& color, AchievementsUI::CardTheme theme) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 iconPos = ImGui::GetCursorScreenPos();
    ImVec2 iconSize = ImVec2(AchievementsUI::Card::ICON_SIZE, AchievementsUI::Card::ICON_SIZE);

    ImVec4 bgColor = GetCardColors(theme, true);
    bgColor.w = 0.2f;

    float rounding = 6.0f;
    drawList->AddRectFilled(iconPos, ImVec2(iconPos.x + iconSize.x, iconPos.y + iconSize.y),
                            ImGui::ColorConvertFloat4ToU32(bgColor), rounding);

    ImVec4 borderColor = GetCardColors(theme, false);
    borderColor.w = 0.6f;
    drawList->AddRect(iconPos, ImVec2(iconPos.x + iconSize.x, iconPos.y + iconSize.y),
                      ImGui::ColorConvertFloat4ToU32(borderColor), rounding, 0, 1.0f);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    ImGui::SetWindowFontScale(AchievementsUI::Icon::FONT_SCALE);

    ImVec2 textSize = ImGui::CalcTextSize(icon);
    ImVec2 textPos = ImVec2(iconPos.x + (iconSize.x - textSize.x) * 0.5f - 2.5f,
                            iconPos.y + (iconSize.y - textSize.y) * 0.5f - 3.0f);

    drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize() * AchievementsUI::Icon::FONT_SCALE, textPos,
                      ImGui::ColorConvertFloat4ToU32(color), icon);

    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopFont();

    ImGui::Dummy(iconSize);
}

void AchievementsWindow::DrawFilterButtons() {
    bool isAllActive = !mShowLockedOnly && !mShowUnlockedOnly;
    bool isUnlockedActive = mShowUnlockedOnly;
    bool isLockedActive = mShowLockedOnly;

    float totalWidth = ImGui::GetContentRegionAvail().x;
    float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
    float buttonWidth = (totalWidth - (buttonSpacing * 2)) / 3.0f;

    if (UIWidgets::Button("All", UIWidgets::ButtonOptions()
                                     .Size(ImVec2(buttonWidth, 0))
                                     .Color(isAllActive ? UIWidgets::Colors::Orange : UIWidgets::Colors::Gray))) {
        mShowLockedOnly = mShowUnlockedOnly = false;
    }

    ImGui::SameLine();

    if (UIWidgets::Button("Unlocked",
                          UIWidgets::ButtonOptions()
                              .Size(ImVec2(buttonWidth, 0))
                              .Color(isUnlockedActive ? UIWidgets::Colors::Green : UIWidgets::Colors::Gray))) {
        mShowLockedOnly = false;
        mShowUnlockedOnly = true;
    }

    ImGui::SameLine();

    if (UIWidgets::Button("Locked",
                          UIWidgets::ButtonOptions()
                              .Size(ImVec2(buttonWidth, 0))
                              .Color(isLockedActive ? UIWidgets::Colors::LightBlue : UIWidgets::Colors::Gray))) {
        mShowLockedOnly = true;
        mShowUnlockedOnly = false;
    }
}

void AchievementsWindow::DrawActivationDisclaimer() {
    ImVec2 popupSize(480, 0);
    ImGui::SetNextWindowSizeConstraints(ImVec2(popupSize.x, 0), ImVec2(popupSize.x, FLT_MAX));

    if (ImGui::BeginPopupModal("AchievementEnableDisclaimer", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoSavedSettings)) {

        const char* titleText = "Achievements Successfully Enabled!";
        float titleWidth = ImGui::CalcTextSize(titleText).x;
        float windowWidth = ImGui::GetWindowWidth();
        ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
        ImGui::TextUnformatted(titleText);

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped(
            "Progress tracking and unlocks will now consider actions from this point forward for this save file.");
        ImGui::TextWrapped("Previously completed objectives on this save will not retroactively grant achievements.");
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, UIWidgets::ColorValues.at(UIWidgets::Colors::Orange));
        ImGui::TextWrapped("IMPORTANT: This change is active for your current play session. To make it permanent for "
                           "this save file, you MUST save your game (e.g., using an Owl Statue or the Song of Time).");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Spacing();

        float buttonWidth = 120.0f;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

        if (UIWidgets::Button("OK", UIWidgets::ButtonOptions().Size(
                                        ImVec2(buttonWidth, ImGui::GetTextLineHeightWithSpacing() * 1.5f)))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// Utility Methods

AchievementsWindow::ProgressStats AchievementsWindow::CalculateProgressStats() const {
    if (!mStatsNeedUpdate) {
        return mCachedStats;
    }

    ProgressStats stats = { 0, 0, 0, 0 };

    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        AchievementId id = static_cast<AchievementId>(i);
        const Achievement* achievement = Achievements::StaticData::GetAchievement(id);

        if (!achievement) {
            SPDLOG_DEBUG("Skipping null achievement at index {}", i);
            continue;
        }

        if ((achievement->category == AchievementCategory::RANDO && !mIsRandomizerMode) ||
            (achievement->category == AchievementCategory::VANILLA && mIsRandomizerMode)) {
            continue;
        }

        stats.totalCount++;
        stats.totalScore += achievement->gamerscore;

        if (IS_ACH_UNLOCKED(id)) {
            stats.unlockedCount++;
            stats.unlockedScore += achievement->gamerscore;
        }
    }

    mCachedStats = stats;
    mStatsNeedUpdate = false;

    return stats;
}

AchievementsUI::CardTheme AchievementsWindow::DetermineCardTheme(const Achievement* achievement,
                                                                 bool& hasProgress) const {
    hasProgress = false;
    bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);
    bool isSecret = achievement->secret;

    if (isUnlocked) {
        return AchievementsUI::CardTheme::UNLOCKED;
    }

    if (!isSecret && achievement->requiredEvents.size() > 1) {
        for (AchievementEvent eventId : achievement->requiredEvents) {
            if (IS_ACH_TRIGGERED(eventId)) {
                hasProgress = true;
                break;
            }
        }
    }

    return hasProgress ? AchievementsUI::CardTheme::PROGRESS : AchievementsUI::CardTheme::LOCKED;
}

bool AchievementsWindow::ShouldShowAchievement(const Achievement* achievement) const {
    if (!achievement)
        return false;

    bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);

    if ((mShowLockedOnly && isUnlocked) || (mShowUnlockedOnly && !isUnlocked)) {
        return false;
    }

    if ((achievement->category == AchievementCategory::RANDO && !mIsRandomizerMode) ||
        (achievement->category == AchievementCategory::VANILLA && mIsRandomizerMode)) {
        return false;
    }

    std::string displayName = GetDisplayName(achievement, isUnlocked);
    std::string displayDescription = GetDisplayDescription(achievement, isUnlocked);

    if (!mAchievementFilter.PassFilter(displayName.c_str()) &&
        !mAchievementFilter.PassFilter(displayDescription.c_str())) {
        return false;
    }

    return true;
}

std::string AchievementsWindow::GetDisplayName(const Achievement* achievement, bool isUnlocked) const {
    if (!achievement)
        return "";

    if (achievement->secret && !isUnlocked) {
        return "Secret Achievement";
    }

    std::string displayName = achievement->name;
    if (achievement->secret && isUnlocked) {
        displayName += " [Secret]";
    }

    return displayName;
}

std::string AchievementsWindow::GetDisplayDescription(const Achievement* achievement, bool isUnlocked) const {
    if (!achievement)
        return "";

    if (achievement->secret && !isUnlocked) {
        return "Keep playing to discover this achievement!";
    }

    return achievement->description;
}

bool AchievementsWindow::IsRandomizerMode() const {
    return IS_RANDO;
}

// Resource Management

void AchievementsWindow::InitializeTextures() {
    if (mTextures.initialized)
        return;

    try {
        mTextures.bombersNotebook = (const char*)gItemIconBombersNotebookTex;
        mTextures.heartPiece = (const char*)gItemIconHeartPiece1Tex;
        mTextures.ribbon = (const char*)gBombersNotebookEntryIconRibbonTex;
        mTextures.wallet = (const char*)gItemIconGiantsWalletTex;
        mTextures.initialized = true;

        SPDLOG_DEBUG("Achievement window textures initialized");
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Failed to initialize achievement window textures: {}", e.what());
    }
}

const char* AchievementsWindow::GetBestIconTexture() const {
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (!gui) {
        SPDLOG_ERROR("Failed to get GUI context for icon texture");
        return nullptr;
    }

    if (mTextures.ribbon && gui->HasTextureByName(mTextures.ribbon)) {
        return mTextures.ribbon;
    }
    if (mTextures.heartPiece && gui->HasTextureByName(mTextures.heartPiece)) {
        return mTextures.heartPiece;
    }
    if (mTextures.wallet && gui->HasTextureByName(mTextures.wallet)) {
        return mTextures.wallet;
    }
    if (mTextures.bombersNotebook && gui->HasTextureByName(mTextures.bombersNotebook)) {
        return mTextures.bombersNotebook;
    }

    return nullptr;
}

void AchievementsWindow::InvalidateCache() {
    mStatsNeedUpdate = true;
}

// Layout Helpers

ImVec4 AchievementsWindow::GetCardColors(AchievementsUI::CardTheme theme, bool isBackground) const {
    switch (theme) {
        case AchievementsUI::CardTheme::UNLOCKED:
            return isBackground ? ImVec4(1.0f, 0.7f, 0.0f, 0.12f)
                                : UIWidgets::ColorValues.at(UIWidgets::Colors::Orange);
        case AchievementsUI::CardTheme::PROGRESS:
            return isBackground ? ImVec4(0.0f, 0.2f, 0.5f, 0.15f)
                                : UIWidgets::ColorValues.at(UIWidgets::Colors::LightBlue);
        case AchievementsUI::CardTheme::LOCKED:
        default:
            return isBackground ? ImVec4(0.1f, 0.1f, 0.1f, 0.6f) : UIWidgets::ColorValues.at(UIWidgets::Colors::Gray);
    }
}

ImVec4 AchievementsWindow::GetTextColor(AchievementsUI::CardTheme theme, bool isPrimary) const {
    switch (theme) {
        case AchievementsUI::CardTheme::UNLOCKED:
            return isPrimary ? UIWidgets::ColorValues.at(UIWidgets::Colors::Orange)
                             : UIWidgets::ColorValues.at(UIWidgets::Colors::White);
        case AchievementsUI::CardTheme::PROGRESS:
            return isPrimary ? UIWidgets::ColorValues.at(UIWidgets::Colors::LightBlue)
                             : ImVec4(0.4f, 0.55f, 0.75f, 1.0f);
        case AchievementsUI::CardTheme::LOCKED:
        default:
            return isPrimary ? UIWidgets::ColorValues.at(UIWidgets::Colors::LightGray)
                             : UIWidgets::ColorValues.at(UIWidgets::Colors::Gray);
    }
}

// Panel Helpers

void AchievementsWindow::BeginAchievementsPanel() {
    const float borderRounding = 6.0f;
    const float borderThickness = 1.5f;
    const float titlePadding = 2.0f;
    const float titleBarHeight = ImGui::GetTextLineHeight() + AchievementsUI::Layout::TIGHT_SPACING;

    ImVec2 panelStart = ImGui::GetCursorScreenPos();
    ImVec2 panelSize = ImGui::GetContentRegionAvail();
    ImVec2 panelEnd = ImVec2(panelStart.x + panelSize.x, panelStart.y + panelSize.y);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImU32 panelBgColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.08f, 0.08f, 0.09f, 0.95f));
    const ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.4f, 0.4f, 0.6f));

    drawList->AddRectFilled(panelStart, panelEnd, panelBgColor, borderRounding);
    drawList->AddRect(panelStart, panelEnd, borderColor, borderRounding, 0, borderThickness);

    const char* title = "Achievement Tracker";
    ImVec2 titleSize = ImGui::CalcTextSize(title);
    ImVec2 titlePos = ImVec2(panelStart.x + AchievementsUI::Layout::PANEL_PADDING * titlePadding,
                             panelStart.y + (borderThickness - titleSize.y) * 0.5f);

    drawList->AddRectFilled(
        ImVec2(titlePos.x - AchievementsUI::Layout::PANEL_PADDING, panelStart.y),
        ImVec2(titlePos.x + titleSize.x + AchievementsUI::Layout::PANEL_PADDING, panelStart.y + borderThickness),
        panelBgColor);

    drawList->AddText(titlePos, ImGui::GetColorU32(ImGuiCol_Text), title);

    ImVec2 contentPos = ImVec2(panelStart.x + AchievementsUI::Layout::PANEL_PADDING, panelStart.y + titleBarHeight);
    ImVec2 contentSize = ImVec2(panelSize.x - (AchievementsUI::Layout::PANEL_PADDING * 2),
                                panelSize.y - titleBarHeight - AchievementsUI::Layout::PANEL_PADDING);

    ImGui::SetCursorScreenPos(contentPos);

    ImGui::BeginChild("##ContentArea", contentSize, ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground);
}

void AchievementsWindow::EndAchievementsPanel() {
    ImGui::EndChild();
}

// Misc. Helper Methods

void AchievementsWindow::DrawProgressIcon() {
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (mTextures.bombersNotebook && gui && gui->HasTextureByName(mTextures.bombersNotebook)) {
        ImGui::Image(gui->GetTextureByName(mTextures.bombersNotebook),
                     ImVec2(AchievementsUI::Icon::HEADER_SIZE, AchievementsUI::Icon::HEADER_SIZE));
    }
}

void AchievementsWindow::DrawScoreIcon() {
    const char* iconTexture = GetBestIconTexture();
    if (!iconTexture)
        return;

    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (gui) {
        ImGui::Image(gui->GetTextureByName(iconTexture),
                     ImVec2(AchievementsUI::Icon::HEADER_SIZE, AchievementsUI::Icon::HEADER_SIZE));
    }
}

void AchievementsWindow::DrawProgressBar(float progress, float width, float height, const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, AchievementsUI::ProgressBar::ROUNDING);
    ImGui::ProgressBar(progress, ImVec2(width, height), "");
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void AchievementsWindow::DrawCardBackground(const ImVec2& pos, const ImVec2& size, AchievementsUI::CardTheme theme,
                                            bool hovered) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec4 bgColor = GetCardColors(theme, true);
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::ColorConvertFloat4ToU32(bgColor),
                            AchievementsUI::Card::BORDER_ROUNDING);

    ImVec4 borderColor = GetCardColors(theme, false);
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::ColorConvertFloat4ToU32(borderColor),
                      AchievementsUI::Card::BORDER_ROUNDING, 0, 1.5f);

    if (hovered) {
        ImVec4 hoverColor = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::ColorConvertFloat4ToU32(hoverColor),
                                AchievementsUI::Card::BORDER_ROUNDING);
    }
}

// Settings Management

void AchievementsWindow::LoadSettings() {
    mShowLockedOnly = CVarGetInteger(CVAR_FILTER_LOCKED_ONLY, 0);
    mShowUnlockedOnly = CVarGetInteger(CVAR_FILTER_UNLOCKED_ONLY, 0);
}

void AchievementsWindow::SaveSettings() {
    CVarSetInteger(CVAR_FILTER_LOCKED_ONLY, mShowLockedOnly);
    CVarSetInteger(CVAR_FILTER_UNLOCKED_ONLY, mShowUnlockedOnly);
    CVarSave();
}
