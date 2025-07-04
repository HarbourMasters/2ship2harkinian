#include "AchievementEditor.h"

#include <algorithm>
#include <string>
#include <vector>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <IconsFontAwesome4.h>
#include <libultraship/libultraship.h>
#include <public/bridge/consolevariablebridge.h>

#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/Achievements/Core.h"
#include "2s2h/Achievements/StaticData/Registry.h"
#include "2s2h/Achievements/UI/AchievementsWindow.h"

extern "C" {
#include "variables.h"
}

namespace {
constexpr const char* CVAR_CATEGORY_FILTER = CVAR_PREFIX_DEVELOPER_TOOLS ".AchievementEditor.CategoryFilter";
constexpr const char* CVAR_SHOW_UNLOCKED = CVAR_PREFIX_DEVELOPER_TOOLS ".AchievementEditor.ShowUnlocked";
constexpr const char* CVAR_SHOW_LOCKED = CVAR_PREFIX_DEVELOPER_TOOLS ".AchievementEditor.ShowLocked";
constexpr const char* CVAR_SHOW_SECRET = CVAR_PREFIX_DEVELOPER_TOOLS ".AchievementEditor.ShowSecret";
constexpr const char* CVAR_ACTIVE_TAB = CVAR_PREFIX_DEVELOPER_TOOLS ".AchievementEditor.ActiveTab";
} // namespace

namespace BenGui {
extern std::shared_ptr<Achievements::UI::AchievementsWindow> mAchievementsWindow;
}

namespace Achievements {

namespace DeveloperTools {

// Static Data

static const char* TAB_NAMES[] = { "Browser", "Events", "Diagnostics", "Bulk Operations" };

static const char* CATEGORY_NAMES[] = { "All Categories", "General", "Vanilla", "Randomizer" };

// Lifecycle Methods

void AchievementEditor::InitElement() {
    mCategoryFilter = static_cast<AchievementCategory>(CVarGetInteger(CVAR_CATEGORY_FILTER, -1));
    mShowUnlockedOnly = CVarGetInteger(CVAR_SHOW_UNLOCKED, 0);
    mShowLockedOnly = CVarGetInteger(CVAR_SHOW_LOCKED, 0);
    mShowSecretAchievements = CVarGetInteger(CVAR_SHOW_SECRET, 1);
    mActiveTab = static_cast<Tab>(CVarGetInteger(CVAR_ACTIVE_TAB, 0));

    SPDLOG_DEBUG("AchievementEditor initialized");
}

void AchievementEditor::UpdateElement() {
    if (mValidationDirty) {
        RefreshValidation();
        mValidationDirty = false;
    }
}

void AchievementEditor::DrawElement() {
    if (!IS_ACHIEVEMENTS) {
        DrawSystemStatus();
        return;
    }

    if (ImGui::BeginTabBar("AchievementEditorTabs", ImGuiTabBarFlags_None)) {
        for (int i = 0; i < 4; i++) {
            if (ImGui::BeginTabItem(TAB_NAMES[i])) {
                if (mActiveTab != static_cast<Tab>(i)) {
                    mActiveTab = static_cast<Tab>(i);
                    CVarSetInteger(CVAR_ACTIVE_TAB, i);
                    CVarSave();
                }

                ImGui::BeginChild("TabContent", ImVec2(0, 0), false);
                switch (mActiveTab) {
                    case Tab::BROWSER:
                        DrawAchievementBrowser();
                        break;
                    case Tab::EVENTS:
                        DrawEventTesting();
                        break;
                    case Tab::DIAGNOSTICS:
                        DrawSystemDiagnostics();
                        break;
                    case Tab::BULK_OPS:
                        DrawBulkOperations();
                        break;
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void AchievementEditor::DrawSystemStatus() {
    ImGui::SeparatorText("Achievement System Status");

    ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Orange),
                       ICON_FA_EXCLAMATION_TRIANGLE " Achievement System Inactive");
    ImGui::Spacing();

    ImGui::TextWrapped("The achievement system is not active for this save file. "
                       "Achievements must be enabled when creating a save to use this editor.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("To enable achievements:");
    ImGui::BulletText("Create a new save file");
    ImGui::BulletText("Check the 'Enable Achievements' option");
    ImGui::BulletText("Or use the main Achievements window to enable for existing saves");

    ImGui::Spacing();

    if (UIWidgets::Button(
            "Open Achievements Window",
            UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline).Color(UIWidgets::Colors::LightBlue))) {
        auto achievementsWindow = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Achievements");
        if (achievementsWindow) {
            achievementsWindow->Show();
        }
    }
}

void AchievementEditor::DrawAchievementBrowser() {
    if (ImGui::BeginTable("BrowserLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Browser", ImGuiTableColumnFlags_WidthStretch, 0.6f);
        ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch, 0.4f);

        ImGui::TableNextColumn();
        ImGui::BeginChild("BrowserPane", ImVec2(0, 0), false);

        DrawFilterControls();
        ImGui::Separator();
        DrawAchievementList();

        ImGui::EndChild();

        ImGui::TableNextColumn();
        ImGui::BeginChild("DetailsPane", ImVec2(0, 0), false);

        DrawAchievementDetails();

        ImGui::EndChild();

        ImGui::EndTable();
    }
}

void AchievementEditor::DrawFilterControls() {
    ImGui::SeparatorText("Filters");

    ImGui::PushItemWidth(-1);
    if (ImGui::InputTextWithHint("##SearchBox", "Search achievements...", mSearchText, sizeof(mSearchText))) {
        // Search updated
    }
    ImGui::PopItemWidth();

    ImGui::Spacing();

    int categoryIndex = static_cast<int>(mCategoryFilter) + 1; // +1 for "All"
    if (UIWidgets::Combobox("Category", &categoryIndex, CATEGORY_NAMES,
                            UIWidgets::ComboboxOptions().LabelPosition(UIWidgets::LabelPosition::Above))) {
        mCategoryFilter = static_cast<AchievementCategory>(categoryIndex - 1);
        CVarSetInteger(CVAR_CATEGORY_FILTER, categoryIndex - 1);
        CVarSave();
    }

    if (UIWidgets::Checkbox("Show Unlocked Only", &mShowUnlockedOnly)) {
        if (mShowUnlockedOnly)
            mShowLockedOnly = false;
        CVarSetInteger(CVAR_SHOW_UNLOCKED, mShowUnlockedOnly);
        CVarSave();
    }

    if (UIWidgets::Checkbox("Show Locked Only", &mShowLockedOnly)) {
        if (mShowLockedOnly)
            mShowUnlockedOnly = false;
        CVarSetInteger(CVAR_SHOW_LOCKED, mShowLockedOnly);
        CVarSave();
    }

    if (UIWidgets::Checkbox("Show Secret Achievements", &mShowSecretAchievements)) {
        CVarSetInteger(CVAR_SHOW_SECRET, mShowSecretAchievements);
        CVarSave();
    }

    ImGui::Spacing();
    if (UIWidgets::Button("Clear Filters",
                          UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline).Color(UIWidgets::Colors::Gray))) {
        memset(mSearchText, 0, sizeof(mSearchText));
        mCategoryFilter = static_cast<AchievementCategory>(-1);
        mShowUnlockedOnly = false;
        mShowLockedOnly = false;
        mShowSecretAchievements = true;

        CVarSetInteger(CVAR_CATEGORY_FILTER, -1);
        CVarSetInteger(CVAR_SHOW_UNLOCKED, 0);
        CVarSetInteger(CVAR_SHOW_LOCKED, 0);
        CVarSetInteger(CVAR_SHOW_SECRET, 1);
        CVarSave();
    }
}

void AchievementEditor::DrawAchievementList() {
    ImGui::SeparatorText("Achievements");

    uint32_t totalCount = GetTotalCount();
    uint32_t unlockedCount = GetUnlockedCount();
    uint32_t totalScore = GetTotalGamerscore();
    uint32_t unlockedScore = GetUnlockedGamerscore();

    ImGui::Text("Progress: %u/%u (%u%%) | Score: %u/%u", unlockedCount, totalCount,
                totalCount > 0 ? (unlockedCount * 100) / totalCount : 0, unlockedScore, totalScore);

    ImGui::Spacing();

    if (ImGui::BeginChild("AchievementList", ImVec2(0, 0), true)) {
        bool foundAny = false;

        for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
            AchievementId id = static_cast<AchievementId>(i);
            const Achievement* achievement = Achievements::StaticData::GetAchievement(id);

            if (!achievement || !ShouldShowAchievement(achievement)) {
                continue;
            }

            foundAny = true;
            DrawAchievementCard(achievement);
        }

        if (!foundAny) {
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() / 2 - 100, ImGui::GetWindowHeight() / 2 - 10));
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No achievements match current filters");
        }
    }
    ImGui::EndChild();
}

void AchievementEditor::DrawAchievementCard(const Achievement* achievement) {
    if (!achievement) {
        SPDLOG_ERROR("Attempted to draw null achievement card in editor");
        return;
    }

    bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);
    bool isSelected = (mSelectedAchievementId == achievement->id);

    ImGui::PushID(static_cast<int>(achievement->id));

    float cardWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 cardPos = ImGui::GetCursorScreenPos();
    ImVec2 cardSize = ImVec2(cardWidth, AchievementEditorUI::Layout::CARD_HEIGHT);

    ImGui::InvisibleButton("##card", cardSize);
    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    if (clicked) {
        mSelectedAchievementId = achievement->id;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImVec4 bgColor, borderColor;
    if (isSelected) {
        bgColor = ImVec4(0.0f, 0.4f, 0.8f, 0.3f);
        borderColor = UIWidgets::ColorValues.at(UIWidgets::Colors::LightBlue);
    } else if (hovered) {
        bgColor = ImVec4(0.2f, 0.2f, 0.2f, 0.8f);
        borderColor = ImVec4(0.5f, 0.5f, 0.5f, 0.8f);
    } else {
        bgColor = ImVec4(0.1f, 0.1f, 0.1f, 0.6f);
        borderColor = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
    }

    drawList->AddRectFilled(cardPos, cardPos + cardSize, ImGui::ColorConvertFloat4ToU32(bgColor), 5.0f);
    drawList->AddRect(cardPos, cardPos + cardSize, ImGui::ColorConvertFloat4ToU32(borderColor), 5.0f, 0, 1.5f);

    float padding = 8.0f;
    ImGui::SetCursorScreenPos(ImVec2(cardPos.x + padding, cardPos.y + padding));

    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (achievement->iconPath && strlen(achievement->iconPath) > 0 && gui &&
        gui->HasTextureByName(achievement->iconPath)) {

        ImTextureID iconTexture = gui->GetTextureByName(achievement->iconPath);
        ImGui::Image(iconTexture,
                     ImVec2(AchievementEditorUI::Layout::ICON_SIZE, AchievementEditorUI::Layout::ICON_SIZE));
    } else {
        ImVec2 iconPos = ImGui::GetCursorScreenPos();
        ImVec4 iconColor = isUnlocked ? UIWidgets::ColorValues.at(UIWidgets::Colors::Green)
                                      : UIWidgets::ColorValues.at(UIWidgets::Colors::Gray);

        drawList->AddRectFilled(
            iconPos, iconPos + ImVec2(AchievementEditorUI::Layout::ICON_SIZE, AchievementEditorUI::Layout::ICON_SIZE),
            ImGui::ColorConvertFloat4ToU32(iconColor), 3.0f);

        ImVec2 textSize = ImGui::CalcTextSize("?");
        ImVec2 textPos =
            iconPos +
            (ImVec2(AchievementEditorUI::Layout::ICON_SIZE, AchievementEditorUI::Layout::ICON_SIZE) - textSize) * 0.5f;
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), "?");

        ImGui::Dummy(ImVec2(AchievementEditorUI::Layout::ICON_SIZE, AchievementEditorUI::Layout::ICON_SIZE));
    }

    ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::Text("%s", achievement->name);
    if (isUnlocked) {
        ImGui::SameLine();
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Green), ICON_FA_CHECK);
    }
    if (achievement->secret) {
        ImGui::SameLine();
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Orange), ICON_FA_EYE_SLASH);
    }

    std::string desc = achievement->description;
    if (desc.length() > 80) {
        desc = desc.substr(0, 77) + "...";
    }

    float availableWidth = cardWidth - AchievementEditorUI::Layout::ICON_SIZE - padding * 3;
    if (achievement->gamerscore > 0) {
        std::string scoreText = "Score: " + std::to_string(achievement->gamerscore);
        float scoreWidth = ImGui::CalcTextSize(scoreText.c_str()).x;
        availableWidth -= scoreWidth + padding;
    }

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + availableWidth);
    ImGui::TextWrapped("%s", desc.c_str());
    ImGui::PopTextWrapPos();

    ImGui::EndGroup();

    if (achievement->gamerscore > 0) {
        std::string scoreText = "Score: " + std::to_string(achievement->gamerscore);
        float scoreWidth = ImGui::CalcTextSize(scoreText.c_str()).x;

        ImGui::SetCursorScreenPos(ImVec2(cardPos.x + cardWidth - scoreWidth - padding, cardPos.y + padding));
        ImGui::Text("%s", scoreText.c_str());
    }

    ImGui::SetCursorScreenPos(ImVec2(cardPos.x, cardPos.y + cardSize.y));

    ImGui::PopID();
}

void AchievementEditor::DrawAchievementDetails() {
    ImGui::SeparatorText("Achievement Details");

    if (mSelectedAchievementId >= AchievementId::ACHIEVEMENT_ID_MAX) {
        ImGui::TextWrapped("Select an achievement from the list to view its details and testing options.");
        return;
    }

    const Achievement* achievement = Achievements::StaticData::GetAchievement(mSelectedAchievementId);
    if (!achievement) {
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Red), "Error: Could not load achievement data");
        return;
    }

    DrawSelectedAchievementInfo();
    ImGui::Separator();
    DrawEventProgressDisplay();
    ImGui::Separator();
    DrawAchievementActions();
}

void AchievementEditor::DrawSelectedAchievementInfo() {
    const Achievement* achievement = Achievements::StaticData::GetAchievement(mSelectedAchievementId);
    if (!achievement)
        return;

    bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);

    // Icon and basic info
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (!gui) {
        SPDLOG_ERROR("Failed to get GUI context for achievement editor details");
        return;
    }

    if (achievement->iconPath && strlen(achievement->iconPath) > 0 && gui->HasTextureByName(achievement->iconPath)) {

        ImTextureID iconTexture = gui->GetTextureByName(achievement->iconPath);
        ImGui::Image(iconTexture,
                     ImVec2(AchievementEditorUI::Layout::ICON_SIZE, AchievementEditorUI::Layout::ICON_SIZE));
        ImGui::SameLine();
    }

    ImGui::BeginGroup();
    ImGui::Text("Name: %s", achievement->name);
    ImGui::Text("ID: %d", static_cast<int>(achievement->id));

    // Status
    ImGui::Text("Status: ");
    ImGui::SameLine();
    ImGui::TextColored(isUnlocked ? UIWidgets::ColorValues.at(UIWidgets::Colors::Green)
                                  : UIWidgets::ColorValues.at(UIWidgets::Colors::Red),
                       "%s", isUnlocked ? "Unlocked" : "Locked");

    if (achievement->secret) {
        ImGui::Text("Type: Secret Achievement");
    }

    if (achievement->gamerscore > 0) {
        ImGui::Text("Gamerscore: %d", achievement->gamerscore);
    }

    // Category
    const char* categoryName = "Unknown";
    switch (achievement->category) {
        case AchievementCategory::GENERAL:
            categoryName = "General";
            break;
        case AchievementCategory::VANILLA:
            categoryName = "Vanilla";
            break;
        case AchievementCategory::RANDO:
            categoryName = "Randomizer";
            break;
    }
    ImGui::Text("Category: %s", categoryName);

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::TextWrapped("Description: %s", achievement->description);
}

void AchievementEditor::DrawEventProgressDisplay() {
    const Achievement* achievement = Achievements::StaticData::GetAchievement(mSelectedAchievementId);
    if (!achievement || achievement->requiredEvents.empty()) {
        ImGui::Text("This achievement has no associated events.");
        return;
    }

    ImGui::Text("Required Events (%zu):", achievement->requiredEvents.size());
    ImGui::Spacing();

    uint32_t completedEvents = 0;
    for (AchievementEvent eventId : achievement->requiredEvents) {
        if (IS_ACH_TRIGGERED(eventId)) {
            completedEvents++;
        }
    }

    // Progress bar
    float progress = achievement->requiredEvents.size() > 0
                         ? static_cast<float>(completedEvents) / achievement->requiredEvents.size()
                         : 0.0f;

    ImGui::ProgressBar(
        progress, ImVec2(-1, 0),
        (std::to_string(completedEvents) + "/" + std::to_string(achievement->requiredEvents.size())).c_str());

    ImGui::Spacing();

    // Event list
    if (ImGui::BeginChild("EventList", ImVec2(0, 150), true)) {
        if (ImGui::BeginTable("EventTable", 3, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("Event", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 90.0f);

            for (size_t i = 0; i < achievement->requiredEvents.size(); i++) {
                AchievementEvent eventId = achievement->requiredEvents[i];
                bool isTriggered = IS_ACH_TRIGGERED(eventId);

                ImGui::PushID(static_cast<int>(i));
                ImGui::TableNextRow();

                // Status
                ImGui::TableNextColumn();
                ImGui::TextColored(isTriggered ? UIWidgets::ColorValues.at(UIWidgets::Colors::Green)
                                               : UIWidgets::ColorValues.at(UIWidgets::Colors::Red),
                                   "%s", isTriggered ? ICON_FA_CHECK_CIRCLE : ICON_FA_CIRCLE);

                // Event name column
                ImGui::TableNextColumn();
                const auto* event = Achievements::StaticData::GetEvent(eventId);
                const char* eventName = event ? event->name : "Unknown Event";
                ImGui::TextWrapped("%s", eventName);

                // Action button column
                ImGui::TableNextColumn();
                if (UIWidgets::Button(isTriggered ? "Reset" : "Trigger",
                                      UIWidgets::ButtonOptions()
                                          .Size(UIWidgets::Sizes::Inline)
                                          .Color(isTriggered ? UIWidgets::Colors::Orange : UIWidgets::Colors::Green))) {
                    if (isTriggered) {
                        Achievements::ResetEvent(eventId);
                    } else {
                        Achievements::TriggerEvent(eventId, true);
                    }
                    if (BenGui::mAchievementsWindow) {
                        BenGui::mAchievementsWindow->InvalidateCache();
                    }
                }
                if (isTriggered) {
                    UIWidgets::Tooltip("Reset this event and lock all dependent achievements");
                } else {
                    UIWidgets::Tooltip("Trigger this event and check for achievement unlocks");
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void AchievementEditor::DrawAchievementActions() {
    const Achievement* achievement = Achievements::StaticData::GetAchievement(mSelectedAchievementId);
    if (!achievement)
        return;

    bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);

    ImGui::Text("Testing Actions:");
    ImGui::Spacing();

    // Primary action button
    if (isUnlocked) {
        if (UIWidgets::Button(
                "Lock Achievement",
                UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::Orange))) {
            Achievements::Lock(achievement->id);
            // Reset all required events to provide a clean testing state
            for (AchievementEvent eventId : achievement->requiredEvents) {
                Achievements::ResetEvent(eventId);
            }
            // Invalidate achievements window cache
            if (BenGui::mAchievementsWindow) {
                BenGui::mAchievementsWindow->InvalidateCache();
            }
        }
        UIWidgets::Tooltip("Force-lock this achievement and reset its events for testing purposes");
    } else {
        if (UIWidgets::Button(
                "Unlock Achievement",
                UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::Green))) {
            // Trigger all required events to unlock
            for (AchievementEvent eventId : achievement->requiredEvents) {
                Achievements::TriggerEvent(eventId, true);
            }
            // Invalidate achievements window cache
            if (BenGui::mAchievementsWindow) {
                BenGui::mAchievementsWindow->InvalidateCache();
            }
        }
        UIWidgets::Tooltip("Trigger all required events to unlock this achievement");
    }

    // Note: Individual event manipulation is available in the Event Progress section above
}

void AchievementEditor::DrawEventTesting() {
    ImGui::SeparatorText("Event Testing");

    // Two-column layout: event list | controls
    if (ImGui::BeginTable("EventLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Events", ImGuiTableColumnFlags_WidthStretch, 0.7f);
        ImGui::TableSetupColumn("Controls", ImGuiTableColumnFlags_WidthStretch, 0.3f);

        ImGui::TableNextColumn();
        DrawEventStatusGrid();

        ImGui::TableNextColumn();
        DrawEventTriggerControls();

        ImGui::EndTable();
    }
}

void AchievementEditor::DrawEventStatusGrid() {
    ImGui::Text("All Achievement Events");
    ImGui::Spacing();

    if (ImGui::BeginChild("EventGrid", ImVec2(0, 0), true)) {
        // Use a scrollable list instead of grid for better UX
        if (ImGui::BeginTable("AllEventsTable", 3,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("Event Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            int eventCount = static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX);
            for (int i = 0; i < eventCount; i++) {
                AchievementEvent eventId = static_cast<AchievementEvent>(i);
                bool isTriggered = IS_ACH_TRIGGERED(eventId);
                bool isSelected = (mSelectedEventId == eventId);

                ImGui::PushID(i);
                ImGui::TableNextRow();

                // Highlight selected row
                if (isSelected) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                                           ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.4f, 0.8f, 0.3f)));
                }

                // ID column
                ImGui::TableNextColumn();
                ImGui::Text("%d", i);

                // Event name
                ImGui::TableNextColumn();
                const auto* event = Achievements::StaticData::GetEvent(eventId);
                const char* eventName = event ? event->name : "Unknown Event";
                if (ImGui::Selectable(eventName, isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                    mSelectedEventId = eventId;
                }

                // Status
                ImGui::TableNextColumn();
                ImGui::TextColored(isTriggered ? UIWidgets::ColorValues.at(UIWidgets::Colors::Green)
                                               : UIWidgets::ColorValues.at(UIWidgets::Colors::Red),
                                   "%s %s", isTriggered ? ICON_FA_CHECK_CIRCLE : ICON_FA_CIRCLE,
                                   isTriggered ? "Active" : "Inactive");

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void AchievementEditor::DrawEventTriggerControls() {
    ImGui::Text("Event Controls");
    ImGui::Spacing();

    // Selected event info
    ImGui::Text("Selected Event:");
    ImGui::Indent();
    const auto* event = Achievements::StaticData::GetEvent(mSelectedEventId);
    const char* eventName = event ? event->name : "Unknown Event";
    ImGui::TextWrapped("%s", eventName);
    ImGui::Unindent();

    bool isSelected = IS_ACH_TRIGGERED(mSelectedEventId);
    ImGui::Text("Status: %s", isSelected ? "Triggered" : "Not Triggered");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (UIWidgets::Button(isSelected ? "Reset Event" : "Trigger Event",
                          UIWidgets::ButtonOptions()
                              .Size(UIWidgets::Sizes::Fill)
                              .Color(isSelected ? UIWidgets::Colors::Orange : UIWidgets::Colors::Green))) {
        if (isSelected) {
            Achievements::ResetEvent(mSelectedEventId);
        } else {
            Achievements::TriggerEvent(mSelectedEventId, true);
        }
        // Invalidate achievements window cache
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    if (isSelected) {
        UIWidgets::Tooltip("Reset this event and lock all achievements that depend on it");
    } else {
        UIWidgets::Tooltip("Trigger this event and check for achievement completions");
    }

    ImGui::Spacing();

    ImGui::Text("Bulk Operations:");

    if (UIWidgets::Button(
            "Trigger All Events",
            UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::LightBlue))) {
        for (int i = 0; i < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX); i++) {
            Achievements::TriggerEvent(static_cast<AchievementEvent>(i), true);
        }
        // Invalidate achievements window cache
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    UIWidgets::Tooltip("Triggers all events and checks for achievement completions");

    if (UIWidgets::Button("Reset All Events",
                          UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::Red))) {
        for (int i = 0; i < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX); i++) {
            Achievements::ResetEvent(static_cast<AchievementEvent>(i));
        }
        // Invalidate achievements window cache
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    UIWidgets::Tooltip("Resets all events and locks all dependent achievements");
}

void AchievementEditor::DrawSystemDiagnostics() {
    ImGui::SeparatorText("System Diagnostics");

    // Three sections: info | statistics | validation
    if (ImGui::BeginTabBar("DiagnosticsTabs")) {
        if (ImGui::BeginTabItem("System Info")) {
            DrawSystemInfo();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Statistics")) {
            DrawStatistics();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Validation")) {
            DrawValidationResults();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void AchievementEditor::DrawSystemInfo() {
    ImGui::Text("Achievement System Information");
    ImGui::Spacing();

    ImGui::BulletText("System Active: %s", IS_ACHIEVEMENTS ? "Yes" : "No");
    ImGui::BulletText("Total Achievements: %u", GetTotalCount());
    ImGui::BulletText("Total Events: %u", static_cast<uint32_t>(AchievementEvent::ACHIEVEMENT_EVENT_MAX));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Save File Information:");
    if (IS_ACHIEVEMENTS) {
        ImGui::BulletText("Achievements Enabled: Yes");
        ImGui::BulletText("Unlocked Achievements: %u", GetUnlockedCount());
        ImGui::BulletText("Triggered Events: %u", GetTriggeredEventCount());
    } else {
        ImGui::BulletText("Achievements Enabled: No");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Memory Usage (Estimated):");
    size_t achievementDataSize = GetTotalCount() * sizeof(Achievement);
    size_t eventDataSize = static_cast<size_t>(AchievementEvent::ACHIEVEMENT_EVENT_MAX) * sizeof(bool);
    ImGui::BulletText("Achievement Data: ~%zu bytes", achievementDataSize);
    ImGui::BulletText("Event Data: ~%zu bytes", eventDataSize);
}

void AchievementEditor::DrawStatistics() {
    uint32_t totalCount = GetTotalCount();
    uint32_t unlockedCount = GetUnlockedCount();
    uint32_t totalScore = GetTotalGamerscore();
    uint32_t unlockedScore = GetUnlockedGamerscore();

    ImGui::Text("Achievement Statistics");
    ImGui::Spacing();

    ImGui::Text("Overall Progress:");
    float progressPercent = totalCount > 0 ? (static_cast<float>(unlockedCount) / totalCount) * 100.0f : 0.0f;
    ImGui::ProgressBar(static_cast<float>(unlockedCount) / totalCount, ImVec2(-1, 0),
                       (std::to_string(unlockedCount) + " / " + std::to_string(totalCount) + " (" +
                        std::to_string(static_cast<int>(progressPercent)) + "%)")
                           .c_str());

    ImGui::Spacing();

    ImGui::Text("Gamerscore:");
    float scorePercent = totalScore > 0 ? (static_cast<float>(unlockedScore) / totalScore) * 100.0f : 0.0f;
    ImGui::ProgressBar(static_cast<float>(unlockedScore) / totalScore, ImVec2(-1, 0),
                       (std::to_string(unlockedScore) + " / " + std::to_string(totalScore) + " (" +
                        std::to_string(static_cast<int>(scorePercent)) + "%)")
                           .c_str());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("By Category:");
    uint32_t generalCount = 0, vanillaCount = 0, randoCount = 0;
    uint32_t generalUnlocked = 0, vanillaUnlocked = 0, randoUnlocked = 0;

    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        const Achievement* achievement = Achievements::StaticData::GetAchievement(static_cast<AchievementId>(i));
        if (!achievement)
            continue;

        bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);

        switch (achievement->category) {
            case AchievementCategory::GENERAL:
                generalCount++;
                if (isUnlocked)
                    generalUnlocked++;
                break;
            case AchievementCategory::VANILLA:
                vanillaCount++;
                if (isUnlocked)
                    vanillaUnlocked++;
                break;
            case AchievementCategory::RANDO:
                randoCount++;
                if (isUnlocked)
                    randoUnlocked++;
                break;
        }
    }

    if (generalCount > 0) {
        ImGui::BulletText("General: %u/%u (%.0f%%)", generalUnlocked, generalCount,
                          (static_cast<float>(generalUnlocked) / generalCount) * 100.0f);
    }
    if (vanillaCount > 0) {
        ImGui::BulletText("Vanilla: %u/%u (%.0f%%)", vanillaUnlocked, vanillaCount,
                          (static_cast<float>(vanillaUnlocked) / vanillaCount) * 100.0f);
    }
    if (randoCount > 0) {
        ImGui::BulletText("Randomizer: %u/%u (%.0f%%)", randoUnlocked, randoCount,
                          (static_cast<float>(randoUnlocked) / randoCount) * 100.0f);
    }
}

void AchievementEditor::DrawValidationResults() {
    ImGui::Text("System Validation");
    ImGui::Spacing();

    if (UIWidgets::Button(
            "Refresh Validation",
            UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline).Color(UIWidgets::Colors::LightBlue))) {
        mValidationDirty = true;
    }

    ImGui::SameLine();

    if (UIWidgets::Button("Fix Completion Issues",
                          UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Inline).Color(UIWidgets::Colors::Green))) {
        FixCompletionIssues();
        mValidationDirty = true;
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    UIWidgets::Tooltip("Automatically unlock achievements that have all required events but are still locked");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (!mValidationErrors.empty()) {
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Red),
                           "Errors (%zu):", mValidationErrors.size());
        for (const auto& error : mValidationErrors) {
            ImGui::BulletText("%s", error.c_str());
        }
        ImGui::Spacing();
    }

    if (!mValidationWarnings.empty()) {
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Orange),
                           "Warnings (%zu):", mValidationWarnings.size());
        for (const auto& warning : mValidationWarnings) {
            ImGui::BulletText("%s", warning.c_str());
        }
        ImGui::Spacing();
    }

    if (mValidationErrors.empty() && mValidationWarnings.empty()) {
        ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Green),
                           ICON_FA_CHECK " No validation issues found");
    }
}

void AchievementEditor::DrawBulkOperations() {
    ImGui::SeparatorText("Bulk Operations");

    ImGui::TextWrapped("These operations affect all achievements and should be used carefully. "
                       "They are primarily intended for testing purposes.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Achievement Operations:");
    ImGui::Spacing();

    if (UIWidgets::Button("Unlock All Achievements",
                          UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::Green))) {
        UnlockAllAchievements();
        // Invalidate achievements window cache
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    UIWidgets::Tooltip("Triggers all required events for every achievement");

    if (UIWidgets::Button("Lock All Achievements",
                          UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::Orange))) {
        for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
            Achievements::Lock(static_cast<AchievementId>(i));
        }
        // Invalidate achievements window cache
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    UIWidgets::Tooltip("Force-locks all achievements");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Event Operations:");
    ImGui::Spacing();

    if (UIWidgets::Button(
            "Trigger All Events",
            UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::LightBlue))) {
        for (int i = 0; i < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX); i++) {
            Achievements::TriggerEvent(static_cast<AchievementEvent>(i), true);
        }
        // Invalidate achievements window cache
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    UIWidgets::Tooltip("Triggers every achievement event");

    if (UIWidgets::Button("Reset All Events",
                          UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::Red))) {
        for (int i = 0; i < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX); i++) {
            Achievements::ResetEvent(static_cast<AchievementEvent>(i));
        }
        // Invalidate achievements window cache
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    UIWidgets::Tooltip("Resets all events and locks all dependent achievements");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::Red), "Danger Zone:");
    ImGui::Spacing();

    if (UIWidgets::Button("Clear All Progress",
                          UIWidgets::ButtonOptions().Size(UIWidgets::Sizes::Fill).Color(UIWidgets::Colors::Red))) {
        ClearAllProgress();
        // Invalidate achievements window cache
        if (BenGui::mAchievementsWindow) {
            BenGui::mAchievementsWindow->InvalidateCache();
        }
    }
    UIWidgets::Tooltip("Completely resets all achievement progress (locks all achievements and resets all events)");
}

bool AchievementEditor::ShouldShowAchievement(const Achievement* achievement) const {
    if (!achievement)
        return false;

    if (strlen(mSearchText) > 0) {
        std::string searchLower = mSearchText;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        std::string nameLower = achievement->name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

        std::string descLower = achievement->description;
        std::transform(descLower.begin(), descLower.end(), descLower.begin(), ::tolower);

        if (nameLower.find(searchLower) == std::string::npos && descLower.find(searchLower) == std::string::npos) {
            return false;
        }
    }

    if (mCategoryFilter != static_cast<AchievementCategory>(-1) && achievement->category != mCategoryFilter) {
        return false;
    }

    bool isUnlocked = IS_ACH_UNLOCKED(achievement->id);
    if (mShowUnlockedOnly && !isUnlocked)
        return false;
    if (mShowLockedOnly && isUnlocked)
        return false;

    if (!mShowSecretAchievements && achievement->secret)
        return false;

    return true;
}

void AchievementEditor::RefreshValidation() {
    mValidationErrors.clear();
    mValidationWarnings.clear();

    if (!IS_ACHIEVEMENTS) {
        if (gPlayState && gSaveContext.gameMode != GAMEMODE_TITLE_SCREEN &&
            gSaveContext.gameMode != GAMEMODE_FILE_SELECT) {
            mValidationWarnings.push_back("Achievement system is not active for this save");
        }
        return;
    }
    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        AchievementId achId = static_cast<AchievementId>(i);
        const Achievement* achievement = Achievements::StaticData::GetAchievement(achId);
        if (!achievement) {
            mValidationErrors.push_back("Missing achievement data for ID " + std::to_string(i));
            continue;
        }

        if (!achievement->name || strlen(achievement->name) == 0) {
            mValidationErrors.push_back("Achievement " + std::to_string(i) + " has no name");
        }

        if (!achievement->description || strlen(achievement->description) == 0) {
            mValidationWarnings.push_back("Achievement " + std::to_string(i) + " has no description");
        }
        for (AchievementEvent eventId : achievement->requiredEvents) {
            if (eventId >= AchievementEvent::ACHIEVEMENT_EVENT_MAX) {
                mValidationErrors.push_back("Achievement " + std::to_string(i) + " references invalid event " +
                                            std::to_string(static_cast<int>(eventId)));
            }
        }

        if (!IS_ACH_UNLOCKED(achId) && !achievement->requiredEvents.empty()) {
            bool allEventsTriggered = true;
            for (AchievementEvent eventId : achievement->requiredEvents) {
                if (!IS_ACH_TRIGGERED(eventId)) {
                    allEventsTriggered = false;
                    break;
                }
            }

            if (allEventsTriggered) {
                mValidationWarnings.push_back("Achievement \"" + std::string(achievement->name) +
                                              "\" has all events triggered but is still locked");
            }
        }
    }
}

uint32_t AchievementEditor::GetUnlockedCount() const {
    if (!IS_ACHIEVEMENTS) {
        return 0;
    }

    uint32_t count = 0;
    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        if (IS_ACH_UNLOCKED(static_cast<AchievementId>(i))) {
            count++;
        }
    }
    return count;
}

uint32_t AchievementEditor::GetTotalCount() const {
    return static_cast<uint32_t>(AchievementId::ACHIEVEMENT_ID_MAX);
}

uint32_t AchievementEditor::GetTotalGamerscore() const {
    uint32_t total = 0;
    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        const Achievement* achievement = Achievements::StaticData::GetAchievement(static_cast<AchievementId>(i));
        if (achievement) {
            total += achievement->gamerscore;
        }
    }
    return total;
}

uint32_t AchievementEditor::GetUnlockedGamerscore() const {
    if (!IS_ACHIEVEMENTS) {
        return 0;
    }

    uint32_t unlocked = 0;
    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        AchievementId id = static_cast<AchievementId>(i);
        if (IS_ACH_UNLOCKED(id)) {
            const Achievement* achievement = Achievements::StaticData::GetAchievement(id);
            if (achievement) {
                unlocked += achievement->gamerscore;
            } else {
                SPDLOG_DEBUG("Achievement {} is unlocked but has no data", i);
            }
        }
    }
    return unlocked;
}

uint32_t AchievementEditor::GetTriggeredEventCount() const {
    if (!IS_ACHIEVEMENTS) {
        return 0;
    }

    uint32_t count = 0;
    for (int i = 0; i < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX); i++) {
        if (IS_ACH_TRIGGERED(static_cast<AchievementEvent>(i))) {
            count++;
        }
    }
    return count;
}

void AchievementEditor::UnlockAllAchievements() {
    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        const Achievement* achievement = Achievements::StaticData::GetAchievement(static_cast<AchievementId>(i));
        if (achievement) {
            // Trigger all required events for this achievement
            for (AchievementEvent eventId : achievement->requiredEvents) {
                Achievements::TriggerEvent(eventId, true);
            }
        }
    }
}

void AchievementEditor::ClearAllProgress() {
    // Lock all achievements
    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        Achievements::Lock(static_cast<AchievementId>(i));
    }

    // Reset all events
    for (int i = 0; i < static_cast<int>(AchievementEvent::ACHIEVEMENT_EVENT_MAX); i++) {
        Achievements::ResetEvent(static_cast<AchievementEvent>(i));
    }
}

void AchievementEditor::FixCompletionIssues() {
    if (!IS_ACHIEVEMENTS) {
        SPDLOG_WARN("Attempted to fix completion issues but achievements are not active");
        return;
    }

    uint32_t fixedCount = 0;

    for (int i = 0; i < static_cast<int>(AchievementId::ACHIEVEMENT_ID_MAX); i++) {
        AchievementId achId = static_cast<AchievementId>(i);
        const Achievement* achievement = Achievements::StaticData::GetAchievement(achId);

        if (!achievement || achievement->requiredEvents.empty() || IS_ACH_UNLOCKED(achId)) {
            continue;
        }

        // Check if all required events are triggered
        bool allEventsTriggered = true;
        for (AchievementEvent eventId : achievement->requiredEvents) {
            if (!IS_ACH_TRIGGERED(eventId)) {
                allEventsTriggered = false;
                break;
            }
        }

        // If all events are triggered, re-trigger one to force completion check
        if (allEventsTriggered && !achievement->requiredEvents.empty()) {
            SPDLOG_DEBUG("Fixing completion issue for achievement: {}", achievement->name);
            Achievements::TriggerEvent(achievement->requiredEvents[0], true);
            fixedCount++;
        }
    }

    SPDLOG_INFO("Fixed {} achievement completion issues", fixedCount);
}

} // namespace DeveloperTools

} // namespace Achievements