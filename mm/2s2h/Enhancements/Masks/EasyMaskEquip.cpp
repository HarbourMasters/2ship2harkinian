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
    Player* player = GET_PLAYER(gPlayState);

    // Allow all masks to be equipped if the Unrestricted Items cheat is enabled
    if (CVarGetInteger("gCheats.UnrestrictedItems", 0)) {
        return true;
    }

    // Check if the player is underwater
    if ((Player_GetEnvironmentalHazard(gPlayState) >= PLAYER_ENV_HAZARD_UNDERWATER_FLOOR) &&
        (Player_GetEnvironmentalHazard(gPlayState) <= PLAYER_ENV_HAZARD_UNDERWATER_FREE)) {

        // Only allow equipping Zora Mask underwater
        if (cursorItem != ITEM_MASK_ZORA) {
            return false; // Prevent equipping any mask other than Zora Mask
        }
    }

    // Check if the player is climbing (some of these might be redundant, but it works)
    if ((player->stateFlags1 & (PLAYER_STATE1_4 | PLAYER_STATE1_4000 | PLAYER_STATE1_40000 | PLAYER_STATE1_200000)) &&
        IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        // Player is climbing and trying to equip a transformation mask, do not allow
        return false;
    }

    // Prevent equipping transformation masks if interacting with an object
    if (IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        if ((player->stateFlags2 & PLAYER_STATE2_10) ||  // Pushing/Pulling
            (player->stateFlags1 & PLAYER_STATE1_800) || // Carrying
            (player->stateFlags2 & PLAYER_STATE2_1)) {   // Grabbing
            return false;
        }
    }

    // Check if the player is interacting with Epona
    if (player->rideActor != NULL && player->rideActor->id == ACTOR_EN_HORSE &&
        IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        return false; // Prevent equipping transformation masks while interacting with Epona
    }

    // Check if a minigame timer is active (prevent equipping masks during minigames)
    if (gSaveContext.timerStates[TIMER_ID_MINIGAME_1] != TIMER_STATE_OFF ||
        gSaveContext.timerStates[TIMER_ID_MINIGAME_2] != TIMER_STATE_OFF) {
        // If a minigame timer is active, do not equip any mask
        return false;
    }

    // Prevent equipping transformation masks when swinging a melee weapon
    if (player->meleeWeaponState != PLAYER_MELEE_WEAPON_STATE_0 &&
        IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        return false;
    }

    // Check if the mask is Fierce Deity Mask and if the player is in an allowed location
    if (cursorItem == ITEM_MASK_FIERCE_DEITY) {
        if (!CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0) && gPlayState->sceneId != SCENE_MITURIN_BS &&
            gPlayState->sceneId != SCENE_HAKUGIN_BS && gPlayState->sceneId != SCENE_SEA_BS &&
            gPlayState->sceneId != SCENE_INISIE_BS && gPlayState->sceneId != SCENE_LAST_BS) {
            return false; // Prevent equipping Fierce Deity Mask outside allowed locations
        }
    }

    // Check if the player is in the air (not on the ground) and if the mask is a transformation mask
    if (!(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        (IsTransformationMask(static_cast<ItemId>(cursorItem)) || player->transformation != PLAYER_FORM_HUMAN)) {
        return false; // Player is in the air and in a transformation form, cannot equip any mask
    }

    // Check if the player is interacting with a Deku Flower (some might be redundant because of the above check, but it
    // works)
    if (player->stateFlags3 & (PLAYER_STATE3_200 | PLAYER_STATE3_2000 | PLAYER_STATE3_100)) {
        return false; // Prevent equipping any mask while interacting with a Deku Flower
    }

    // Check if the mask is Giant's Mask and if the player is in the allowed location with enough magic
    if (cursorItem == ITEM_MASK_GIANT) {
        if (gPlayState->sceneId != SCENE_INISIE_BS || gSaveContext.save.saveInfo.playerData.magic == 0) {
            return false; // Prevent equipping Giant's Mask outside SCENE_INISIE_BS or if magic is depleted
        }
    }

    // Prevent equipping any mask if Giant's Mask is already equipped, except for Giant's Mask
    if (player->currentMask == PLAYER_MASK_GIANT && cursorItem != ITEM_MASK_GIANT) {
        return false;
    }

    // Prevent equipping Giant's Mask while wearing a transformation mask
    if ((player->transformation != PLAYER_FORM_HUMAN) && cursorItem == ITEM_MASK_GIANT) {
        return false; // Prevent equipping Giant's Mask while wearing a transformation mask
    }

    // Prevent equipping transformation masks in first-person mode
    if ((player->stateFlags1 & PLAYER_STATE1_100000) && IsTransformationMask(static_cast<ItemId>(cursorItem))) {
        return false;
    }

    // We can add tons more checks here, but I need help to account for all scenarios

    return true; // Allow equipping other masks
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
        if (cursorItem != ITEM_NONE && !ShouldEquipMask(cursorItem)) {
            gDPSetGrayscaleColor(POLY_OPA_DISP++, 109, 109, 109, 255);
            gSPGrayscale(POLY_OPA_DISP++, true);
            gSPVertex(POLY_OPA_DISP++, reinterpret_cast<uintptr_t>(&pauseCtx->maskVtx[i * 4]), 4, 0); // Fixed cast
            KaleidoScope_DrawTexQuadRGBA32(gfxCtx, gItemIcons[cursorItem], 32, 32, 0);
            gSPGrayscale(POLY_OPA_DISP++, false);
        }
    }

    CLOSE_DISPS(gfxCtx);
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

            if (!ShouldEquipMask(cursorItem)) {
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
