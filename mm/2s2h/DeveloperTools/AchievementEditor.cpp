#include "AchievementEditor.h"
#include "2s2h/Enhancements/Achievements/Achievements.h"
#include <libultraship/libultraship.h>
#include <imgui.h>
#include <string>
#include <algorithm>

namespace Ship {

AchievementEditor::AchievementEditor(const std::string& consoleVariable, const std::string& name)
    : GuiWindow(consoleVariable, name) {
    mAchievementSystem = nullptr;
    mSelectedAchievementId = "";
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
        std::string idLower = achievement->getId();
        std::string nameLower = achievement->getName();
        std::transform(idLower.begin(), idLower.end(), idLower.begin(), ::tolower);
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

        if (filterLower.empty() || idLower.find(filterLower) != std::string::npos ||
            nameLower.find(filterLower) != std::string::npos) {
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

        if (ImGui::Selectable((achievement->getId() + "##" + achievement->getId()).c_str(), isSelected)) {
            mSelectedAchievementId = achievement->getId();
        }
        if (isSelected) {
            ImGui::SetItemDefaultFocus();
        }
    }
}

void AchievementEditor::DrawDetailsPane() {
    if (!mAchievementSystem || mSelectedAchievementId.empty()) {
        ImGui::TextWrapped("Select an achievement from the list on the left to view its details and actions.");
        return;
    }

    auto selectedAchievement = mAchievementSystem->GetAchievement(mSelectedAchievementId);

    if (!selectedAchievement) {
        ImGui::Text("Error: Could not find details for the selected achievement (ID: %s).",
                    mSelectedAchievementId.c_str());
        if (ImGui::Button("Clear Selection")) {
            mSelectedAchievementId = "";
        }
        return;
    }

    ImGui::Separator();
    ImGui::Text("ID:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", selectedAchievement->getId().c_str());

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
    ImGui::Text("Description:");
    ImGui::TextWrapped("%s", selectedAchievement->getDescription().c_str());

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

    ImGui::Separator();
    ImGui::Spacing();

    // Action Buttons (Lock/Unlock)
    if (isUnlocked) {
        if (ImGui::Button("Lock Achievement", ImVec2(-1, 0))) {
            mAchievementSystem->LockAchievement(selectedAchievement->getId());
        }
        ImGui::SetTooltip("Force-lock this achievement (for testing).");
    } else {
        if (ImGui::Button("Unlock Achievement", ImVec2(-1, 0))) {
            mAchievementSystem->UnlockAchievement(selectedAchievement->getId());
        }
        ImGui::SetTooltip("Force-unlock this achievement (for testing). This will trigger a notification.");
    }
}

} // namespace Ship