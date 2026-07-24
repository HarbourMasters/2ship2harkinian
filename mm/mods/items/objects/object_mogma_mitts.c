/**
 * object_mogma_mitts.c - Mogma Mitts 3D model and draw functions
 *
 * Draws white gauntlets on Link's right hand when mitts are active.
 * Model: Custom DL in mogma_mittsDL/
 */

#include "z64.h"
#include "../custom_items.h"
#include "macros.h"
#include "functions.h"

// Mitts model now in soh.o2r (object_nei_mogma_mitts). Cached gated load.
extern u8 ResourceMgr_FileExists(const char* resName);
extern Gfx* ResourceMgr_LoadGfxByName(const char* path);

static Gfx* MogmaMitts_GetDL(void) {
    static Gfx* sDL = NULL;
    static u8 sTried = 0;
    if (!sTried) {
        sTried = 1;
        const char* otr = "__OTR__objects/object_nei_mogma_mitts/gMogmaMittsGiveDL";
        if (ResourceMgr_FileExists(otr)) {
            sDL = ResourceMgr_LoadGfxByName(otr);
        }
    }
    return sDL;
}

void CustomItems_DrawMogmaMitts(Player* player, PlayState* play) {
    if (!gCustomItemState.mogmaMittsActive)
        return;
    if (MogmaMitts_GetDL() == NULL)
        return;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Opa(play->state.gfxCtx);

    // Position at right hand
    Vec3f handPos = player->bodyPartsPos[PLAYER_BODYPART_R_HAND];
    f32 forwardOffset = 8.0f;

    Matrix_Translate(handPos.x + Math_SinS(player->actor.shape.rot.y) * forwardOffset, handPos.y + 3.0f,
                     handPos.z + Math_CosS(player->actor.shape.rot.y) * forwardOffset, MTXMODE_NEW);
    Matrix_RotateY(BINANG_TO_RAD(player->actor.shape.rot.y), MTXMODE_APPLY);
    // The model's verts span ±30 units; at the old 0.01 world-space scale that rendered as a
    // ~0.3-unit sub-pixel speck (invisible) on a ~65-unit-tall Link. 0.12 ≈ hand-sized. Skijer's NEI
    Matrix_Scale(0.12f, 0.12f, 0.12f, MTXMODE_APPLY);

    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx, (char*)__FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    gSPDisplayList(POLY_OPA_DISP++, MogmaMitts_GetDL());

    CLOSE_DISPS(play->state.gfxCtx);
}
