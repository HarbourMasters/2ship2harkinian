#include "./CarouselGUI.h"
#include "../CarouselItemSlots.h"
#include "InputEditorWindow.h"

void CarouselListerOptions::drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotLister* manager) {
    CarouselItemSlotLister* m = (CarouselItemSlotLister*)manager;
    Ship::InputEditorWindow* inputWindow = (Ship::InputEditorWindow*)(void*)Ship::Context::GetInstance()
                                               ->GetWindow()
                                               ->GetGui()
                                               ->GetGuiWindow("Input Editor")
                                               .get();
    if (inputWindow != nullptr) {
        inputWindow->DrawButtonLine("Use/Assign Item", 0, 0, m->equipButtonIntent, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        inputWindow->DrawButtonLine("Swap Item Left", 0, 0, m->swapLeftIntent, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        inputWindow->DrawButtonLine("Swap Item Right", 0, 0, m->swapRightIntent, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    }

    ImGui::SliderAngle("Scroll Direction", &m->carouselDirectionAngle);

    ImGui::InputInt("Carousel Slot Radius", &m->carouselIndexRadius);

    int count = m->slotCount;
    ImGui::InputInt("Slot Count", &count);

    if (count != m->slotCount) {
        m->resetSlotCount(count);
    }

    if (ImGui::CollapsingHeader("Default State##carouselParentState")) {
        this->drawSlotState(win, &m->parentState, "##carouselParentState");
    }
    if (ImGui::CollapsingHeader("Scrolling State##carouselScrollingState")) {
        this->drawSlotState(win, &m->scrollingState, "##carouselScrollingState");
    }
    if (ImGui::CollapsingHeader("Scrolling Selected State##carouselScrollingSelectedState")) {
        this->drawSlotState(win, &m->scrollingSelectedState, "##carouselScrollingSelectedState");
    }
    if (ImGui::CollapsingHeader("Disabled State##carouselDisabledState")) {
        this->drawSlotState(win, &m->disabledState, "##carouselDisabledState");
    }
    m->saveCVars();
    CVarSave();
}