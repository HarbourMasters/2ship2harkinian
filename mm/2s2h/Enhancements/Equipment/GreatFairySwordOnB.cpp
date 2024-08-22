#include <libultraship/bridge.h>
#include <spdlog/spdlog.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "Enhancements/FrameInterpolation/FrameInterpolation.h"

extern "C" {
#include "src/overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/interface/parameter_static/parameter_static.h"

extern PlayState* gPlayState;
}

 
// TODO: Decide whether to allow equiping GFS on C while active. If so, should the white border be suppressed?
// TODO: Bremen and Blast mask actions aren't available when state is on. Should that be changed?


void RestoreSwordState() {
    u8 bItemId = ITEM_SWORD_KOKIRI + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI;
    // Check to prevent Light Arrows from being set to B if you're missing your normal sword.
    if (bItemId > ITEM_BOW_LIGHT && bItemId < ITEM_SHIELD_HERO) {
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = bItemId;
    } else {
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_NONE;
    }
    Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
}

void UpdateGreatFairySwordState() {
    static Vtx* persistentItemsVtx;
    static HOOK_ID beforePageDrawHook = 0;
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::BeforeKaleidoDrawPage>(beforePageDrawHook);

    if (!CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB", 0)) {
        CVarClear("gEnhancements.Equipment.GreatFairySwordB.State");
        if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) == ITEM_SWORD_GREAT_FAIRY) {
            RestoreSwordState();
        }
        return;
    }

    // If they don't have the item, clear the state
    if (INV_CONTENT(ITEM_SWORD_GREAT_FAIRY) != ITEM_SWORD_GREAT_FAIRY) {
        CVarClear("gEnhancements.Equipment.GreatFairySwordB.State");
    }

    // This hook sets up the quad and draws the "active" green border around the item in the pause menu
    beforePageDrawHook = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::BeforeKaleidoDrawPage>(
        PAUSE_ITEM, [](PauseContext* _, u16 __) {
            GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
            PauseContext* pauseCtx = &gPlayState->pauseCtx;
            s16 i = 0;
            s16 j = 0;
            s16 k;

            OPEN_DISPS(gfxCtx);

            persistentItemsVtx = (Vtx*)GRAPH_ALLOC(gfxCtx, (4 * 4) * sizeof(Vtx));

            if ((pauseCtx->state == PAUSE_STATE_MAIN)) {
                s16 slot = SLOT_SWORD_GREAT_FAIRY;
                s16 slotX = slot % ITEM_GRID_COLS;
                s16 slotY = slot / ITEM_GRID_COLS;
                s16 initialX = 0 - (ITEM_GRID_COLS * ITEM_GRID_CELL_WIDTH) / 2;
                s16 initialY = (ITEM_GRID_ROWS * ITEM_GRID_CELL_HEIGHT) / 2 - 6;
                s16 vtxX = (initialX + (slotX * ITEM_GRID_CELL_WIDTH)) + ITEM_GRID_QUAD_MARGIN;
                s16 vtxY = (initialY - (slotY * ITEM_GRID_CELL_HEIGHT)) + pauseCtx->offsetY - ITEM_GRID_QUAD_MARGIN;
                persistentItemsVtx[i + 0].v.ob[0] = persistentItemsVtx[i + 2].v.ob[0] =
                    vtxX + ITEM_GRID_SELECTED_QUAD_MARGIN;
                persistentItemsVtx[i + 1].v.ob[0] = persistentItemsVtx[i + 3].v.ob[0] =
                    persistentItemsVtx[i + 0].v.ob[0] + ITEM_GRID_SELECTED_QUAD_WIDTH;
                persistentItemsVtx[i + 0].v.ob[1] = persistentItemsVtx[i + 1].v.ob[1] =
                    vtxY - ITEM_GRID_SELECTED_QUAD_MARGIN;

                persistentItemsVtx[i + 2].v.ob[1] = persistentItemsVtx[i + 3].v.ob[1] =
                    persistentItemsVtx[i + 0].v.ob[1] - ITEM_GRID_SELECTED_QUAD_HEIGHT;

                persistentItemsVtx[i + 0].v.ob[2] = persistentItemsVtx[i + 1].v.ob[2] =
                    persistentItemsVtx[i + 2].v.ob[2] = persistentItemsVtx[i + 3].v.ob[2] = 0;

                persistentItemsVtx[i + 0].v.flag = persistentItemsVtx[i + 1].v.flag = persistentItemsVtx[i + 2].v.flag =
                    persistentItemsVtx[i + 3].v.flag = 0;

                persistentItemsVtx[i + 0].v.tc[0] = persistentItemsVtx[i + 0].v.tc[1] =
                    persistentItemsVtx[i + 1].v.tc[1] = persistentItemsVtx[i + 2].v.tc[0] = 0;

                persistentItemsVtx[i + 1].v.tc[0] = persistentItemsVtx[i + 2].v.tc[1] =
                    persistentItemsVtx[i + 3].v.tc[0] = persistentItemsVtx[i + 3].v.tc[1] =
                        ITEM_GRID_SELECTED_QUAD_TEX_SIZE * (1 << 5);

                persistentItemsVtx[i + 0].v.cn[0] = persistentItemsVtx[i + 1].v.cn[0] =
                    persistentItemsVtx[i + 2].v.cn[0] = persistentItemsVtx[i + 3].v.cn[0] =
                        persistentItemsVtx[i + 0].v.cn[1] = persistentItemsVtx[i + 1].v.cn[1] =
                            persistentItemsVtx[i + 2].v.cn[1] = persistentItemsVtx[i + 3].v.cn[1] =
                                persistentItemsVtx[i + 0].v.cn[2] = persistentItemsVtx[i + 1].v.cn[2] =
                                    persistentItemsVtx[i + 2].v.cn[2] = persistentItemsVtx[i + 3].v.cn[2] = 255;

                persistentItemsVtx[i + 0].v.cn[3] = persistentItemsVtx[i + 1].v.cn[3] =
                    persistentItemsVtx[i + 2].v.cn[3] = persistentItemsVtx[i + 3].v.cn[3] = pauseCtx->alpha;

                if (CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB.State", 0)) {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 100, 255, 120, 255);
                    gSPVertex(POLY_OPA_DISP++, (uintptr_t)persistentItemsVtx, 4, 0);
                    POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, (TexturePtr)gEquippedItemOutlineTex, 32, 32, 0);
                }
            }
            CLOSE_DISPS(gfxCtx);
        });
}

void RegisterGreatFairySwordOnB() {
    UpdateGreatFairySwordState();

    // Easiest way to handle things like loading into a different file, debug warping, etc.
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>(
        [](s8 sceneId, s8 spawnNum) { UpdateGreatFairySwordState(); });

    // Ensure that the state is set when loading from a save (make sure the green square appears)
    // Or restore normal sword if enhancement was turned off
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(
        ACTOR_PLAYER, [](Actor* actor) {
            
            if (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) == ITEM_SWORD_GREAT_FAIRY) {
                if (CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB", 0)) {
                    CVarSetInteger("gEnhancements.Equipment.GreatFairySwordB.State", 1);
                } else {
                    RestoreSwordState();
                }
            }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnItemStolen>( [](u8 item) {
        if (CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB", 0) && CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB.State", 0)) {
            if (item == ITEM_SWORD_GREAT_FAIRY) {
                CVarClear("gEnhancements.Equipment.GreatFairySwordB.State");
                RestoreSwordState();
            } else if (item >= ITEM_SWORD_KOKIRI && item <= ITEM_SWORD_GILDED) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_GREAT_FAIRY;
                Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
            }
            
        }
    });

    // When transforming from FD, clear the state. 
    // In testing, this always executed before FastTransformation's hook. If execution order changed,
    // this would execute when transforming TO FD, which I would consider preferable.
    REGISTER_VB_SHOULD(GI_VB_PREVENT_MASK_TRANSFORMATION_CS, {
        Player* player = GET_PLAYER(gPlayState);
        if (player->transformation == PLAYER_FORM_FIERCE_DEITY) {
            CVarClear("gEnhancements.Equipment.GreatFairySwordB.State");
            // if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_DIETY) {
            //     RestoreSwordState();
            // }
        }
    });

    //Could this be used to handle Blast and Bremen?
    // This hook doesn't handle transformation masks
    // REGISTER_VB_SHOULD(GI_VB_USE_ITEM_EQUIP_MASK, {
    //     PlayerMask* maskId = (PlayerMask*)opt;
    //     Player* player = GET_PLAYER(gPlayState);
    //     LUSLOG_DEBUG("GI_VB_USE...", NULL);

    //     if (*maskId == PLAYER_MASK_FIERCE_DEITY && CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB", 0)) {
    //         CVarClear("gEnhancements.Equipment.GreatFairySwordB.State");
    //         LUSLOG_DEBUG("Cleared state", NULL);
    //     }
    // });

    // Override "A" press behavior on kaleido scope to toggle the item state
    REGISTER_VB_SHOULD(GI_VB_KALEIDO_DISPLAY_ITEM_TEXT, {
        ItemId* itemId = (ItemId*)opt;
        Player* player = GET_PLAYER(gPlayState);
        //LUSLOG_DEBUG("itemID: %x", *itemId);
        if (CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB", 0) && player->transformation == PLAYER_FORM_HUMAN && (*itemId & 0xFF) == ITEM_SWORD_GREAT_FAIRY ) {
            *should = false;
            CVarSetInteger("gEnhancements.Equipment.GreatFairySwordB.State",
                           !CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB.State", 0));
            if (CVarGetInteger("gEnhancements.Equipment.GreatFairySwordB.State", 0)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_GREAT_FAIRY;
                Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
                //Audio_PlaySfx(NA_SE_SY_CAMERA_ZOOM_DOWN);
                Audio_PlaySfx(NA_SE_SY_FAIRY_MASK_SUCCESS);
            } else {
                RestoreSwordState();
                Audio_PlaySfx(NA_SE_SY_CAMERA_ZOOM_UP);
            }
        } 
    });

    // // Prevent the "equipped" white border from being drawn so ours shows instead (ours was drawn before it, so it's
    // // underneath)
    // REGISTER_VB_SHOULD(GI_VB_DRAW_ITEM_EQUIPPED_OUTLINE, {
    //     if (*(ItemId*)opt == ITEM_MASK_BUNNY && CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
    //         *should = false;
    //     }
    // });
}
