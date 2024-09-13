#include "EasyMaskEquip.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "Enhancements/FrameInterpolation/FrameInterpolation.h"

extern "C" {
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "interface/parameter_static/parameter_static.h"
#include "macros.h"
#include "variables.h"
extern u16 sMasksGivenOnMoonBits[];
extern s16 sPauseMenuVerticalOffset;
}

static Vtx* easyMaskEquipVtx = nullptr;
static s16 sPendingMask = ITEM_NONE;
static s16 sLastEquippedMask = ITEM_NONE;
static bool sIsTransforming = false;

// Define transformation masks for easy reference
constexpr std::array<ItemId, 5> TransformationMasks = { ITEM_MASK_DEKU, ITEM_MASK_GORON, ITEM_MASK_ZORA,
                                                        ITEM_MASK_FIERCE_DEITY, ITEM_MASK_GIANT };

// Helper function to check if a mask is a transformation mask
bool IsTransformationMask(ItemId mask) {
    return std::find(TransformationMasks.begin(), TransformationMasks.end(), mask) != TransformationMasks.end();
}

// Check if EasyMaskEquip is enabled
bool EasyMaskEquip_IsEnabled() {
    return CVarGetInteger("gEnhancements.Masks.EasyMaskEquip", 0) &&
           gPlayState->pauseCtx.debugEditor == DEBUG_EDITOR_NONE;
}

// Get the equipped mask slot, prioritizing pending masks
s16 GetEquippedMaskSlot() {
    s16 maskToCheck =
        (sPendingMask != ITEM_NONE && !sIsTransforming) ? sPendingMask : Player_GetCurMaskItemId(gPlayState);
    maskToCheck = (maskToCheck == ITEM_NONE || sIsTransforming) ? sLastEquippedMask : maskToCheck;

    auto& items = gSaveContext.save.saveInfo.inventory.items;

    // Search for pending mask first
    if (sPendingMask != ITEM_NONE) {
        for (s16 i = 0; i < MASK_NUM_SLOTS; ++i) {
            if (items[i + ITEM_NUM_SLOTS] == sPendingMask) {
                return i;
            }
        }
    }

    // Then search for the equipped mask
    for (s16 i = 0; i < MASK_NUM_SLOTS; ++i) {
        if (items[i + ITEM_NUM_SLOTS] == maskToCheck) {
            return i;
        }
    }

    return -1;
}

// Update vertex positions based on the equipped mask slot
void UpdateEasyMaskEquipVtx(PauseContext* pauseCtx) {
    s16 slot = GetEquippedMaskSlot();
    if (slot == -1)
        return;

    const s16 slotX = slot % MASK_GRID_COLS;
    const s16 slotY = slot / MASK_GRID_COLS;
    const s16 initialX = -(MASK_GRID_COLS * MASK_GRID_CELL_WIDTH) / 2;
    const s16 initialY = (MASK_GRID_ROWS * MASK_GRID_CELL_HEIGHT) / 2 - 6;
    const s16 vtxX = initialX + (slotX * MASK_GRID_CELL_WIDTH);
    const s16 vtxY = initialY - (slotY * MASK_GRID_CELL_HEIGHT) + pauseCtx->offsetY;

    const std::array<s16, 4> xCoords = { vtxX, vtxX + MASK_GRID_CELL_WIDTH, vtxX, vtxX + MASK_GRID_CELL_WIDTH };
    const std::array<s16, 4> yCoords = { vtxY, vtxY, vtxY - MASK_GRID_CELL_HEIGHT, vtxY - MASK_GRID_CELL_HEIGHT };

    for (int i = 0; i < 4; ++i) {
        easyMaskEquipVtx[i].v.ob[0] = xCoords[i];
        easyMaskEquipVtx[i].v.ob[1] = yCoords[i];
    }
}

// Determine if a mask should be equipped based on various conditions
bool ShouldEquipMask(s16 cursorItem) {
    const Player* player = GET_PLAYER(gPlayState);

    // Check if player is airborne and attempting to equip a transformation mask
    bool isAirborne = !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND);
    bool notInWater = !(Player_GetEnvironmentalHazard(gPlayState) >= PLAYER_ENV_HAZARD_UNDERWATER_FLOOR &&
                        Player_GetEnvironmentalHazard(gPlayState) <= PLAYER_ENV_HAZARD_UNDERWATER_FREE);
    if (isAirborne && notInWater) {
        if (player->transformation == PLAYER_FORM_HUMAN && IsTransformationMask(static_cast<ItemId>(cursorItem))) {
            return true;
        }
        if (player->transformation != PLAYER_FORM_HUMAN) {
            return true;
        }
    }

    // Check if underwater and trying to equip non-Zora masks
    auto hazard = Player_GetEnvironmentalHazard(gPlayState);
    if (hazard >= PLAYER_ENV_HAZARD_UNDERWATER_FLOOR && hazard <= PLAYER_ENV_HAZARD_UNDERWATER_FREE &&
        cursorItem != ITEM_MASK_ZORA) {
        return true;
    }

    // Check if the mask is given on the moon. Need to test this.
    if (cursorItem >= ITEM_NUM_SLOTS && (cursorItem - ITEM_NUM_SLOTS) < MASK_NUM_SLOTS &&
        CHECK_GIVEN_MASK_ON_MOON(cursorItem - ITEM_NUM_SLOTS)) {
        return true;
    }

    // Check if player is busy and trying to equip a transformation mask
    if (IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        const auto& flags1 = player->stateFlags1;
        const auto& flags2 = player->stateFlags2;
        const auto heldAction = player->heldItemAction;

        bool isBusy = flags1 & (PLAYER_STATE1_800 | PLAYER_STATE1_400 | PLAYER_STATE1_20000000) ||
                      flags2 & (PLAYER_STATE2_10 | PLAYER_STATE2_80) || heldAction == PLAYER_IA_BOW ||
                      heldAction == PLAYER_IA_BOW_FIRE || heldAction == PLAYER_IA_BOW_ICE ||
                      heldAction == PLAYER_IA_BOW_LIGHT || heldAction == PLAYER_IA_BOMB ||
                      heldAction == PLAYER_IA_BOMBCHU || heldAction == PLAYER_IA_HOOKSHOT ||
                      heldAction == PLAYER_IA_POWDER_KEG;

        if (isBusy)
            return true;
    }

    return false;
}

// Draw the border around the equipped mask
void DrawEasyMaskEquipBorder(PauseContext* pauseCtx) {
    s16 slot = GetEquippedMaskSlot();
    if (slot == -1 || gSaveContext.save.saveInfo.inventory.items[slot + ITEM_NUM_SLOTS] == ITEM_NONE ||
        Player_GetCurMaskItemId(gPlayState) == sPendingMask) {
        return;
    }

    GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
    OPEN_DISPS(gfxCtx);

    UpdateEasyMaskEquipVtx(pauseCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
    gSPVertex(POLY_OPA_DISP++, reinterpret_cast<uintptr_t>(easyMaskEquipVtx), 4, 0); // Fixed cast
    gDPLoadTextureBlock(POLY_OPA_DISP++, gEquippedItemOutlineTex, G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

    CLOSE_DISPS(gfxCtx);
}

// Allocate memory for the vertex buffer
void AllocateEasyMaskEquipVtx(GraphicsContext* gfxCtx) {
    easyMaskEquipVtx = static_cast<Vtx*>(GRAPH_ALLOC(gfxCtx, 4 * sizeof(Vtx)));
    for (int i = 0; i < 4; ++i) {
        easyMaskEquipVtx[i].v.ob[2] = 0;
        easyMaskEquipVtx[i].v.flag = 0;
        easyMaskEquipVtx[i].v.tc[0] = (i & 1) ? 32 << 5 : 0;
        easyMaskEquipVtx[i].v.tc[1] = (i & 2) ? 32 << 5 : 0;
        std::fill(std::begin(easyMaskEquipVtx[i].v.cn), std::end(easyMaskEquipVtx[i].v.cn), static_cast<u8>(255));
    }
}

// Apply grayscale to mask icons that shouldn't be equipped
void ApplyMaskIconGrayscale(PauseContext* pauseCtx, GraphicsContext* gfxCtx) {
    OPEN_DISPS(gfxCtx);

    for (int i = 0; i < MASK_NUM_SLOTS; ++i) {
        const s16 cursorItem = gSaveContext.save.saveInfo.inventory.items[i + ITEM_NUM_SLOTS];
        if (cursorItem != ITEM_NONE && ShouldEquipMask(cursorItem)) {
            gDPSetGrayscaleColor(POLY_OPA_DISP++, 109, 109, 109, 255);
            gSPGrayscale(POLY_OPA_DISP++, true);
            gSPVertex(POLY_OPA_DISP++, reinterpret_cast<uintptr_t>(&pauseCtx->maskVtx[i * 4]), 4, 0); // Fixed cast
            KaleidoScope_DrawTexQuadRGBA32(gfxCtx, gItemIcons[cursorItem], 32, 32, 0);
            gSPGrayscale(POLY_OPA_DISP++, false);
        }
    }

    CLOSE_DISPS(gfxCtx);
}

// Placeholder for additional grayscale functionality on the item page
void ApplyItemPageMaskGrayscale(PauseContext* pauseCtx, GraphicsContext* gfxCtx) {
    // TODO: Implement, but I'm at a loss for how to do this.
}

// Handle equipping masks based on player input
void HandleEasyMaskEquip(PauseContext* pauseCtx) {
    if (pauseCtx->state != PAUSE_STATE_MAIN || pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE)
        return;

    // Directly access the pressed buttons without using a Controller* variable
    u16 pressedButtons = CONTROLLER1(&gPlayState->state)->press.button;

    if (CHECK_BTN_ALL(pressedButtons, BTN_A)) {
        s16 cursorItem = pauseCtx->cursorItem[PAUSE_MASK];
        if (cursorItem != PAUSE_ITEM_NONE) {
            // If sPendingMask is already the cursor item, remove the border and reset sPendingMask
            if (sPendingMask == cursorItem) {
                sPendingMask = ITEM_NONE;
                sIsTransforming = false;
                return;
            }

            if (ShouldEquipMask(cursorItem)) {
                Audio_PlaySfx(NA_SE_SY_ERROR);
                return;
            }

            sPendingMask = cursorItem;
            sIsTransforming = IsTransformationMask(static_cast<ItemId>(cursorItem));
            UpdateEasyMaskEquipVtx(pauseCtx);
            Audio_PlaySfx(NA_SE_SY_DECIDE);
        }
    } else if (CHECK_BTN_ALL(pressedButtons, BTN_B)) {
        sPendingMask = ITEM_NONE;
        sIsTransforming = false;
        sLastEquippedMask = ITEM_NONE;
    }
}

// Register hooks for the EasyMaskEquip functionality
void RegisterEasyMaskEquip() {
    auto gameInteractor = GameInteractor::Instance;

    // Handle mask equipping logic during KaleidoScope update
    gameInteractor->RegisterGameHook<GameInteractor::OnKaleidoUpdate>([](PauseContext* pauseCtx) {
        if (EasyMaskEquip_IsEnabled() && pauseCtx->pageIndex == PAUSE_MASK) {
            HandleEasyMaskEquip(pauseCtx);
        }
    });

    // Apply grayscale to mask icons after drawing the mask page
    gameInteractor->RegisterGameHookForID<GameInteractor::AfterKaleidoDrawPage>(
        PAUSE_MASK, [](PauseContext* pauseCtx, u16) {
            if (EasyMaskEquip_IsEnabled()) {
                ApplyMaskIconGrayscale(pauseCtx, gPlayState->state.gfxCtx);
            }
        });

    // Apply grayscale to item page masks after drawing the item page
    gameInteractor->RegisterGameHookForID<GameInteractor::AfterKaleidoDrawPage>(
        PAUSE_ITEM, [](PauseContext* pauseCtx, u16) {
            if (EasyMaskEquip_IsEnabled()) {
                ApplyItemPageMaskGrayscale(pauseCtx, gPlayState->state.gfxCtx);
            }
        });

    // Draw the equip border before drawing the mask page
    gameInteractor->RegisterGameHookForID<GameInteractor::BeforeKaleidoDrawPage>(
        PAUSE_MASK, [](PauseContext* pauseCtx, u16) {
            if (EasyMaskEquip_IsEnabled() && pauseCtx->pageIndex == PAUSE_MASK) {
                GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
                OPEN_DISPS(gfxCtx);
                AllocateEasyMaskEquipVtx(gfxCtx);
                UpdateEasyMaskEquipVtx(pauseCtx);
                DrawEasyMaskEquipBorder(pauseCtx);
                CLOSE_DISPS(gfxCtx);
            }
        });

    // Handle mask equipping when KaleidoScope closes
    gameInteractor->RegisterGameHook<GameInteractor::OnKaleidoClose>([]() {
        if (!EasyMaskEquip_IsEnabled() || gPlayState->pauseCtx.pageIndex != PAUSE_MASK)
            return;

        Player* player = GET_PLAYER(gPlayState);
        if (sPendingMask == ITEM_NONE)
            return;

        bool isTransformation = IsTransformationMask(static_cast<ItemId>(sPendingMask));
        bool isCurrentlyTransformed = (player->transformation != PLAYER_FORM_HUMAN);

        if (!isTransformation) {
            if (isCurrentlyTransformed) {
                // Equip a regular mask while transformed: transform back to human first
                Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                sLastEquippedMask = sPendingMask;
                // Retain sPendingMask to equip after transformation
                sIsTransforming = true;
            } else {
                // Equip a regular mask while already human
                Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                sLastEquippedMask = sPendingMask;
                sPendingMask = ITEM_NONE;
                sIsTransforming = false;
            }
        } else {
            // Equip a transformation mask
            Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
            sLastEquippedMask = sPendingMask;
            sPendingMask = ITEM_NONE;
            sIsTransforming = true;
        }
    });

    // Update mask equipping state during actor updates
    gameInteractor->RegisterGameHook<GameInteractor::OnActorUpdate>([](Actor* actor) {
        if (!EasyMaskEquip_IsEnabled() || actor->id != ACTOR_PLAYER)
            return;

        Player* player = reinterpret_cast<Player*>(actor);
        if (sIsTransforming && player->transformation == PLAYER_FORM_HUMAN &&
            player->skelAnime.curFrame >= player->skelAnime.endFrame) {
            if (sPendingMask != ITEM_NONE) {
                Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                sLastEquippedMask = sPendingMask;
                sPendingMask = ITEM_NONE;
                sIsTransforming = false;
            }
        }
    });
}
