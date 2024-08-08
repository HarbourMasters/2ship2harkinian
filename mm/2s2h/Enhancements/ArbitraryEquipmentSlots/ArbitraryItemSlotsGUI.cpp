#include "ArbitraryItemSlotsGUI.h"
#include "ArbitraryItemSlots.h"
#include "Context.h"

void ArbitraryItemSlotsWindow::InitElement() {
    return;
}
void ArbitraryItemSlotsWindow::DrawElement() {
    ImGui::SetNextWindowSize(ImVec2(480, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Arbitrary Equipment Slots", &mIsVisible, ImGuiWindowFlags_NoFocusOnAppearing)) {
        std::shared_ptr<ArbitraryItemSlotLister> lister = ArbitraryItemSlotLister::getLister();
        lister->options->drawOptions(this, lister.get());
        ImGui::End();
        return;
    }

    ImGui::End();
}
void ArbitraryItemSlotsWindow::UpdateElement() {
}

ArbitraryItemSlotsWindow::ArbitraryItemSlotsWindow() : Ship::GuiWindow("", "") {
}

void ArbitraryItemSlotsListerOptions::drawSlotState(ArbitraryItemSlotsWindow* win, SlotState* state,
                                                    std::string appendex) {
    ImGui::DragInt(("Position left" + appendex).c_str(), &state->posLeft);
    ImGui::DragInt(("Position top" + appendex).c_str(), &state->posTop);
    ImGui::DragFloat(("Scale" + appendex).c_str(), &state->scale, 0.01, 0.001, 0);
    ImGui::DragFloat(("Transparency" + appendex).c_str(), &state->transparency, 0.01, 0.001, 2);
    ImGui::ColorEdit4(("Color" + appendex).c_str(), state->rgb, ImGuiColorEditFlags_NoInputs);
}

void ArbitraryItemSlotsListerOptions::drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotLister* manager) {
    Ship::InputEditorWindow* inputWindow = (Ship::InputEditorWindow*)(void*)Ship::Context::GetInstance()
                                               ->GetWindow()
                                               ->GetGui()
                                               ->GetGuiWindow("Input Editor")
                                               .get();
    ImGui::Text("Default State");
    this->drawSlotState(win, &manager->parentState, "##parentState");
    ImGui::Text("Disabled State");
    this->drawSlotState(win, &manager->disabledState, "##parentDisabledState");

    size_t i = 1;

    for (auto slot : manager->slots) {
        std::string label = "Slot #";
        label += std::to_string(i);
        std::string slotUniqueAppendex = "##";
        slotUniqueAppendex += std::to_string(i);
        label += slotUniqueAppendex;

        if (ImGui::CollapsingHeader(label.c_str())) {
            ImGui::Text("Default State");
            this->drawSlotState(win, &slot->state, slotUniqueAppendex);
            ImGui::Text("Disabled State");
            this->drawSlotState(win, &slot->disabledState, slotUniqueAppendex + "disabledState");

            if (inputWindow != nullptr) {
                inputWindow->DrawButtonLine(
                    "Use/Assign Item", 0, 0, slot->specialButtonId,
                    ImVec4(slot->state.rgb[0], slot->state.rgb[1], slot->state.rgb[2], slot->state.rgb[3]));
            }
        }
        i++;
        slot->saveCVars();
    }

    manager->saveCVars();
    CVarSave();
}

std::shared_ptr<ArbitraryItemSlotsWindow> arbItemSlotsWindow;
