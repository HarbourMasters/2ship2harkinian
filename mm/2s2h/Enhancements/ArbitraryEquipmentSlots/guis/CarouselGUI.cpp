#include "./CarouselGUI.h"
#include "../CarouselItemSlots.h"

void CarouselListerOptions::drawOptions(ArbitraryItemSlotLister* manager){
    CarouselItemSlotLister * m = (CarouselItemSlotLister*) manager;
    ImGui::DragInt("Position Left", &m->rectLeft);
    ImGui::DragInt("Position Top", &m->rectTop);
    ImGui::ColorPicker3("Slots Color", m->rgb);

    for(auto p : m->carouselSlots){
        ImGui::Text("Item slot.");
    }
}