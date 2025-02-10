#include "2s2h/Enhancements/Enhancements.h"
#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "interface/parameter_static/parameter_static.h"
#include "macros.h"
#include "variables.h"
}

// Configuration
#define CVAR_NAME "gEnhancements.Masks.EasyMaskEquip"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// State definitions
enum MaskEquipState {
    MASK_EQUIP_NONE,           // No pending operation.
    MASK_EQUIP_PENDING_EQUIP,    // A new transformation mask is pending equip.
    MASK_EQUIP_PENDING_UNEQUIP,  // A transformation mask is pending unequip.
    MASK_EQUIP_TRANSFORMING      // The transformation is in progress.
};

static MaskEquipState gMaskEquipState = MASK_EQUIP_NONE;
static ItemId gPendingMask = ITEM_NONE;      // Pending equip mask (if any)
static ItemId gLastEquippedMask = ITEM_NONE;   // Last equipped transformation mask
static Vtx* gEasyMaskEquipVtx = nullptr;       // Vertex buffer for drawing the equip border

// Transformation Mask Definitions
constexpr std::array<ItemId, 5> kTransformationMasks = { 
    ITEM_MASK_DEKU, 
    ITEM_MASK_GORON, 
    ITEM_MASK_ZORA,
    ITEM_MASK_FIERCE_DEITY, 
    ITEM_MASK_GIANT 
};

// Helper Functions

/// Returns true if the given mask is a transformation mask.
bool isTransformationMask(ItemId mask) {
    return std::binary_search(kTransformationMasks.begin(), kTransformationMasks.end(), mask);
}

/// Returns the inventory slot where the transformation mask should be drawn.
/// If a pending unequip is active or no mask is found, returns -1.
s16 getEquippedMaskSlot() {
    if (gMaskEquipState == MASK_EQUIP_PENDING_UNEQUIP)
        return -1;

    auto& items = gSaveContext.save.saveInfo.inventory.items;
    // If pending an equip, use that mask; otherwise use the last equipped one.
    ItemId target = (gMaskEquipState == MASK_EQUIP_PENDING_EQUIP && gPendingMask != ITEM_NONE)
                      ? gPendingMask : gLastEquippedMask;
    if (target != ITEM_NONE) {
        for (s16 slot = 0; slot < MASK_NUM_SLOTS; ++slot) {
            if (items[slot + ITEM_NUM_SLOTS] == target)
                return slot;
        }
    }
    return -1;
}

/// Updates the vertex positions for the equip border based on the mask’s slot.
void updateEquipBorderVertices(PauseContext* pauseCtx) {
    if (const s16 slot = getEquippedMaskSlot(); slot != -1) {
        const s16 slotX    = slot % MASK_GRID_COLS;
        const s16 slotY    = slot / MASK_GRID_COLS;
        const s16 initialX = -(MASK_GRID_COLS * MASK_GRID_CELL_WIDTH) / 2;
        const s16 initialY = (MASK_GRID_ROWS * MASK_GRID_CELL_HEIGHT) / 2 - 6;
        const s16 posX     = initialX + (slotX * MASK_GRID_CELL_WIDTH);
        const s16 posY     = initialY - (slotY * MASK_GRID_CELL_HEIGHT) + pauseCtx->offsetY;

        const std::array<s16, 4> xCoords = { posX, posX + MASK_GRID_CELL_WIDTH, posX, posX + MASK_GRID_CELL_WIDTH };
        const std::array<s16, 4> yCoords = { posY, posY, posY - MASK_GRID_CELL_HEIGHT, posY - MASK_GRID_CELL_HEIGHT };

        for (size_t i = 0; i < xCoords.size(); ++i) {
            gEasyMaskEquipVtx[i].v.ob[0] = xCoords[i];
            gEasyMaskEquipVtx[i].v.ob[1] = yCoords[i];
        }
    }
}

/// Returns true if the transformation mask represented by `cursorItem` is allowed to be equipped.
bool shouldEquipMask(s16 cursorItem) {
    const ItemId mask = static_cast<ItemId>(cursorItem);
    if (!isTransformationMask(mask))
        return false;

    // Cheat override.
    if (CVarGetInteger("gCheats.UnrestrictedItems", 0))
        return true;

    // Underwater: only allow Zora Mask.
    const s16 hazard = Player_GetEnvironmentalHazard(gPlayState);
    if (hazard >= PLAYER_ENV_HAZARD_UNDERWATER_FLOOR && hazard <= PLAYER_ENV_HAZARD_UNDERWATER_FREE)
        return mask == ITEM_MASK_ZORA;

    Player* player = GET_PLAYER(gPlayState);
    if (player->stateFlags1 & (PLAYER_STATE1_4 | PLAYER_STATE1_4000 | PLAYER_STATE1_40000 |
                               PLAYER_STATE1_200000 | PLAYER_STATE1_2000))
        return false;
    if ((player->stateFlags2 & PLAYER_STATE2_10) ||
        (player->stateFlags1 & PLAYER_STATE1_800) ||
        (player->stateFlags2 & PLAYER_STATE2_1))
        return false;
    if (player->rideActor != nullptr && player->rideActor->id == ACTOR_EN_HORSE)
        return false;
    if (gSaveContext.timerStates[TIMER_ID_MINIGAME_1] != TIMER_STATE_OFF ||
        gSaveContext.timerStates[TIMER_ID_MINIGAME_2] != TIMER_STATE_OFF)
        return false;
    if (player->meleeWeaponState != PLAYER_MELEE_WEAPON_STATE_0)
        return false;

    // Restrictions for Fierce Deity.
    if (mask == ITEM_MASK_FIERCE_DEITY) {
        if (!CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0) &&
            gPlayState->sceneId != SCENE_MITURIN_BS &&
            gPlayState->sceneId != SCENE_HAKUGIN_BS &&
            gPlayState->sceneId != SCENE_SEA_BS &&
            gPlayState->sceneId != SCENE_INISIE_BS &&
            gPlayState->sceneId != SCENE_LAST_BS)
        {
            return false;
        }
    }

    if (!(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND))
        return false;
    if (player->stateFlags3 & (PLAYER_STATE3_200 | PLAYER_STATE3_2000 | PLAYER_STATE3_100))
        return false;
    // Restrictions for Giant Mask.
    if (mask == ITEM_MASK_GIANT) {
        if (gPlayState->sceneId != SCENE_INISIE_BS || gSaveContext.save.saveInfo.playerData.magic == 0)
            return false;
    }
    if (player->currentMask == PLAYER_MASK_GIANT && mask != ITEM_MASK_GIANT)
        return false;
    if (player->transformation != PLAYER_FORM_HUMAN && mask == ITEM_MASK_GIANT)
        return false;
    if (player->stateFlags1 & PLAYER_STATE1_100000)
        return false;

    return true;
}

/// Draws the lavender equip border for the transformation mask.
void drawEquipBorder(PauseContext* pauseCtx) {
    const s16 slot = getEquippedMaskSlot();
    if (slot == -1 ||
        gSaveContext.save.saveInfo.inventory.items[slot + ITEM_NUM_SLOTS] == ITEM_NONE ||
        Player_GetCurMaskItemId(gPlayState) == gPendingMask)
    {
        return;
    }

    GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
    OPEN_DISPS(gfxCtx);

    updateEquipBorderVertices(pauseCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 230, 190, 255, pauseCtx->alpha);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT,
                      TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT,
                      TEXEL0, 0, PRIMITIVE, 0);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
    gSPVertex(POLY_OPA_DISP++, reinterpret_cast<uintptr_t>(gEasyMaskEquipVtx), 4, 0);
    gDPLoadTextureBlock(POLY_OPA_DISP++, gEquippedItemOutlineTex, G_IM_FMT_IA, G_IM_SIZ_8b,
                         32, 32, 0,
                         G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                         G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

    CLOSE_DISPS(gfxCtx);
}

/// Allocates the vertex buffer used for drawing the equip border.
void allocateEquipBorderVertices(GraphicsContext* gfxCtx) {
    gEasyMaskEquipVtx = static_cast<Vtx*>(GRAPH_ALLOC(gfxCtx, 4 * sizeof(Vtx)));
    for (int i = 0; i < 4; ++i) {
        gEasyMaskEquipVtx[i].v.ob[2] = 0;
        gEasyMaskEquipVtx[i].v.flag   = 0;
        gEasyMaskEquipVtx[i].v.tc[0]  = (i & 1) ? (32 << 5) : 0;
        gEasyMaskEquipVtx[i].v.tc[1]  = (i & 2) ? (32 << 5) : 0;
        std::fill(std::begin(gEasyMaskEquipVtx[i].v.cn),
                  std::end(gEasyMaskEquipVtx[i].v.cn), 255);
    }
}

/// Renders the mask icon in the pause menu. If the mask cannot be equipped,
/// it is drawn in grayscale.
void renderMaskItem(PauseContext* pauseCtx, u16 itemId, s16 index) {
    GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
    OPEN_DISPS(gfxCtx);

    const bool applyGrayscale = !shouldEquipMask(itemId);
    if (applyGrayscale) {
        gDPSetGrayscaleColor(POLY_OPA_DISP++, 109, 109, 109, 255);
        gSPGrayscale(POLY_OPA_DISP++, true);
    }

    gSPVertex(POLY_OPA_DISP++, reinterpret_cast<uintptr_t>(&pauseCtx->maskVtx[index]), 4, 0);
    KaleidoScope_DrawTexQuadRGBA32(gfxCtx, gItemIcons[itemId], 32, 32, 0);

    if (applyGrayscale)
        gSPGrayscale(POLY_OPA_DISP++, false);

    CLOSE_DISPS(gfxCtx);
}

// Input Handling

/*
 * Processes player input in the pause menu for transformation mask equipping.
 * The state machine supports:
 *   - MASK_EQUIP_NONE:         No pending operation.
 *   - MASK_EQUIP_PENDING_EQUIP: A new mask is pending equip.
 *   - MASK_EQUIP_PENDING_UNEQUIP: The current mask is marked for unequip.
 *   - MASK_EQUIP_TRANSFORMING:  The transformation is in progress.
 */
void handleEasyMaskEquip(PauseContext* pauseCtx) {
    if (pauseCtx->state != PAUSE_STATE_MAIN || pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE)
        return;

    const s16 cursorItem = pauseCtx->cursorItem[PAUSE_MASK];
    if (!isTransformationMask(static_cast<ItemId>(cursorItem)))
        return;

    const u16 pressedButtons = CONTROLLER1(&gPlayState->state)->press.button;
    const s16 currentMask = Player_GetCurMaskItemId(gPlayState);

    // Process the A button.
    if (CHECK_BTN_ALL(pressedButtons, BTN_A)) {
        // If the selected mask is already equipped (or was last equipped), toggle unequip.
        if (cursorItem == currentMask || cursorItem == gLastEquippedMask) {
            if (gMaskEquipState == MASK_EQUIP_NONE)
            {
                gMaskEquipState = MASK_EQUIP_PENDING_UNEQUIP;
                Audio_PlaySfx(NA_SE_SY_CANCEL);
            }
            else  // (either pending unequip or pending equip)
            {
                gPendingMask = ITEM_NONE;
                gMaskEquipState = MASK_EQUIP_NONE;
                Audio_PlaySfx(NA_SE_SY_DECIDE);
            }
            return;
        }

        // If the same mask is selected as the pending equip, cancel it.
        if (cursorItem == gPendingMask && gMaskEquipState == MASK_EQUIP_PENDING_EQUIP) {
            gPendingMask = ITEM_NONE;
            gMaskEquipState = (currentMask != ITEM_NONE) ? MASK_EQUIP_PENDING_UNEQUIP : MASK_EQUIP_NONE;
            Audio_PlaySfx(NA_SE_SY_CANCEL);
            return;
        }

        // For a new mask selection, validate and mark it as pending.
        if (!shouldEquipMask(cursorItem)) {
            Audio_PlaySfx(NA_SE_SY_ERROR);
            return;
        }
        gPendingMask = static_cast<ItemId>(cursorItem);
        gMaskEquipState = MASK_EQUIP_PENDING_EQUIP;
        Audio_PlaySfx(NA_SE_SY_DECIDE);
    }
    // Process the B button: cancel any pending operation.
    else if (CHECK_BTN_ALL(pressedButtons, BTN_B)) {
        gPendingMask = ITEM_NONE;
        gMaskEquipState = MASK_EQUIP_NONE;
        gLastEquippedMask = ITEM_NONE;
    }
}

// Hook Registrations

void RegisterEasyMaskEquip() {
    // Process transformation mask input during pause menu updates.
    COND_HOOK(OnKaleidoUpdate, CVAR, [](PauseContext* pauseCtx) {
        if (pauseCtx->pageIndex == PAUSE_MASK)
            handleEasyMaskEquip(pauseCtx);
    });

    // Hook for drawing mask icons.
    COND_VB_SHOULD(VB_DRAW_MASK_ITEM, CVAR, {
        u16* itemId = va_arg(args, u16*);
        s16* index  = va_arg(args, s16*);
        if (isTransformationMask(static_cast<ItemId>(*itemId))) {
            renderMaskItem(&gPlayState->pauseCtx, *itemId, *index);
            *should = false;
        } else {
            *should = true;
        }
    });

    // Hook for drawing the equip border (transformation masks only).
    COND_ID_HOOK(BeforeKaleidoDrawPage, PAUSE_MASK, CVAR, [](PauseContext* pauseCtx, u16) {
        if (pauseCtx->pageIndex == PAUSE_MASK) {
            allocateEquipBorderVertices(gPlayState->state.gfxCtx);
            updateEquipBorderVertices(pauseCtx);
            drawEquipBorder(pauseCtx);
        }
    });

    // Hook for equipping pending transformation masks when the pause menu closes.
    COND_HOOK(OnKaleidoClose, CVAR, [](u16 pauseIndex) {
        if (pauseIndex != PAUSE_MASK)
            return;

        Player* player = GET_PLAYER(gPlayState);
        if (gMaskEquipState == MASK_EQUIP_PENDING_UNEQUIP) {
            if (Player_GetCurMaskItemId(gPlayState) != ITEM_NONE)
                Player_UseItem(gPlayState, player, static_cast<ItemId>(Player_GetCurMaskItemId(gPlayState)));
            gLastEquippedMask = ITEM_NONE;
            gMaskEquipState = MASK_EQUIP_NONE;
        }
        else if (gMaskEquipState == MASK_EQUIP_PENDING_EQUIP) {
            Player_UseItem(gPlayState, player, gPendingMask);
            gLastEquippedMask = gPendingMask;
            gPendingMask = ITEM_NONE;
            gMaskEquipState = MASK_EQUIP_TRANSFORMING;
        }
    });

    // Hook for updating equip state during actor updates.
    COND_HOOK(OnActorUpdate, CVAR, [](Actor* actor) {
        if (actor->id != ACTOR_PLAYER)
            return;
        Player* player = reinterpret_cast<Player*>(actor);
        if (gMaskEquipState == MASK_EQUIP_TRANSFORMING && player->transformation == PLAYER_FORM_HUMAN)
            gMaskEquipState = MASK_EQUIP_NONE;
    });

    // Hook for controlling the display of the selected item textbox.
    COND_VB_SHOULD(VB_KALEIDO_DISPLAY_ITEM_TEXT, CVAR, {
        *should = !isTransformationMask(static_cast<ItemId>(gPlayState->pauseCtx.cursorItem[PAUSE_MASK]));
    });

    // Hook for drawing the white "equipped" border.
    COND_VB_SHOULD(VB_DRAW_ITEM_EQUIPPED_OUTLINE, CVAR, {
        ItemId* itemId = va_arg(args, ItemId*);
        if (isTransformationMask(*itemId)) {
            const s16 currentMask = Player_GetCurMaskItemId(gPlayState);
            *should = !((gMaskEquipState == MASK_EQUIP_PENDING_EQUIP && *itemId == gPendingMask) ||
                        (gMaskEquipState != MASK_EQUIP_PENDING_EQUIP && *itemId == currentMask));
        } else {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterEasyMaskEquip, { CVAR_NAME });
