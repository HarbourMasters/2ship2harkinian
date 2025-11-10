#include "ItemTrackerSettings.h"
#include "ItemTracker.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "ShipUtils.h"

namespace BenGui {
extern std::shared_ptr<ItemTrackerWindow> mItemTrackerWindow;
}

void ItemTrackerSettingsWindow::UpdateElement() {
}

void ItemTrackerSettingsWindow::InitElement() {
    BenGui::mItemTrackerWindow->mainItemWindow = {
        .windowName = "Main",
        .columnLength = 6,
        .windowScale = 1.0f,
        .windowOpacity = 0.5f,
    };
}

#define WIDGET_COLOR UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5))

static const char* windowTypes[2] = { "Floating", "Window" };
static const char* displayTypes[3] = { "Hidden", "Main Window", "Separate" };
static const char* displayModes[2] = { "Always", "Combo Button Hold" };

int16_t popupSlot = SLOT_OCARINA;
static std::string trackerInputName;
bool shouldTrackerPopUpOpen = false;

std::map<std::string, std::pair<int16_t, int16_t>> defaultItemLists = {
    { "Inventory", { ITEM_OCARINA_OF_TIME, ITEM_LETTER_TO_KAFEI } },
    { "Masks", { ITEM_MASK_POSTMAN, ITEM_MASK_FIERCE_DEITY } },
    { "Songs", { ITEM_SONG_TIME, ITEM_SONG_OATH } },
    { "Quest", { ITEM_REMAINS_ODOLWA, ITEM_BOMBERS_NOTEBOOK } },
};

std::pair<uint32_t, uint32_t> GetItemMapRange(uint32_t start, uint32_t end) {
    std::pair<uint32_t, uint32_t> indexRange;

    for (size_t i = 0; i < itemIdToItemNameMap.size(); i++) {
        if (itemIdToItemNameMap[i].first == start) {
            indexRange.first = static_cast<int>(i);
        }
        if (itemIdToItemNameMap[i].first == end) {
            indexRange.second = static_cast<int>(i);
        }
    }

    return indexRange;
}

TexturePtr GetItemImage(int16_t itemId) {
    return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName((const char*)gItemIcons[itemId]);
};

void ItemTrackerPopUpContext(int16_t slot) {
    bool shouldClose = false;
    uint32_t availableSlots = 0;
    uint32_t windowIndex = 0;
    if (shouldTrackerPopUpOpen && ImGui::BeginPopup("ItemWindowSubMenu")) {
        auto findInMain = std::find(BenGui::mItemTrackerWindow->mainItemWindow.itemList.begin(),
                                    BenGui::mItemTrackerWindow->mainItemWindow.itemList.end(), slot);
        if (findInMain == BenGui::mItemTrackerWindow->mainItemWindow.itemList.end()) {
            if (UIWidgets::Button("Add to Main", { .size = ImVec2(0, 0), .color = WIDGET_COLOR })) {
                BenGui::mItemTrackerWindow->mainItemWindow.itemList.push_back(slot);
                shouldClose = true;
            }
            availableSlots++;
        }
        for (auto& object : BenGui::mItemTrackerWindow->namedItemWindows) {
            ImGui::PushID(windowIndex);
            auto findInObject = std::find(object.itemList.begin(), object.itemList.end(), slot);
            if (findInObject == object.itemList.end()) {
                std::string windowStr = "Add to ";
                windowStr += object.windowName;
                if (UIWidgets::Button(windowStr.c_str(), { .size = ImVec2(0, 0), .color = WIDGET_COLOR })) {
                    BenGui::mItemTrackerWindow->namedItemWindows[windowIndex].itemList.push_back(slot);
                    shouldClose = true;
                }
                availableSlots++;
            }
            ImGui::PopID();
            windowIndex++;
        }
        if (availableSlots == 0) {
            ImGui::Text("No Slot Available");
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
        ImGui::ImageButton(std::to_string(itemWindow[i]).c_str(), GetItemImage((int16_t)itemWindow[i]),
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

void DrawItemTrackerSlot(std::vector<int16_t>& itemList, int16_t itemId, bool shouldAdd) {
    TrackerImageObject imageObject = GetTextureObject(itemId);
    imageObject.textureColor.w = 1.0f;
    if (ImGui::ImageButton(std::to_string(itemId).c_str(), imageObject.textureId, imageObject.textureDimensions,
                           ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), imageObject.textureColor)) {
        if (shouldAdd) {
            shouldTrackerPopUpOpen = true;
            popupSlot = itemId;
            ImGui::OpenPopup("ItemWindowSubMenu");
        } else {
            uint32_t index = 0;
            for (int i = 0; i < itemList.size(); i++) {
                if (itemList[i] == itemId) {
                    index = i;
                }
            }
            itemList.erase(itemList.begin() + index);
        }
    }
    UIWidgets::Tooltip(Ship_GetItemNameById(itemId).c_str());
}

void DrawItemList(std::string listName, int columns) {
    if (ImGui::BeginChild(listName.c_str(), ImVec2(0, 0),
                          ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX |
                              ImGuiChildFlags_AutoResizeY)) {
        if (ImGui::BeginTable(listName.c_str(), columns)) {
            ImVec2 framePadding = ImVec2(listName == "Songs" ? 8.0f : 0, 0);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
            std::pair<uint32_t, uint32_t> range =
                GetItemMapRange(defaultItemLists.at(listName).first, defaultItemLists.at(listName).second);
            for (int i = range.first; i <= range.second; i++) {
                ImGui::TableNextColumn();
                std::vector<int16_t> emptyList;
                DrawItemTrackerSlot(emptyList, itemIdToItemNameMap[i].first, true);
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
            BenGui::mItemTrackerWindow->mainItemWindow.itemList.clear();
        }
        if (BenGui::mItemTrackerWindow->mainItemWindow.itemList.size() != 0) {
            if (ImGui::BeginTable("Main Preview", BenGui::mItemTrackerWindow->mainItemWindow.columnLength)) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
                for (int i = 0; i < BenGui::mItemTrackerWindow->mainItemWindow.itemList.size(); i++) {
                    ImGui::TableNextColumn();
                    DrawItemTrackerSlot(BenGui::mItemTrackerWindow->mainItemWindow.itemList,
                                        (int16_t)BenGui::mItemTrackerWindow->mainItemWindow.itemList[i], false);
                    ItemTrackerDragAndDrop(BenGui::mItemTrackerWindow->mainItemWindow.itemList, i);
                }
                ImGui::PopStyleVar(2);
                ImGui::EndTable();
            }
        }
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
            if (ImGui::BeginTable(std::to_string(listIndex).c_str(), object.columnLength)) {
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

void DrawTrackerWindowOptions(int32_t windowIndex, TrackerItemListObject& windowObject) {
    ImGui::PushID(windowIndex);
    int32_t columns = windowObject.columnLength;
    float scale = windowObject.windowScale;
    float opacity = windowObject.windowOpacity;
    std::string trackerInputRename;
    if (windowIndex >= 0) {
        if (UIWidgets::InputString("##windowname", &trackerInputRename,
                                   UIWidgets::InputOptions()
                                       .LabelPosition(UIWidgets::LabelPosition::None)
                                       .Color(WIDGET_COLOR)
                                       .PlaceholderText(windowObject.windowName)
                                       .DefaultValue(windowObject.windowName))) {
            windowObject.windowName = trackerInputRename;
        }
    }
    if (UIWidgets::SliderInt("Columns", &columns,
                             UIWidgets::IntSliderOptions()
                                 .Min(1)
                                 .Max(8)
                                 .DefaultValue(6)
                                 .LabelPosition(UIWidgets::LabelPosition::None)
                                 .Format("Columns: %i")
                                 .Color(WIDGET_COLOR)
                                 .Size(ImVec2(ImGui::GetContentRegionAvail().x / 2, 0)))) {
        windowObject.columnLength = columns;
    }
    ImGui::SameLine();
    if (UIWidgets::SliderFloat("Scale", &scale,
                               UIWidgets::FloatSliderOptions()
                                   .Min(0.5f)
                                   .Max(5.0f)
                                   .DefaultValue(1.0f)
                                   .LabelPosition(UIWidgets::LabelPosition::None)
                                   .Format("Scale: %.1f")
                                   .Step(0.5f)
                                   .Color(WIDGET_COLOR)
                                   .Size(ImVec2(ImGui::GetContentRegionAvail().x, 0)))) {
        windowObject.windowScale = scale;
    }
    if (UIWidgets::SliderFloat("Opacity", &opacity,
                               UIWidgets::FloatSliderOptions()
                                   .Min(0)
                                   .Max(1.0f)
                                   .DefaultValue(0.5f)
                                   .LabelPosition(UIWidgets::LabelPosition::None)
                                   .Format("Opacity: %.1f")
                                   .Step(0.1f)
                                   .Color(WIDGET_COLOR)
                                   .Size(ImVec2(ImGui::GetContentRegionAvail().x, 0)))) {
        windowObject.windowOpacity = opacity;
    }
    ImGui::PopID();
    UIWidgets::Separator();
}

void DrawTrackerOptions() {
    int32_t windowIndex = 0;
    ImGui::SeparatorText("Presets");
    if (ImGui::BeginTable("PresetsList", 2)) {
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Default Preset", { .color = WIDGET_COLOR })) {
            BenGui::mItemTrackerWindow->mainItemWindow.itemList.clear();
            BenGui::mItemTrackerWindow->namedItemWindows.clear();
            std::pair<uint32_t, uint32_t> range = GetItemMapRange(ITEM_OCARINA_OF_TIME, ITEM_BOMBERS_NOTEBOOK);
            for (int i = range.first; i <= range.second; i++) {
                BenGui::mItemTrackerWindow->mainItemWindow.itemList.push_back(itemIdToItemNameMap[i].first);
            }
        }
        UIWidgets::Tooltip("Places all items in the Main Window");

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Split Panel Preset", { .color = WIDGET_COLOR })) {
            BenGui::mItemTrackerWindow->mainItemWindow.itemList.clear();
            BenGui::mItemTrackerWindow->namedItemWindows.clear();
            for (auto& list : defaultItemLists) {
                std::vector<int16_t> defaultItems;
                std::pair<uint32_t, uint32_t> range = GetItemMapRange(list.second.first, list.second.second);
                for (int i = range.first; i <= range.second; i++) {
                    defaultItems.push_back(itemIdToItemNameMap[i].first);
                }
                TrackerItemListObject trackerObject = {
                    .windowName = list.first,
                    .columnLength = 6,
                    .windowScale = 1.0f,
                    .windowOpacity = 0.5f,
                    .itemList = defaultItems,
                };
                if (trackerObject.windowName == "Songs") {
                    trackerObject.columnLength = 5;
                }
                if (trackerObject.windowName == "Quest") {
                    trackerObject.columnLength = 4;
                }
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
                .columnLength = 6,
                .windowScale = 1.0f,
                .windowOpacity = 0.5f,
                .itemList = itemTrackerList,
            };

            BenGui::mItemTrackerWindow->namedItemWindows.push_back(trackerObject);
            trackerInputName.clear();
        }
        ImGui::EndTable();
    }
    ImGui::SeparatorText("Window Options");
    DrawTrackerWindowOptions(-1, BenGui::mItemTrackerWindow->mainItemWindow);
    if (BenGui::mItemTrackerWindow->namedItemWindows.size() != 0) {
        for (auto& window : BenGui::mItemTrackerWindow->namedItemWindows) {
            DrawTrackerWindowOptions(windowIndex, window);
            windowIndex++;
        }
    }
}

void DrawTrackerCustomizationOptions() {
    if (ImGui::BeginChild("Item Lists")) {
        DrawItemList("Inventory", 6);
        DrawItemList("Masks", 6);
        DrawItemList("Songs", 5);
        DrawItemList("Quest", 4);
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
            if (ImGui::BeginChild("TrackerChild")) {
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
                ImGui::EndChild();
            }

            ImGui::TableNextColumn();
            if (ImGui::BeginChild("WindowChild")) {
                if (ImGui::BeginTabBar("WindowTab")) {
                    if (ImGui::BeginTabItem("Window Layouts")) {
                        DrawPreviewPane();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndChild();
            }
            ImGui::EndTable();
        }
        UIWidgets::PopStyleTabs();

        ImGui::PopStyleColor(3);
        ImGui::EndChild();
    }
}
