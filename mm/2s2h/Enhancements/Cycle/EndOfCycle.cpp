#include <variables.h>
#include <libultraship/libultraship.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include <overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h>
}

void RegisterEndOfCycleSaveHooks() {
    static u8 tempConsumable;
    static u8 tempRazorSword;
    static u8 tempRupees;
    static u8 tempItem;
    static u8 tempAmmo;
    static u8 tempItem2[255];
    static u8 tempAmmo2[255];
    static u8 tempBottle[6];

    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeEndOfCycleSave>([]() {
        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetRupees", 0)) {
            tempRupees = gSaveContext.save.saveInfo.playerData.rupees;
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetConsumables", 0)) {
            if (INV_CONTENT(ITEM_BOMB) == ITEM_BOMB) {
                tempItem = INV_CONTENT(ITEM_BOMB);
                tempAmmo = AMMO(tempItem);
            }

            if (INV_CONTENT(ITEM_DEKU_NUT) == ITEM_DEKU_NUT) {
                tempItem = INV_CONTENT(ITEM_DEKU_NUT);
                tempAmmo = AMMO(tempItem);
            }

            if (INV_CONTENT(ITEM_DEKU_STICK) == ITEM_DEKU_STICK) {
                tempItem = INV_CONTENT(ITEM_DEKU_STICK);
                tempAmmo = AMMO(tempItem);
            }

            if (INV_CONTENT(ITEM_BOW) == ITEM_BOW) {
                tempItem = INV_CONTENT(ITEM_BOW);
                tempAmmo = AMMO(tempItem);
            }

            for (int i = 0; i < ITEM_NUM_SLOTS; i++) {
                if (gAmmoItems[i] != ITEM_NONE) {
                    if ((gSaveContext.save.saveInfo.inventory.items[i] != ITEM_NONE) && (i != SLOT_PICTOGRAPH_BOX)) {
                        tempItem2[i] = gSaveContext.save.saveInfo.inventory.items[i];
                        tempAmmo2[i] = gSaveContext.save.saveInfo.inventory.ammo[i];
                    }
                }
            }
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetBottleContent", 0)) {
            for (int i = SLOT_BOTTLE_1; i <= SLOT_BOTTLE_6; i++) {
                tempBottle[i - SLOT_BOTTLE_1] = gSaveContext.save.saveInfo.inventory.items[i];
            }
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetRazorSword", 0) &&
            GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_RAZOR) {
            tempRazorSword = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_RAZOR;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::AfterEndOfCycleSave>([]() {
        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetRupees", 0)) {
            gSaveContext.save.saveInfo.playerData.rupees = tempRupees;
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetConsumables", 0)) {
            if (INV_CONTENT(ITEM_BOMB) == ITEM_BOMB) {
                INV_CONTENT(ITEM_BOMB)  = tempItem;
                AMMO(tempItem) = tempAmmo;
            }

            if (INV_CONTENT(ITEM_DEKU_NUT) == ITEM_DEKU_NUT) {
                INV_CONTENT(ITEM_DEKU_NUT) = tempItem;
                AMMO(tempItem) = tempAmmo;
            }

            if (INV_CONTENT(ITEM_DEKU_STICK) == ITEM_DEKU_STICK) {
                INV_CONTENT(ITEM_DEKU_STICK) = tempItem;
                AMMO(tempItem) = tempAmmo;
            }

            if (INV_CONTENT(ITEM_BOW) == ITEM_BOW) {
                INV_CONTENT(ITEM_BOW) = tempItem;
                AMMO(tempItem) = tempAmmo;
            }
            
            for (int i = 0; i < ITEM_NUM_SLOTS; i++) {
                if (gAmmoItems[i] != ITEM_NONE) {
                    if ((gSaveContext.save.saveInfo.inventory.items[i] != ITEM_NONE) && (i != SLOT_PICTOGRAPH_BOX)) {
                        gSaveContext.save.saveInfo.inventory.items[i] = tempItem2[i];
                        gSaveContext.save.saveInfo.inventory.ammo[i] = tempAmmo2[i];
                    }
                }
            }
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetBottleContent", 0)) {
            for (int i = SLOT_BOTTLE_1; i <= SLOT_BOTTLE_6; i++) {
                for (int j = EQUIP_SLOT_C_LEFT; j <= EQUIP_SLOT_C_RIGHT; j++) {
                    if (GET_CUR_FORM_BTN_ITEM(j) == ITEM_BOTTLE) {
                        SET_CUR_FORM_BTN_ITEM(j, tempBottle[i - SLOT_BOTTLE_1]);
                    }
                }
                gSaveContext.save.saveInfo.inventory.items[i] = tempBottle[i - SLOT_BOTTLE_1];
            }
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetRazorSword", 0) && tempRazorSword) {
            SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_RAZOR);
            CUR_FORM_EQUIP(EQUIP_SLOT_B) = ITEM_SWORD_RAZOR;
        }
    });
}
