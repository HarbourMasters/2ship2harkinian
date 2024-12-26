#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "macros.h"
#include "variables.h"
#include "functions.h"
void Player_InitItemAction(PlayState* play, Player* thisx, PlayerItemAction itemAction);
}

#define CVAR_NAME "gEnhancements.PlayerActions.ArrowCycle"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// The magic arrow costs from z_player.c
static u8 sArrowCycleCosts[] = {
    4, // ARROW_MAGIC_FIRE
    4, // ARROW_MAGIC_ICE
    8, // ARROW_MAGIC_LIGHT
};

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

// Returns true if the given arrow type is available in the player's inventory
static bool hasArrowType(PlayerItemAction arrowType) {
    switch (arrowType) {
        case PLAYER_IA_BOW:
            // Normal arrows are always considered available if we have the bow
            return true;
        case PLAYER_IA_BOW_FIRE:
            return (INV_CONTENT(ITEM_ARROW_FIRE) == ITEM_ARROW_FIRE);
        case PLAYER_IA_BOW_ICE:
            return (INV_CONTENT(ITEM_ARROW_ICE) == ITEM_ARROW_ICE);
        case PLAYER_IA_BOW_LIGHT:
            return (INV_CONTENT(ITEM_ARROW_LIGHT) == ITEM_ARROW_LIGHT);
        default:
            return false;
    }
}

// Get the next arrow type when cycling
static s8 nextArrowType(s8 currentArrowType) {
    // The order in which we cycle
    static const PlayerItemAction arrowOrder[] = {
        PLAYER_IA_BOW,
        PLAYER_IA_BOW_FIRE,
        PLAYER_IA_BOW_ICE,
        PLAYER_IA_BOW_LIGHT,
    };

    // Find the current arrow's position in the cycle
    int currentIndex = 0;
    for (int i = 0; i < (int)ARRAY_COUNT(arrowOrder); i++) {
        if (arrowOrder[i] == currentArrowType) {
            currentIndex = i;
            break;
        }
    }

    // Try each subsequent arrow in the cycle
    for (int offset = 1; offset <= (int)ARRAY_COUNT(arrowOrder); offset++) {
        int nextIndex = (currentIndex + offset) % ARRAY_COUNT(arrowOrder);
        if (hasArrowType(arrowOrder[nextIndex])) {
            return arrowOrder[nextIndex];
        }
    }

    // Fallback (should never happen if we have at least normal arrows)
    return PLAYER_IA_BOW;
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

// Check if arrow cycling should be allowed
static bool canCycleArrows() {
    // Check if player has bow
    if (INV_CONTENT(SLOT_BOW) != ITEM_BOW) {
        return false;
    }

    // Check if player has at least one magic arrow type
    return (INV_CONTENT(ITEM_ARROW_FIRE) == ITEM_ARROW_FIRE) || (INV_CONTENT(ITEM_ARROW_ICE) == ITEM_ARROW_ICE) ||
           (INV_CONTENT(ITEM_ARROW_LIGHT) == ITEM_ARROW_LIGHT);
}

void RegisterArrowCycle() {
    // Prevent minimap toggle when holding bow
    COND_VB_SHOULD(VB_MINIMAP_TOGGLE, CVAR, {
        if (CVAR && gPlayState != NULL && canCycleArrows()) {
            Player* player = GET_PLAYER(gPlayState);
            if (holdingBow(player)) {
                *should = false;
            }
        }
    });

    // Prevent magic consumption for magic arrows
    COND_VB_SHOULD(VB_MAGIC_ARROW_CONSUME, CVAR, {
        if (CVAR && gPlayState != NULL && canCycleArrows()) {
            Player* player = GET_PLAYER(gPlayState);
            if (holdingMagicArrow(player)) {
                *should = false;
            }
        }
    });

    // Handle magic consumption when arrow is fired instead of on draw
    COND_HOOK(OnGameStateUpdate, CVAR, []() {
        if (gPlayState == nullptr || !canCycleArrows())
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
                u8 magicCost = sArrowCycleCosts[player->heldItemAction - PLAYER_IA_BOW_FIRE];
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
        if (gPlayState == nullptr || !canCycleArrows())
            return;

        Player* player = GET_PLAYER(gPlayState);
        Input* input = CONTROLLER1(&gPlayState->state);

        // Don't allow cycling during magic consumption states (link is tired)
        if (gSaveContext.magicState >= MAGIC_STATE_CONSUME_SETUP &&
            gSaveContext.magicState <= MAGIC_STATE_METER_FLASH_3) {
            if (holdingBow(player) && CHECK_BTN_ALL(input->press.button, BTN_L)) {
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
            return;
        }

        // Block cycling during bow draw (PLAYER_STATE3_40=aim mode, unk_ACE=0=drawing)
        // Prevents rapid cycling from breaking camera transition and arrow spawn
        if ((player->stateFlags3 & PLAYER_STATE3_40) && player->unk_ACE == 0) {
            if (holdingBow(player) && CHECK_BTN_ALL(input->press.button, BTN_L)) {
                return;
            }
        }

        if (holdingBow(player) && CHECK_BTN_ALL(input->press.button, BTN_L)) {
            // Cycle to the next arrow type
            s8 nextArrow = nextArrowType(player->heldItemAction);

            // Kill the original arrow actor
            if (player->heldActor != NULL) {
                Actor_Kill(player->heldActor);
                player->heldActor = NULL;
            }

            // Update player's held item and initialize the new arrow type
            player->heldItemAction = nextArrow;
            Player_InitItemAction(gPlayState, player, static_cast<PlayerItemAction>(nextArrow));
            updateEquippedBow(gPlayState, nextArrow);
            Audio_PlaySfx(NA_SE_PL_CHANGE_ARMS);
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterArrowCycle, { CVAR_NAME });
