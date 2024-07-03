#include "./CarouselGUI.h"
#include "../CarouselItemSlots.h"

void CarouselListerOptions::drawOptions(ArbitraryItemSlotLister* manager){
    CarouselItemSlotLister * m = (CarouselItemSlotLister*) manager;
    ImGui::DragInt("Position Left", &m->rectLeft);
    ImGui::DragInt("Position Top", &m->rectTop);
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