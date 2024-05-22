#include <libultraship/bridge.h>
#include <spdlog/spdlog.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "Enhancements/FrameInterpolation/FrameInterpolation.h"

extern "C" {
#include "z64.h"
#include "z64player.h"
#include "functions.h"
#include "macros.h"
#include "gfx.h"
#include "src/overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/interface/parameter_static/parameter_static.h"

void func_8082E1F0(Player* player, u16 sfxId);
void Player_DrawBunnyHood(PlayState* play);
extern PlayState* gPlayState;
extern const char* D_801C0B20[28];
extern SaveContext gSaveContext;
extern u8 gPlayerFormItemRestrictions[PLAYER_FORM_MAX][114];
}

static Vtx* persistantMasksVtx = {};

void UpdatePersistentMasksState() {
    static HOOK_ID beforePageDrawHook = 0;
    static HOOK_ID onPlayerPostLimbDrawHook = 0;
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::BeforeKaleidoDrawPage>(beforePageDrawHook);
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnPlayerPostLimbDraw>(onPlayerPostLimbDrawHook);

    if (!CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0)) {
        CVarClear("gEnhancements.Masks.PersistentBunnyHood.State");
        gPlayerFormItemRestrictions[PLAYER_FORM_DEKU][ITEM_MASK_BUNNY] = 0;
        gPlayerFormItemRestrictions[PLAYER_FORM_GORON][ITEM_MASK_BUNNY] = 0;
        gPlayerFormItemRestrictions[PLAYER_FORM_ZORA][ITEM_MASK_BUNNY] = 0;
        return;
    }

    gPlayerFormItemRestrictions[PLAYER_FORM_DEKU][ITEM_MASK_BUNNY] = 1;
    gPlayerFormItemRestrictions[PLAYER_FORM_GORON][ITEM_MASK_BUNNY] = 1;
    gPlayerFormItemRestrictions[PLAYER_FORM_ZORA][ITEM_MASK_BUNNY] = 1;

    // If the mask is equipped, unequip it
    if (gSaveContext.save.equippedMask == PLAYER_MASK_BUNNY) {
        gSaveContext.save.equippedMask = PLAYER_MASK_NONE;
        CVarSetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 1);

        if (gPlayState != NULL) {
            Player* player = GET_PLAYER(gPlayState);
            player->prevMask = player->currentMask;
            player->currentMask = PLAYER_MASK_NONE;
        }
    } else {
        CVarClear("gEnhancements.Masks.PersistentBunnyHood.State");
    }

    onPlayerPostLimbDrawHook = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnPlayerPostLimbDraw>(
        PLAYER_LIMB_HEAD, [](Player* player, s32 limbIndex) {
            if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
                OPEN_DISPS(gPlayState->state.gfxCtx);
                Matrix_Push();
                Player_DrawBunnyHood(gPlayState);
                gSPDisplayList(POLY_OPA_DISP++, (Gfx*)D_801C0B20[PLAYER_MASK_BUNNY - 1]);
                Matrix_Pop();
                CLOSE_DISPS(gPlayState->state.gfxCtx);
            }
        });

    beforePageDrawHook = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::BeforeKaleidoDrawPage>(
        PAUSE_MASK, [](PauseContext* _, u16 __) {
            GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
            PauseContext* pauseCtx = &gPlayState->pauseCtx;
            s16 i = 0;
            s16 j = 0;
            s16 k;

            OPEN_DISPS(gfxCtx);

            persistantMasksVtx = (Vtx*)GRAPH_ALLOC(gfxCtx, (4 * 4) * sizeof(Vtx));

            if ((pauseCtx->state == PAUSE_STATE_MAIN)) {
                s16 slot = SLOT_MASK_BUNNY - ITEM_NUM_SLOTS;
                s16 slotX = slot % MASK_GRID_COLS;
                s16 slotY = slot / MASK_GRID_COLS;
                s16 initialX = 0 - (MASK_GRID_COLS * MASK_GRID_CELL_WIDTH) / 2;
                s16 initialY = (MASK_GRID_ROWS * MASK_GRID_CELL_HEIGHT) / 2 - 6;
                s16 vtxX = (initialX + (slotX * MASK_GRID_CELL_WIDTH)) + MASK_GRID_QUAD_MARGIN;
                s16 vtxY = (initialY - (slotY * MASK_GRID_CELL_HEIGHT)) + pauseCtx->offsetY - MASK_GRID_QUAD_MARGIN;
                persistantMasksVtx[i + 0].v.ob[0] = persistantMasksVtx[i + 2].v.ob[0] =
                    vtxX + MASK_GRID_SELECTED_QUAD_MARGIN;
                persistantMasksVtx[i + 1].v.ob[0] = persistantMasksVtx[i + 3].v.ob[0] =
                    persistantMasksVtx[i + 0].v.ob[0] + MASK_GRID_SELECTED_QUAD_WIDTH;
                persistantMasksVtx[i + 0].v.ob[1] = persistantMasksVtx[i + 1].v.ob[1] =
                    vtxY - MASK_GRID_SELECTED_QUAD_MARGIN;

                persistantMasksVtx[i + 2].v.ob[1] = persistantMasksVtx[i + 3].v.ob[1] =
                    persistantMasksVtx[i + 0].v.ob[1] - MASK_GRID_SELECTED_QUAD_HEIGHT;

                persistantMasksVtx[i + 0].v.ob[2] = persistantMasksVtx[i + 1].v.ob[2] =
                    persistantMasksVtx[i + 2].v.ob[2] = persistantMasksVtx[i + 3].v.ob[2] = 0;

                persistantMasksVtx[i + 0].v.flag = persistantMasksVtx[i + 1].v.flag = persistantMasksVtx[i + 2].v.flag =
                    persistantMasksVtx[i + 3].v.flag = 0;

                persistantMasksVtx[i + 0].v.tc[0] = persistantMasksVtx[i + 0].v.tc[1] =
                    persistantMasksVtx[i + 1].v.tc[1] = persistantMasksVtx[i + 2].v.tc[0] = 0;

                persistantMasksVtx[i + 1].v.tc[0] = persistantMasksVtx[i + 2].v.tc[1] =
                    persistantMasksVtx[i + 3].v.tc[0] = persistantMasksVtx[i + 3].v.tc[1] =
                        MASK_GRID_SELECTED_QUAD_TEX_SIZE * (1 << 5);

                persistantMasksVtx[i + 0].v.cn[0] = persistantMasksVtx[i + 1].v.cn[0] =
                    persistantMasksVtx[i + 2].v.cn[0] = persistantMasksVtx[i + 3].v.cn[0] =
                        persistantMasksVtx[i + 0].v.cn[1] = persistantMasksVtx[i + 1].v.cn[1] =
                            persistantMasksVtx[i + 2].v.cn[1] = persistantMasksVtx[i + 3].v.cn[1] =
                                persistantMasksVtx[i + 0].v.cn[2] = persistantMasksVtx[i + 1].v.cn[2] =
                                    persistantMasksVtx[i + 2].v.cn[2] = persistantMasksVtx[i + 3].v.cn[2] = 255;

                persistantMasksVtx[i + 0].v.cn[3] = persistantMasksVtx[i + 1].v.cn[3] =
                    persistantMasksVtx[i + 2].v.cn[3] = persistantMasksVtx[i + 3].v.cn[3] = pauseCtx->alpha;

                if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 100, 200, 255, 255);
                    gSPVertex(POLY_OPA_DISP++, (uintptr_t)&persistantMasksVtx[0], 4, 0);
                    POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, (TexturePtr)gEquippedItemOutlineTex, 32, 32, 0);
                }
            }

            CLOSE_DISPS(gfxCtx);
        });
}

void RegisterPersistentMasks() {
    UpdatePersistentMasksState();

    REGISTER_VB_SHOULD(GI_VB_CONSIDER_BUNNY_HOOD_EQUIPPED, {
        if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
            *should = true;
        }
    });

    REGISTER_VB_SHOULD(GI_VB_USE_ITEM_CONSIDER_LINK_HUMAN, {
        if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0) &&
            *(PlayerItemAction*)opt == PLAYER_IA_MASK_BUNNY) {
            *should = true;
        }
    });

    REGISTER_VB_SHOULD(GI_VB_USE_ITEM_EQUIP_MASK, {
        PlayerMask* maskId = (PlayerMask*)opt;
        Player* player = GET_PLAYER(gPlayState);

        if (*maskId == PLAYER_MASK_BUNNY && CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0)) {
            *should = false;

            CVarSetInteger("gEnhancements.Masks.PersistentBunnyHood.State",
                           !CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0));
            if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
                func_8082E1F0(player, NA_SE_PL_CHANGE_ARMS);
            } else {
                func_8082E1F0(player, NA_SE_PL_TAKE_OUT_SHIELD);
            }
        }
    });

    REGISTER_VB_SHOULD(GI_VB_DRAW_ITEM_EQUIPPED_OUTLINE, {
        if (*(ItemId*)opt == ITEM_MASK_BUNNY && CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
            *should = false;
        }
    });

    REGISTER_VB_SHOULD(GI_VB_KALEIDO_DISPLAY_ITEM_TEXT, {
        ItemId* itemId = (ItemId*)opt;
        if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0) && *itemId == ITEM_MASK_BUNNY) {
            *should = false;
            CVarSetInteger("gEnhancements.Masks.PersistentBunnyHood.State",
                           !CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0));
            if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
                Audio_PlaySfx(NA_SE_SY_CAMERA_ZOOM_DOWN);
            } else {
                Audio_PlaySfx(NA_SE_SY_CAMERA_ZOOM_UP);
            }
        } else if (CVarGetInteger("gEnhancements.Masks.PersistentGreatFairyMask.Enabled", 0) &&
                   *itemId == ITEM_MASK_GREAT_FAIRY) {
            *should = false;
            CVarSetInteger("gEnhancements.Masks.PersistentGreatFairyMask.State",
                           !CVarGetInteger("gEnhancements.Masks.PersistentGreatFairyMask.State", 0));
        }
    });
}
