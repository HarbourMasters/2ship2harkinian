#include "ItemTracker.h"
#include "ItemTrackerSettings.h"

#include "2s2h/BenGui/UIWidgets.hpp"
#include "Rando/Rando.h"

#include "2s2h/ShipUtils.h"

extern "C" {
#include "z64save.h"
#include "variables.h"
#include <macros.h>
#include <functions.h>
#include "2s2h_assets.h"
}

namespace BenGui {
extern std::shared_ptr<ItemTrackerWindow> mItemTrackerWindow;
}

ImVec4 trackerWindowOpacity = ImVec4(0, 0, 0, 0.5f);

extern TrackerImageObject GetTextureObject(int16_t itemId, bool isRandoItem) {
    int16_t currentItemId = ITEM_NONE;
    int16_t bottleId = 0;

    if (isRandoItem) {
        TrackerImageObject randoImageObject;
        float alpha = 0.4f;
        randoImageObject.textureColor = ImVec4(1, 1, 1, 1);

        switch (itemId) {
            case RI_FROG_BLUE:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_33_01) ? 1 : 0.4f;
                break;
            case RI_FROG_CYAN:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_32_40) ? 1 : 0.4f;
                break;
            case RI_FROG_PINK:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_32_80) ? 1 : 0.4f;
                break;
            case RI_FROG_WHITE:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_33_02) ? 1 : 0.4f;
                break;
            case RI_OWL_CLOCK_TOWN_SOUTH:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_CLOCK_TOWN) ? 1 : 0.4f;
                break;
            case RI_OWL_GREAT_BAY_COAST:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_GREAT_BAY_COAST) ? 1 : 0.4f;
                break;
            case RI_OWL_IKANA_CANYON:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_IKANA_CANYON) ? 1 : 0.4f;
                break;
            case RI_OWL_MILK_ROAD:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_MILK_ROAD) ? 1 : 0.4f;
                break;
            case RI_OWL_MOUNTAIN_VILLAGE:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_MOUNTAIN_VILLAGE) ? 1 : 0.4f;
                break;
            case RI_OWL_SNOWHEAD:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_SNOWHEAD) ? 1 : 0.4f;
                break;
            case RI_OWL_SOUTHERN_SWAMP:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_SOUTHERN_SWAMP) ? 1 : 0.4f;
                break;
            case RI_OWL_STONE_TOWER:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_STONE_TOWER) ? 1 : 0.4f;
                break;
            case RI_OWL_WOODFALL:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_WOODFALL) ? 1 : 0.4f;
                break;
            case RI_OWL_ZORA_CAPE:
                alpha = GET_OWL_STATUE_ACTIVATED(OWL_WARP_ZORA_CAPE) ? 1 : 0.4f;
                break;
            case RI_SOUL_GOHT:
            case RI_SOUL_GYORG:
            case RI_SOUL_MAJORA:
            case RI_SOUL_ODOLWA:
            case RI_SOUL_TWINMOLD:
                alpha = Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_GOHT + (itemId - RI_SOUL_GOHT)) ? 1 : 0.4f;
                break;
            case RI_TINGLE_MAP_CLOCK_TOWN:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_CLOCK_TOWN) ? 1 : 0.4f;
                break;
            case RI_TINGLE_MAP_WOODFALL:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_WOODFALL) ? 1 : 0.4f;
                break;
            case RI_TINGLE_MAP_SNOWHEAD:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_SNOWHEAD) ? 1 : 0.4f;
                break;
            case RI_TINGLE_MAP_ROMANI_RANCH:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_ROMANI_RANCH) ? 1 : 0.4f;
                break;
            case RI_TINGLE_MAP_GREAT_BAY:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_GREAT_BAY) ? 1 : 0.4f;
                break;
            case RI_TINGLE_MAP_STONE_TOWER:
                alpha = CHECK_WEEKEVENTREG(WEEKEVENTREG_TINGLE_MAP_BOUGHT_STONE_TOWER) ? 1 : 0.4f;
                break;
            case RI_TRIFORCE_PIECE:
                alpha = gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces > 0 ? 1 : 0.4f;
                break;
            default:
                break;
        }
        randoImageObject.textureColor.w = alpha;

        randoImageObject.textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)Rando::StaticData::GetIconTexturePath((RandoItemId)itemId));
        randoImageObject.textureDimensions =
            ImVec2(46.0f, itemId >= RI_OWL_CLOCK_TOWN_SOUTH && itemId <= RI_OWL_ZORA_CAPE ? 32.0f : 46.0f);

        return randoImageObject;
    } else {
        if (itemId >= ITEM_BOTTLE_1 && itemId <= ITEM_BOTTLE_6) {
            bottleId = SLOT_BOTTLE_1 + (itemId - ITEM_BOTTLE_1);
            currentItemId = ITEM_BOTTLE;
        }
        if (itemId == ITEM_SKULL_TOKEN_SWAMP || itemId == ITEM_SKULL_TOKEN_OCEAN) {
            currentItemId = ITEM_SKULL_TOKEN;
        }
        if (itemId >= ITEM_WOODFALL_STRAY_FAIRY && itemId <= ITEM_STONE_TOWER_STRAY_FAIRY) {
            currentItemId = ITEM_STRAY_FAIRIES;
        }
        if (itemId >= ITEM_WOODFALL_DUNGEON_MAP && itemId <= ITEM_STONE_TOWER_KEY_BOSS) {
            const int dungeonIndex = (itemId - ITEM_WOODFALL_DUNGEON_MAP) / 4;
            const int itemTypeIndex = (itemId - ITEM_WOODFALL_DUNGEON_MAP) % 4;

            switch (itemTypeIndex) {
                case 0:
                    currentItemId = ITEM_DUNGEON_MAP;
                    break;
                case 1:
                    currentItemId = ITEM_COMPASS;
                    break;
                case 2:
                    currentItemId = ITEM_KEY_SMALL;
                    break;
                case 3:
                    currentItemId = ITEM_KEY_BOSS;
                    break;
            }
        }
    }

    if (currentItemId == ITEM_NONE) {
        switch (itemId) {
            case ITEM_MOONS_TEAR:
                currentItemId = ITEM_MOONS_TEAR;
                for (int16_t i = ITEM_MOONS_TEAR; i <= ITEM_DEED_OCEAN; i++) {
                    if (gSaveContext.save.saveInfo.inventory.items[SLOT_TRADE_DEED] == i) {
                        currentItemId = i;
                        break;
                    }
                }
                break;
            case ITEM_ROOM_KEY:
                currentItemId = ITEM_ROOM_KEY;
                for (int16_t i = ITEM_ROOM_KEY; i <= ITEM_LETTER_MAMA; i++) {
                    if (gSaveContext.save.saveInfo.inventory.items[SLOT_TRADE_KEY_MAMA] == i) {
                        currentItemId = i;
                        break;
                    }
                }
                break;
            case ITEM_LETTER_TO_KAFEI:
                currentItemId = ITEM_LETTER_TO_KAFEI;
                for (int16_t i = ITEM_LETTER_TO_KAFEI; i <= ITEM_PENDANT_OF_MEMORIES; i++) {
                    if (gSaveContext.save.saveInfo.inventory.items[SLOT_TRADE_COUPLE] == i) {
                        currentItemId = i;
                        break;
                    }
                }
                break;
            case ITEM_SWORD_KOKIRI:
                currentItemId = ITEM_SWORD_KOKIRI + (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI);
                if (currentItemId < ITEM_SWORD_KOKIRI) {
                    currentItemId = ITEM_SWORD_KOKIRI;
                }
                break;
            case ITEM_SHIELD_HERO:
                currentItemId = ITEM_SHIELD_HERO + (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) - EQUIP_VALUE_SHIELD_HERO);
                if (currentItemId < ITEM_SHIELD_HERO) {
                    currentItemId = ITEM_SHIELD_HERO;
                }
                break;
            case ITEM_WALLET_ADULT:
                currentItemId = ITEM_WALLET_ADULT + CUR_UPG_VALUE(UPG_WALLET) - 1;
                if (currentItemId < ITEM_WALLET_ADULT) {
                    currentItemId = ITEM_WALLET_ADULT;
                }
                break;
            case ITEM_MAGIC_JAR_SMALL:
                if (gSaveContext.save.saveInfo.playerData.magicLevel <= 1) {
                    currentItemId = ITEM_MAGIC_JAR_SMALL;
                } else {
                    currentItemId = ITEM_MAGIC_JAR_BIG;
                }
                break;
            default:
                currentItemId = itemId;
                break;
        }
    }

    TrackerImageObject imageObject = {
        .textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[currentItemId]),
        .textureColor = Ship_GetItemColorTint(currentItemId),
        .textureDimensions =
            ImVec2(currentItemId >= ITEM_SONG_SONATA && currentItemId <= ITEM_SONG_SUN ? 46.0f / 1.5f : 46.0f, 46.0f),
    };

    if (itemId >= ITEM_REMAINS_ODOLWA && itemId <= ITEM_BOMBERS_NOTEBOOK) {
        imageObject.textureColor.w = CHECK_QUEST_ITEM(Ship_ConvertItemIdToQuest(itemId)) ? 1 : 0.4f;
    } else if (itemId >= ITEM_SWORD_KOKIRI && itemId <= ITEM_SWORD_GILDED) {
        imageObject.textureColor.w = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) >= EQUIP_VALUE_SWORD_KOKIRI ? 1 : 0.4f;
    } else if (itemId == ITEM_SHIELD_HERO || itemId == ITEM_SHIELD_MIRROR) {
        imageObject.textureColor.w = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) >= EQUIP_VALUE_SHIELD_HERO ? 1 : 0.4f;
    } else if (itemId == ITEM_WALLET_ADULT || itemId == ITEM_WALLET_GIANT) {
        imageObject.textureColor.w = CUR_UPG_VALUE(UPG_WALLET) >= 1 ? 1 : 0.4f;
    } else if (itemId == ITEM_MAGIC_JAR_SMALL || itemId == ITEM_MAGIC_JAR_BIG) {
        imageObject.textureColor.w = gSaveContext.save.saveInfo.playerData.magicLevel != 0 ? 1 : 0.4f;
    } else if (itemId == ITEM_HEART_CONTAINER) {
        imageObject.textureColor.w = gSaveContext.save.saveInfo.playerData.doubleDefense ? 1 : 0.4f;
    } else if (itemId >= ITEM_BOTTLE_1 && itemId <= ITEM_BOTTLE_6) {
        if (gSaveContext.save.saveInfo.inventory.items[bottleId] != ITEM_NONE && gPlayState) {
            imageObject.textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[gSaveContext.save.saveInfo.inventory.items[bottleId]]);
        } else {
            imageObject.textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[ITEM_BOTTLE]);
        }
        imageObject.textureColor.w = gSaveContext.save.saveInfo.inventory.items[bottleId] != ITEM_NONE ? 1 : 0.4f;
    } else if (itemId == ITEM_SKULL_TOKEN_SWAMP || itemId == ITEM_SKULL_TOKEN_OCEAN) {
        uint32_t tokenCount = 0;
        if (itemId == ITEM_SKULL_TOKEN_SWAMP) {
            tokenCount = Inventory_GetSkullTokenCount(SCENE_KINSTA1);
        } else {
            tokenCount = Inventory_GetSkullTokenCount(SCENE_KINDAN2);
        }
        imageObject.textureColor.w = tokenCount > 0 ? 1 : 0.4f;
    } else if (itemId >= ITEM_WOODFALL_STRAY_FAIRY && itemId <= ITEM_STONE_TOWER_STRAY_FAIRY) {
        imageObject.textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)fairyIconTextures[itemId - ITEM_WOODFALL_STRAY_FAIRY]);
        imageObject.textureColor.w =
            gSaveContext.save.saveInfo.inventory.strayFairies[itemId - ITEM_WOODFALL_STRAY_FAIRY] > 0 ? 1 : 0.4f;
    } else if (itemId >= ITEM_WOODFALL_DUNGEON_MAP && itemId <= ITEM_STONE_TOWER_KEY_BOSS) {
        const int dungeonIndex = (itemId - ITEM_WOODFALL_DUNGEON_MAP) / 4;
        const int itemTypeIndex = (itemId - ITEM_WOODFALL_DUNGEON_MAP) % 4;
        switch (itemTypeIndex) {
            case 0:
                imageObject.textureColor.w = CHECK_DUNGEON_ITEM(DUNGEON_COMPASS, dungeonIndex) ? 1.0f : 0.4f;
                break;
            case 1:
                imageObject.textureColor.w = CHECK_DUNGEON_ITEM(DUNGEON_MAP, dungeonIndex) ? 1.0f : 0.4f;
                break;
            case 2:
                imageObject.textureColor.w = DUNGEON_KEY_COUNT(dungeonIndex) > 0 ? 1.0f : 0.4f;
                break;
            case 3:
                imageObject.textureColor.w = CHECK_DUNGEON_ITEM(DUNGEON_BOSS_KEY, dungeonIndex) ? 1.0f : 0.4f;
                break;
        }
    } else {
        imageObject.textureColor.w = INV_CONTENT(itemId) != ITEM_NONE ? 1 : 0.4f;
    }

    return imageObject;
}

void DrawItemSlot(int16_t itemId, float scale, bool isRandoItem) {
    TrackerImageObject imageObject = GetTextureObject(itemId, isRandoItem);
    ImGui::Image(imageObject.textureId,
                 ImVec2(imageObject.textureDimensions.x * scale, imageObject.textureDimensions.y * scale), ImVec2(0, 0),
                 ImVec2(1, 1), imageObject.textureColor, ImVec4(0, 0, 0, 0));
    UIWidgets::Tooltip(GetItemTrackerItemName(itemId, isRandoItem).c_str());
}

void DrawItemWindowList(TrackerItemListObject windowObject, bool isRandoItem) {
    int columns = windowObject.columnLength;
    if (windowObject.itemList.size() < windowObject.columnLength) {
        columns = windowObject.itemList.size();
    }

    if (ImGui::BeginTable(windowObject.windowName.c_str(), columns)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));

        for (auto& item : windowObject.itemList) {
            ImGui::TableNextColumn();
            ImVec2 framePadding = ImVec2(item >= ITEM_SONG_SONATA && item <= ITEM_SONG_SUN ? 8.0f : 0, 0);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
            DrawItemSlot(item, windowObject.windowScale, isRandoItem);
            ImGui::PopStyleVar(1);
        }

        ImGui::PopStyleVar(1);
        ImGui::EndTable();
    }
}

void ItemTrackerWindow::Draw() {
    if (!IsVisible()) {
        return;
    }

    if (!gPlayState) {
        return;
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    if (!CVarGetInteger("gSettings.ItemTracker.WindowType", 0)) {
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking;
    }
    shouldWindowSplit = CVarGetInteger("gSettings.ItemTracker.WindowGroup", 0);

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);

    std::vector<std::vector<TrackerItemListObject>*> windows = {
        &BenGui::mItemTrackerWindow->namedItemWindows,
        &BenGui::mItemTrackerWindow->randoItemWindows,
    };

    uint32_t windowIndex = TRACKER_MAIN;
    for (auto* window : windows) {
        bool singleWindowOpen = false;
        if (!shouldWindowSplit) {
            ImVec4 mainBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            mainBg.w = BenGui::mItemTrackerWindow->namedItemWindows[0].windowOpacity;
            ImGui::PushStyleColor(ImGuiCol_WindowBg, mainBg);
            singleWindowOpen =
                ImGui::Begin(windowIndex == TRACKER_MAIN ? "Main Tracker" : "Rando Tracker", nullptr, windowFlags);
        }

        uint32_t index = 0;
        for (auto& object : *window) {
            if (object.itemList.empty()) {
                index++;
                continue;
            }

            bool isWindowOpen = false;

            if (shouldWindowSplit) {
                ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                bg.w = object.windowOpacity;
                ImGui::PushStyleColor(ImGuiCol_WindowBg, bg);
                std::string windowName = std::string(object.windowName) + "##" + std::to_string(index);
                isWindowOpen = ImGui::Begin(windowName.c_str(), nullptr, windowFlags);
            } else {
                isWindowOpen = singleWindowOpen;
            }

            if (isWindowOpen) {
                DrawItemWindowList(object, windowIndex == TRACKER_MAIN ? false : true);
            }

            if (shouldWindowSplit) {
                ImGui::PopStyleColor(1);
                ImGui::End();
            }

            index++;
        }
        if (!shouldWindowSplit) {
            ImGui::PopStyleColor(1);
            ImGui::End();
        }
        windowIndex++;
    }

    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(1);
}

void ItemTrackerWindow::InitElement() {
}

void ItemTrackerWindow::DrawElement() {
}
