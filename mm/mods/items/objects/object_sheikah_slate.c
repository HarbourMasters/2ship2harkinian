/**
 * object_sheikah_slate.c — the Sheikah Slate in Link's hand (Skijer's NEI).
 *
 * Same idiom as the Cane of Somaria: the model is not a held-item the engine knows about, it is
 * drawn every frame from the R-hand body part, oriented along the forearm→hand vector so it follows
 * whatever animation is playing. That is what makes it read as "held" rather than stuck to a bone.
 *
 * The whole placement (offset, rotation, scale) is live-tunable through gItemEditor.Slate.* CVars,
 * because getting a flat tablet to sit in a fist the way the Hookshot does is pure eyeballing — the
 * defaults here are a starting point, not a measurement. The CVar names are shared with 2ship so a
 * preset tuned in one game carries to the other.
 */

#include "z64.h"
#include "../custom_items.h"
#include "macros.h"
#include "functions.h"
#include <math.h>

// The slate model ships in the custom archive (assets/custom/objects/object_nei_sheikah_slate).
// ResourceMgr_LoadGfxByName crashes on a missing path, so gate on FileExists and simply draw
// nothing while the archive has not been rebuilt.
extern u8 ResourceMgr_FileExists(const char* resName);
extern Gfx* ResourceMgr_LoadGfxByName(const char* path);
extern u8 Slate_IsDrawn(void); // equip state, owned by item_sheikah_slate.c (same TU)

// Placement, baked. These are the values tuned in-game with the Item Editor sliders — the tablet
// sits in the fist the way the Hookshot does — and they are now the whole answer: the Item Editor
// no longer carries Slate sliders, and the CVars are no longer read, so a stray preset cannot
// quietly override the pose with no UI left to correct it.
//
// NOT the same numbers as soh's. Different hand bone, different model scale; the two were tuned
// separately and copying either set across would put the tablet through Link's wrist.
#define SLATE_DEF_OFF_X -3.036f
#define SLATE_DEF_OFF_Y -12.327f
#define SLATE_DEF_OFF_Z -0.264f
#define SLATE_DEF_ROT_X 78.416f
#define SLATE_DEF_ROT_Y 180.0f
#define SLATE_DEF_ROT_Z 13.664f
#define SLATE_DEF_SCALE 0.146f

static Gfx* Slate_GetHandDL(void) {
    static Gfx* sCached = NULL;
    static u8 sTried = 0;

    if (!sTried) {
        sTried = 1;
        const char* otr = "__OTR__objects/object_nei_sheikah_slate/gNeiSheikahSlateDL";
        if (ResourceMgr_FileExists(otr)) {
            sCached = ResourceMgr_LoadGfxByName(otr);
        }
    }
    return sCached;
}

void CustomItems_DrawSheikahSlate(Player* player, PlayState* play) {
    Gfx* handDL;
    Vec3f forearmPos;
    Vec3f handPos;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 handYaw;
    f32 handPitch;
    f32 horizDist;
    f32 scale;

    if (!Slate_IsDrawn()) {
        return;
    }

    handDL = Slate_GetHandDL();
    if (handDL == NULL) {
        return; // archive not rebuilt yet — no crash, just no tablet
    }

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    // Orient along the forearm→hand vector so the tablet tracks every animation, including the
    // hookshot-style aim pose the cast plays.
    // Native MM body-part names (OoT spells them R_FOREARM / R_HAND; the aliases live in
    // nei_oot_compat.h, which this file does not pull in).
    forearmPos = player->bodyPartsPos[PLAYER_BODYPART_RIGHT_FOREARM];
    handPos = player->bodyPartsPos[PLAYER_BODYPART_RIGHT_HAND];

    dx = handPos.x - forearmPos.x;
    dy = handPos.y - forearmPos.y;
    dz = handPos.z - forearmPos.z;

    handYaw = atan2f(dx, dz);
    horizDist = sqrtf(dx * dx + dz * dz);
    handPitch = atan2f(dy, horizDist);

    Matrix_Translate(handPos.x, handPos.y, handPos.z, MTXMODE_NEW);
    Matrix_RotateY(handYaw, MTXMODE_APPLY);
    Matrix_RotateX(-handPitch, MTXMODE_APPLY);

    // ── Placement ───────────────────────────────────────────────────────────
    Matrix_RotateY(DEG_TO_RAD(SLATE_DEF_ROT_Y), MTXMODE_APPLY);
    Matrix_RotateX(DEG_TO_RAD(SLATE_DEF_ROT_X), MTXMODE_APPLY);
    Matrix_RotateZ(DEG_TO_RAD(SLATE_DEF_ROT_Z), MTXMODE_APPLY);

    // Offset AFTER the rotations, so it sits along the tablet's own axes — "up" means up the
    // tablet no matter which way the hand is pointing.
    Matrix_Translate(SLATE_DEF_OFF_X, SLATE_DEF_OFF_Y, SLATE_DEF_OFF_Z, MTXMODE_APPLY);

    scale = SLATE_DEF_SCALE;
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, handDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
