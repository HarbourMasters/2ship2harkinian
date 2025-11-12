#include "ItemTrackerSettings.h"
#include "ItemTracker.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "ShipUtils.h"

namespace BenGui {
extern std::shared_ptr<ItemTrackerWindow> mItemTrackerWindow;
}

void ItemTrackerSettingsWindow::UpdateElement() {
}

#define WIDGET_COLOR UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5))

static const char* windowTypes[2] = { "Floating", "Window" };

bool shouldWindowSplit = false;
int16_t popupSlot = SLOT_OCARINA;
int16_t indexToRemove = ITEM_OCARINA_OF_TIME;
static std::string trackerInputName;
bool shouldTrackerPopUpOpen = false;
bool shouldRemove = false;

std::vector<std::string> listOrder = { "Inventory", "Bottles",       "Masks",   "Songs", "Quest",
                                       "Tokens",    "Stray Fairies", "Dungeon", };

std::vector<std::string> randoListOrder = { "Frogs", "Boss Souls", "Owl Statues" };

std::map<std::string, std::tuple<int16_t, int16_t, int16_t>> defaultItemLists = {
    { "Inventory", { ITEM_OCARINA_OF_TIME, ITEM_LETTER_TO_KAFEI, 6 } },
    { "Bottles", { ITEM_BOTTLE_1, ITEM_BOTTLE_6, 6 } },
    { "Masks", { ITEM_MASK_POSTMAN, ITEM_MASK_FIERCE_DEITY, 6 } },
    { "Songs", { ITEM_SONG_TIME, ITEM_SONG_OATH, 5 } },
    { "Quest", { ITEM_REMAINS_ODOLWA, ITEM_BOMBERS_NOTEBOOK, 4 } },
    { "Tokens", { ITEM_SKULL_TOKEN_SWAMP, ITEM_SKULL_TOKEN_OCEAN, 2 } },
    { "Stray Fairies", { ITEM_WOODFALL_STRAY_FAIRY, ITEM_STONE_TOWER_STRAY_FAIRY, 4 } },
    { "Dungeon", { ITEM_WOODFALL_DUNGEON_MAP, ITEM_STONE_TOWER_KEY_BOSS, 4 } },
};

std::map<std::string, std::tuple<int16_t, int16_t, int16_t>> randoItemLists = {
    { "Frogs", { RI_FROG_BLUE, RI_FROG_WHITE, 4 } },
    { "Boss Souls", { RI_SOUL_GOHT, RI_SOUL_TWINMOLD, 5 } },
    { "Owl Statues", { RI_OWL_CLOCK_TOWN_SOUTH, RI_OWL_ZORA_CAPE, 5 } },
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

std::string GetItemTrackerItemName(int16_t itemId, bool isRandoItem) {
    std::vector<std::string> dungeonPrefix = {
        "Woodfall",
        "Snowhead",
        "Great Bay",
        "Stone Tower",
    };

    std::string itemName = "";
    if (isRandoItem) {
        itemName = Rando::StaticData::Items[(RandoItemId)itemId].name;
    } else {
        if (itemId >= ITEM_BOTTLE_1 && itemId <= ITEM_BOTTLE_6) {
            itemName = "Bottle";
        } else if (itemId == ITEM_SKULL_TOKEN_SWAMP) {
            itemName = "Swamp Token";
        } else if (itemId == ITEM_SKULL_TOKEN_OCEAN) {
            itemName = "Ocean Token";
        } else if (itemId >= ITEM_WOODFALL_STRAY_FAIRY && itemId <= ITEM_STONE_TOWER_STRAY_FAIRY) {
            itemName = "Stray Fairy";
        } else if (itemId >= ITEM_WOODFALL_DUNGEON_MAP && itemId <= ITEM_STONE_TOWER_KEY_BOSS) {
            switch (itemId) {
                case ITEM_WOODFALL_DUNGEON_MAP:
                case ITEM_SNOWHEAD_DUNGEON_MAP:
                case ITEM_GREAT_BAY_DUNGEON_MAP:
                case ITEM_STONE_TOWER_DUNGEON_MAP:
                    itemName = dungeonPrefix[(itemId - ITEM_WOODFALL_DUNGEON_MAP) / 4] + " Dungeon Map";
                    break;
                case ITEM_WOODFALL_DUNGEON_COMPASS:
                case ITEM_SNOWHEAD_DUNGEON_COMPASS:
                case ITEM_GREAT_BAY_DUNGEON_COMPASS:
                case ITEM_STONE_TOWER_DUNGEON_COMPASS:
                    itemName = dungeonPrefix[(itemId - ITEM_WOODFALL_DUNGEON_COMPASS) / 4] + " Compass";
                    break;
                case ITEM_WOODFALL_KEY_SMALL:
                case ITEM_SNOWHEAD_KEY_SMALL:
                case ITEM_GREAT_BAY_KEY_SMALL:
                case ITEM_STONE_TOWER_KEY_SMALL:
                    itemName = dungeonPrefix[(itemId - ITEM_WOODFALL_KEY_SMALL) / 4] + " Small Key";
                    break;
                case ITEM_WOODFALL_KEY_BOSS:
                case ITEM_SNOWHEAD_KEY_BOSS:
                case ITEM_GREAT_BAY_KEY_BOSS:
                case ITEM_STONE_TOWER_KEY_BOSS:
                    itemName = dungeonPrefix[(itemId - ITEM_WOODFALL_KEY_BOSS) / 4] + " Boss Key";
                    break;
                default:
                    break;
            }
        } else if (itemId == ITEM_MAGIC_JAR_SMALL || itemId == ITEM_MAGIC_JAR_BIG) {
            itemName = gSaveContext.save.saveInfo.playerData.magicLevel > 1 ? "Double Magic" : "Single Magic";
        } else if (itemId == ITEM_HEART_CONTAINER) {
            itemName = "Double Defense";
        } else {
            itemName = Ship_GetItemNameById(itemId);
        }
    }
    
    return itemName;
}

void CreateMainTrackerWindows() {
    TrackerItemListObject initMainObject = {
        .windowName = "Main",
        .columnLength = 6,
        .windowScale = 1.0f,
        .windowOpacity = 0.5f,
    };
    TrackerItemListObject initRandoObject = {
        .windowName = "Rando",
        .columnLength = 6,
        .windowScale = 1.0f,
        .windowOpacity = 0.5f,
    };
    BenGui::mItemTrackerWindow->namedItemWindows.push_back(initMainObject);
    BenGui::mItemTrackerWindow->namedItemWindows.push_back(initRandoObject);
}

void ItemTrackerPopUpContext(int16_t itemId) {
    bool shouldClose = false;
    uint32_t availableSlots = 0;
    uint32_t windowIndex = 0;
    if (shouldTrackerPopUpOpen && ImGui::BeginPopup("ItemWindowSubMenu")) {
        for (auto& object : BenGui::mItemTrackerWindow->namedItemWindows) {
            if (object.windowName == BenGui::mItemTrackerWindow->namedItemWindows[1].windowName) {
                continue;
            }
            ImGui::PushID(windowIndex);
            auto findInObject = std::find(object.itemList.begin(), object.itemList.end(), itemId);
            if (findInObject == object.itemList.end()) {
                std::string windowStr = "Add to ";
                windowStr += object.windowName;
                if (UIWidgets::Button(windowStr.c_str(), { .size = ImVec2(0, 0), .color = WIDGET_COLOR })) {
                    BenGui::mItemTrackerWindow->namedItemWindows[windowIndex].itemList.push_back(itemId);
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
    if (shouldTrackerPopUpOpen && ImGui::BeginPopup("RandoItemWindowSubMenu")) {
        bool randoItemExists = false;
        for (auto& item : BenGui::mItemTrackerWindow->namedItemWindows[1].itemList) {
            if (item == itemId) {
                randoItemExists = true;
                break;
            }
        }
        if (!randoItemExists) {
            ImGui::PushID(windowIndex);
            std::string randoWindowStr = "Add to ";
            randoWindowStr += BenGui::mItemTrackerWindow->namedItemWindows[1].windowName;
            if (UIWidgets::Button(randoWindowStr.c_str(), { .size = ImVec2(0, 0), .color = WIDGET_COLOR })) {
                BenGui::mItemTrackerWindow->namedItemWindows[1].itemList.push_back(itemId);
                shouldClose = true;
            }
            ImGui::PopID();
        } else {
            ImGui::Text("No Slot Available");
        }
        if (shouldClose) {
            ImGui::CloseCurrentPopup();
            shouldTrackerPopUpOpen = false;
        }

        ImGui::EndPopup();
    }
}

void ItemTrackerDragAndDrop(std::vector<int16_t>& itemWindow, size_t i, TrackerImageObject itemObject) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("TRACKER_DRAG", &i, sizeof(size_t));
        ImGui::ImageButton(std::to_string(itemWindow[i]).c_str(), itemObject.textureId, itemObject.textureDimensions,
                           ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), itemObject.textureColor);
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

void DrawRandoItemTrackerSlot(std::vector<int16_t>& itemList, RandoItemId randoItemId, bool shouldAdd) {
    shouldRemove = false;
    TrackerImageObject imageObject = GetTextureObject(randoItemId, true);
    imageObject.textureColor.w = 1.0f;
    if (ImGui::ImageButton(std::to_string(randoItemId).c_str(), imageObject.textureId, imageObject.textureDimensions,
                           ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), imageObject.textureColor)) {
        if (shouldAdd) {
            shouldTrackerPopUpOpen = true;
            popupSlot = randoItemId;
            ImGui::OpenPopup("RandoItemWindowSubMenu");
        } else {
            for (int i = 0; i < itemList.size(); i++) {
                if (itemList[i] == randoItemId) {
                    indexToRemove = i;
                    shouldRemove = true;
                    break;
                }
            }
        }
    }
    UIWidgets::Tooltip(GetItemTrackerItemName(randoItemId, true).c_str());
}

void DrawItemTrackerSlot(std::vector<int16_t>& itemList, int16_t itemId, bool shouldAdd) {
    shouldRemove = false;
    TrackerImageObject imageObject = GetTextureObject(itemId);
    imageObject.textureColor.w = 1.0f;
    if (ImGui::ImageButton(std::to_string(itemId).c_str(), imageObject.textureId, imageObject.textureDimensions,
                           ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), imageObject.textureColor)) {
        if (shouldAdd) {
            shouldTrackerPopUpOpen = true;
            popupSlot = itemId;
            ImGui::OpenPopup("ItemWindowSubMenu");
        } else {
            for (int i = 0; i < itemList.size(); i++) {
                if (itemList[i] == itemId) {
                    indexToRemove = i;
                    shouldRemove = true;
                    break;
                }
            }
        }
    }
    UIWidgets::Tooltip(GetItemTrackerItemName(itemId).c_str());
}

void DrawItemList(std::string listName, int columns) {
    if (ImGui::BeginChild(listName.c_str(), ImVec2(0, 0),
                          ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX |
                              ImGuiChildFlags_AutoResizeY)) {
        if (ImGui::BeginTable(listName.c_str(), columns)) {
            ImVec2 framePadding = ImVec2(listName == "Songs" ? 8.0f : 0, 0);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
            std::vector<int16_t> emptyList;

            if (listName == "Frogs" || listName == "Boss Souls" || listName == "Owl Statues") {
                for (int j = std::get<0>(randoItemLists.at(listName)); j <= std::get<1>(randoItemLists.at(listName));
                     j++) {
                    ImGui::TableNextColumn();
                    DrawRandoItemTrackerSlot(emptyList, (RandoItemId)j, true);
                }
            } else if (listName == "Bottles" || listName == "Tokens" || listName == "Stray Fairies" || listName == "Dungeon") {
                for (int j = std::get<0>(defaultItemLists.at(listName));
                     j <= std::get<1>(defaultItemLists.at(listName)); j++) {
                    ImGui::TableNextColumn();
                    DrawItemTrackerSlot(emptyList, j, true);
                }
            } else {
                std::pair<uint32_t, uint32_t> range = GetItemMapRange(std::get<0>(defaultItemLists.at(listName)),
                                                                      std::get<1>(defaultItemLists.at(listName)));
                for (int i = range.first; i <= range.second; i++) {

                    if (itemIdToItemNameMap[i].first == ITEM_WALLET_ADULT) {
                        ImGui::TableNextColumn();
                        DrawItemTrackerSlot(emptyList, ITEM_MAGIC_JAR_SMALL, true);
                        ImGui::TableNextColumn();
                        DrawItemTrackerSlot(emptyList, ITEM_HEART_CONTAINER, true);
                    }
                    ImGui::TableNextColumn();
                    DrawItemTrackerSlot(emptyList, itemIdToItemNameMap[i].first, true);
                }
            }
            ImGui::PopStyleVar(2);
            ItemTrackerPopUpContext(popupSlot);
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void DrawPreviewPane() {
    bool isRandoItem = false;
    uint32_t listIndex = 0;
    if (ImGui::BeginChild("List Previews")) {
        for (auto& object : BenGui::mItemTrackerWindow->namedItemWindows) {
            ImGui::PushID(listIndex);
            ImGui::SeparatorText(object.windowName.c_str());
            ImGui::SameLine(ImGui::GetContentRegionMax().x - (ImGui::CalcTextSize("Clear Main").x * 1.5f));
            if (UIWidgets::Button("Clear", { .size = ImVec2(0, 0), .color = UIWidgets::Colors::Red })) {
                BenGui::mItemTrackerWindow->namedItemWindows[listIndex].itemList.clear();
            }
            if (listIndex > 1) {
                ImGui::SameLine();
                if (UIWidgets::Button("x", { .size = ImVec2(0, 0), .color = UIWidgets::Colors::Red })) {
                    BenGui::mItemTrackerWindow->namedItemWindows.erase(
                        BenGui::mItemTrackerWindow->namedItemWindows.begin() + listIndex);
                }
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
                    if (object.windowName == BenGui::mItemTrackerWindow->namedItemWindows[1].windowName) {
                        DrawRandoItemTrackerSlot(object.itemList, (RandoItemId)object.itemList[i], false);
                        isRandoItem = true;
                    } else {
                        DrawItemTrackerSlot(object.itemList, (int16_t)object.itemList[i], false);
                    }
                    ItemTrackerDragAndDrop(BenGui::mItemTrackerWindow->namedItemWindows[listIndex].itemList, i,
                                           GetTextureObject(object.itemList[i], isRandoItem));
                }
                if (shouldRemove) {
                    object.itemList.erase(object.itemList.begin() + indexToRemove);
                    shouldRemove = false;
                    indexToRemove = ITEM_OCARINA_OF_TIME;
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
                                 .Max(12)
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

void ApplyRandoItemGroup(std::string listName) {
    std::tuple<int16_t, int16_t, int16_t> list = randoItemLists.at(listName);
    for (int j = std::get<0>(list); j <= std::get<1>(list); j++) {
        for (int i = 0; i < BenGui::mItemTrackerWindow->namedItemWindows[1].itemList.size(); i++) {
            if (BenGui::mItemTrackerWindow->namedItemWindows[1].itemList[i] == j) {
                BenGui::mItemTrackerWindow->namedItemWindows[1].itemList.erase(
                    BenGui::mItemTrackerWindow->namedItemWindows[1].itemList.begin() + i);
                break;
            }
        }
        BenGui::mItemTrackerWindow->namedItemWindows[1].itemList.push_back(j);
    }
    return;
}

void ApplyDefaultItemGroup(std::string listName) {
    std::tuple<int16_t, int16_t, int16_t> list = defaultItemLists.at(listName);
    TrackerItemListObject itemObject = {
        .windowName = listName,
        .columnLength = std::get<2>(list),
        .windowScale = 1.0f,
        .windowOpacity = 0.5f,
    };

    if (listName == "Bottles") {
        for (int j = std::get<0>(list); j <= std::get<1>(list); j++) {
            int16_t bottle = ITEM_BOTTLE_1 + (j - ITEM_BOTTLE_1);
            itemObject.itemList.push_back(bottle);
        }
    } else if (listName == "Tokens") {
        for (int j = std::get<0>(list); j <= std::get<1>(list); j++) {
            int16_t skullToken = ITEM_SKULL_TOKEN_SWAMP + (j - ITEM_SKULL_TOKEN_SWAMP);
            itemObject.itemList.push_back(skullToken);
        }
    } else if (listName == "Stray Fairies") {
        for (int j = std::get<0>(list); j <= std::get<1>(list); j++) {
            int16_t strayFairy = ITEM_WOODFALL_STRAY_FAIRY + (j - ITEM_WOODFALL_STRAY_FAIRY);
            itemObject.itemList.push_back(strayFairy);
        }
    } else if (listName == "Dungeon") {
        for (int j = std::get<0>(list); j <= std::get<1>(list); j++) {
            int16_t dungeonItem = ITEM_WOODFALL_DUNGEON_MAP + (j - ITEM_WOODFALL_DUNGEON_MAP);
            itemObject.itemList.push_back(dungeonItem);
        }
    } else {
        std::pair<uint32_t, uint32_t> range = GetItemMapRange(std::get<0>(list), std::get<1>(list));
        for (int i = range.first; i <= range.second; i++) {
            if (itemIdToItemNameMap[i].first == ITEM_WALLET_ADULT) {
                itemObject.itemList.push_back(ITEM_MAGIC_JAR_SMALL);
                itemObject.itemList.push_back(ITEM_HEART_CONTAINER);
            }
            itemObject.itemList.push_back(itemIdToItemNameMap[i].first);
        }
    }
    BenGui::mItemTrackerWindow->namedItemWindows.push_back(itemObject);
}

void ApplyDefaultItemPreset() {
    BenGui::mItemTrackerWindow->namedItemWindows.clear();
    CreateMainTrackerWindows();

    for (int key = 0; key < defaultItemLists.size(); key++) {
        if (listOrder[key] == "Rando") {
            continue;
        }
        ApplyDefaultItemGroup(listOrder[key]);
    }
}

void DrawTrackerOptions() {
    int32_t windowIndex = 0;
    ImGui::SeparatorText("Presets");
    if (ImGui::BeginTable("PresetsList", 2)) {
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Default Preset", { .color = WIDGET_COLOR })) {
            ApplyDefaultItemPreset();
            shouldWindowSplit = false;
        }
        UIWidgets::Tooltip("Places all items in a single Window");

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Split Panel Preset", { .color = WIDGET_COLOR })) {
            ApplyDefaultItemPreset();
            shouldWindowSplit = true;
        }
        UIWidgets::Tooltip("Places each group of items in its own Window.");
        ImGui::TableNextColumn();
        UIWidgets::CVarCombobox("Window Type", "gSettings.ItemTracker.WindowType", windowTypes,
                                { .alignment = UIWidgets::ComponentAlignment::Right,
                                  .labelPosition = UIWidgets::LabelPosition::Near,
                                  .color = WIDGET_COLOR });

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
    if (BenGui::mItemTrackerWindow->namedItemWindows.size() != 0) {
        for (auto& window : BenGui::mItemTrackerWindow->namedItemWindows) {
            DrawTrackerWindowOptions(windowIndex, window);
            windowIndex++;
        }
    }
}

void DrawTrackerCustomizationOptions() {
    if (ImGui::BeginChild("Item Lists")) {
        for (int key = 0; key < defaultItemLists.size(); key++) {
            ImGui::PushID(key);
            std::tuple<int16_t, int16_t, int16_t> list = defaultItemLists.at(listOrder[key]);
            ImGui::SeparatorText(listOrder[key].c_str());
            ImGui::SameLine(ImGui::GetContentRegionMax().x - (ImGui::CalcTextSize("Set All").x * 1.5f));
            if (UIWidgets::Button("Set All", { .size = ImVec2(0, 0), .color = UIWidgets::Colors::Green })) {
                ApplyDefaultItemGroup(listOrder[key]);
            }
            DrawItemList(listOrder[key], std::get<2>(list));
            ImGui::PopID();
        }
        for (int rkey = 0; rkey < randoItemLists.size(); rkey++) {
            ImGui::PushID(rkey + RI_MAX);
            std::tuple<int16_t, int16_t, int16_t> list = randoItemLists.at(randoListOrder[rkey]);
            ImGui::SeparatorText(randoListOrder[rkey].c_str());
            ImGui::SameLine(ImGui::GetContentRegionMax().x - (ImGui::CalcTextSize("Set All").x * 1.5f));
            if (UIWidgets::Button("Set All", { .size = ImVec2(0, 0), .color = UIWidgets::Colors::Green })) {
                ApplyRandoItemGroup(randoListOrder[rkey]);
            }
            DrawItemList(randoListOrder[rkey], std::get<2>(list));
            ImGui::PopID();
        }

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

void ItemTrackerSettingsWindow::InitElement() {
    CreateMainTrackerWindows();
}
