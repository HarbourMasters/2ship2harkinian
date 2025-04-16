#include "AchievementEditor.h"
#include "2s2h/Enhancements/Achievements/Achievements.h" // Include the full definition
#include <libultraship/libultraship.h>                   // For ImGui, Ship::Context, etc.
#include <imgui.h>
#include <string>
#include <algorithm> // For case-insensitive search

namespace Ship {

// Constructor implementation
AchievementEditor::AchievementEditor(const std::string& consoleVariable, const std::string& name)
    : GuiWindow(consoleVariable, name) {
    // Initialize member variables
    mAchievementSystem = nullptr;
    mSelectedAchievementId = "";
    memset(mFilterText, 0, sizeof(mFilterText)); // Clear filter text buffer
}

// InitElement: Called once when the window is initialized
void AchievementEditor::InitElement() {
    // We get the instance pointer dynamically in DrawElement as it might not be ready at initialization.
    mAchievementSystem = nullptr;
}

// UpdateElement: Called periodically for updates (can be empty if not needed)
void AchievementEditor::UpdateElement() {
    // For this simple editor, we fetch data directly in DrawElement.
    // This could be optimized later if performance is an issue.
}

// DrawElement: Main drawing function called every frame the window is visible
void AchievementEditor::DrawElement() {
    // Always try to get the current instance using the getter
    mAchievementSystem = &AchievementSystem::Instance(); // Use '&' to get pointer from reference

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
        ImGui::TableSetupColumn("ListColumn", ImGuiTableColumnFlags_WidthStretch, 0.4f);    // 40% width for list
        ImGui::TableSetupColumn("DetailsColumn", ImGuiTableColumnFlags_WidthStretch, 0.6f); // 60% width for details

        // --- Left Pane: Achievement List ---
        ImGui::TableNextColumn();
        ImGui::BeginChild("AchievementListPane", ImVec2(0, 0), ImGuiChildFlags_Border);
        DrawAchievementList();
        ImGui::EndChild(); // End AchievementListPane

        // --- Right Pane: Details ---
        ImGui::TableNextColumn();
        ImGui::BeginChild("DetailsPane", ImVec2(0, 0), ImGuiChildFlags_Border);
        DrawDetailsPane();
        ImGui::EndChild(); // End DetailsPane

        ImGui::EndTable();
    }
}

// DrawAchievementList: Renders the scrollable, filterable list of achievement IDs
void AchievementEditor::DrawAchievementList() {
    if (!mAchievementSystem)
        return;

    // Filter logic remains the same (searches ID and Title)
    std::string filterLower = mFilterText;
    std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::vector<std::shared_ptr<Achievement>> filteredList;
    for (const auto& achievement : mAchievementsList) {
        if (!achievement)
            continue;
        std::string idLower = achievement->id;
        std::string nameLower = achievement->name;
        std::transform(idLower.begin(), idLower.end(), idLower.begin(), ::tolower);
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        if (filterLower.empty() || idLower.find(filterLower) != std::string::npos ||
            nameLower.find(filterLower) != std::string::npos) {
            filteredList.push_back(achievement);
        }
    }

    // Use ImGuiListClipper
    ImGuiListClipper clipper;
    clipper.Begin(filteredList.size());
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            if (i < 0 || i >= filteredList.size())
                continue; // Bounds check
            const auto& achievement = filteredList[i];
            if (!achievement)
                continue;

            bool isSelected = (mSelectedAchievementId == achievement->id);
            // Display ONLY the ID in the Selectable, use ID for the unique ## label
            if (ImGui::Selectable((achievement->id + "##" + achievement->id).c_str(), isSelected)) {
                mSelectedAchievementId = achievement->id;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
    }
    clipper.End();
}

// DrawDetailsPane: Renders the details and action buttons for the selected achievement
void AchievementEditor::DrawDetailsPane() {
    // Check if an achievement is selected and the system is available
    if (!mAchievementSystem || mSelectedAchievementId.empty()) {
        ImGui::TextWrapped("Select an achievement from the list on the left to view its details and actions.");
        return;
    }

    // Attempt to retrieve the selected achievement's data
    auto selectedAchievement = mAchievementSystem->GetAchievement(mSelectedAchievementId);

    // Handle case where the selected ID might be invalid (e.g., list changed)
    if (!selectedAchievement) {
        ImGui::Text("Error: Could not find details for the selected achievement (ID: %s).",
                    mSelectedAchievementId.c_str());
        ImGui::Text("It might have been removed or the list changed.");
        if (ImGui::Button("Clear Selection")) {
            mSelectedAchievementId = ""; // Allow user to clear the invalid selection
        }
        return;
    }

    // Display achievement details
    ImGui::Text("Details for:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", selectedAchievement->id.c_str()); // Highlight ID
    ImGui::Separator();

    // --- Icon Display ---
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    bool iconDrawn = false;
    if (!selectedAchievement->iconPath.empty() && gui && gui->HasTextureByName(selectedAchievement->iconPath)) {
        ImTextureID textureId = gui->GetTextureByName(selectedAchievement->iconPath);
        ImVec2 iconSize = ImVec2(48, 48); // Define desired icon size
        ImGui::Image(textureId, iconSize);
        iconDrawn = true;
    } else {
        // Optional: Draw a placeholder if no icon or texture not found
        ImGui::Dummy(ImVec2(48, 48)); // Reserve space even if no icon
    }
    // Place subsequent elements on the same line if an icon was drawn, otherwise start on a new line
    if (iconDrawn) {
        ImGui::SameLine();
    }
    // Use a group to keep Title and Description together, vertically aligned next to the icon
    ImGui::BeginGroup();
    // --- End Icon Display ---

    ImGui::Text("Title:");
    ImGui::SameLine();
    ImGui::TextWrapped("%s", selectedAchievement->name.c_str());
    // Removed separator after Title to keep it closer to Description when grouped

    ImGui::Text("Description:");
    ImGui::SameLine();
    ImGui::TextWrapped("%s", selectedAchievement->description.c_str());
    // End the group containing Title and Description
    ImGui::EndGroup();
    ImGui::Separator(); // Separator after the icon/text block

    // Display Harbour Mastery (Gamerscore) if applicable
    if (selectedAchievement->gamerscore > 0) {
        ImGui::Text("Harbour Mastery (HM):");
        ImGui::SameLine();
        ImGui::Text("%d", selectedAchievement->gamerscore);
        ImGui::Separator();
    }

    // Display current status and action buttons
    bool isUnlocked = mAchievementSystem->IsAchievementUnlocked(selectedAchievement->id);
    ImGui::Text("Status: %s", isUnlocked ? "Unlocked" : "Locked");
    ImGui::Separator();

    // Provide Lock/Unlock buttons based on current state
    if (isUnlocked) {
        // Display Lock button if currently unlocked
        if (ImGui::Button("Lock Achievement", ImVec2(-1, 0))) { // Stretch button width
            mAchievementSystem->LockAchievement(selectedAchievement->id);
            // Note: The status text will update on the next frame automatically
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Force-lock this achievement (for testing).");
        }
    } else {
        // Display Unlock button if currently locked
        if (ImGui::Button("Unlock Achievement", ImVec2(-1, 0))) { // Stretch button width
            mAchievementSystem->UnlockAchievement(selectedAchievement->id);
            // Note: The status text will update on the next frame automatically
            // UnlockAchievement also triggers a notification by default.
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Force-unlock this achievement (for testing). This will trigger a notification.");
        }
    }
}

} // namespace Ship