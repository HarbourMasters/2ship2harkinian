#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include <z64.h>
#include <macros.h>
#include <global.h>
#include <functions.h>
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

static std::vector<uint32_t> availableArrows;
static uint32_t nextArrow;

void ChangeItemAction(Player* player, uint32_t item) {
    uint32_t itemAction = (item - PLAYER_IA_MASK_ROMANI);

    if (itemAction != player->heldItemAction) {
        player->itemAction = player->heldItemAction = itemAction;
    }
}

void NextArrowInSlot(uint32_t item) {
    bool continueLoop = true;
    availableArrows.clear();

    for (int i = ITEM_ARROW_FIRE; i <= ITEM_ARROW_LIGHT; i++) {
        if (GET_INV_CONTENT(i) == i) {
            availableArrows.push_back(ITEM_MASK_SCENTS + i);
        }
    }
    for (int a = item; a <= ITEM_BOW_LIGHT; a++) {
        uint32_t loopCounter = 0;
        if (continueLoop == true) {
            for (auto& str : availableArrows) {
                if (continueLoop == true) {
                    if (a == availableArrows[loopCounter]) {
                        nextArrow = availableArrows[loopCounter];
                        continueLoop = false;
                    }
                }
                loopCounter++;
            }
        }
    }
    if (continueLoop == true) {
        nextArrow = availableArrows[0];
        continueLoop = false;
    }
}

void RegisterCycleMagicArrows() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPassPlayerInputs>([](Input* input) {
        if (gPlayState == NULL) {
            return;
        }
        if (gSaveContext.save.playerForm != PLAYER_FORM_HUMAN) {
            return;
        }

        if (CVarGetInteger("gEnhancements.Equipment.CycleMagicArrows", 0)) {
            Player* player = GET_PLAYER(gPlayState);

            if (player->stateFlags1 & PLAYER_STATE1_100000) {
                if (CHECK_BTN_ALL(input->press.button, BTN_L)) {
                    bool shouldChange = true;
                    for (int i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                        for (int v = ITEM_BOW_FIRE; v <= ITEM_BOW_LIGHT; v++) {
                            if (gSaveContext.save.saveInfo.equips.buttonItems[0][i] == v && shouldChange == true) {
                                if (v == ITEM_BOW_LIGHT) {
                                    NextArrowInSlot(ITEM_BOW_FIRE);
                                    BUTTON_ITEM_EQUIP(0, i) = nextArrow;
                                    Interface_LoadItemIconImpl(gPlayState, i);
                                    shouldChange = false;
                                } else {
                                    NextArrowInSlot((ItemId)v + 1);
                                    BUTTON_ITEM_EQUIP(0, i) = nextArrow;
                                    Interface_LoadItemIconImpl(gPlayState, i);
                                    shouldChange = false;
                                }
                                ChangeItemAction(player, BUTTON_ITEM_EQUIP(0, i));
                            }
                        }
                    }
                    if (shouldChange == true) {
                        for (int i = EQUIP_SLOT_D_RIGHT; i <= EQUIP_SLOT_D_UP; i++) {
                            for (int v = ITEM_BOW_FIRE; v <= ITEM_BOW_LIGHT; v++) {
                                if (gSaveContext.save.shipSaveInfo.dpadEquips.dpadItems[0][i] == v &&
                                    shouldChange == true) {
                                    if (v == ITEM_BOW_LIGHT) {
                                        NextArrowInSlot(ITEM_BOW_FIRE);
                                        DPAD_BUTTON_ITEM_EQUIP(0, i) = nextArrow;
                                        Interface_Dpad_LoadItemIconImpl(gPlayState, i);
                                        shouldChange = false;
                                    } else {
                                        NextArrowInSlot((ItemId)v + 1);
                                        DPAD_BUTTON_ITEM_EQUIP(0, i) = nextArrow;
                                        Interface_Dpad_LoadItemIconImpl(gPlayState, i);
                                        shouldChange = false;
                                    }
                                    ChangeItemAction(player, DPAD_BUTTON_ITEM_EQUIP(0, i));
                                }
                            }
                        }
                    }
                }
            }
        }
    });
}
