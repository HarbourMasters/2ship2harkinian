#include "ItemTracker.h"
#include "ItemTrackerSettings.h"

#include "2s2h/BenGui/UIWidgets.hpp"
#include "Rando/Rando.h"

#include "2s2h/ShipUtils.h"
#include "interface/parameter_static/parameter_static.h"
#include "assets/archives/icon_item_static/icon_item_static_yar.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "archives/icon_item_24_static/icon_item_24_static_yar.h"

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

extern TrackerImageObject GetTextureObject(int16_t itemId) {
    int16_t currentItemId = itemId;

    switch (itemId) {
        case ITEM_MOONS_TEAR:
            for (int16_t i = ITEM_MOONS_TEAR; i <= ITEM_DEED_OCEAN; i++) {
                if (gSaveContext.save.saveInfo.inventory.items[SLOT_TRADE_DEED] == i) {
                    currentItemId = i;
                    break;
                }
            }
            break;
        case ITEM_ROOM_KEY:
            for (int16_t i = ITEM_ROOM_KEY; i <= ITEM_LETTER_MAMA; i++) {
                if (gSaveContext.save.saveInfo.inventory.items[SLOT_TRADE_KEY_MAMA] == i) {
                    currentItemId = i;
                    break;
                }
            }
            break;
        case ITEM_LETTER_TO_KAFEI:
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
            currentItemId = ITEM_WALLET_ADULT + (CUR_UPG_VALUE(UPG_WALLET) - ITEM_WALLET_ADULT);
            if (currentItemId < ITEM_WALLET_ADULT) {
                currentItemId = ITEM_WALLET_ADULT;
            }
            break;
        default:
            break;
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
    } else {
        imageObject.textureColor.w = INV_CONTENT(itemId) != ITEM_NONE ? 1 : 0.4f;
    }
    return imageObject;
}

void DrawItemSlot(int16_t itemId, float scale) {
    TrackerImageObject imageObject = GetTextureObject(itemId);
    ImGui::Image(imageObject.textureId,
                 ImVec2(imageObject.textureDimensions.x * scale, imageObject.textureDimensions.y * scale), ImVec2(0, 0),
                 ImVec2(1, 1), imageObject.textureColor, ImVec4(0, 0, 0, 0));
}

void DrawItemWindowList(TrackerItemListObject windowObject) {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, windowObject.windowOpacity));
    int columns = windowObject.columnLength;
    if (windowObject.itemList.size() < windowObject.columnLength) {
        columns = windowObject.itemList.size();
    }
    if (ImGui::Begin(windowObject.windowName.c_str(), nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::BeginTable(windowObject.windowName.c_str(), columns)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));

            for (auto& item : windowObject.itemList) {
                ImGui::TableNextColumn();
                ImVec2 framePadding = ImVec2(item >= ITEM_SONG_SONATA && item <= ITEM_SONG_SUN ? 8.0f : 0, 0);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);
                DrawItemSlot(item, windowObject.windowScale);
                ImGui::PopStyleVar(1);
            }

            ImGui::PopStyleVar(1);
            ImGui::EndTable();
        }
        ImGui::End();
    }
    ImGui::PopStyleColor(1);
}

void ItemTrackerWindow::Draw() {
    // if (!IsVisible()) {
    //     return;
    // }

    if (!gPlayState) {
        return;
    }
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);

    if (BenGui::mItemTrackerWindow->mainItemWindow.itemList.size() != 0) {
        DrawItemWindowList(BenGui::mItemTrackerWindow->mainItemWindow);
    }

    uint32_t index = 0;
    for (auto& object : BenGui::mItemTrackerWindow->namedItemWindows) {
        if (object.itemList.size() == 0) {
            continue;
            index++;
        }
        DrawItemWindowList(object);
        index++;
    }
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(1);
}
void ItemTrackerWindow::InitElement() {
}

void ItemTrackerWindow::DrawElement() {
}
