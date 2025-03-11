#include "ItemTrackerSettings.h"
#include "ItemTracker.h"
#include "../../BenGui/UIWidgets.hpp"

namespace BenGui {
extern std::shared_ptr<ItemTrackerWindow> mItemTrackerWindow;
}

using namespace BenGui;

static const char* windowTypes[2] = { "Floating", "Window" };
static const char* displayTypes[4] = { "Hidden", "Main Window", "Sub Window", "Separate" };

void ItemTrackerSettingsWindow::DrawElement() {
    ImGui::SetNextWindowSize(ImVec2(733, 472), ImGuiCond_FirstUseEver);

    if (!ImGui::BeginChild("Item Tracker Settings")) {
        ImGui::EndChild();
        return;
    }
    auto menuThemeIndex = static_cast<UIWidgets::Colors>(CVarGetInteger("gSettings.Menu.Theme", 0));

    ImGui::SeparatorText("Item Tracker Settings");
    UIWidgets::WindowButton("Show Item Tracker", "gWindows.ItemTracker", mItemTrackerWindow,
                            { .size = UIWidgets::Sizes::Inline, .color = menuThemeIndex });

    ImGui::BeginTable("Settings Table", 2);
    ImGui::TableSetupColumn("Options", ImGuiTableColumnFlags_WidthFixed, 300.0f);
    ImGui::TableNextColumn();
    for (auto& options : itemTrackerSettingsOptions) {
        if (UIWidgets::CVarCheckbox(options.first, options.second, { .color = menuThemeIndex })) {
            UpdateTrackerSettings();
        }
    }
    if (UIWidgets::CVarSliderInt(
            "Icon Size", "ItemTracker.IconSize",
            { .showButtons = true, .min = 16, .max = 56, .defaultValue = 32, .color = menuThemeIndex })) {
        UpdateTrackerSettings();
    }
    ImGui::TableNextColumn();
    for (auto& options : itemTrackerPanelOptions) {
        if (UIWidgets::CVarCombobox(options.first, options.second, displayTypes, { .color = menuThemeIndex })) {
            UpdateTrackerWindows();
        }
    }

    ImGui::EndTable();
    ImGui::EndChild();
}
