#include "2s2h/Enhancements/Enhancements.h"
#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
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
static bool sShouldUnequipMask = false;

// Define transformation masks for easy reference
constexpr std::array<ItemId, 5> TransformationMasks = { ITEM_MASK_DEKU, ITEM_MASK_GORON, ITEM_MASK_ZORA,
                                                        ITEM_MASK_FIERCE_DEITY, ITEM_MASK_GIANT };

// Helper function to check if a given mask is a transformation mask
bool IsTransformationMask(ItemId mask) {
    return std::binary_search(TransformationMasks.begin(), TransformationMasks.end(), mask);
}

// Check if the Easy Mask Equip feature is enabled
bool EasyMaskEquip_IsEnabled() {
    return CVarGetInteger("gEnhancements.Masks.EasyMaskEquip", 0) &&
           gPlayState->pauseCtx.debugEditor == DEBUG_EDITOR_NONE;
}

// Get the slot index of the currently equipped mask, prioritizing any pending mask
s16 GetEquippedMaskSlot() {
    if (sShouldUnequipMask) {
        // We intend to unequip the mask, so return -1 to indicate no mask
        return -1;
    }

    s16 maskToCheck =
        (sPendingMask != ITEM_NONE && !sIsTransforming) ? sPendingMask : Player_GetCurMaskItemId(gPlayState);
    maskToCheck = (maskToCheck == ITEM_NONE || sIsTransforming) ? sLastEquippedMask : maskToCheck;

    auto& items = gSaveContext.save.saveInfo.inventory.items;

    // Search for the pending mask first
    if (sPendingMask != ITEM_NONE) {
        for (s16 i = 0; i < MASK_NUM_SLOTS; ++i) {
            if (items[i + ITEM_NUM_SLOTS] == sPendingMask) {
                return i;
            }
        }
    }

    // Then search for the currently equipped mask
    for (s16 i = 0; i < MASK_NUM_SLOTS; ++i) {
        if (items[i + ITEM_NUM_SLOTS] == maskToCheck) {
            return i;
        }
    }

    return -1; // Mask not found
}

// Update the vertex positions of the mask equip border based on the equipped mask slot
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

// Determine if a mask can be equipped based on various conditions
bool ShouldEquipMask(s16 cursorItem) {
    Player* player = GET_PLAYER(gPlayState);

    if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0) && cursorItem == ITEM_MASK_BUNNY) {
        return true; // Always allow equipping the Bunny Hood if the persistent option is enabled
    }

    // Allow all masks to be equipped if the Unrestricted Items cheat is enabled
    if (CVarGetInteger("gCheats.UnrestrictedItems", 0)) {
        return true;
    }

    // Check if the player is underwater
    if ((Player_GetEnvironmentalHazard(gPlayState) >= PLAYER_ENV_HAZARD_UNDERWATER_FLOOR) &&
        (Player_GetEnvironmentalHazard(gPlayState) <= PLAYER_ENV_HAZARD_UNDERWATER_FREE)) {

        // Only allow equipping the Zora Mask underwater
        if (cursorItem == ITEM_MASK_ZORA) {
            return true;
        } else {
            return false;
        }
    }

    // Prevent equipping transformation masks while climbing
    if ((player->stateFlags1 &
         (PLAYER_STATE1_4 | PLAYER_STATE1_4000 | PLAYER_STATE1_40000 | PLAYER_STATE1_200000 | PLAYER_STATE1_2000)) &&
        IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        return false;
    }

    // Prevent equipping transformation masks while interacting with objects
    if (IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        if ((player->stateFlags2 & PLAYER_STATE2_10) ||  // Pushing/Pulling
            (player->stateFlags1 & PLAYER_STATE1_800) || // Carrying
            (player->stateFlags2 & PLAYER_STATE2_1)) {   // Grabbing
            return false;
        }
    }

    // Prevent equipping transformation masks while riding Epona
    if (player->rideActor != NULL && player->rideActor->id == ACTOR_EN_HORSE &&
        IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        return false;
    }

    // Prevent equipping masks during minigames with active timers
    if (gSaveContext.timerStates[TIMER_ID_MINIGAME_1] != TIMER_STATE_OFF ||
        gSaveContext.timerStates[TIMER_ID_MINIGAME_2] != TIMER_STATE_OFF) {
        return false;
    }

    // Prevent equipping transformation masks while swinging a melee weapon
    if (player->meleeWeaponState != PLAYER_MELEE_WEAPON_STATE_0 &&
        IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        return false;
    }

    // Prevent equipping Fierce Deity Mask outside allowed locations if the enhancement is not enabled
    if (cursorItem == ITEM_MASK_FIERCE_DEITY) {
        if (!CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0) && gPlayState->sceneId != SCENE_MITURIN_BS &&
            gPlayState->sceneId != SCENE_HAKUGIN_BS && gPlayState->sceneId != SCENE_SEA_BS &&
            gPlayState->sceneId != SCENE_INISIE_BS && gPlayState->sceneId != SCENE_LAST_BS) {
            return false;
        }
    }

    // Prevent equipping transformation masks while in the air (not on the ground)
    if (!(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        (IsTransformationMask(static_cast<ItemId>(cursorItem)) || player->transformation != PLAYER_FORM_HUMAN)) {
        return false;
    }

    // Prevent equipping any mask while interacting with a Deku Flower
    if (player->stateFlags3 & (PLAYER_STATE3_200 | PLAYER_STATE3_2000 | PLAYER_STATE3_100)) {
        return false;
    }

    // Prevent equipping Giant's Mask outside of the allowed scene or without magic
    if (cursorItem == ITEM_MASK_GIANT) {
        if (gPlayState->sceneId != SCENE_INISIE_BS || gSaveContext.save.saveInfo.playerData.magic == 0) {
            return false;
        }
    }

    // Prevent equipping any mask other than Giant's Mask if already wearing the Giant's Mask
    if (player->currentMask == PLAYER_MASK_GIANT && cursorItem != ITEM_MASK_GIANT) {
        return false;
    }

    // Prevent equipping Giant's Mask while wearing another transformation mask
    if ((player->transformation != PLAYER_FORM_HUMAN) && cursorItem == ITEM_MASK_GIANT) {
        return false;
    }

    // Prevent equipping transformation masks in first-person mode
    if ((player->stateFlags1 & PLAYER_STATE1_100000) && IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        return false;
    }

    // Additional checks can be added here to account for more scenarios

    return true; // Allow equipping the mask
}

// Draw the border around the equipped mask in the pause menu
void DrawEasyMaskEquipBorder(PauseContext* pauseCtx) {
    s16 slot = GetEquippedMaskSlot();
    if (slot == -1 || gSaveContext.save.saveInfo.inventory.items[slot + ITEM_NUM_SLOTS] == ITEM_NONE ||
        Player_GetCurMaskItemId(gPlayState) == sPendingMask) {
        return; // No mask equipped or pending
    }

    GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
    OPEN_DISPS(gfxCtx);

    UpdateEasyMaskEquipVtx(pauseCtx);

    // Set up drawing parameters
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 230, 190, 255, pauseCtx->alpha); // Lavender color
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
    gSPVertex(POLY_OPA_DISP++, reinterpret_cast<uintptr_t>(easyMaskEquipVtx), 4, 0);
    gDPLoadTextureBlock(POLY_OPA_DISP++, gEquippedItemOutlineTex, G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

    CLOSE_DISPS(gfxCtx);
}

// Allocate memory for the vertex buffer used in drawing the equip border
void AllocateEasyMaskEquipVtx(GraphicsContext* gfxCtx) {
    easyMaskEquipVtx = static_cast<Vtx*>(GRAPH_ALLOC(gfxCtx, 4 * sizeof(Vtx)));
    for (int i = 0; i < 4; ++i) {
        easyMaskEquipVtx[i].v.ob[2] = 0;
        easyMaskEquipVtx[i].v.flag = 0;
        easyMaskEquipVtx[i].v.tc[0] = (i & 1) ? 32 << 5 : 0;
        easyMaskEquipVtx[i].v.tc[1] = (i & 2) ? 32 << 5 : 0;
        std::fill(std::begin(easyMaskEquipVtx[i].v.cn), std::end(easyMaskEquipVtx[i].v.cn), 255);
    }
}

// Render the mask item in the pause menu, applying grayscale if it cannot be equipped
void RenderMaskItem(PauseContext* pauseCtx, u16 itemId, s16 j) {
    GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
    OPEN_DISPS(gfxCtx);

    bool shouldGrayscale = !ShouldEquipMask(itemId);
    if (shouldGrayscale) {
        gDPSetGrayscaleColor(POLY_OPA_DISP++, 109, 109, 109, 255);
        gSPGrayscale(POLY_OPA_DISP++, true);
    }

    gSPVertex(POLY_OPA_DISP++, reinterpret_cast<uintptr_t>(&pauseCtx->maskVtx[j]), 4, 0);
    KaleidoScope_DrawTexQuadRGBA32(gfxCtx, gItemIcons[itemId], 32, 32, 0);

    if (shouldGrayscale) {
        gSPGrayscale(POLY_OPA_DISP++, false);
    }

    CLOSE_DISPS(gfxCtx);
}

// Handle equipping masks based on player input in the pause menu
void HandleEasyMaskEquip(PauseContext* pauseCtx) {
    if (pauseCtx->state != PAUSE_STATE_MAIN || pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE)
        return;

    u16 pressedButtons = CONTROLLER1(&gPlayState->state)->press.button;

    if (CHECK_BTN_ALL(pressedButtons, BTN_A)) {
        s16 cursorItem = pauseCtx->cursorItem[PAUSE_MASK];
        if (cursorItem != PAUSE_ITEM_NONE) {
            // Handle early exit for Persistent Bunny Hood
            if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0) && cursorItem == ITEM_MASK_BUNNY) {
                return;
            }

            Player* player = GET_PLAYER(gPlayState);
            bool isTransformation = IsTransformationMask(static_cast<ItemId>(cursorItem));
            bool isCurrentlyTransformed = (player->transformation != PLAYER_FORM_HUMAN);
            s16 currentMaskItemId = Player_GetCurMaskItemId(gPlayState);

            if (cursorItem == currentMaskItemId) {
                if (sPendingMask == ITEM_NONE && !sShouldUnequipMask) {
                    // Currently wearing the mask, no pending action
                    // Initiate unequip action
                    sShouldUnequipMask = true;
                    Audio_PlaySfx(NA_SE_SY_CANCEL); // Play the cancel sound
                } else if (sShouldUnequipMask) {
                    // Cancel the unequip action
                    sShouldUnequipMask = false;
                    Audio_PlaySfx(NA_SE_SY_DECIDE); // Play the select sound
                } else if (sPendingMask != ITEM_NONE) {
                    // Cancel pending mask equip and keep current mask
                    sPendingMask = ITEM_NONE;
                    sIsTransforming = false;
                    sShouldUnequipMask = false;
                    Audio_PlaySfx(NA_SE_SY_DECIDE); // Play the select sound
                }
                return;
            }

            if (cursorItem == sPendingMask) {
                // Mask is already pending, cancel pending equip
                sPendingMask = ITEM_NONE;
                sIsTransforming = false;
                Audio_PlaySfx(NA_SE_SY_CANCEL);

                if (currentMaskItemId != ITEM_NONE) {
                    // Set flag to unequip current mask
                    sShouldUnequipMask = true;
                }

                return;
            }

            if (!ShouldEquipMask(cursorItem)) {
                Audio_PlaySfx(NA_SE_SY_ERROR);
                return;
            }

            // Determine if we can equip the mask immediately
            if (!isTransformation && !isCurrentlyTransformed) {
                // Regular mask and player is human, equip immediately
                Player_UseItem(gPlayState, player, static_cast<ItemId>(cursorItem));
                sLastEquippedMask = cursorItem;
                sPendingMask = ITEM_NONE;
                sIsTransforming = false;
                sShouldUnequipMask = false;
                Audio_PlaySfx(NA_SE_SY_DECIDE);
            } else {
                // Transformation mask or player is transformed, set as pending
                sPendingMask = cursorItem;
                sIsTransforming = isTransformation;
                sShouldUnequipMask = false;
                Audio_PlaySfx(NA_SE_SY_DECIDE);
            }
        }
    } else if (CHECK_BTN_ALL(pressedButtons, BTN_B)) {
        // Cancel any pending mask equip or unequip action
        sPendingMask = ITEM_NONE;
        sIsTransforming = false;
        sLastEquippedMask = ITEM_NONE;
        sShouldUnequipMask = false;
    }
}

// Register the Easy Mask Equip functionality with the game interactor
void RegisterEasyMaskEquip() {
    auto gameInteractor = GameInteractor::Instance;

    // Handle mask equipping logic during pause menu update
    gameInteractor->RegisterGameHook<GameInteractor::OnKaleidoUpdate>([](PauseContext* pauseCtx) {
        if (EasyMaskEquip_IsEnabled() && pauseCtx->pageIndex == PAUSE_MASK) {
            HandleEasyMaskEquip(pauseCtx);
        }
    });

    // Override mask item rendering to apply grayscale if necessary
    gameInteractor->RegisterGameHook<GameInteractor::ShouldVanillaBehavior>(
        [](GIVanillaBehavior flag, bool* should, va_list args) {
            if (flag == VB_DRAW_MASK_ITEM && EasyMaskEquip_IsEnabled()) {
                u16* itemId = va_arg(args, u16*);
                s16* j = va_arg(args, s16*);
                *should = false;
                RenderMaskItem(&gPlayState->pauseCtx, *itemId, *j);
            }
        });

    // Draw the equip border before drawing the mask page
    gameInteractor->RegisterGameHookForID<GameInteractor::BeforeKaleidoDrawPage>(
        PAUSE_MASK, [](PauseContext* pauseCtx, u16) {
            if (EasyMaskEquip_IsEnabled() && pauseCtx->pageIndex == PAUSE_MASK) {
                AllocateEasyMaskEquipVtx(gPlayState->state.gfxCtx);
                UpdateEasyMaskEquipVtx(pauseCtx);
                DrawEasyMaskEquipBorder(pauseCtx);
            }
        });

    // Handle mask equipping when the pause menu closes
    gameInteractor->RegisterGameHook<GameInteractor::OnKaleidoClose>([]() {
        if (!EasyMaskEquip_IsEnabled() || gPlayState->pauseCtx.pageIndex != PAUSE_MASK)
            return;

        Player* player = GET_PLAYER(gPlayState);

        if (sPendingMask == ITEM_NONE) {
            if (sShouldUnequipMask) {
                // No pending mask to equip, but we intend to unequip the current mask
                s16 currentMaskItemId = Player_GetCurMaskItemId(gPlayState);
                if (currentMaskItemId != ITEM_NONE) {
                    // Simulate using the current mask to unequip it
                    Player_UseItem(gPlayState, player, static_cast<ItemId>(currentMaskItemId));
                    sLastEquippedMask = ITEM_NONE;
                }
                sShouldUnequipMask = false; // Reset the flag
            }
            return;
        }

        // Equip pending mask
        bool isTransformation = IsTransformationMask(static_cast<ItemId>(sPendingMask));
        bool isCurrentlyTransformed = (player->transformation != PLAYER_FORM_HUMAN);

        if (!isTransformation) {
            if (isCurrentlyTransformed) {
                // Equip a regular mask after transforming back to human
                Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                sLastEquippedMask = sPendingMask;
                // Retain sPendingMask to equip after transformation
                sIsTransforming = true;
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
        if (sIsTransforming && player->transformation == PLAYER_FORM_HUMAN) {
            if (sPendingMask != ITEM_NONE) {
                // If we have a pending mask, equip it now
                Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                sLastEquippedMask = sPendingMask;
                sPendingMask = ITEM_NONE;
                sIsTransforming = false;
            }
        }
    });

    // Disable the selected item textbox in the pause menu
    REGISTER_VB_SHOULD(VB_KALEIDO_DISPLAY_ITEM_TEXT, {
        if (EasyMaskEquip_IsEnabled()) {
            *should = false;
        }
    });

    // Remove the restriction requiring masks to be equipped from C or D buttons
    REGISTER_VB_SHOULD(VB_ALLOW_EQUIP_MASK, {
        if (EasyMaskEquip_IsEnabled()) {
            *should = false;
        }
    });

    // Prevent the white "equipped" border from being drawn for masks that are currently equipped or pending to be
    // equipped
    REGISTER_VB_SHOULD(VB_DRAW_ITEM_EQUIPPED_OUTLINE, {
        ItemId* itemId = va_arg(args, ItemId*);
        if (EasyMaskEquip_IsEnabled()) {
            s16 currentMaskItemId = Player_GetCurMaskItemId(gPlayState);

            // Suppress white border for the pending mask (lavender border will be drawn)
            if (*itemId == sPendingMask) {
                *should = false;
            }
            // Suppress white border for the currently equipped mask, if not being unequipped
            else if (sPendingMask == ITEM_NONE && !sShouldUnequipMask && *itemId == currentMaskItemId) {
                *should = false;
            }
            // Else, allow the white border to be drawn (default behavior)
        }
    });
}
