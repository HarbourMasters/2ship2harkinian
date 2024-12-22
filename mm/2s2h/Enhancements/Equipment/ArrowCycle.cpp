#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "z64player.h"
#include "z64item.h"
#include "z64interface.h"
#include "z64save.h"
#include "z64.h"

extern "C" {
#include "macros.h"
#include "variables.h"
#include "functions.h"
u8 sMagicArrowCosts[];
void Player_InitItemAction(PlayState* play, Player* thisx, PlayerItemAction itemAction);
}

#define CVAR_NAME "gEnhancements.PlayerActions.ArrowCycle"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// Check if player is holding a bow
static bool holdingBow(Player* player) {
    return player->heldItemAction == PLAYER_IA_BOW || player->heldItemAction == PLAYER_IA_BOW_FIRE ||
           player->heldItemAction == PLAYER_IA_BOW_ICE || player->heldItemAction == PLAYER_IA_BOW_LIGHT;
}

// Check if player is holding a magic arrow
static bool holdingMagicArrow(Player* player) {
    return player->heldItemAction == PLAYER_IA_BOW_FIRE || player->heldItemAction == PLAYER_IA_BOW_ICE ||
           player->heldItemAction == PLAYER_IA_BOW_LIGHT;
}

// Get the next arrow type when cycling
static s8 nextArrowType(s8 currentArrowType) {
    switch (currentArrowType) {
        case PLAYER_IA_BOW:
            return PLAYER_IA_BOW_FIRE;
        case PLAYER_IA_BOW_FIRE:
            return PLAYER_IA_BOW_ICE;
        case PLAYER_IA_BOW_ICE:
            return PLAYER_IA_BOW_LIGHT;
        case PLAYER_IA_BOW_LIGHT:
            return PLAYER_IA_BOW;
        default:
            return PLAYER_IA_BOW;
    }
}

// Get the bow item for an arrow type
static s32 bowItemForArrow(PlayerItemAction arrowType) {
    switch (arrowType) {
        case PLAYER_IA_BOW_FIRE:
            return ITEM_BOW_FIRE;
        case PLAYER_IA_BOW_ICE:
            return ITEM_BOW_ICE;
        case PLAYER_IA_BOW_LIGHT:
            return ITEM_BOW_LIGHT;
        default:
            return ITEM_BOW;
    }
}

// Update equipped bow item
static void updateEquippedBow(PlayState* play, s8 arrowType) {
    s32 bowItem = bowItemForArrow(static_cast<PlayerItemAction>(arrowType));

    for (s32 i = 0; i < 3; i++) {
        if ((BUTTON_ITEM_EQUIP(0, i) == ITEM_BOW) ||
            (BUTTON_ITEM_EQUIP(0, i) >= ITEM_BOW_FIRE && BUTTON_ITEM_EQUIP(0, i) <= ITEM_BOW_LIGHT)) {
            BUTTON_ITEM_EQUIP(0, i) = bowItem;
            C_SLOT_EQUIP(0, i) = SLOT_BOW;
            Interface_LoadItemIcon(play, i);
            gSaveContext.buttonStatus[i] = BTN_ENABLED;
        }
    }

    for (s32 i = 0; i < 4; i++) {
        if ((DPAD_BUTTON_ITEM_EQUIP(0, i) == ITEM_BOW) ||
            (DPAD_BUTTON_ITEM_EQUIP(0, i) >= ITEM_BOW_FIRE && DPAD_BUTTON_ITEM_EQUIP(0, i) <= ITEM_BOW_LIGHT)) {
            DPAD_BUTTON_ITEM_EQUIP(0, i) = bowItem;
            DPAD_SLOT_EQUIP(0, i) = SLOT_BOW;
            Interface_Dpad_LoadItemIcon(play, i);
            gSaveContext.shipSaveContext.dpad.status[i] = BTN_ENABLED;
        }
    }
}

void RegisterArrowCycle() {
    // Prevent minimap toggle when holding bow
    COND_VB_SHOULD(VB_MINIMAP_TOGGLE, CVAR, {
        if (CVAR && gPlayState != NULL) {
            Player* player = GET_PLAYER(gPlayState);
            if (holdingBow(player)) {
                *should = false;
            }
        }
    });

    // Prevent magic consumption for magic arrows
    COND_VB_SHOULD(VB_MAGIC_ARROW_CONSUME, CVAR, {
        if (CVAR && gPlayState != NULL) {
            Player* player = GET_PLAYER(gPlayState);
            if (holdingMagicArrow(player)) {
                *should = false;
            }
        }
    });

    // Handle magic consumption when arrow is fired instead of on draw
    COND_HOOK(OnGameStateUpdate, CVAR, []() {
        if (gPlayState == nullptr)
            return;

        Player* player = GET_PLAYER(gPlayState);
        static bool wasHoldingArrow = false;
        static s32 lastArrowType = PLAYER_IA_BOW;
        bool isHoldingArrow = (player->heldActor != NULL);

        // Update last arrow type when grabbing a new arrow
        if (isHoldingArrow && !wasHoldingArrow) {
            lastArrowType = player->heldItemAction;
        }

        // Consume magic when releasing a magic arrow
        if (wasHoldingArrow && !isHoldingArrow) {
            if (lastArrowType == player->heldItemAction && holdingMagicArrow(player)) {
                u8 magicCost = sMagicArrowCosts[player->heldItemAction - PLAYER_IA_BOW_FIRE];
                if (gSaveContext.save.saveInfo.playerData.magic >= magicCost) {
                    gSaveContext.save.saveInfo.playerData.magic -= magicCost;
                    if (gSaveContext.magicState == MAGIC_STATE_IDLE) {
                        gSaveContext.magicState = MAGIC_STATE_CONSUME_SETUP;
                    }
                }
            }
        }
        wasHoldingArrow = isHoldingArrow;
    });

    // Handle arrow cycling when L is pressed
    COND_HOOK(OnGameStateUpdate, CVAR, []() {
        if (gPlayState == nullptr)
            return;

        Player* player = GET_PLAYER(gPlayState);
        Input* input = CONTROLLER1(&gPlayState->state);

        if (holdingBow(player) && CHECK_BTN_ALL(input->press.button, BTN_L)) {
            // Cycle to the next arrow type
            s8 nextArrow = nextArrowType(player->heldItemAction);

            // Kill the original arrow actor
            if (player->heldActor != NULL) {
                Actor_Kill(player->heldActor);
                player->heldActor = NULL;
            }

            // Reset magic state if switching to a magic arrow
            if (nextArrow >= PLAYER_IA_BOW_FIRE && nextArrow <= PLAYER_IA_BOW_LIGHT) {
                if (gSaveContext.magicState != MAGIC_STATE_IDLE) {
                    gSaveContext.magicState = MAGIC_STATE_IDLE;
                }
            }

            player->heldItemAction = nextArrow;

            Player_InitItemAction(gPlayState, player, static_cast<PlayerItemAction>(nextArrow));

            updateEquippedBow(gPlayState, nextArrow);
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterArrowCycle, { CVAR_NAME });
