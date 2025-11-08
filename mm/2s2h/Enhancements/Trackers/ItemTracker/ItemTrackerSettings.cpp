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

int16_t popupSlot = SLOT_OCARINA;
static std::string trackerInputName;
bool shouldTrackerPopUpOpen = false;

std::map<std::string, std::pair<int16_t, int16_t>> defaultItemLists = {
    { "Inventory", { SLOT_OCARINA, SLOT_BOTTLE_6 } },
    { "Masks", { SLOT_MASK_POSTMAN, SLOT_MASK_FIERCE_DEITY } },
};

TexturePtr GetSlotImage(int16_t slot) {
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
        if (slot >= SLOT_BOTTLE_1 && slot <= SLOT_BOTTLE_6) {
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[ITEM_BOTTLE]);
        } else {
            for (int i = 0; i < sizeof(gItemSlots); i++) {
                if (gItemSlots[i] == slot) {
                    return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                        (const char*)gItemIcons[i]);
                }
            }
        }
    }

    return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
        (const char*)gItemIcons[ITEM_OCARINA_OF_TIME]);
}

void ItemTrackerPopUpContext(int16_t slot) {
    bool shouldClose = false;
    bool availableSlot = false;
    uint32_t windowIndex = 0;
    if (shouldTrackerPopUpOpen && ImGui::BeginPopup("ItemWindowSubMenu")) {
        auto findInMain = std::find(BenGui::mItemTrackerWindow->mainItemWindow.begin(),
                                    BenGui::mItemTrackerWindow->mainItemWindow.end(), slot);
        if (findInMain != BenGui::mItemTrackerWindow->mainItemWindow.end()) {
            availableSlot = false;
        } else {
            if (UIWidgets::Button("Add to Main", { .size = ImVec2(0, 0), .color = WIDGET_COLOR })) {
                BenGui::mItemTrackerWindow->mainItemWindow.push_back(slot);
                shouldClose = true;
            }
            availableSlot = true;
        }
        for (auto& object : BenGui::mItemTrackerWindow->namedItemWindows) {
            auto findInObject = std::find(object.itemList.begin(), object.itemList.end(), slot);
            if (findInObject != object.itemList.end()) {
                availableSlot = false;
            } else {
                std::string windowStr = "Add to ";
                windowStr += object.windowName;
                if (UIWidgets::Button(windowStr.c_str(), { .size = ImVec2(0, 0), .color = WIDGET_COLOR })) {
                    BenGui::mItemTrackerWindow->namedItemWindows[windowIndex].itemList.push_back(slot);
                    shouldClose = true;
                }
                availableSlot = true;
            }
            windowIndex++;
        }
        if (shouldClose || !availableSlot) {
            ImGui::CloseCurrentPopup();
            shouldTrackerPopUpOpen = false;
        }

        ImGui::EndPopup();
    }
}

void ItemTrackerDragAndDrop(std::vector<int16_t>& itemWindow, size_t i) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("TRACKER_DRAG", &i, sizeof(size_t));
        ImGui::ImageButton(std::to_string(itemWindow[i]).c_str(), GetSlotImage((int16_t)itemWindow[i]),
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

void DrawItemTrackerSlot(std::vector<int16_t>& itemList, int16_t slot, bool shouldAdd) {
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
                int16_t slot = static_cast<int16_t>(i);
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

void DrawPreviewPane() {
    uint32_t listIndex = 0;
    if (ImGui::BeginChild("List Previews")) {
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
                                        (int16_t)BenGui::mItemTrackerWindow->mainItemWindow[i], false);
                    ItemTrackerDragAndDrop(BenGui::mItemTrackerWindow->mainItemWindow, i);
                }
                ImGui::PopStyleVar(2);
                ImGui::EndTable();
            }
        }
        ImGui::SeparatorText("Sub Item Windows");
        for (auto& object : BenGui::mItemTrackerWindow->namedItemWindows) {
            ImGui::PushID(listIndex);
            ImGui::SeparatorText(object.windowName.c_str());
            ImGui::SameLine(ImGui::GetContentRegionMax().x - (ImGui::CalcTextSize("Clear Main").x * 1.5f));
            if (UIWidgets::Button("Clear", { .size = ImVec2(0, 0), .color = UIWidgets::Colors::Red })) {
                BenGui::mItemTrackerWindow->namedItemWindows[listIndex].itemList.clear();
            }
            ImGui::SameLine();
            if (UIWidgets::Button("x", { .size = ImVec2(0, 0), .color = UIWidgets::Colors::Red })) {
                BenGui::mItemTrackerWindow->namedItemWindows.erase(
                    BenGui::mItemTrackerWindow->namedItemWindows.begin() + listIndex);
            }
            ImGui::PopID();
            if (object.itemList.size() == 0) {
                listIndex++;
                continue;
            }
            if (ImGui::BeginTable(std::to_string(listIndex).c_str(), 6)) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
                for (int i = 0; i < object.itemList.size(); i++) {
                    ImGui::TableNextColumn();
                    DrawItemTrackerSlot(object.itemList, (int16_t)object.itemList[i], false);
                    ItemTrackerDragAndDrop(BenGui::mItemTrackerWindow->namedItemWindows[listIndex].itemList, i);
                }
                ImGui::PopStyleVar(2);
                ImGui::EndTable();
            }
            listIndex++;
        }
        ImGui::EndChild();
    }
}

void DrawTrackerOptions() {
    ImGui::SeparatorText("Presets");
    if (ImGui::BeginTable("PresetsList", 2)) {
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Default Preset", { .color = WIDGET_COLOR })) {
            BenGui::mItemTrackerWindow->mainItemWindow.clear();
            for (int i = SLOT_OCARINA; i <= SLOT_MASK_FIERCE_DEITY; i++) {
                BenGui::mItemTrackerWindow->mainItemWindow.push_back((int16_t)i);
            }
        }
        UIWidgets::Tooltip("Places all items in the Main Window");

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Split Panel Preset", { .color = WIDGET_COLOR })) {
            BenGui::mItemTrackerWindow->mainItemWindow.clear();
            BenGui::mItemTrackerWindow->namedItemWindows.clear();
            for (auto& list : defaultItemLists) {
                std::vector<int16_t> defaultItems;
                for (int i = list.second.first; i <= list.second.second; i++) {
                    defaultItems.push_back(i);
                }
                TrackerItemListObject trackerObject = { .windowName = list.first, .itemList = defaultItems };
                BenGui::mItemTrackerWindow->namedItemWindows.push_back(trackerObject);
            }
        }
        UIWidgets::Tooltip("Places each group of items in its own Window.");
        ImGui::EndTable();
    }
    ImGui::SeparatorText("Custom Windows");
    if (ImGui::BeginTable("OptionsList", 2)) {
        ImGui::TableNextColumn();
        UIWidgets::InputString("Window Name", &trackerInputName,
                               {
                                   .labelPosition = UIWidgets::LabelPosition::None,
                                   .color = WIDGET_COLOR,
                                   .placeholder = "Enter new window name",
                               });

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Create New Window",
                              { .size = ImVec2(ImGui::GetContentRegionMax().x / 2, 0), .color = WIDGET_COLOR })) {
            std::vector<int16_t> itemTrackerList;
            std::string windowText = trackerInputName.c_str();
            TrackerItemListObject trackerObject = {
                .windowName = trackerInputName.c_str(),
                .itemList = itemTrackerList,
            };

            BenGui::mItemTrackerWindow->namedItemWindows.push_back(trackerObject);
            trackerInputName.clear();
        }
        ImGui::EndTable();
    }
}

void DrawTrackerCustomizationOptions() {
    if (ImGui::BeginChild("Item Lists")) {
        DrawItemList("Inventory", 6);
        DrawItemList("Masks", 6);
        ImGui::EndChild();
    }
}

void ItemTrackerSettingsWindow::DrawElement() {
    ImGui::SetNextWindowSize(ImVec2(733, 472), ImGuiCond_FirstUseEver);
    if (ImGui::BeginChild("Item Tracker Settings")) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));

        if (CVarGetInteger("gWindows.ItemTracker", 0)) {
            UIWidgets::WindowButton("Hide Item Tracker", "gWindows.ItemTracker", BenGui::mItemTrackerWindow,
                                    { .size = UIWidgets::Sizes::Inline, .color = WIDGET_COLOR });
        } else {
            UIWidgets::WindowButton("Show Item Tracker", "gWindows.ItemTracker", BenGui::mItemTrackerWindow,
                                    { .size = UIWidgets::Sizes::Inline, .color = WIDGET_COLOR });
        }
        UIWidgets::Separator();
        UIWidgets::PushStyleTabs(WIDGET_COLOR);
        if (ImGui::BeginTable("TrackerTabs", 2)) {
            ImGui::TableNextColumn();
            if (ImGui::BeginTabBar("TrackerTabs")) {
                if (ImGui::BeginTabItem("Customization")) {
                    DrawTrackerCustomizationOptions();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Options")) {
                    DrawTrackerOptions();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::TableNextColumn();
            if (ImGui::BeginTabBar("WindowTab")) {
                if (ImGui::BeginTabItem("Window Layouts")) {
                    DrawPreviewPane();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTable();
        }
        UIWidgets::PopStyleTabs();

        ImGui::PopStyleColor(3);
        ImGui::EndChild();
    }
}
