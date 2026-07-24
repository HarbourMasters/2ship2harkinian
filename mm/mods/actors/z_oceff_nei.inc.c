/*
 * z_oceff_nei.inc.c — Skijer's NEI: ocarina song effect for the 3 CUSTOM songs.
 *
 * A parameterized clone of OceffWipe (Song of Time effect): the same camera-attached expanding
 * frustum ring (reuses ovl_Oceff_Wipe's texture/DLs — no new art), with a distinct color per song:
 *   params 0 = Fugue of Home   → amber/orange (future: warp song, like Soaring)
 *   params 1 = Command Melody  → magenta       (future: change-character song)
 *   params 2 = Ballad of Hero  → gold          (future: zone-dependent, like Lullaby)
 *
 * Spawned by Message_SpawnSongEffect (z_message.c) when a custom song (ocarina slots 30-32) is
 * successfully played. Registered as ACTOR_OCEFF_NEI (actor_table.h); unity-included into the
 * z_player.c TU via custom_items.c, like z_en_partner.inc.c.
 */

#include "assets/overlays/ovl_Oceff_Wipe/ovl_Oceff_Wipe.h"

#define OCEFF_NEI_FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_UPDATE_DURING_OCARINA)

typedef struct OceffNei {
    /* 0x000 */ Actor actor;
    /* 0x144 */ u16 counter;
} OceffNei;

void OceffNei_Init(Actor* thisx, PlayState* play);
void OceffNei_Destroy(Actor* thisx, PlayState* play);
void OceffNei_Update(Actor* thisx, PlayState* play);
void OceffNei_Draw(Actor* thisx, PlayState* play);

ActorProfile Oceff_Nei_Profile = {
    ACTOR_OCEFF_NEI,
    ACTORCAT_ITEMACTION,
    OCEFF_NEI_FLAGS,
    GAMEPLAY_KEEP,
    sizeof(OceffNei),
    (ActorFunc)OceffNei_Init,
    (ActorFunc)OceffNei_Destroy,
    (ActorFunc)OceffNei_Update,
    (ActorFunc)OceffNei_Draw,
};

void OceffNei_Init(Actor* thisx, PlayState* play) {
    OceffNei* this = (OceffNei*)thisx;

    Actor_SetScale(&this->actor, 0.1f);
    this->counter = 0;
    this->actor.world.pos = GET_ACTIVE_CAM(play)->eye;
}

void OceffNei_Destroy(Actor* thisx, PlayState* play) {
    Magic_Reset(play);
    play->msgCtx.ocarinaSongEffectActive = false;
}

void OceffNei_Update(Actor* thisx, PlayState* play) {
    OceffNei* this = (OceffNei*)thisx;

    this->actor.world.pos = GET_ACTIVE_CAM(play)->eye;
    if (this->counter < 100) {
        this->counter++;
    } else {
        Actor_Kill(&this->actor);
    }
}

static u8 sNeiOceffAlphaIndices[] = {
    0x01, 0x10, 0x22, 0x01, 0x20, 0x12, 0x01, 0x20, 0x12, 0x01,
    0x10, 0x22, 0x01, 0x20, 0x12, 0x01, 0x12, 0x21, 0x01, 0x02,
};

// Per-song ring colors: prim (ring body) + env (glow). Distinct from every existing OcEff
// (WIPE cyan/blue, WIPE2/WIPE3 greens, WIPE4 white, SPOT yellow beam, STORM grey).
static Color_RGBA8 sNeiOceffPrimColors[3] = {
    { 255, 170, 50, 255 },  // Fugue of Home — amber/orange
    { 255, 120, 255, 255 }, // Command Melody — magenta
    { 255, 230, 120, 255 }, // Ballad of Hero — gold
};
static Color_RGBA8 sNeiOceffEnvColors[3] = {
    { 190, 80, 0, 128 },  // Fugue of Home
    { 150, 0, 190, 128 }, // Command Melody
    { 200, 140, 0, 128 }, // Ballad of Hero
};

void OceffNei_Draw(Actor* thisx, PlayState* play) {
    u32 scroll = play->state.frames & 0xFF;
    OceffNei* this = (OceffNei*)thisx;
    s32 variant = this->actor.params;
    f32 z;
    u8 alphaTable[3];
    s32 i;
    Vec3f eye = GET_ACTIVE_CAM(play)->eye;
    Vtx* vtxPtr;
    Vec3f quakeOffset;

    if ((variant < 0) || (variant > 2)) {
        variant = 0;
    }

    quakeOffset = Camera_GetQuakeOffset(GET_ACTIVE_CAM(play));

    OPEN_DISPS(play->state.gfxCtx);

    // Same widescreen handling as OceffWipe (2S2H).
    f32 effectDistance;
    s32 x = OTRGetRectDimensionFromLeftEdge(0) << 2;
    if (x < 0) {
        effectDistance = 1360.0f / (OTRGetAspectRatio() * 0.85f);
    } else {
        effectDistance = 1360.0f;
    }

    if (this->counter < 32) {
        z = Math_SinS(this->counter << 9) * effectDistance;
    } else {
        z = effectDistance;
    }

    if (this->counter >= 80) {
        alphaTable[0] = 0;
        alphaTable[1] = (100 - this->counter) * 8;
        alphaTable[2] = (100 - this->counter) * 12;
    } else {
        alphaTable[0] = 0;
        alphaTable[1] = 160;
        alphaTable[2] = 255;
    }

    vtxPtr = ResourceMgr_LoadVtxByName(sSongOfTimeFrustumVtx);

    for (i = 0; i < 20; i++) {
        vtxPtr[i * 2 + 0].v.cn[3] = alphaTable[(sNeiOceffAlphaIndices[i] & 0xF0) >> 4];
        vtxPtr[i * 2 + 1].v.cn[3] = alphaTable[sNeiOceffAlphaIndices[i] & 0xF];
    }

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    Matrix_Translate(eye.x + quakeOffset.x, eye.y + quakeOffset.y, eye.z + quakeOffset.z, MTXMODE_NEW);
    Matrix_Scale(0.1f, 0.1f, 0.1f, MTXMODE_APPLY);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_RotateXS(0x708, MTXMODE_APPLY);
    Matrix_Translate(0.0f, 0.0f, -z, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);

    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, sNeiOceffPrimColors[variant].r, sNeiOceffPrimColors[variant].g,
                    sNeiOceffPrimColors[variant].b, sNeiOceffPrimColors[variant].a);
    gDPSetEnvColor(POLY_XLU_DISP++, sNeiOceffEnvColors[variant].r, sNeiOceffEnvColors[variant].g,
                   sNeiOceffEnvColors[variant].b, sNeiOceffEnvColors[variant].a);

    gSPDisplayList(POLY_XLU_DISP++, sSongOfTimeFrustumMaterialDL);
    gSPDisplayList(POLY_XLU_DISP++, Gfx_TwoTexScrollEx(play->state.gfxCtx, G_TX_RENDERTILE, 0 - scroll, scroll * -2, 32,
                                                       32, 1, 0 - scroll, scroll * -2, 32, 32, -1, -2, -1, -2));
    gSPDisplayList(POLY_XLU_DISP++, sSongOfTimeFrustumModelDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
