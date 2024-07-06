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
    
    ImGui::DragInt("Group root position left", &manager->parentLeft);
    ImGui::DragInt("Group root position top", &manager->parentTop);
    ImGui::DragFloat("Group scale", &manager->parentScale, 0.01, 0.001, 0);
    ImGui::DragFloat("Group transparency", &manager->parentTransparency, 0.01, 0.001, 1);
    ImGui::ColorEdit3("Parent Color", manager->rgb, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
    
    size_t i = 1;

    for(auto slot : manager->slots){
        std::string label = "Slot #";
        label += std::to_string(i);
        std::string slotUniqueAppendex = "##";
        slotUniqueAppendex += std::to_string(i);
        label += slotUniqueAppendex;

        if(ImGui::CollapsingHeader(label.c_str())){
            ImGui::DragInt(("Position left" + slotUniqueAppendex).c_str(), &slot->positionLeft);
            ImGui::DragInt(("Position top" + slotUniqueAppendex).c_str(), &slot->positionTop);
            ImGui::DragFloat(("Scale" + slotUniqueAppendex).c_str(), &slot->scale, 0.01, 0.001, 0);
            ImGui::DragFloat(("Transparency" + slotUniqueAppendex).c_str(), &slot->transparency, 0.01, 0.001, 2);
            ImGui::ColorEdit4(("Blend Color" + slotUniqueAppendex).c_str(), slot->rgb, ImGuiColorEditFlags_NoInputs);
            if(inputWindow != nullptr){
                inputWindow->DrawButtonLine(
                    "Use/Assign Item",
                    0,
                    0,
                    slot->specialButtonId,
                    ImVec4(slot->rgb[0], slot->rgb[1], slot->rgb[2], slot->rgb[3])
                );
            }
        }
        i++;
    }
}

std::shared_ptr<ArbitraryItemSlotsWindow> arbItemSlotsWindow;
