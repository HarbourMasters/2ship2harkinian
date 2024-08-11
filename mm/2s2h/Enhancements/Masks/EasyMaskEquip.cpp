#include "EasyMaskEquip.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "Enhancements/FrameInterpolation/FrameInterpolation.h"

extern "C" {
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "interface/parameter_static/parameter_static.h"
}

static Vtx* easyMaskEquipVtx;
static s16 sPendingMask = ITEM_NONE;
static bool sIsTransforming = false;

extern "C" bool EasyMaskEquip_IsEnabled() {
    return CVarGetInteger("gEnhancements.Masks.EasyMaskEquip", 0) &&
           gPlayState->pauseCtx.debugEditor == DEBUG_EDITOR_NONE;
}

s16 GetEquippedMaskSlot() {
    s16 equippedMask = Player_GetCurMaskItemId(gPlayState);
    if (equippedMask == ITEM_NONE || sIsTransforming) {
        equippedMask = sPendingMask;
    }

    // Check for pending mask first
    if (sPendingMask != ITEM_NONE) {
        for (s16 i = 0; i < MASK_NUM_SLOTS; i++) {
            if (gSaveContext.save.saveInfo.inventory.items[i + ITEM_NUM_SLOTS] == sPendingMask) {
                return i;
            }
        }
    }

    // Check for equipped mask
    for (s16 i = 0; i < MASK_NUM_SLOTS; i++) {
        if (gSaveContext.save.saveInfo.inventory.items[i + ITEM_NUM_SLOTS] == equippedMask) {
            return i;
        }
    }
    return -1;
}

void UpdateEasyMaskEquipVtx(PauseContext* pauseCtx) {
    s16 slot = GetEquippedMaskSlot();
    if (slot == -1) {
        return;
    }

    s16 slotX = slot % MASK_GRID_COLS;
    s16 slotY = slot / MASK_GRID_COLS;
    s16 initialX = 0 - (MASK_GRID_COLS * MASK_GRID_CELL_WIDTH) / 2;
    s16 initialY = (MASK_GRID_ROWS * MASK_GRID_CELL_HEIGHT) / 2 - 6;
    s16 vtxX = initialX + (slotX * MASK_GRID_CELL_WIDTH);
    s16 vtxY = initialY - (slotY * MASK_GRID_CELL_HEIGHT) + pauseCtx->offsetY;

    easyMaskEquipVtx[0].v.ob[0] = easyMaskEquipVtx[2].v.ob[0] = vtxX;
    easyMaskEquipVtx[1].v.ob[0] = easyMaskEquipVtx[3].v.ob[0] = vtxX + MASK_GRID_CELL_WIDTH;
    easyMaskEquipVtx[0].v.ob[1] = easyMaskEquipVtx[1].v.ob[1] = vtxY;
    easyMaskEquipVtx[2].v.ob[1] = easyMaskEquipVtx[3].v.ob[1] = vtxY - MASK_GRID_CELL_HEIGHT;
}

void DrawEasyMaskEquipBorder(PauseContext* pauseCtx) {
    s16 slot = GetEquippedMaskSlot();
    if (slot == -1) {
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
    gSPVertex(POLY_OPA_DISP++, (uintptr_t)easyMaskEquipVtx, 4, 0);
    gDPLoadTextureBlock(POLY_OPA_DISP++, gEquippedItemOutlineTex, G_IM_FMT_IA, G_IM_SIZ_8b, 32, 32, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

    CLOSE_DISPS(gfxCtx);
}

void AllocateEasyMaskEquipVtx(GraphicsContext* gfxCtx) {
    easyMaskEquipVtx = (Vtx*)GRAPH_ALLOC(gfxCtx, 4 * sizeof(Vtx));
    for (int i = 0; i < 4; i++) {
        easyMaskEquipVtx[i].v.ob[2] = 0;
        easyMaskEquipVtx[i].v.flag = 0;
        easyMaskEquipVtx[i].v.tc[0] = (i & 1) ? 32 << 5 : 0;
        easyMaskEquipVtx[i].v.tc[1] = (i & 2) ? 32 << 5 : 0;
        easyMaskEquipVtx[i].v.cn[0] = easyMaskEquipVtx[i].v.cn[1] = easyMaskEquipVtx[i].v.cn[2] =
            easyMaskEquipVtx[i].v.cn[3] = 255;
    }
}

void HandleEasyMaskEquip(PauseContext* pauseCtx) {
    if (pauseCtx->state == PAUSE_STATE_MAIN && pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) {
        if (CHECK_BTN_ALL(CONTROLLER1(&gPlayState->state)->press.button, BTN_A)) {
            s16 cursorItem = pauseCtx->cursorItem[PAUSE_MASK];
            if (cursorItem != PAUSE_ITEM_NONE) {
                sPendingMask = cursorItem;
                sIsTransforming =
                    (cursorItem == ITEM_MASK_DEKU || cursorItem == ITEM_MASK_GORON || cursorItem == ITEM_MASK_ZORA ||
                     cursorItem == ITEM_MASK_FIERCE_DEITY || cursorItem == ITEM_MASK_GIANT);
                UpdateEasyMaskEquipVtx(pauseCtx);
            }
        } else if (CHECK_BTN_ALL(CONTROLLER1(&gPlayState->state)->press.button, BTN_B)) {
            sPendingMask = ITEM_NONE;
            sIsTransforming = false;
        }
    }
}

void RegisterEasyMaskEquip() {
    auto gameInteractor = GameInteractor::Instance;

    gameInteractor->RegisterGameHook<GameInteractor::OnKaleidoUpdate>([](PauseContext* pauseCtx) {
        if (EasyMaskEquip_IsEnabled() && pauseCtx->pageIndex == PAUSE_MASK) {
            HandleEasyMaskEquip(pauseCtx);
        }
    });

    gameInteractor->RegisterGameHookForID<GameInteractor::BeforeKaleidoDrawPage>(
        PAUSE_MASK, [](PauseContext* pauseCtx, u16 pageIndex) {
            if (EasyMaskEquip_IsEnabled() && pauseCtx->pageIndex == PAUSE_MASK) {
                GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
                OPEN_DISPS(gfxCtx);
                AllocateEasyMaskEquipVtx(gfxCtx);
                UpdateEasyMaskEquipVtx(pauseCtx);
                DrawEasyMaskEquipBorder(pauseCtx);
                CLOSE_DISPS(gfxCtx);
            }
        });

    gameInteractor->RegisterGameHook<GameInteractor::OnKaleidoClose>([]() {
        if (EasyMaskEquip_IsEnabled() && gPlayState->pauseCtx.pageIndex == PAUSE_MASK) {
            Player* player = GET_PLAYER(gPlayState);
            if (sPendingMask != ITEM_NONE) {
                if (sPendingMask != ITEM_MASK_DEKU && sPendingMask != ITEM_MASK_GORON &&
                    sPendingMask != ITEM_MASK_ZORA && sPendingMask != ITEM_MASK_FIERCE_DEITY &&
                    sPendingMask != ITEM_MASK_GIANT) {
                    if (player->transformation != PLAYER_FORM_HUMAN) {
                        // Equip the new mask directly if not a transformation mask and player is not human
                        Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                        sIsTransforming = true;
                    } else {
                        // Equip the new mask directly if not a transformation mask and player is human
                        Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                        sPendingMask = ITEM_NONE; // Clear pending mask after equipping
                    }
                } else {
                    // Handle transformation masks
                    Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                    sIsTransforming = true;
                }
            }
        }
    });

    gameInteractor->RegisterGameHook<GameInteractor::OnActorUpdate>([](Actor* actor) {
        if (EasyMaskEquip_IsEnabled() && actor->id == ACTOR_PLAYER) {
            Player* player = (Player*)actor;
            if (sIsTransforming && player->transformation == PLAYER_FORM_HUMAN) {
                if (player->skelAnime.curFrame == player->skelAnime.endFrame) {
                    if (sPendingMask != ITEM_NONE) {
                        Player_UseItem(gPlayState, player, static_cast<ItemId>(sPendingMask));
                        sPendingMask = ITEM_NONE; // Clear pending mask after equipping
                        sIsTransforming = false;
                    }
                }
            }
        }
    });
}
