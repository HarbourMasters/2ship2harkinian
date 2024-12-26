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

// Flash effect constants
static const s16 BUTTON_FLASH_DURATION = 3;  // Shorter duration for quicker blink
static const s16 BUTTON_FLASH_COUNT = 2;     // Number of blinks
static const s16 BUTTON_HIGHLIGHT_ALPHA = 128;  // Alpha value for highlight state
static s16 sButtonFlashTimer = 0;
static s16 sButtonFlashCount = 0;

// Magic costs
static const u8 ARROW_MAGIC_COST_FIRE = 4;
static const u8 ARROW_MAGIC_COST_ICE = 4;
static const u8 ARROW_MAGIC_COST_LIGHT = 8;

// The magic arrow costs from z_player.c
static u8 sArrowCycleCosts[] = {
    ARROW_MAGIC_COST_FIRE,  // ARROW_MAGIC_FIRE
    ARROW_MAGIC_COST_ICE,   // ARROW_MAGIC_ICE
    ARROW_MAGIC_COST_LIGHT, // ARROW_MAGIC_LIGHT
};

// Arrow types for better code readability
static enum ArrowType {
    ARROW_NORMAL = PLAYER_IA_BOW,
    ARROW_FIRE = PLAYER_IA_BOW_FIRE,
    ARROW_ICE = PLAYER_IA_BOW_ICE,
    ARROW_LIGHT = PLAYER_IA_BOW_LIGHT,
};

// Arrow cycling order
static const PlayerItemAction arrowCycleOrder[] = {
    static_cast<PlayerItemAction>(ARROW_NORMAL),
    static_cast<PlayerItemAction>(ARROW_FIRE),
    static_cast<PlayerItemAction>(ARROW_ICE),
    static_cast<PlayerItemAction>(ARROW_LIGHT),
};

// Check if player is holding a bow
static bool holdingBow(Player* player) {
    return player->heldItemAction >= ARROW_NORMAL && player->heldItemAction <= ARROW_LIGHT;
}

// Check if player is holding a magic arrow
static bool holdingMagicArrow(Player* player) {
    return player->heldItemAction >= ARROW_FIRE && player->heldItemAction <= ARROW_LIGHT;
}

// Returns true if the given arrow type is available in the player's inventory
static bool hasArrowType(PlayerItemAction arrowType) {
    switch (arrowType) {
        case ARROW_NORMAL:
            // Normal arrows are always considered available if we have the bow
            return true;
        case ARROW_FIRE:
            return (INV_CONTENT(ITEM_ARROW_FIRE) == ITEM_ARROW_FIRE);
        case ARROW_ICE:
            return (INV_CONTENT(ITEM_ARROW_ICE) == ITEM_ARROW_ICE);
        case ARROW_LIGHT:
            return (INV_CONTENT(ITEM_ARROW_LIGHT) == ITEM_ARROW_LIGHT);
        default:
            return false;
    }
}

// Check if player has enough magic for arrow type
static bool hasEnoughMagic(PlayerItemAction arrowType) {
    if (arrowType == ARROW_NORMAL) {
        return true;
    }
    
    u8 magicCost = sArrowCycleCosts[arrowType - ARROW_FIRE];
    return gSaveContext.save.saveInfo.playerData.magic >= magicCost;
}

// Get the next arrow type when cycling
static s8 nextArrowType(s8 currentArrowType) {
    // Find the current arrow's position in the cycle
    int currentIndex = 0;
    for (int i = 0; i < (int)ARRAY_COUNT(arrowCycleOrder); i++) {
        if (arrowCycleOrder[i] == currentArrowType) {
            currentIndex = i;
            break;
        }
    }

    // Try each subsequent arrow in the cycle
    for (int offset = 1; offset <= (int)ARRAY_COUNT(arrowCycleOrder); offset++) {
        int nextIndex = (currentIndex + offset) % ARRAY_COUNT(arrowCycleOrder);
        if (hasArrowType(arrowCycleOrder[nextIndex])) {
            if (!hasEnoughMagic(arrowCycleOrder[nextIndex])) {
                continue;
            }
            return arrowCycleOrder[nextIndex];
        }
    }

    // Fallback to normal arrows
    return ARROW_NORMAL;
}

// Get the bow item for an arrow type
static s32 bowItemForArrow(PlayerItemAction arrowType) {
    switch (arrowType) {
        case ARROW_FIRE:
            return ITEM_BOW_FIRE;
        case ARROW_ICE:
            return ITEM_BOW_ICE;
        case ARROW_LIGHT:
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
            // Start flash effect
            sButtonFlashTimer = BUTTON_FLASH_DURATION;
            sButtonFlashCount = 0;
        }
    }

    for (s32 i = 0; i < 4; i++) {
        if ((DPAD_BUTTON_ITEM_EQUIP(0, i) == ITEM_BOW) ||
            (DPAD_BUTTON_ITEM_EQUIP(0, i) >= ITEM_BOW_FIRE && DPAD_BUTTON_ITEM_EQUIP(0, i) <= ITEM_BOW_LIGHT)) {
            DPAD_BUTTON_ITEM_EQUIP(0, i) = bowItem;
            DPAD_SLOT_EQUIP(0, i) = SLOT_BOW;
            Interface_Dpad_LoadItemIcon(play, i);
            gSaveContext.shipSaveContext.dpad.status[i] = BTN_ENABLED;
            // Start flash effect
            sButtonFlashTimer = BUTTON_FLASH_DURATION;
            sButtonFlashCount = 0;
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

// Update button alpha for a single button
static void updateButtonAlpha(s16 flashAlpha, bool isButtonBow, s16* buttonAlpha) {
    if (isButtonBow) {
        *buttonAlpha = flashAlpha;
        if (sButtonFlashTimer == 0) *buttonAlpha = 255;  // Restore visibility
    }
}

// Update flash effect for all buttons
static void updateFlashEffect(PlayState* play) {
    if (sButtonFlashTimer <= 0) return;
    
    sButtonFlashTimer--;
    s16 flashAlpha = (sButtonFlashTimer % 2) ? 255 : BUTTON_HIGHLIGHT_ALPHA;
    
    if (sButtonFlashTimer == 0 && sButtonFlashCount < BUTTON_FLASH_COUNT) {
        sButtonFlashTimer = BUTTON_FLASH_DURATION;
        sButtonFlashCount++;
    }

    // C buttons
    updateButtonAlpha(flashAlpha, 
        (GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_LEFT) == ITEM_BOW) || 
        (GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_LEFT) >= ITEM_BOW_FIRE && GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_LEFT) <= ITEM_BOW_LIGHT),
        &play->interfaceCtx.cLeftAlpha);
    
    updateButtonAlpha(flashAlpha,
        (GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_DOWN) == ITEM_BOW) || 
        (GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_DOWN) >= ITEM_BOW_FIRE && GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_DOWN) <= ITEM_BOW_LIGHT),
        &play->interfaceCtx.cDownAlpha);
    
    updateButtonAlpha(flashAlpha,
        (GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_RIGHT) == ITEM_BOW) || 
        (GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_FIRE && GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_LIGHT),
        &play->interfaceCtx.cRightAlpha);
    
    // D-pad buttons
    updateButtonAlpha(flashAlpha,
        (DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_RIGHT) == ITEM_BOW) || 
        (DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_FIRE && DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_LIGHT),
        &play->interfaceCtx.shipInterface.dpad.dRightAlpha);
    
    updateButtonAlpha(flashAlpha,
        (DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_LEFT) == ITEM_BOW) || 
        (DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_LEFT) >= ITEM_BOW_FIRE && DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_LEFT) <= ITEM_BOW_LIGHT),
        &play->interfaceCtx.shipInterface.dpad.dLeftAlpha);
    
    updateButtonAlpha(flashAlpha,
        (DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_DOWN) == ITEM_BOW) || 
        (DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_DOWN) >= ITEM_BOW_FIRE && DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_DOWN) <= ITEM_BOW_LIGHT),
        &play->interfaceCtx.shipInterface.dpad.dDownAlpha);
    
    updateButtonAlpha(flashAlpha,
        (DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_UP) == ITEM_BOW) || 
        (DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_UP) >= ITEM_BOW_FIRE && DPAD_GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_UP) <= ITEM_BOW_LIGHT),
        &play->interfaceCtx.shipInterface.dpad.dUpAlpha);
}

// Handle cycling to next arrow type and updating player state
static void cycleToNextArrow(PlayState* play, Player* player) {
    // Check if any magic arrows are affordable before cycling
    bool canCycleMagic = false;
    for (int i = 1; i < 4; i++) {  // Start from 1 to skip normal arrows
        PlayerItemAction arrowType = static_cast<PlayerItemAction>(PLAYER_IA_BOW + i);
        if (hasArrowType(arrowType) && hasEnoughMagic(arrowType)) {
            canCycleMagic = true;
            break;
        }
    }

    // If no affordable magic arrows and we're on normal arrows, play error and return
    if (!canCycleMagic && player->heldItemAction == PLAYER_IA_BOW) {
        Audio_PlaySfx(NA_SE_SY_ERROR);
        return;
    }

    // Cycle to the next arrow type
    s8 nextArrow = nextArrowType(player->heldItemAction);

    // Kill the original arrow actor
    if (player->heldActor != NULL) {
        Actor_Kill(player->heldActor);
        player->heldActor = NULL;
    }

    // Update player's held item and initialize the new arrow type
    player->heldItemAction = nextArrow;
    Player_InitItemAction(play, player, static_cast<PlayerItemAction>(nextArrow));
    updateEquippedBow(play, nextArrow);
    Audio_PlaySfx(NA_SE_PL_CHANGE_ARMS);
}

// Use normal arrow when magic is depleted
static void useNormalArrow(PlayState* play, Player* player) {
    if (player->heldActor != NULL) {
        Actor_Kill(player->heldActor);
        player->heldActor = NULL;
    }
    player->heldItemAction = PLAYER_IA_BOW;
    Player_InitItemAction(play, player, PLAYER_IA_BOW);
    updateEquippedBow(play, PLAYER_IA_BOW);
    Audio_PlaySfx(NA_SE_PL_CHANGE_ARMS);
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
                u8 magicCost = sArrowCycleCosts[player->heldItemAction - ARROW_FIRE];
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

        // Update flash effect
        updateFlashEffect(gPlayState);

        Player* player = GET_PLAYER(gPlayState);
        Input* input = CONTROLLER1(&gPlayState->state);

        // Check if current magic arrow is no longer affordable
        if (holdingMagicArrow(player) && !hasEnoughMagic(static_cast<PlayerItemAction>(player->heldItemAction))) {
            // Force switch to regular arrows
            useNormalArrow(gPlayState, player);
            return;
        }

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
            cycleToNextArrow(gPlayState, player);
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterArrowCycle, { CVAR_NAME });
