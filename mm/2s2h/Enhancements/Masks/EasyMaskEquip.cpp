/**
 * @file EasyMaskEquip.cpp
 * @brief Implements the easy mask equip functionality for transformation masks.
 *
 * This module handles input, state management, and rendering of transformation masks in the pause menu.
 */

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
enum class MaskEquipState {
    None,           // No pending operation
    PendingEquip,   // A new transformation mask is pending equip
    PendingUnequip, // A transformation mask is pending unequip
    Transforming    // The transformation is in progress
};

// Display constants
namespace DisplayConstants {
constexpr s32 BORDER_ALPHA = 255;
constexpr s32 EQUIP_COLOR_R = 30;
constexpr s32 EQUIP_COLOR_G = 255;
constexpr s32 EQUIP_COLOR_B = 30;
constexpr s32 UNEQUIP_COLOR_R = 255;
constexpr s32 UNEQUIP_COLOR_G = 30;
constexpr s32 UNEQUIP_COLOR_B = 30;
constexpr f32 PULSE_SPEED = 0.045f;
constexpr f32 PULSE_SCALE = 6.28f;
constexpr s32 MIN_EQUIP_ALPHA = 175;
constexpr s32 MAX_EQUIP_ALPHA = 255;
constexpr s32 MAX_UNEQUIP_ALPHA = 127;
} // namespace DisplayConstants

// Vertex handling
struct VertexBuffer {
    Vtx* vertices;
    s32 count;

    void initialize(GraphicsContext* gfxCtx, s32 vertexCount) {
        vertices = static_cast<Vtx*>(GRAPH_ALLOC(gfxCtx, vertexCount * sizeof(Vtx)));
        count = vertexCount;
        for (s32 i = 0; i < count; ++i) {
            vertices[i].v.ob[0] = 0;
            vertices[i].v.ob[1] = 0;
            vertices[i].v.ob[2] = 0;
            vertices[i].v.flag = 0;
            vertices[i].v.tc[0] = (i & 1) ? (32 << 5) : 0;
            vertices[i].v.tc[1] = (i & 2) ? (32 << 5) : 0;
            vertices[i].v.cn[0] = 255;
            vertices[i].v.cn[1] = 255;
            vertices[i].v.cn[2] = 255;
            vertices[i].v.cn[3] = 255;
        }
    }

    void updatePositions(const std::array<s16, 4>& xCoords, const std::array<s16, 4>& yCoords) {
        for (s32 i = 0; i < 4; ++i) {
            vertices[i].v.ob[0] = xCoords[i];
            vertices[i].v.ob[1] = yCoords[i];
            vertices[i].v.ob[2] = 0;
        }
    }
};

// Global state tracker for mask equip
struct MaskEquipStateTracker {
    MaskEquipState state = MaskEquipState::None;
    ItemId pendingMask = ITEM_NONE;
    ItemId lastEquippedMask = ITEM_NONE;
    ItemId lastFrameMask = ITEM_NONE;
    f32 pulsePhase = 0.0f;
    VertexBuffer equipBorderVtx;
    VertexBuffer pendingEquipVtx;
};
static MaskEquipStateTracker gMaskState;

// Helper function to draw a textured quad with a given color.
// Uses a reference to the display list pointer so that it can be updated.
static inline void drawQuadWithColor(Gfx*& disp, const VertexBuffer& vb, s32 r, s32 g, s32 b, s32 a) {
    gDPPipeSync(disp++);
    gDPSetPrimColor(disp++, 0, 0, r, g, b, a);
    gDPSetCombineLERP(disp++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetEnvColor(disp++, 0, 0, 0, 0);
    gSPVertex(disp++, reinterpret_cast<uintptr_t>(vb.vertices), 4, 0);
    gDPLoadTextureBlock(disp++, gEquippedItemOutlineTex, G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSP1Quadrangle(disp++, 0, 2, 3, 1, 0);
}

// Transformation Mask Definitions
constexpr std::array<ItemId, 5> kTransformationMasks = { ITEM_MASK_DEKU, ITEM_MASK_GORON, ITEM_MASK_ZORA,
                                                         ITEM_MASK_FIERCE_DEITY, ITEM_MASK_GIANT };

/* ============================================================================
   Helper Functions
   ============================================================================ */

/**
 * @brief Checks if the provided mask is a transformation mask.
 * @param mask The mask item ID.
 * @return True if it is a transformation mask, false otherwise.
 */
bool isTransformationMask(ItemId mask) {
    return mask == ITEM_MASK_DEKU || mask == ITEM_MASK_GORON || mask == ITEM_MASK_ZORA ||
           mask == ITEM_MASK_FIERCE_DEITY || mask == ITEM_MASK_GIANT;
}

/**
 * @brief Gets the inventory slot for the currently equipped mask.
 * @return The slot index if found, -1 otherwise.
 */
s16 getEquippedMaskSlot() {
    if (gMaskState.state == MaskEquipState::PendingEquip && gMaskState.pendingMask != ITEM_NONE)
        return -1;
    auto& items = gSaveContext.save.saveInfo.inventory.items;
    const s16 currentMask = Player_GetCurMaskItemId(gPlayState);
    if (currentMask != ITEM_NONE) {
        for (s16 slot = 0; slot < MASK_NUM_SLOTS; ++slot) {
            if (items[slot + ITEM_NUM_SLOTS] == currentMask)
                return slot;
        }
    }
    if (gMaskState.lastEquippedMask != ITEM_NONE) {
        for (s16 slot = 0; slot < MASK_NUM_SLOTS; ++slot) {
            if (items[slot + ITEM_NUM_SLOTS] == gMaskState.lastEquippedMask)
                return slot;
        }
    }
    return -1;
}

/**
 * @brief Finds the inventory slot for a given mask.
 * @param maskId The mask item ID.
 * @return The slot index if found, -1 otherwise.
 */
s16 getMaskSlot(ItemId maskId) {
    auto& items = gSaveContext.save.saveInfo.inventory.items;
    for (s16 slot = 0; slot < MASK_NUM_SLOTS; ++slot) {
        if (items[slot + ITEM_NUM_SLOTS] == maskId)
            return slot;
    }
    return -1;
}

/**
 * @brief Computes the screen coordinates for a mask slot.
 * @param slot The inventory slot.
 * @param pauseCtx The pause context.
 * @param posX Output x coordinate.
 * @param posY Output y coordinate.
 */
void getMaskSlotCoordinates(s16 slot, PauseContext* pauseCtx, s16* posX, s16* posY) {
    const s16 slotX = slot % MASK_GRID_COLS;
    const s16 slotY = slot / MASK_GRID_COLS;
    const s16 initialX = -(MASK_GRID_COLS * MASK_GRID_CELL_WIDTH) / 2;
    const s16 initialY = (MASK_GRID_ROWS * MASK_GRID_CELL_HEIGHT) / 2 - 6;
    *posX = initialX + (slotX * MASK_GRID_CELL_WIDTH);
    *posY = initialY - (slotY * MASK_GRID_CELL_HEIGHT) + pauseCtx->offsetY;
}

/**
 * @brief Updates the vertex positions for the equip border based on the currently equipped mask slot.
 * @param pauseCtx The pause context.
 */
void updateEquipBorderVertices(PauseContext* pauseCtx) {
    s16 slot = getEquippedMaskSlot();
    if (slot != -1) {
        s16 posX, posY;
        getMaskSlotCoordinates(slot, pauseCtx, &posX, &posY);
        const std::array<s16, 4> xCoords = { posX, posX + MASK_GRID_CELL_WIDTH, posX, posX + MASK_GRID_CELL_WIDTH };
        const std::array<s16, 4> yCoords = { posY, posY, posY - MASK_GRID_CELL_HEIGHT, posY - MASK_GRID_CELL_HEIGHT };
        gMaskState.equipBorderVtx.updatePositions(xCoords, yCoords);
    }
}

/**
 * @brief Updates the vertex positions for pending action animations.
 * @param pauseCtx The pause context.
 * @param targetSlot The target mask slot.
 * @param vertexBuffer The vertex buffer to update.
 * @param isUnequip True for unequip animation, false for equip.
 */
void updatePendingActionVertices(PauseContext* pauseCtx, s16 targetSlot, VertexBuffer& vertexBuffer,
                                 bool isUnequip = false) {
    if (targetSlot == -1)
        return;
    s16 posX, posY;
    getMaskSlotCoordinates(targetSlot, pauseCtx, &posX, &posY);
    if (!isUnequip) {
        const f32 pulseScale = (sinf(gMaskState.pulsePhase * DisplayConstants::PULSE_SCALE) + 1.0f) / 16.0f;
        const s32 offset = static_cast<s32>(pulseScale * MASK_GRID_CELL_WIDTH);
        const std::array<s16, 4> xCoords = { posX - offset, posX + MASK_GRID_CELL_WIDTH + offset, posX - offset,
                                             posX + MASK_GRID_CELL_WIDTH + offset };
        const std::array<s16, 4> yCoords = { posY + offset, posY + offset, posY - MASK_GRID_CELL_HEIGHT - offset,
                                             posY - MASK_GRID_CELL_HEIGHT - offset };
        vertexBuffer.updatePositions(xCoords, yCoords);
    } else {
        const std::array<s16, 4> xCoords = { posX, posX + MASK_GRID_CELL_WIDTH, posX, posX + MASK_GRID_CELL_WIDTH };
        const std::array<s16, 4> yCoords = { posY, posY, posY - MASK_GRID_CELL_HEIGHT, posY - MASK_GRID_CELL_HEIGHT };
        vertexBuffer.updatePositions(xCoords, yCoords);
    }
}

/**
 * @brief Determines whether the transformation mask represented by cursorItem can be equipped.
 * @param cursorItem The mask item ID from the cursor.
 * @return True if the mask can be equipped, false otherwise.
 */
bool shouldEquipMask(s16 cursorItem) {
    const ItemId mask = static_cast<ItemId>(cursorItem);
    if (!isTransformationMask(mask))
        return false;
    if (CVarGetInteger("gCheats.UnrestrictedItems", 0))
        return true;
    const s16 hazard = Player_GetEnvironmentalHazard(gPlayState);
    if (hazard >= PLAYER_ENV_HAZARD_UNDERWATER_FLOOR && hazard <= PLAYER_ENV_HAZARD_UNDERWATER_FREE)
        return mask == ITEM_MASK_ZORA;
    Player* player = GET_PLAYER(gPlayState);
    if (player->stateFlags1 &
        (PLAYER_STATE1_4 | PLAYER_STATE1_4000 | PLAYER_STATE1_40000 | PLAYER_STATE1_200000 | PLAYER_STATE1_2000))
        return false;
    if ((player->stateFlags2 & PLAYER_STATE2_10) || (player->stateFlags1 & PLAYER_STATE1_800) ||
        (player->stateFlags2 & PLAYER_STATE2_1))
        return false;
    if (player->rideActor != nullptr && player->rideActor->id == ACTOR_EN_HORSE)
        return false;
    if (gSaveContext.timerStates[TIMER_ID_MINIGAME_1] != TIMER_STATE_OFF ||
        gSaveContext.timerStates[TIMER_ID_MINIGAME_2] != TIMER_STATE_OFF)
        return false;
    if (player->meleeWeaponState != PLAYER_MELEE_WEAPON_STATE_0)
        return false;
    if (mask == ITEM_MASK_FIERCE_DEITY) {
        if (!CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0) && gPlayState->sceneId != SCENE_MITURIN_BS &&
            gPlayState->sceneId != SCENE_HAKUGIN_BS && gPlayState->sceneId != SCENE_SEA_BS &&
            gPlayState->sceneId != SCENE_INISIE_BS && gPlayState->sceneId != SCENE_LAST_BS)
            return false;
    }
    if (!(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND))
        return false;
    if (player->stateFlags3 & (PLAYER_STATE3_200 | PLAYER_STATE3_2000 | PLAYER_STATE3_100))
        return false;
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

/**
 * @brief Allocates vertex buffers for drawing mask equip borders.
 * @param gfxCtx The graphics context.
 */
void allocateEquipBorderVertices(GraphicsContext* gfxCtx) {
    gMaskState.equipBorderVtx.initialize(gfxCtx, 4);
    gMaskState.pendingEquipVtx.initialize(gfxCtx, 4);
}

/* ============================================================================
   Drawing Functions
   ============================================================================ */

/**
 * @brief Draws the border around the currently equipped transformation mask.
 * @param pauseCtx The pause context.
 */
void drawEquipBorder(PauseContext* pauseCtx) {
    if (gMaskState.state == MaskEquipState::PendingEquip || gMaskState.state == MaskEquipState::PendingUnequip)
        return;
    const s16 slot = getEquippedMaskSlot();
    if (slot == -1 || gSaveContext.save.saveInfo.inventory.items[slot + ITEM_NUM_SLOTS] == ITEM_NONE ||
        Player_GetCurMaskItemId(gPlayState) == gMaskState.pendingMask)
        return;
    GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
    OPEN_DISPS(gfxCtx);
    updateEquipBorderVertices(pauseCtx);
    drawQuadWithColor(POLY_OPA_DISP, gMaskState.equipBorderVtx, DisplayConstants::EQUIP_COLOR_R,
                      DisplayConstants::EQUIP_COLOR_G, DisplayConstants::EQUIP_COLOR_B, DisplayConstants::BORDER_ALPHA);
    CLOSE_DISPS(gfxCtx);
}

/**
 * @brief Draws the pulsing effect for pending equip/unequip actions.
 * @param pauseCtx The pause context.
 */
void drawPendingActionEffect(PauseContext* pauseCtx) {
    if (gMaskState.state == MaskEquipState::None)
        return;
    GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
    OPEN_DISPS(gfxCtx);

    // Update pulse phase and calculate alpha values
    gMaskState.pulsePhase += DisplayConstants::PULSE_SPEED;
    if (gMaskState.pulsePhase > 1.0f)
        gMaskState.pulsePhase -= 1.0f;
    const f32 equipPulse = (sinf(gMaskState.pulsePhase * DisplayConstants::PULSE_SCALE) + 1.0f) / 4.0f;
    const s32 equipAlpha = DisplayConstants::MIN_EQUIP_ALPHA + static_cast<s32>(equipPulse * 80.0f);
    const f32 unequipPulse = sinf(gMaskState.pulsePhase * DisplayConstants::PULSE_SCALE);
    const s32 unequipAlpha = static_cast<s32>((unequipPulse + 1.0f) * DisplayConstants::MAX_UNEQUIP_ALPHA);

    if (gMaskState.state == MaskEquipState::PendingEquip && gMaskState.pendingMask != ITEM_NONE) {
        s16 targetSlot = getMaskSlot(gMaskState.pendingMask);
        updatePendingActionVertices(pauseCtx, targetSlot, gMaskState.pendingEquipVtx, false);
        drawQuadWithColor(POLY_OPA_DISP, gMaskState.pendingEquipVtx, DisplayConstants::EQUIP_COLOR_R,
                          DisplayConstants::EQUIP_COLOR_G, DisplayConstants::EQUIP_COLOR_B, equipAlpha);
        // Draw unequip effect on currently equipped mask if needed.
        if (Player_GetCurMaskItemId(gPlayState) != ITEM_NONE) {
            s16 currentMaskSlot = getMaskSlot(static_cast<ItemId>(Player_GetCurMaskItemId(gPlayState)));
            updatePendingActionVertices(pauseCtx, currentMaskSlot, gMaskState.equipBorderVtx, true);
            drawQuadWithColor(POLY_OPA_DISP, gMaskState.equipBorderVtx, DisplayConstants::UNEQUIP_COLOR_R,
                              DisplayConstants::UNEQUIP_COLOR_G, DisplayConstants::UNEQUIP_COLOR_B, unequipAlpha);
        }
    } else if (gMaskState.state == MaskEquipState::PendingUnequip) {
        s16 currentMaskSlot = getMaskSlot(static_cast<ItemId>(Player_GetCurMaskItemId(gPlayState)));
        updatePendingActionVertices(pauseCtx, currentMaskSlot, gMaskState.pendingEquipVtx, true);
        drawQuadWithColor(POLY_OPA_DISP, gMaskState.pendingEquipVtx, DisplayConstants::UNEQUIP_COLOR_R,
                          DisplayConstants::UNEQUIP_COLOR_G, DisplayConstants::UNEQUIP_COLOR_B, unequipAlpha);
    }
    CLOSE_DISPS(gfxCtx);
}

/**
 * @brief Renders a mask icon in the pause menu. If the mask cannot be equipped, it is drawn in grayscale.
 * @param pauseCtx The pause context.
 * @param itemId The mask item ID.
 * @param index The index for the vertex array.
 */
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

/* ============================================================================
   Input and State Management
   ============================================================================ */

/**
 * @brief Manages mask equip input and state.
 */
class MaskEquipManager {
  public:
    /**
     * @brief Handles input for transformation mask actions.
     * @param pauseCtx The pause context.
     */
    void handleInput(PauseContext* pauseCtx) {
        if (pauseCtx->state != PAUSE_STATE_MAIN || pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE)
            return;
        const s16 cursorItem = pauseCtx->cursorItem[PAUSE_MASK];
        if (!isTransformationMask(static_cast<ItemId>(cursorItem)))
            return;
        const u16 pressedButtons = CONTROLLER1(&gPlayState->state)->press.button;
        const s16 currentMask = Player_GetCurMaskItemId(gPlayState);
        if (CHECK_BTN_ALL(pressedButtons, BTN_A)) {
            handleAButtonPress(cursorItem, currentMask);
        } else if (CHECK_BTN_ALL(pressedButtons, BTN_B)) {
            cancelPendingOperation();
        }
    }

    /**
     * @brief Updates mask equip state.
     * @param currentMask The currently equipped mask.
     */
    void updateState(ItemId currentMask) {
        if (currentMask != ITEM_NONE)
            gMaskState.lastEquippedMask = currentMask;
        if (gMaskState.state == MaskEquipState::Transforming && currentMask != gMaskState.lastFrameMask) {
            gMaskState.state = MaskEquipState::None;
            gMaskState.pendingMask = ITEM_NONE;
        }
        gMaskState.lastFrameMask = currentMask;
    }

    /**
     * @brief Handles pending mask equip/unequip when the pause menu closes.
     */
    void handlePauseClose() {
        Player* player = GET_PLAYER(gPlayState);
        if (gMaskState.state == MaskEquipState::PendingUnequip) {
            if (Player_GetCurMaskItemId(gPlayState) != ITEM_NONE)
                Player_UseItem(gPlayState, player, static_cast<ItemId>(Player_GetCurMaskItemId(gPlayState)));
            gMaskState.lastEquippedMask = ITEM_NONE;
            gMaskState.state = MaskEquipState::None;
        } else if (gMaskState.state == MaskEquipState::PendingEquip) {
            Player_UseItem(gPlayState, player, gMaskState.pendingMask);
            gMaskState.lastEquippedMask = gMaskState.pendingMask;
            gMaskState.pendingMask = ITEM_NONE;
            gMaskState.state = MaskEquipState::Transforming;
        }
    }

    /**
     * @brief Handles player state updates related to mask equip.
     * @param player The player instance.
     */
    void handlePlayerUpdate(Player* player) {
        if (gMaskState.state == MaskEquipState::Transforming && player->transformation == PLAYER_FORM_HUMAN) {
            gMaskState.state = MaskEquipState::None;
            gMaskState.lastEquippedMask = ITEM_NONE;
        }
    }

  private:
    /**
     * @brief Processes the A button press for mask selection.
     * @param cursorItem The mask item ID from the cursor.
     * @param currentMask The currently equipped mask.
     */
    void handleAButtonPress(s16 cursorItem, s16 currentMask) {
        if (gMaskState.state != MaskEquipState::None && cursorItem == currentMask) {
            gMaskState.pendingMask = ITEM_NONE;
            gMaskState.state = MaskEquipState::None;
            Audio_PlaySfx(NA_SE_SY_CANCEL);
            return;
        }
        if (cursorItem == gMaskState.pendingMask && gMaskState.state == MaskEquipState::PendingEquip) {
            gMaskState.pendingMask = ITEM_NONE;
            gMaskState.state = MaskEquipState::None;
            Audio_PlaySfx(NA_SE_SY_CANCEL);
            return;
        }
        if (cursorItem == currentMask) {
            if (gMaskState.state == MaskEquipState::None) {
                gMaskState.state = MaskEquipState::PendingUnequip;
                Audio_PlaySfx(NA_SE_SY_CANCEL);
            } else if (gMaskState.state == MaskEquipState::PendingUnequip) {
                gMaskState.state = MaskEquipState::None;
                Audio_PlaySfx(NA_SE_SY_DECIDE);
            }
            return;
        }
        if (cursorItem == gMaskState.lastEquippedMask && currentMask != gMaskState.lastEquippedMask) {
            if (gMaskState.state == MaskEquipState::None) {
                gMaskState.pendingMask = static_cast<ItemId>(cursorItem);
                gMaskState.state = MaskEquipState::PendingEquip;
                Audio_PlaySfx(NA_SE_SY_DECIDE);
            } else if (gMaskState.state == MaskEquipState::PendingEquip &&
                       gMaskState.pendingMask == static_cast<ItemId>(cursorItem)) {
                gMaskState.pendingMask = ITEM_NONE;
                gMaskState.state = MaskEquipState::None;
                Audio_PlaySfx(NA_SE_SY_CANCEL);
            } else if (gMaskState.state == MaskEquipState::PendingEquip) {
                gMaskState.pendingMask = static_cast<ItemId>(cursorItem);
                Audio_PlaySfx(NA_SE_SY_DECIDE);
            }
            return;
        }
        if (!shouldEquipMask(cursorItem)) {
            Audio_PlaySfx(NA_SE_SY_ERROR);
            return;
        }
        gMaskState.pendingMask = static_cast<ItemId>(cursorItem);
        gMaskState.state = MaskEquipState::PendingEquip;
        Audio_PlaySfx(NA_SE_SY_DECIDE);
    }

    /**
     * @brief Cancels any pending mask operation.
     */
    void cancelPendingOperation() {
        gMaskState.pendingMask = ITEM_NONE;
        gMaskState.state = MaskEquipState::None;
    }
};

static MaskEquipManager maskEquipManager;

/* ============================================================================
   Hook Registration
   ============================================================================ */

/**
 * @brief Registers mask equip hooks and initializes the module.
 */
void registerEasyMaskEquip() {
    COND_HOOK(OnKaleidoUpdate, CVAR, [](PauseContext* pauseCtx) {
        if (pauseCtx->pageIndex == PAUSE_MASK) {
            maskEquipManager.updateState(static_cast<ItemId>(Player_GetCurMaskItemId(gPlayState)));
            maskEquipManager.handleInput(pauseCtx);
        }
    });

    COND_VB_SHOULD(VB_DRAW_MASK_ITEM, CVAR, {
        u16* itemId = va_arg(args, u16*);
        s16* index = va_arg(args, s16*);
        if (isTransformationMask(static_cast<ItemId>(*itemId))) {
            renderMaskItem(&gPlayState->pauseCtx, *itemId, *index);
            *should = false;
        } else {
            *should = true;
        }
    });

    COND_ID_HOOK(BeforeKaleidoDrawPage, PAUSE_MASK, CVAR, [](PauseContext* pauseCtx, u16) {
        if (pauseCtx->pageIndex == PAUSE_MASK) {
            allocateEquipBorderVertices(gPlayState->state.gfxCtx);
            drawEquipBorder(pauseCtx);
            drawPendingActionEffect(pauseCtx);
        }
    });

    COND_HOOK(OnKaleidoClose, CVAR, [](u16 pauseIndex) {
        if (pauseIndex != PAUSE_MASK)
            return;
        maskEquipManager.handlePauseClose();
    });

    COND_HOOK(OnActorUpdate, CVAR, [](Actor* actor) {
        if (actor->id != ACTOR_PLAYER)
            return;
        maskEquipManager.handlePlayerUpdate(reinterpret_cast<Player*>(actor));
    });

    COND_VB_SHOULD(VB_KALEIDO_DISPLAY_ITEM_TEXT, CVAR, {
        if (isTransformationMask(static_cast<ItemId>(gPlayState->pauseCtx.cursorItem[PAUSE_MASK])))
            *should = false;
    });

    COND_VB_SHOULD(VB_DRAW_ITEM_EQUIPPED_OUTLINE, CVAR, {
        ItemId* itemId = va_arg(args, ItemId*);
        const s16 currentMask = Player_GetCurMaskItemId(gPlayState);
        *should = !(isTransformationMask(*itemId) &&
                    ((gMaskState.state == MaskEquipState::PendingEquip && *itemId == gMaskState.pendingMask) ||
                     (gMaskState.state != MaskEquipState::PendingEquip && *itemId == currentMask)));
    });
}

static RegisterShipInitFunc initFunc(registerEasyMaskEquip, { CVAR_NAME });
