#include "ItemTracker.h"
#include "libultraship/libultraship.h"

#include "ShipUtils.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/Rando/Rando.h"

extern "C" {
#include "z64save.h"
#include "variables.h"
#include "functions.h"
#include "assets/archives/icon_item_static/icon_item_static_yar.h"
#include "interface/icon_item_dungeon_static/icon_item_dungeon_static.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "archives/icon_item_24_static/icon_item_24_static_yar.h"
#include "assets/objects/object_mag/object_mag.h"
}

bool isInitialized = false;
float iconSize = 32.0f;
float bgOpacity = 0.5f;

std::vector<ItemTrackerWindow::ItemTrackerPanel> panelList = {
    { TRACKER_INVENTORY, "Inventory", 6, {} }, { TRACKER_MASKS, "Masks", 6, {} },
    { TRACKER_QUEST, "Quest", 5, {} },         { TRACKER_SONGS, "Songs", 5, {} },
    { TRACKER_DUNGEON, "Dungeon", 4, {} },     { TRACKER_STRAY_FAIRIES, "Stray Fairies", 5, {} },
    { TRACKER_RANDO, "Randomizer", 5, {} },
};
std::vector<ItemTrackerWindow::ItemTrackerPanel> mainTrackerWindow;
std::vector<ItemTrackerWindow::ItemTrackerPanel> subTrackerWindow;
std::vector<ItemTrackerWindow::ItemTrackerPanel> separateTrackerWindow;

std::vector<std::pair<const char*, const char*>> itemTrackerPanelOptions = {
    { "Inventory", "ItemTracker.Inventory" },   { "Masks", "ItemTracker.Masks" },
    { "Quest", "ItemTracker.Quest" },           { "Songs", "ItemTracker.Songs" },
    { "Dungeon", "ItemTracker.Dungeon" },       { "Stray Fairies", "ItemTracker.StrayFairies" },
    { "Randomizer", "ItemTracker.Randomizer" },
};

std::vector<std::pair<const char*, const char*>> itemTrackerSettingsOptions = {
    { "Show Capacity", "ItemTracker.Capacity" },
    { "Hide Background", "ItemTracker.Background" },
    { "Hide Map/Compass", "ItemTracker.MapCompass" },
    { "Condensed Keys", "ItemTracker.KeyDisplay" },
};

int16_t getItemBySlot(InventorySlot slot) {
    return orderedInventoryItemList[slot];
}

ImTextureID randoTextureId(RandoItemId randoItem) {
    switch (randoItem) {
        case RI_SOUL_GOHT:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[ITEM_REMAINS_GOHT]);
        case RI_SOUL_GYORG:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[ITEM_REMAINS_GYORG]);
        case RI_SOUL_MAJORA:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gTitleScreenMajorasMaskTex);
        case RI_SOUL_ODOLWA:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[ITEM_REMAINS_ODOLWA]);
        case RI_SOUL_TWINMOLD:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[ITEM_REMAINS_TWINMOLD]);
        default:
            break;
    }
}

ImTextureID dungeonTextureId(ItemId dungeonItem, int16_t dungeonIndex) {
    switch (dungeonItem) {
        case ITEM_KEY_BOSS:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gQuestIconBossKeyTex);
        case ITEM_COMPASS:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gQuestIconCompassTex);
        case ITEM_DUNGEON_MAP:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gQuestIconDungeonMapTex);
        case ITEM_KEY_SMALL:
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gQuestIconSmallKeyTex);
        case ITEM_STRAY_FAIRIES:
            if (dungeonIndex == 0) {
                return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                    gDungeonStrayFairyWoodfallIconTex);
            } else {
                return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                    (const char*)fairyIcons[dungeonIndex - 1]);
            }
        default:
            break;
    }
}

ImTextureID textureId(ItemId itemId, int16_t index) {
    if (itemId == ITEM_BOTTLE) {
        if (gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_1 + index] != ITEM_NONE) {
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_1 + index]]);
        } else {
            return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[ITEM_BOTTLE]);
        }
    }
    if (itemId >= ITEM_SWORD_KOKIRI && itemId <= ITEM_SWORD_GILDED) {
        int swordValue = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD);
        if (swordValue == EQUIP_VALUE_SWORD_NONE) {
            swordValue = EQUIP_VALUE_SWORD_KOKIRI;
        }
        return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[itemId + swordValue - EQUIP_VALUE_SWORD_KOKIRI]);
    }
    if (itemId == ITEM_SHIELD_HERO || itemId == ITEM_SHIELD_MIRROR) {
        int shieldValue = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD);
        if (shieldValue == EQUIP_VALUE_SHIELD_NONE) {
            shieldValue = EQUIP_VALUE_SHIELD_HERO;
        }
        int16_t shield = itemId + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) - EQUIP_VALUE_SHIELD_HERO;
        return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[itemId + shieldValue - EQUIP_VALUE_SHIELD_HERO]);
    }
    return Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName((const char*)gItemIcons[itemId]);
}

ImVec4 randoImageColor(RandoItemId randoItem) {
    if (randoItem >= RI_SOUL_GOHT && randoItem <= RI_SOUL_TWINMOLD) {
        return ImVec4(1, 1, 1,
                      Flags_GetRandoInf((randoItem - RI_SOUL_GOHT) + RANDO_INF_OBTAINED_SOUL_OF_GOHT) ? 1 : 0.4f);
    }
}

ImVec4 imageColor(ItemId itemId, int16_t index) {
    ImVec4 alpha = ImVec4(1, 1, 1, 0.4f);

    if (itemId == ITEM_NONE) {
        return alpha;
    }

    if (itemId == ITEM_BOTTLE) {
        if (gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_1 + index] != ITEM_NONE) {
            alpha = ImVec4(1, 1, 1, 1);
        }
    } else if (itemId >= ITEM_MOONS_TEAR && itemId <= ITEM_PENDANT_OF_MEMORIES) {
        for (int i = ITEM_MOONS_TEAR; i <= ITEM_PENDANT_OF_MEMORIES; i++) {
            if (INV_CONTENT(i) == i) {
                alpha = ImVec4(1, 1, 1, 1);
                break;
            }
        }
    } else if (itemId >= ITEM_REMAINS_ODOLWA && itemId <= ITEM_BOMBERS_NOTEBOOK) {
        alpha = Ship_SongColors(itemId);

        if (itemId == ITEM_SONG_LULLABY || itemId == ITEM_SONG_LULLABY_INTRO) {
            if (!CHECK_QUEST_ITEM((QuestItem)findQuestByItem(ITEM_SONG_LULLABY)) &&
                !CHECK_QUEST_ITEM((QuestItem)findQuestByItem(ITEM_SONG_LULLABY_INTRO))) {
                alpha.w = 0.4f;
            }
        } else {
            if (!CHECK_QUEST_ITEM((QuestItem)findQuestByItem(itemId))) {
                alpha.w = 0.4f;
            }
        }
    } else if (itemId >= ITEM_KEY_BOSS && itemId <= ITEM_KEY_SMALL) {
        if (itemId == ITEM_STRAY_FAIRIES) {
            if (index == 0) {
                alpha = ImVec4(1.0f, 0.9f, 0.5f, 1.0f);
                if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_08_80)) {
                    alpha.w = 0.4f;
                }
            } else {
                if (gSaveContext.save.saveInfo.inventory.strayFairies[index - 1] > 0) {
                    alpha = ImVec4(1, 1, 1, 1);
                }
            }
        } else if (itemId == ITEM_KEY_SMALL || itemId == ITEM_KEY_BOSS) {
            alpha = Ship_DungeonKeyColors(index);
            if (itemId == ITEM_KEY_BOSS && !(CHECK_DUNGEON_ITEM(DUNGEON_BOSS_KEY, index))) {
                alpha.w = 0.4f;
            }
            if (itemId == ITEM_KEY_SMALL && !(DUNGEON_KEY_COUNT(index) > 0)) {
                alpha.w = 0.4f;
            }
        } else {
            if (CHECK_DUNGEON_ITEM((itemId - ITEM_KEY_BOSS), index)) {
                alpha = ImVec4(1, 1, 1, 1);
            }
        }
    } else if (itemId >= ITEM_SWORD_KOKIRI && itemId <= ITEM_SWORD_GILDED) {
        if (!(GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_NONE)) {
            alpha = ImVec4(1, 1, 1, 1);
        }
    } else if (itemId == ITEM_SHIELD_HERO || itemId == ITEM_SHIELD_MIRROR) {
        if (!(GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) == EQUIP_VALUE_SHIELD_NONE)) {
            alpha = ImVec4(1, 1, 1, 1);
        }
    } else if (itemId == ITEM_SKULL_TOKEN) {
        switch (index) {
            case 0: // Swamp
                if (((gSaveContext.save.saveInfo.skullTokenCount >> 16) & 0xFFFF) > 0) {
                    alpha = ImVec4(1, 1, 1, 0.4f);
                }
                break;
            case 1: // Ocean
                if ((gSaveContext.save.saveInfo.skullTokenCount & 0xFFFF) > 0) {
                    alpha = ImVec4(1, 1, 1, 0.8f);
                }
                break;
            default:
                break;
        }
    } else {
        if (INV_CONTENT(itemId) == itemId) {
            alpha = ImVec4(1, 1, 1, 1);
        }
    }

    return alpha;
}

std::vector<int16_t> createItemVector(ItemTrackerWindow::ItemTrackerPanel panel) {
    std::vector<int16_t> itemList;

    switch (panel.panelId) {
        case TRACKER_INVENTORY:
            for (int i = SLOT_OCARINA; i <= SLOT_BOTTLE_6; i++) {
                itemList.push_back(getItemBySlot((InventorySlot)i));
            }
            break;
        case TRACKER_MASKS:
            for (int i = SLOT_MASK_POSTMAN; i <= SLOT_MASK_FIERCE_DEITY; i++) {
                itemList.push_back(getItemBySlot((InventorySlot)i));
            }
            break;
        case TRACKER_QUEST:
            for (int i = QUEST_REMAINS_ODOLWA; i <= QUEST_REMAINS_TWINMOLD; i++) {
                itemList.push_back(questToItemMap[(QuestItem)i]);
            }
            itemList.push_back(questToItemMap[(QuestItem)QUEST_BOMBERS_NOTEBOOK]);
            itemList.push_back(ITEM_SWORD_KOKIRI);
            itemList.push_back(ITEM_SHIELD_HERO);
            itemList.push_back(ITEM_SKULL_TOKEN); // Swamp
            itemList.push_back(ITEM_SKULL_TOKEN); // Ocean
            break;
        case TRACKER_SONGS:
            for (int i = QUEST_SONG_TIME; i <= QUEST_SONG_STORMS; i++) {
                itemList.push_back(questToItemMap[(QuestItem)i]);
            }
            for (int i = QUEST_SONG_SONATA; i <= QUEST_SONG_OATH; i++) {
                itemList.push_back(questToItemMap[(QuestItem)i]);
            }
            break;
        case TRACKER_DUNGEON:
            for (int i = DUNGEON_INDEX_WOODFALL_TEMPLE; i <= DUNGEON_INDEX_STONE_TOWER_TEMPLE; i++) {
                itemList.push_back(ITEM_KEY_BOSS);
                itemList.push_back(ITEM_KEY_SMALL);
                itemList.push_back(ITEM_DUNGEON_MAP);
                itemList.push_back(ITEM_COMPASS);
            }
            break;
        case TRACKER_STRAY_FAIRIES:
            itemList.push_back(ITEM_STRAY_FAIRIES); // Clock Town
            for (int i = DUNGEON_INDEX_WOODFALL_TEMPLE; i <= DUNGEON_INDEX_STONE_TOWER_TEMPLE; i++) {
                itemList.push_back(ITEM_STRAY_FAIRIES);
            }
            break;
        case TRACKER_RANDO:
            for (int i = RI_SOUL_GOHT; i <= RI_SOUL_TWINMOLD; i++) {
                itemList.push_back(i);
            }
            break;
        default:
            break;
    }

    return itemList;
}

void ItemTrackerOverlayText(int16_t itemId, int16_t index) {
    if (!CVarGetInteger("ItemTracker.Capacity", 0)) {
        return;
    }

    std::string overlayText;

    switch (itemId) {
        case ITEM_BOW:
            overlayText = std::to_string(CAPACITY(UPG_QUIVER, CUR_UPG_VALUE(UPG_QUIVER))).c_str();
            break;
        case ITEM_BOMB:
        case ITEM_BOMBCHU:
            overlayText = std::to_string(CAPACITY(UPG_BOMB_BAG, CUR_UPG_VALUE(UPG_BOMB_BAG))).c_str();
            break;
        case ITEM_KEY_SMALL:
            if (DUNGEON_KEY_COUNT(index) > 0) {
                overlayText = std::to_string(DUNGEON_KEY_COUNT(index)).c_str();
            }
            break;
        case ITEM_SKULL_TOKEN:
            if (index == 0) {
                overlayText = std::to_string((gSaveContext.save.saveInfo.skullTokenCount >> 16) & 0xFFFF);
            } else {
                overlayText = std::to_string(gSaveContext.save.saveInfo.skullTokenCount & 0xFFFF);
            }
            break;
        case ITEM_STRAY_FAIRIES:
            if (index != 0) {
                overlayText = std::to_string(gSaveContext.save.saveInfo.inventory.strayFairies[index - 1]);
            }
            break;
        default:
            break;
    }

    // Overlay the item count text on the existing button
    ImVec2 imageMin = ImGui::GetItemRectMin();
    ImVec2 imageMax = ImGui::GetItemRectMax();
    ImVec2 textPos = ImVec2(ImVec2(imageMax.x - ImGui::CalcTextSize(overlayText.c_str()).x - 2,
                                   imageMax.y - ImGui::CalcTextSize(overlayText.c_str()).y - 2));

    ImGui::SetCursorScreenPos(textPos);
    ImGui::TextColored(UIWidgets::ColorValues.at(UIWidgets::Colors::White), overlayText.c_str());
}

void DrawBossSoulColor(ImVec2 cursor, RandoItemId randoItem) {
    ImVec4 color;
    switch (randoItem) {
        case RI_SOUL_GOHT:
            color = ImVec4(0.5f, 0.68f, 0.27f, 1);
            break;
        case RI_SOUL_GYORG:
            color = ImVec4(0.38f, 0.35f, 0.72f, 1);
            break;
        case RI_SOUL_MAJORA:
            color = ImVec4(1.0f, 0.9f, 0.5f, 0.5f);
            break;
        case RI_SOUL_ODOLWA:
            color = ImVec4(0.92f, 0.47f, 0.73f, 1);
            break;
        case RI_SOUL_TWINMOLD:
            color = ImVec4(0.69f, 0.65f, 0.32f, 1);
            break;
        default:
            break;
    }

    ImGui::SetCursorPos(ImVec2(cursor.x - 4.0f, cursor.y - 5.0f));
    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gMagicArrowEquipEffectTex),
                 ImVec2(iconSize + 6.0f, iconSize + 6.0f), ImVec2(0, 0), ImVec2(1, 1), color);
}

void DrawSkulltulaColor(ImVec2 cursor, int16_t index) {
    ImVec4 color = ImVec4(25.0f / 255.0f, 251.0f / 255.0f, 0.0f, 0.5f);
    if (index != 0) {
        color = ImVec4(0.0f, 209.0f / 256.0f, 231.0f / 256.0f, 1.0f);
    }
    ImGui::SetCursorPos(ImVec2(cursor.x - 4.0f, cursor.y - 5.0f));
    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gMagicArrowEquipEffectTex),
                 ImVec2(iconSize + 6.0f, iconSize + 6.0f), ImVec2(0, 0), ImVec2(1, 1), color);
}

void DrawPanelItems(ItemTrackerWindow::ItemTrackerPanel panel, int16_t index) {
    if (ImGui::BeginTable("Item Panel", panel.panelWidth)) {
        ImGui::TableNextColumn();

        for (auto& item : panel.panelContents) {
            if (panel.panelId != TRACKER_RANDO && (item == ITEM_KEY_BOSS || item == ITEM_BOTTLE ||
                                                   item == ITEM_STRAY_FAIRIES || item == ITEM_SKULL_TOKEN)) {
                index++;
            }
            if (panel.panelId != TRACKER_RANDO && (item == ITEM_DUNGEON_MAP || item == ITEM_COMPASS) &&
                CVarGetInteger("ItemTracker.MapCompass", 0)) {
                continue;
            }

            if (panel.panelId == TRACKER_RANDO) {
                ImVec2 cursorPos = ImGui::GetCursorPos();
                if (item >= RI_SOUL_GOHT && item <= RI_SOUL_TWINMOLD) {
                    DrawBossSoulColor(cursorPos, (RandoItemId)item);
                }
                ImGui::SetCursorPos(cursorPos);
            }

            if (item == ITEM_SKULL_TOKEN) {
                ImVec2 cursorPos = ImGui::GetCursorPos();
                DrawSkulltulaColor(cursorPos, index);
                ImGui::SetNextItemAllowOverlap();
                ImGui::SetCursorPos(cursorPos);
            }

            ImGui::Image(
                panel.panelId == TRACKER_DUNGEON || panel.panelId == TRACKER_STRAY_FAIRIES
                    ? dungeonTextureId((ItemId)item, index)
                : panel.panelId == TRACKER_RANDO ? randoTextureId((RandoItemId)item)
                                                 : textureId((ItemId)item, index),
                ImVec2(item >= ITEM_SONG_SONATA && item <= ITEM_SONG_STORMS ? iconSize * 0.75f : iconSize, iconSize),
                ImVec2(0, 0), ImVec2(1, 1),
                panel.panelId == TRACKER_RANDO ? randoImageColor((RandoItemId)item) : imageColor((ItemId)item, index));
            ItemTrackerOverlayText(item, index);
            ImGui::TableNextColumn();
        }
        ImGui::EndTable();
    }
}

void DrawGroupedPanels(ItemTrackerWindow::ItemTrackerPanel window) {
    for (auto& panel : panelList) {
        int16_t index = -1;
        if (panel.panelId != window.panelId) {
            continue;
        }

        ImGui::PushID(panel.panelId);

        DrawPanelItems(panel, index);

        ImGui::PopID();
    }
}

void DrawSeparatePanels(ItemTrackerWindow::ItemTrackerPanel panel) {
    int16_t index = -1;
    ImGui::PushID(panel.panelId);
    ImGui::Begin(panel.panelName, 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

    DrawPanelItems(panel, index);

    ImGui::End();
    ImGui::PopID();
}

void DrawItemTrackerWindowPanels() {
    for (auto& window : mainTrackerWindow) {
        if (mainTrackerWindow.size() == 0) {
            return;
        }
        ImGui::Begin("Main Tracker", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
        DrawGroupedPanels(window);
        ImGui::End();
    }
    for (auto& window : subTrackerWindow) {
        if (subTrackerWindow.size() == 0) {
            return;
        }

        ImGui::Begin("Sub Tracker", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
        DrawGroupedPanels(window);
        ImGui::End();
    }
    for (auto& window : separateTrackerWindow) {
        if (separateTrackerWindow.size() == 0) {
            return;
        }
        DrawSeparatePanels(window);
    }
}

void UpdateTrackerWindows() {
    mainTrackerWindow.clear();
    subTrackerWindow.clear();
    separateTrackerWindow.clear();

    for (auto& panel : panelList) {
        for (auto& option : itemTrackerPanelOptions) {
            if (panel.panelName != option.first) {
                continue;
            }
            switch (CVarGetInteger(option.second, 0)) {
                case SECTION_MAIN:
                    mainTrackerWindow.push_back(panel);
                    break;
                case SECTION_SUB:
                    subTrackerWindow.push_back(panel);
                    break;
                case SECTION_SEPARATE:
                    separateTrackerWindow.push_back(panel);
                    break;
                default:
                    break;
            }
        }
    }
}

void UpdateTrackerSettings() {
    iconSize = CVarGetInteger("ItemTracker.IconSize", 32) * 1.0f;
    bgOpacity = CVarGetInteger("ItemTracker.Background", 0) ? 0 : 0.5f;

    panelList[TRACKER_DUNGEON].panelWidth =
        CVarGetInteger("ItemTracker.MapCompass", 0) ? CVarGetInteger("ItemTracker.KeyDisplay", 0) ? 4 : 2 : 4;

    UpdateTrackerWindows();
}

void ItemTrackerWindow::Draw() {
    if (!IsVisible()) {
        return;
    }

    if (!isInitialized) {
        for (auto& panel : panelList) {
            panel.panelContents = createItemVector(panel);
        }
        UpdateTrackerWindows();
        UpdateTrackerSettings();
        isInitialized = true;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, bgOpacity));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    

    DrawItemTrackerWindowPanels();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(1);
}
