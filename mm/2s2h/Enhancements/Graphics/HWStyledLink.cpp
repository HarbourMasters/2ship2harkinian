#include "HWStyledLink.h"
#include "libultraship/libultraship.h"
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"
#include "Enhancements/FrameInterpolation/FrameInterpolation.h"

extern "C" {
#include "z64.h"
#include "z64player.h"
#include "functions.h"
#include "macros.h"
#include "src/overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

extern PlayState* gPlayState;
extern const char* D_801C0B20[28];
extern SaveContext gSaveContext;
#include "objects/object_link_child/object_link_child.h"
void ResourceMgr_PatchGfxByName(const char* path, const char* patchName, int index, Gfx instruction);
void ResourceMgr_UnpatchGfxByName(const char* path, const char* patchName);
}

void UpdateHWStyledLink() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnPlayerPostLimbDraw>(
    PLAYER_LIMB_HEAD, [](Player* player, s32 limbIndex) {
        if (CVarGetInteger("gModes.HWStyledLink", 0) &&
        player->currentMask == PLAYER_MASK_NONE &&
        player->transformation == PLAYER_FORM_HUMAN &&
        INV_CONTENT(ITEM_MASK_KEATON) == ITEM_MASK_KEATON){
            OPEN_DISPS(gPlayState->state.gfxCtx);
            Matrix_Push();
            Matrix_RotateYS(14563, MTXMODE_APPLY);
            Matrix_RotateZS(-4854, MTXMODE_APPLY);
            Matrix_Translate(300.0f, -250.0f, 77.7f, MTXMODE_APPLY);
            Matrix_Scale(0.648f, 0.648f, 0.648f, MTXMODE_APPLY);
            gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(gPlayState->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_OPA_DISP++, (Gfx*)D_801C0B20[PLAYER_MASK_KEATON - 1]);
            Matrix_Pop();
            CLOSE_DISPS(gPlayState->state.gfxCtx);
        }
    });
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnPlayerPostLimbDraw>(
    PLAYER_LIMB_WAIST, [](Player* player, s32 limbIndex) {
        if (CVarGetInteger("gModes.HWStyledLink", 0) && 
            player->transformation == PLAYER_FORM_HUMAN &&
            player->itemAction != PLAYER_IA_MASK_FIERCE_DEITY &&
            INV_CONTENT(ITEM_MASK_FIERCE_DEITY) == ITEM_MASK_FIERCE_DEITY) {
                OPEN_DISPS(gPlayState->state.gfxCtx);
                Matrix_Push();
                Matrix_RotateXS(-25000, MTXMODE_APPLY);
                Matrix_RotateYS(-2000, MTXMODE_APPLY);
                Matrix_RotateZS(-15000, MTXMODE_APPLY);
                Matrix_Translate(-85.0f, 658.0f, -165.0f, MTXMODE_APPLY);
                Matrix_Scale(0.635f, 0.635f, 0.635f, MTXMODE_APPLY);
                gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(gPlayState->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_OPA_DISP++, (Gfx*)D_801C0B20[PLAYER_MASK_FIERCE_DEITY - 1]);
                Matrix_Pop();
                CLOSE_DISPS(gPlayState->state.gfxCtx);
        }
    });
}

void RegisterHWStyledLink() {
    UpdateHWStyledLink();

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>(
        [](s8 sceneId, s8 spawnNum) { UpdateHWStyledLink(); });
}