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

void ArbitraryItemSlotsListerOptions::drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotLister* manager){
    Ship::InputEditorWindow* inputWindow = (Ship::InputEditorWindow*) (void*) Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Input Editor").get();
    // if(inputWindow != nullptr){
    //     inputWindow->DrawButtonLine("Use/Assign Item", 0, 0, manager->, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    //     inputWindow->DrawButtonLine("Swap Item Left", 0, 0, m->swapLeftIntent, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    //     inputWindow->DrawButtonLine("Swap Item Right", 0, 0, m->swapRightIntent, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    // }
    ImGui::DragInt("Group root position left", &manager->parentLeft);
    ImGui::DragInt("Group root position top", &manager->parentTop);
}

std::shared_ptr<ArbitraryItemSlotsWindow> arbItemSlotsWindow;
