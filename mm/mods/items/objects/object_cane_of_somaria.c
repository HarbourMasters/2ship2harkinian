/**
 * object_cane_of_somaria.c - Cane of Somaria 3D model and draw functions
 */

#include "z64.h"
#include "../custom_items.h"
#include "../logic/item_cane_of_somaria.h"
#include "macros.h"
#include "functions.h"
#include <math.h>
// 3D model lives in soh.o2r: objects/object_somaria/g_somaria_cane_dl (XML emitted
// by apps/dl_c_to_xml.py, packed via rebuild_soh_otr.bat). No inline C model.
// ResourceMgr_LoadGfxByName crashes on a missing path, so gate with FileExists;
// returns NULL (cane simply not drawn) if the archive lacks the object.
extern u8 ResourceMgr_FileExists(const char* resName);
extern Gfx* ResourceMgr_LoadGfxByName(const char* path);

static Gfx* Somaria_GetHandDL(void) {
    static Gfx* sCached = NULL;
    static u8 sTried = 0;
    if (!sTried) {
        sTried = 1;
        const char* otr = "__OTR__objects/object_somaria/g_somaria_cane_dl";
        if (ResourceMgr_FileExists(otr)) {
            sCached = ResourceMgr_LoadGfxByName(otr);
        }
    }
    return sCached;
}

void CustomItems_DrawCaneOfSomaria(Player* player, PlayState* play) {
    if (!shSomariaActive)
        return;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Opa(play->state.gfxCtx);

    // Get forearm and hand positions to calculate hand direction
    Vec3f forearmPos = player->bodyPartsPos[PLAYER_BODYPART_R_FOREARM];
    Vec3f handPos = player->bodyPartsPos[PLAYER_BODYPART_R_HAND];

    // Calculate direction vector from forearm to hand
    f32 dx = handPos.x - forearmPos.x;
    f32 dy = handPos.y - forearmPos.y;
    f32 dz = handPos.z - forearmPos.z;

    // Calculate yaw and pitch from direction
    f32 handYaw = atan2f(dx, dz);
    f32 horizDist = sqrtf(dx * dx + dz * dz);
    f32 handPitch = atan2f(dy, horizDist);

    // Position at hand
    Matrix_Translate(handPos.x, handPos.y, handPos.z, MTXMODE_NEW);

    // Apply hand rotation
    Matrix_RotateY(handYaw, MTXMODE_APPLY);
    Matrix_RotateX(-handPitch, MTXMODE_APPLY);
    Matrix_RotateY(BINANG_TO_RAD(0x4000), MTXMODE_APPLY);

    // Offset up in local Y after rotation
    Matrix_Translate(-2.5f, 15.0f, 1.0f, MTXMODE_APPLY);

    Matrix_Scale(0.05f, 0.05f, 0.05f, MTXMODE_APPLY);

    Gfx* handDL = Somaria_GetHandDL();
    if (handDL != NULL) {
        gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
                  G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(POLY_OPA_DISP++, handDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}
