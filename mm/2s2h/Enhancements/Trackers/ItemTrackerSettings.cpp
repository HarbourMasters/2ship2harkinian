#include "ItemTrackerSettings.h"
#include "ItemTracker.h"
#include "../../BenGui/UIWidgets.hpp"

namespace BenGui {
extern std::shared_ptr<ItemTrackerWindow> mItemTrackerWindow;
}

using namespace BenGui;

void ItemTrackerSettingsWindow::UpdateElement() {
}

void ItemTrackerSettingsWindow::InitElement() {
}

#define CVAR_TRACKER(var) CVAR_PREFIX_TRACKER "." var
#define CVAR_TRACKER_ITEM(var) CVAR_TRACKER(".ItemTracker." var)

static const char* itemTrackerCapacityTrackOptions[5] = {
    "No Numbers",
    "Current Capacity",
    "Current Ammo",
    "Current Capacity / Max Capacity",
    "Current Ammo / Current Capacity",
};
static const char* windowTypes[2] = { "Floating", "Window" };
static const char* displayTypes[3] = { "Hidden", "Main Window", "Separate" };
static const char* displayModes[2] = { "Always", "Combo Button Hold" };

void ItemTrackerSettingsWindow::DrawElement() {
    ImGui::SetNextWindowSize(ImVec2(733, 472), ImGuiCond_FirstUseEver);
    const UIWidgets::FloatSliderOptions sliderOpts = { .format = "%.0f", .step = 1.0f };

    if (!ImGui::Begin("Item Tracker Settings", &mIsVisible, ImGuiWindowFlags_NoFocusOnAppearing)) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 8.0f, 8.0f });
    ImGui::BeginTable("itemTrackerSettingsTable", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV);
    ImGui::TableSetupColumn("General settings", ImGuiTableColumnFlags_WidthStretch, 200.0f);
    ImGui::TableSetupColumn("Section settings", ImGuiTableColumnFlags_WidthStretch, 200.0f);
    ImGui::TableHeadersRow();
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("BG Color");
    ImGui::SameLine();

    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

    if (ImGui::ColorEdit4("BG Color", (float*)mItemTrackerWindow->GetBgColorPtr(),
                          ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_NoLabel)) {

    }

    UIWidgets::Combobox("Window Type", mItemTrackerWindow->GetWindowTypePtr(), windowTypes);
    UIWidgets::Checkbox("Enable Dragging", mItemTrackerWindow->GetIsDraggablePtr());
    UIWidgets::Checkbox("Only enable while paused", mItemTrackerWindow->GetOnlyShowPausedPtr());
   
    if (UIWidgets::CVarCombobox("Display Mode", CVAR_TRACKER_ITEM("DisplayType.Main"), displayModes)) {}
    
    UIWidgets::SliderFloat("Icon size : %.0fpx", mItemTrackerWindow->GetIconSizePtr(), 0.0f, 128.0f, sliderOpts);
    UIWidgets::SliderFloat("Icon margins : %.0fpx", mItemTrackerWindow->GetIconSpacingPtr(), -5.0f, 50.0f, sliderOpts);
    UIWidgets::SliderFloat("Text size : %.0fpx", mItemTrackerWindow->GetTextSizePtr(), 1.0f, 30.0f, sliderOpts);
    UIWidgets::SliderFloat("Text Offset : %0.fpx", mItemTrackerWindow->GetTextOffsetPtr(), 0.0f, 40.0f, sliderOpts);

    ImGui::TableNextColumn();

    UIWidgets::Combobox("Inventory", mItemTrackerWindow->GetDrawModePtr(SECTION_INVENTORY), displayTypes);
    UIWidgets::Combobox("Masks", mItemTrackerWindow->GetDrawModePtr(SECTION_MASKS), displayTypes);
    UIWidgets::Combobox("Songs", mItemTrackerWindow->GetDrawModePtr(SECTION_SONGS), displayTypes);
    UIWidgets::Combobox("Dungeon Items", mItemTrackerWindow->GetDrawModePtr(SECTION_DUNGEON), displayTypes);

    UIWidgets::Checkbox("Draw Current Ammo",
                        mItemTrackerWindow->GetCapacityModePtr(ItemTrackerCapacityMode::DrawCurrent));

    UIWidgets::Checkbox("Draw Current Capacity",
                        mItemTrackerWindow->GetCapacityModePtr(ItemTrackerCapacityMode::DrawCurCapacity));

    UIWidgets::Checkbox("Draw Max Capacity",
                        mItemTrackerWindow->GetCapacityModePtr(ItemTrackerCapacityMode::DrawMaxCapacity));

    ImGui::PopStyleVar(1);
    ImGui::EndTable();

    ImGui::End();

}