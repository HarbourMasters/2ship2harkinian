#include "ItemTracker.h"
#include "libultraship/libultraship.h"

extern "C" {
#include "z64save.h"
#include "variables.h"
}

#define CFG_TRACKER_ITEM(var) ("ItemTracker." var)

std::vector<ItemTrackerWindow::ItemTrackerPanel> panelList;

void DrawItemTrackerPanels() {
    for (auto& panel : panelList) {
        ImGui::Begin(std::to_string(panel.panelId).c_str());
        ImGui::BeginGroup();

        ImGui::EndGroup();
        ImGui::End();
    }
}

void ItemTrackerWindow::Draw() {
    if (!IsVisible()) {
        return;
    }


}

void ItemTrackerWindow::InitElement() {

}

void ItemTrackerWindow::DrawElement() {
}
