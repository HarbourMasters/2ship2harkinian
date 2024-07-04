#include "./CarouselGUI.h"
#include "../CarouselItemSlots.h"
#include "InputEditorWindow.h"

void CarouselListerOptions::drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotLister* manager){
    CarouselItemSlotLister * m = (CarouselItemSlotLister*) manager;
    Ship::InputEditorWindow* inputWindow = (Ship::InputEditorWindow*) (void*) Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Input Editor").get();
    if(inputWindow != nullptr){
        inputWindow->DrawButtonLine("Use/Assign Item", 0, 0, m->equipButtonIntent, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        inputWindow->DrawButtonLine("Swap Item Left", 0, 0, m->swapLeftIntent, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        inputWindow->DrawButtonLine("Swap Item Right", 0, 0, m->swapRightIntent, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    }
    ImGui::DragInt("Rest Position Left", &m->restPositionLeft);
    ImGui::DragInt("Rest Position Top", &m->restPositionTop);
    ImGui::Checkbox("Use different position while scrolling", &m->activePositionDifferent);
    if(m->activePositionDifferent){
        ImGui::DragInt("\tPosition Left", &m->activePositionLeft);
        ImGui::DragInt("\tPosition Position Top", &m->activePositionTop);
    }
    ImGui::SliderAngle("Scroll Direction", &m->carouselDirectionAngle);
    ImGui::ColorPicker3("Slots Color", m->rgb);
    uint8_t size = m->slotCount;
    ImGui::Text("Slot count: %u", m->slotCount);

    if(UIWidgets::Button("Add Slot")){
        m->resetSlotCount(size + 1);
    }

    for(auto p : m->carouselSlots){
        ImGui::Text("Item slot.");
    }
}