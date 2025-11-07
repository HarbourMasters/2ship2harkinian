#include "ItemTrackerSettings.h"
#include "ItemTracker.h"
#include "2s2h/BenGui/UIWidgets.hpp"

namespace BenGui {
extern std::shared_ptr<ItemTrackerWindow> mItemTrackerWindow;
}

void ItemTrackerSettingsWindow::UpdateElement() {
}

void ItemTrackerSettingsWindow::InitElement() {
}

#define WIDGET_COLOR UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5))

static const char* windowTypes[2] = { "Floating", "Window" };
static const char* displayTypes[3] = { "Hidden", "Main Window", "Separate" };
static const char* displayModes[2] = { "Always", "Combo Button Hold" };

InventorySlot popupSlot = SLOT_OCARINA;
bool shouldTrackerPopUpOpen = false;

std::map<std::string, std::pair<InventorySlot, InventorySlot>> defaultItemLists = {
    { "Inventory", { SLOT_OCARINA, SLOT_BOTTLE_1 } },
    { "Masks", { SLOT_MASK_POSTMAN, SLOT_MASK_FIERCE_DEITY } },
};

TexturePtr GetSlotImage(InventorySlot slot) {
    if (slot == SLOT_TRADE_DEED) {
        return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[ITEM_MOONS_TEAR]);
    } else if (slot == SLOT_TRADE_KEY_MAMA) {
        return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[ITEM_ROOM_KEY]);
    } else if (slot == SLOT_TRADE_DEED) {
        return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[ITEM_LETTER_TO_KAFEI]);
    } else {
        for (int i = 0; i < sizeof(gItemSlots); i++) {
            if (gItemSlots[i] == slot) {
                return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                    (const char*)gItemIcons[slot == SLOT_BOTTLE_1 ? ITEM_BOTTLE : i]);
            }
        }
    }

    return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
        (const char*)gItemIcons[ITEM_OCARINA_OF_TIME]);
}

void ItemTrackerPopUpContext(InventorySlot slot) {
    bool shouldClose = false;
    uint32_t windowIndex = 0;
    if (shouldTrackerPopUpOpen && ImGui::BeginPopup("ItemWindowSubMenu")) {
        if (UIWidgets::Button("Add to Main", { .size = ImVec2(0, 0), .color = WIDGET_COLOR })) {
            BenGui::mItemTrackerWindow->mainItemWindow.push_back(slot);
            shouldClose = true;
        }
        for (auto& window : BenGui::mItemTrackerWindow->subItemWindows) {
            std::string windowStr = "Add to ";
            windowStr += std::to_string(windowIndex).c_str();
            if (UIWidgets::Button(windowStr.c_str(), { .size = ImVec2(0, 0), .color = WIDGET_COLOR })) {
                BenGui::mItemTrackerWindow->subItemWindows[windowIndex].push_back(slot);
                shouldClose = true;
            }
            windowIndex++;
        }
        if (shouldClose) {
            ImGui::CloseCurrentPopup();
            shouldTrackerPopUpOpen = false;
        }

        ImGui::EndPopup();
    }
}

void ItemTrackerDragAndDrop(std::vector<int16_t>& itemWindow, size_t i) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("TRACKER_DRAG", &i, sizeof(size_t));
        ImGui::ImageButton(std::to_string(itemWindow[i]).c_str(), GetSlotImage((InventorySlot)itemWindow[i]),
                           ImVec2(46.0f, 46.0f));
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TRACKER_DRAG")) {
            size_t srcIndex = *(const size_t*)payload->Data;
            if (srcIndex != i && srcIndex < itemWindow.size()) {
                auto item = itemWindow[srcIndex];
                itemWindow.erase(itemWindow.begin() + srcIndex);

                if (srcIndex < i) {
                    i--;
                }

                itemWindow.insert(itemWindow.begin() + i, item);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void DrawItemTrackerSlot(std::vector<int16_t>& itemList, InventorySlot slot, bool shouldAdd) {
    if (ImGui::ImageButton(std::to_string(slot).c_str(), GetSlotImage(slot), ImVec2(46.0f, 46.0f))) {
        if (shouldAdd) {
            shouldTrackerPopUpOpen = true;
            popupSlot = slot;
            ImGui::OpenPopup("ItemWindowSubMenu");
        } else {
            uint32_t index = 0;
            for (int i = 0; i < itemList.size(); i++) {
                if (itemList[i] == slot) {
                    index = i;
                }
            }
            itemList.erase(itemList.begin() + index);
        }
    }
}

void DrawItemList(std::string listName, int columns) {
    if (ImGui::BeginChild(listName.c_str(), ImVec2(0, 0),
                          ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX |
                              ImGuiChildFlags_AutoResizeY)) {
        if (ImGui::BeginTable(listName.c_str(), columns)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));

            for (int i = defaultItemLists.at(listName).first; i <= defaultItemLists.at(listName).second; i++) {
                InventorySlot slot = static_cast<InventorySlot>(i);
                ImGui::TableNextColumn();
                std::vector<int16_t> emptyList;
                DrawItemTrackerSlot(emptyList, slot, true);
            }

            ImGui::PopStyleVar(2);
            ItemTrackerPopUpContext(popupSlot);
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void ItemTrackerSettingsWindow::DrawElement() {
    uint32_t listIndex = 0;
    ImGui::SetNextWindowSize(ImVec2(733, 472), ImGuiCond_FirstUseEver);
    if (ImGui::BeginChild("Item Tracker Settings")) {
        if (CVarGetInteger("gWindows.ItemTracker", 0)) {
            UIWidgets::WindowButton("Hide Item Tracker", "gWindows.ItemTracker", BenGui::mItemTrackerWindow,
                                    { .size = UIWidgets::Sizes::Inline, .color = WIDGET_COLOR });
        } else {
            UIWidgets::WindowButton("Show Item Tracker", "gWindows.ItemTracker", BenGui::mItemTrackerWindow,
                                    { .size = UIWidgets::Sizes::Inline, .color = WIDGET_COLOR });
        }
        UIWidgets::Separator();
        if (UIWidgets::Button("Default Preset",
                              { .size = ImVec2(ImGui::GetContentRegionMax().x / 2, 0), .color = WIDGET_COLOR })) {
            BenGui::mItemTrackerWindow->mainItemWindow.clear();
            for (int i = SLOT_OCARINA; i <= SLOT_MASK_FIERCE_DEITY; i++) {
                BenGui::mItemTrackerWindow->mainItemWindow.push_back((InventorySlot)i);
            }
        }
        ImGui::SameLine();
        if (UIWidgets::Button("Create New Window",
                              { .size = ImVec2(ImGui::GetContentRegionMax().x / 2, 0), .color = WIDGET_COLOR })) {
            std::vector<int16_t> itemTrackerList;
            BenGui::mItemTrackerWindow->subItemWindows.push_back(itemTrackerList);
        }
        if (ImGui::BeginTable("Customization", 2)) {
            ImGui::TableNextColumn();
            if (ImGui::BeginChild("Item Lists")) {

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));

                DrawItemList("Inventory", 6);
                DrawItemList("Masks", 6);

                ImGui::PopStyleColor(3);

                ImGui::EndChild();
            }

            ImGui::TableNextColumn();
            if (ImGui::BeginChild("List Previews")) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));

                ImGui::SeparatorText("Main Item Window");
                ImGui::SameLine(ImGui::GetContentRegionMax().x - (ImGui::CalcTextSize("Clear Main").x * 1.5f));
                if (UIWidgets::Button("Clear Main", {
                                                        .size = ImVec2(0, 0),
                                                        .color = UIWidgets::Colors::Red,
                                                    })) {
                    BenGui::mItemTrackerWindow->mainItemWindow.clear();
                }
                if (BenGui::mItemTrackerWindow->mainItemWindow.size() != 0) {
                    if (ImGui::BeginTable("Main Preview", 6)) {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
                        for (int i = 0; i < BenGui::mItemTrackerWindow->mainItemWindow.size(); i++) {
                            ImGui::TableNextColumn();
                            DrawItemTrackerSlot(BenGui::mItemTrackerWindow->mainItemWindow,
                                                (InventorySlot)BenGui::mItemTrackerWindow->mainItemWindow[i], false);
                            ItemTrackerDragAndDrop(BenGui::mItemTrackerWindow->mainItemWindow, i);
                        }
                        ImGui::PopStyleVar(2);
                        ImGui::EndTable();
                    }
                }
                ImGui::SeparatorText("Sub Item Windows");
                for (auto& list : BenGui::mItemTrackerWindow->subItemWindows) {
                    ImGui::SeparatorText(std::to_string(listIndex).c_str());
                    std::string clearStr = "Clear ";
                    clearStr += std::to_string(listIndex).c_str();
                    ImGui::SameLine(ImGui::GetContentRegionMax().x - (ImGui::CalcTextSize("Clear Main").x * 1.5f));
                    if (UIWidgets::Button(clearStr.c_str(), {
                                                                .size = ImVec2(0, 0),
                                                                .color = UIWidgets::Colors::Red,
                                                            })) {
                        BenGui::mItemTrackerWindow->subItemWindows[listIndex].clear();
                    }
                    ImGui::SameLine();
                    ImGui::PushID(listIndex);
                    if (UIWidgets::Button("x", {
                                                   .size = ImVec2(0, 0),
                                                   .color = UIWidgets::Colors::Red,
                                               })) {
                        BenGui::mItemTrackerWindow->subItemWindows.erase(
                            BenGui::mItemTrackerWindow->subItemWindows.begin() + listIndex);
                    }
                    ImGui::PopID();
                    if (list.size() == 0) {
                        listIndex++;
                        continue;
                    }
                    if (ImGui::BeginTable(std::to_string(listIndex).c_str(), 6)) {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
                        for (int i = 0; i < list.size(); i++) {
                            ImGui::TableNextColumn();
                            DrawItemTrackerSlot(list, (InventorySlot)list[i], false);
                            ItemTrackerDragAndDrop(BenGui::mItemTrackerWindow->subItemWindows[listIndex], i);
                        }
                        ImGui::PopStyleVar(2);
                        ImGui::EndTable();
                    }
                    listIndex++;
                }

                ImGui::PopStyleColor(3);

                ImGui::EndChild();
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}
