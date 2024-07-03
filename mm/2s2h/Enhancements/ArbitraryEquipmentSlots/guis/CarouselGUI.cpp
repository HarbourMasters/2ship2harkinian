#include "./CarouselGUI.h"
#include "../CarouselItemSlots.h"

void CarouselListerOptions::drawOptions(ArbitraryItemSlotLister* manager){
    CarouselItemSlotLister * m = (CarouselItemSlotLister*) manager;
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