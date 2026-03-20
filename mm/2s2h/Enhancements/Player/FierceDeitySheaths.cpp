#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
uint8_t ResourceMgr_FileExists(const char* resName);
Gfx* ResourceMgr_LoadGfxByName(const char* path);
}

static const char* sFierceDeitySwordInSheathDLPath = "__OTR__objects/object_link_boy/gLinkFierceDeitySwordInSheathDL";
static const char* sFierceDeitySwordSheathDLPath = "__OTR__objects/object_link_boy/gLinkFierceDeitySwordSheathDL";
static Gfx* sFierceDeitySwordInSheathDL = nullptr;
static Gfx* sFierceDeitySwordSheathDL = nullptr;
static s32 sFierceDeitySheathState = 0;

static bool CustomFDSheath_LoadAssets() {
    if (sFierceDeitySheathState == 0) {
        sFierceDeitySheathState = 1;

        if (ResourceMgr_FileExists(sFierceDeitySwordInSheathDLPath) &&
            ResourceMgr_FileExists(sFierceDeitySwordSheathDLPath)) {
            sFierceDeitySwordInSheathDL = ResourceMgr_LoadGfxByName(sFierceDeitySwordInSheathDLPath);
            sFierceDeitySwordSheathDL = ResourceMgr_LoadGfxByName(sFierceDeitySwordSheathDLPath);
            sFierceDeitySheathState = 1;
        }
    }

    return sFierceDeitySheathState == 1;
}

static Gfx* CustomFDSheath_GetDList(Player* player) {
    if (!CustomFDSheath_LoadAssets()) {
        return nullptr;
    }

    if (player->modelGroup == PLAYER_MODELGROUP_TWO_HAND_SWORD) {
        return sFierceDeitySwordSheathDL;
    }

    return sFierceDeitySwordInSheathDL;
}

void RegisterCustomFDSheath() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnPlayerPostLimbDraw>(
        PLAYER_LIMB_SHEATH, [](Player* player, s32) {
            Gfx* sheathDList;

            if (player->transformation != PLAYER_FORM_FIERCE_DEITY) {
                return;
            }

            sheathDList = CustomFDSheath_GetDList(player);
            if (sheathDList == nullptr) {
                return;
            }

            OPEN_DISPS(gPlayState->state.gfxCtx);

            Matrix_Push();
            // FD sheath seems to be rotated 180 degrees ingame for some reason?
            Matrix_RotateZS(0x8000, MTXMODE_APPLY);
            // FD sheath is a tad lower on the player's back than it should be
            Matrix_Translate(-600.0f, 0.0f, 0.0f, MTXMODE_APPLY);
            MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
            gSPDisplayList(POLY_OPA_DISP++, sheathDList);
            Matrix_Pop();

            CLOSE_DISPS(gPlayState->state.gfxCtx);
        });
}

static RegisterShipInitFunc initFunc(RegisterCustomFDSheath, {});
