#include "ItemTracker.h"

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

float defaultImageSize = 32.0f;

TrackerImageObject GetTextureIDBySlot(InventorySlot slot) {
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);
    TrackerImageObject imageObject;

    switch (slot) {
        case SLOT_TRADE_DEED:
            currentItemId = ITEM_MOONS_TEAR;
            for (int i = ITEM_MOONS_TEAR; i <= ITEM_DEED_OCEAN; i++) {
                if (gSaveContext.save.saveInfo.inventory.items[slot] == i) {
                    currentItemId = (ItemId)i;
                    break;
                }
            }
            break;
        case SLOT_TRADE_KEY_MAMA:
            currentItemId = ITEM_ROOM_KEY;
            for (int i = ITEM_ROOM_KEY; i <= ITEM_LETTER_MAMA; i++) {
                if (gSaveContext.save.saveInfo.inventory.items[slot] == i) {
                    currentItemId = (ItemId)i;
                    break;
                }
            }
            break;
        case SLOT_TRADE_COUPLE:
            currentItemId = ITEM_LETTER_TO_KAFEI;
            for (int i = ITEM_LETTER_TO_KAFEI; i <= ITEM_PENDANT_OF_MEMORIES; i++) {
                if (gSaveContext.save.saveInfo.inventory.items[slot] == i) {
                    currentItemId = (ItemId)i;
                    break;
                }
            }
            break;
        default:
            break;
    }

    if (currentItemId != ITEM_NONE) {
        imageObject.textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[currentItemId]);
    } else {
        if (slot >= SLOT_BOTTLE_1 && slot <= SLOT_BOTTLE_6) {
            imageObject.textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                (const char*)gItemIcons[ITEM_BOTTLE]);
        } else if (slot >= SLOT_MASK_POSTMAN && slot <= SLOT_MASK_FIERCE_DEITY) {
            for (int i = 0; i < sizeof(gItemSlots); i++) {
                if (gItemSlots[i] == slot) {
                    imageObject.textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                        (const char*)gItemIcons[i]);
                    break;
                }
            }
        } else {
            imageObject.textureId =
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName((const char*)gItemIcons[slot]);
        }
    }

    imageObject.fade = gSaveContext.save.saveInfo.inventory.items[slot] != ITEM_NONE ? 1.0f : 0.5f;

    return imageObject;
}

void DrawItemSlot(InventorySlot slot) {
    TrackerImageObject imageObject = GetTextureIDBySlot(slot);
    ImGui::Image(imageObject.textureId, ImVec2(defaultImageSize, defaultImageSize), ImVec2(0, 0), ImVec2(1, 1),
                 ImVec4(1, 1, 1, imageObject.fade), ImVec4(0, 0, 0, 0));
}

void DrawInventory() {
    if (ImGui::Begin("Inventory Item Tracker", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::BeginTable("Inventory Tracker", 6)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));

            for (int i = SLOT_OCARINA; i <= SLOT_BOTTLE_6; i++) {
                InventorySlot slot = static_cast<InventorySlot>(i);
                ImGui::TableNextColumn();
                DrawItemSlot(slot);
            }

            ImGui::PopStyleVar(2);
            ImGui::EndTable();
        }
        ImGui::End();
    }
}

void DrawMasks() {
    if (ImGui::Begin("Mask Item Tracker", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)) {
        if (ImGui::BeginTable("Mask Tracker", 6)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));

            for (int i = SLOT_MASK_POSTMAN; i <= SLOT_MASK_FIERCE_DEITY; i++) {
                InventorySlot slot = static_cast<InventorySlot>(i);
                ImGui::TableNextColumn();
                DrawItemSlot(slot);
            }

            ImGui::PopStyleVar(2);
            ImGui::EndTable();
        }
        ImGui::End();
    }
}

void ItemTrackerWindow::Draw() {
    // if (!IsVisible()) {
    //     return;
    // }

    if (!gPlayState) {
        return;
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    DrawInventory();
    DrawMasks();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(1);
}
void ItemTrackerWindow::InitElement() {
}

void ItemTrackerWindow::DrawElement() {
}
