#include "ArbitraryItemSlotsGUI.h"
#include "ArbitraryItemSlots.h"
#include "Context.h"

void ArbitraryItemSlotsWindow::InitElement(){
    return;
}
void ArbitraryItemSlotsWindow::DrawElement(){
    ImGui::SetNextWindowSize(ImVec2(480, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Arbitrary Equipment Slots", &mIsVisible, ImGuiWindowFlags_NoFocusOnAppearing)) {
        std::shared_ptr<ArbitraryItemSlotLister> lister = ArbitraryItemSlotLister::getLister();
        lister->options->drawOptions(this, lister.get());
        ImGui::End();
        return;
    }

    ImGui::End();
}
void ArbitraryItemSlotsWindow::UpdateElement(){

}
    
ArbitraryItemSlotsWindow::ArbitraryItemSlotsWindow(): Ship::GuiWindow("", ""){
    
}

std::shared_ptr<ArbitraryItemSlotsWindow> arbItemSlotsWindow;
