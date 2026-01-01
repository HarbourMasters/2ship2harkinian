#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "z64.h"
#include "functions.h"
#include "macros.h"
#include "variables.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
}

#define CVAR_NAME "gEnhancements.Equipment.ItemUnequip"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterItemUnequip() {
    COND_VB_SHOULD(VB_KALEIDO_EQUIP_ITEM_TO_BUTTON, CVAR, {
        PlayState* play = va_arg(args, PlayState*);
        u16 cursorSlot = va_arg(args, int);
        u16 cursorItem = va_arg(args, int);

        PauseContext* pauseCtx = &play->pauseCtx;
        s32 targetSlot = -1;

        SPDLOG_INFO("ItemUnequip: equipTargetCBtn = {}, cursorItem = {}, cursorSlot = {}", 
                    pauseCtx->equipTargetCBtn, cursorItem, cursorSlot);

        // Determine which button was pressed based on equipTargetCBtn
        switch (pauseCtx->equipTargetCBtn) {
            case PAUSE_EQUIP_C_LEFT:
                targetSlot = EQUIP_SLOT_C_LEFT;
                break;
            case PAUSE_EQUIP_C_DOWN:
                targetSlot = EQUIP_SLOT_C_DOWN;
                break;
            case PAUSE_EQUIP_C_RIGHT:
                targetSlot = EQUIP_SLOT_C_RIGHT;
                break;
            case PAUSE_EQUIP_D_RIGHT:
                targetSlot = EQUIP_SLOT_D_RIGHT;
                break;
            case PAUSE_EQUIP_D_LEFT:
                targetSlot = EQUIP_SLOT_D_LEFT;
                break;
            case PAUSE_EQUIP_D_DOWN:
                targetSlot = EQUIP_SLOT_D_DOWN;
                break;
            case PAUSE_EQUIP_D_UP:
                targetSlot = EQUIP_SLOT_D_UP;
                break;
            default:
                return;
        }

        u8 equippedItem;
        u8 equippedSlot;
        bool shouldUnequip = false;

        // C-buttons
        if (targetSlot >= EQUIP_SLOT_C_LEFT && targetSlot <= EQUIP_SLOT_C_RIGHT) {
            equippedItem = BUTTON_ITEM_EQUIP(0, targetSlot);
            equippedSlot = C_SLOT_EQUIP(0, targetSlot);
        }
        // D-pad
        else {
            equippedItem = DPAD_BUTTON_ITEM_EQUIP(0, targetSlot);
            equippedSlot = DPAD_SLOT_EQUIP(0, targetSlot);
        }

        SPDLOG_INFO("ItemUnequip: targetSlot = {}, equippedItem = {}, equippedSlot = {}", 
                    targetSlot, equippedItem, equippedSlot);

        // Check if we should unequip
        if (equippedItem == cursorItem) {
            // For bottles, we need to check the slot too (since there are multiple bottle items)
            if (cursorItem >= ITEM_BOTTLE && cursorItem <= ITEM_OBABA_DRINK) {
                if (equippedSlot == cursorSlot) {
                    shouldUnequip = true;
                }
            } else {
                shouldUnequip = true;
            }
        }
        // Handle magic arrows (bow variants)
        else if (cursorItem == ITEM_ARROW_FIRE && equippedItem == ITEM_BOW_FIRE) {
            shouldUnequip = true;
        } else if (cursorItem == ITEM_ARROW_ICE && equippedItem == ITEM_BOW_ICE) {
            shouldUnequip = true;
        } else if (cursorItem == ITEM_ARROW_LIGHT && equippedItem == ITEM_BOW_LIGHT) {
            shouldUnequip = true;
        }

        if (shouldUnequip) {
            SPDLOG_INFO("ItemUnequip: UNEQUIPPING from slot {}", targetSlot);
            // C-buttons
            if (targetSlot >= EQUIP_SLOT_C_LEFT && targetSlot <= EQUIP_SLOT_C_RIGHT) {
                BUTTON_ITEM_EQUIP(0, targetSlot) = ITEM_NONE;
                C_SLOT_EQUIP(0, targetSlot) = SLOT_NONE;
                Interface_LoadItemIconImpl(play, targetSlot);
            }
            // D-pad
            else {
                DPAD_BUTTON_ITEM_EQUIP(0, targetSlot) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, targetSlot) = SLOT_NONE;
                Interface_Dpad_LoadItemIconImpl(play, targetSlot);
            }

            Audio_PlaySfx(NA_SE_SY_DECIDE);
            *should = false;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterItemUnequip, { CVAR_NAME });