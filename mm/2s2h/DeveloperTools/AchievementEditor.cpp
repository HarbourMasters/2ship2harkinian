#include "AchievementEditor.h"
#include "2s2h/Enhancements/Achievements/Achievements.h"
#include "2s2h/Enhancements/Achievements/AchievementData.h"
#include <libultraship/libultraship.h>
#include <imgui.h>
#include <string>
#include <algorithm>

extern "C" {
#include <variables.h>
}

namespace Ship {

AchievementEditor::AchievementEditor(const std::string& consoleVariable, const std::string& name)
    : GuiWindow(consoleVariable, name) {
    mAchievementSystem = nullptr;
    mSelectedAchievementId = AID_UNKNOWN;
    memset(mFilterText, 0, sizeof(mFilterText));
}

void AchievementEditor::InitElement() {
    // We get the instance pointer dynamically in DrawElement as it might not be ready at initialization.
    mAchievementSystem = nullptr;
}

void AchievementEditor::UpdateElement() {
    // For this simple editor, we fetch data directly in DrawElement.
    // This could be optimized later if performance is an issue.
}

void AchievementEditor::DrawElement() {
    mAchievementSystem = &AchievementSystem::Instance();

    if (!mAchievementSystem) {
        ImGui::Text("Achievement System instance is not available.");
        return;
    }

    mAchievementsList = mAchievementSystem->GetAchievements();

    // --- Filter Input (Above the table) ---
    ImGui::PushItemWidth(-1);
    ImGui::InputTextWithHint("##AchievementFilter", "Search by ID or Title...", mFilterText, sizeof(mFilterText),
                             ImGuiInputTextFlags_AutoSelectAll);
    ImGui::PopItemWidth();
    ImGui::Separator();

    // --- Main Layout: Table | Details (Reverted to two columns) ---
    if (ImGui::BeginTable("AchievementEditorLayout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("ListColumn", ImGuiTableColumnFlags_WidthStretch, 0.4f);
        ImGui::TableSetupColumn("DetailsColumn", ImGuiTableColumnFlags_WidthStretch, 0.6f);

        // --- Left Pane: Achievement List ---
        ImGui::TableNextColumn();
        ImGui::BeginChild("AchievementListPane", ImVec2(0, 0), ImGuiChildFlags_Border);
        DrawAchievementList();
        ImGui::EndChild();

        // --- Right Pane: Details ---
        ImGui::TableNextColumn();
        ImGui::BeginChild("DetailsPane", ImVec2(0, 0), ImGuiChildFlags_Border);
        DrawDetailsPane();
        ImGui::EndChild();

        ImGui::EndTable();
    }
}

void AchievementEditor::DrawAchievementList() {
    if (!mAchievementSystem)
        return;

    std::string filterLower = mFilterText;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::vector<std::shared_ptr<Achievement>> filteredList;
    for (const auto& achievement : mAchievementsList) {
        if (!achievement)
            continue;
        std::string nameLower = achievement->getName();
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

        if (filterLower.empty() || nameLower.find(filterLower) != std::string::npos) {
            filteredList.push_back(achievement);
        }
    }

    std::sort(filteredList.begin(), filteredList.end(),
              [](const std::shared_ptr<Achievement>& a, const std::shared_ptr<Achievement>& b) {
                  return a->getId() < b->getId();
              });

    for (size_t i = 0; i < filteredList.size(); ++i) {
        const auto& achievement = filteredList[i];
        if (!achievement)
            continue;

        bool isSelected = (mSelectedAchievementId == achievement->getId());

        std::string label = achievement->getName() + "##" + std::to_string(i);
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            mSelectedAchievementId = achievement->getId();
        }
        if (isSelected) {
            ImGui::SetItemDefaultFocus();
        }
    }
}

void AchievementEditor::DrawDetailsPane() {
    if (!mAchievementSystem || mSelectedAchievementId == AID_UNKNOWN) {
        ImGui::TextWrapped("Select an achievement from the list on the left to view its details and actions.");
        return;
    }

    auto selectedAchievement = mAchievementSystem->GetAchievement(mSelectedAchievementId);

    if (!selectedAchievement) {
        ImGui::Text("Error: Could not find details for the selected achievement (ID: %d).",
                    (int)mSelectedAchievementId);
        if (ImGui::Button("Clear Selection")) {
            mSelectedAchievementId = AID_UNKNOWN;
        }
        return;
    }

    ImGui::Separator();
    ImGui::Text("ID:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%d", (int)selectedAchievement->getId());

    ImGui::Separator();
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (!selectedAchievement->getIconPath().empty() && gui &&
        gui->HasTextureByName(selectedAchievement->getIconPath())) {
        ImTextureID textureId = gui->GetTextureByName(selectedAchievement->getIconPath());
        ImVec2 iconSize(64, 64);
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImGui::Image(textureId, iconSize);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Icon Path: %s", selectedAchievement->getIconPath().c_str());
            ImGui::EndTooltip();
        }
    } else {
        ImGui::Text("Icon: (None or not loaded)");
    }

    ImGui::Separator();
    ImGui::Text("Name:");
    ImGui::TextWrapped("%s", selectedAchievement->getName().c_str());

    ImGui::Separator();
    ImGui::TextWrapped("Description:");
    ImGui::TextWrapped("%s", selectedAchievement->getDescription().c_str());

    // --- Add Progress Display for Editor --- 
    const auto& staticDataIt = AllAchievementData.find(selectedAchievement->getId());
    if (staticDataIt != AllAchievementData.end()) {
        const AchievementStaticData& staticData = staticDataIt->second;
        if (staticData.hasProgressTracking) {
            s32* currentProgressPtr = &gSaveContext.save.shipSaveInfo.achievements.achievementData[(int)selectedAchievement->getId()].currentProgress;
            ImGui::Text("Progress: %d / %d", *currentProgressPtr, staticData.targetProgress);
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
            if (ImGui::InputInt("##SetProgress", currentProgressPtr, 1, 5)) {
                SPDLOG_DEBUG("Achievement Editor: Set progress for ID {} to {}", (int)selectedAchievement->getId(), *currentProgressPtr);
            }
            ImGui::PopItemWidth();
        }
    }
    // --- End Progress Display --- 

    ImGui::Separator();
    if (selectedAchievement->getGamerscore() > 0) {
        ImGui::Text("Gamerscore:");
        ImGui::SameLine();
        ImGui::Text("%d", selectedAchievement->getGamerscore());
        ImGui::Separator();
    }

    bool isUnlocked = mAchievementSystem->IsAchievementUnlocked(selectedAchievement->getId());

    ImGui::Text("Status:");
    ImGui::SameLine();
    ImGui::TextColored(isUnlocked ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                       isUnlocked ? "Unlocked" : "Locked");

    // Display if the achievement is secret
    bool isSecret = selectedAchievement->isSecret();
    ImGui::Text("Secret:");
    ImGui::SameLine();
    ImGui::TextColored(isSecret ? ImVec4(1.0f, 0.5f, 0.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       isSecret ? "Yes" : "No");

    ImGui::Separator();
    ImGui::Spacing();

    // Action Buttons (Lock/Unlock)
    if (isUnlocked) {
        if (ImGui::Button("Lock Achievement", ImVec2(-1, 0))) {
            mAchievementSystem->LockAchievement(selectedAchievement->getId());
        }
        ImGui::SetTooltip("Force-lock this achievement (for testing).");
    } else {
        if (ImGui::Button("Unlock Selected")) {
            mAchievementSystem->DebugUnlockAchievement(selectedAchievement->getId());
        }
        ImGui::SetTooltip("Force-unlock this achievement (for testing). This will trigger a notification.");
    }
}

} // namespace Ship