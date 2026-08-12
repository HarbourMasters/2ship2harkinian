/**
 * equip_magiccape.c - Magic Cape (Extended Tunic Slot 1)
 *
 * Behavior: Ganondorf's cape cloth physics attached to Link's shoulders.
 * Uses the same DLs and textures from ovl_En_Ganon_Mant (gMantDL, gMantTex, etc.)
 * with Verlet cloth simulation adapted for Link's proportions.
 *
 * Cape attaches between PLAYER_LIMB_L_SHOULDER and PLAYER_LIMB_R_SHOULDER,
 * draping down Link's back with full physics simulation.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// No extra includes - unity-built from ext_equip_behavior.c
// which inherits all from extended_equipment.c

// MM has no ovl_En_Ganon_Mant (OoT boss cape) in mm.o2r — but the player's COMPANION oot.o2r does
// (Skijer 2026-07-16). The gMant* symbols below are OTR path strings, exactly how SoH's asset
// headers define them: every consumer (gSPDisplayList, gSPSegmentLoadRes, ResourceMgr_LoadVtxByName,
// Gfx_RegisterBlendedTexture) detects the OTR signature and resolves the resource at use time.
// The cloth enables itself when the OoT archive is mounted; without oot.o2r it stays invisible
// (the passive half-cost never depends on this).
#define dgMantTex "__OTR__overlays/ovl_En_Ganon_Mant/gMantTex"
static const char gMantTex[] = dgMantTex;
#include "soh/ResourceManagerHelpers.h"

// ---------------------------------------------------------------------------
// Constants (adapted from EnGanonMant for Link's scale)
// ---------------------------------------------------------------------------
#define CAPE_NUM_JOINTS 12
#define CAPE_NUM_STRANDS 12
#define CAPE_JOINT_LENGTH 4.5f
#define CAPE_GRAVITY -3.0f
#define CAPE_BACK_PUSH -4.0f
#define CAPE_MIN_DIST 8.0f
#define CAPE_MIN_Y_OFFSET -200.0f // Below actor pos
#define CAPE_TEX_WIDTH 32
#define CAPE_TEX_HEIGHT 64

// ---------------------------------------------------------------------------
// Custom Items editor — live-tunable parameters (Skijer's NEI)
// ---------------------------------------------------------------------------
// Every constant above is now a DEFAULT; the real value is read once per frame
// from the `gItemEditor.Cape.*` CVars driven by the "Item Editor" tab in the
// NEI menu (identical CVar names in Ship and 2ship, so a preset moves between
// both games untouched). With `gItemEditor.Cape.Custom` = 0 the struct is filled
// with the vanilla defaults and the cloth behaves exactly as before.
#define CAPE_CVAR(name) "gItemEditor.Cape." name
#define CAPE_DEG_TO_RAD (M_PI / 180.0f)

typedef struct {
    // Shape
    f32 scale;       // master multiplier over length + width
    f32 jointLength; // per-joint segment length (total length = 11 * this)
    f32 width;       // multiplier over the shoulder half-span
    f32 arcSpan;     // radians the strand roots are spread over (vanilla PI)
    f32 arcBulge;    // how far the root arc bows away from the shoulder line
    f32 arcSpread;   // root arc span along the shoulder line
    // Placement (in the shoulder frame: x = bulge/back, y = up, z = shoulder line)
    f32 offX, offY, offZ;
    f32 yaw, pitch, roll; // radians, added on top of the shoulder-derived angles
    // Physics
    f32 gravity;
    f32 backPush;
    f32 minDist;     // push-away radius from the player's center
    f32 floorOffset; // lowest the cloth may hang, relative to actor Y
    f32 backSway;    // per-unit-of-speed back sway
    f32 sideSway;    // per-unit-of-speed side sway
    f32 damping;     // velocity retained per tick
    f32 velClamp;    // per-axis velocity limit
    f32 decel;       // per-tick approach-zero rate
    // Look
    u8 r, g, b, a;
} CapeParams;

static CapeParams sCapeP;

static void MagicCape_LoadParams(void) {
    CapeParams* p = &sCapeP;

    p->scale = 1.0f;
    p->jointLength = CAPE_JOINT_LENGTH;
    p->width = 1.0f;
    p->arcSpan = M_PI;
    p->arcBulge = 1.0f;
    p->arcSpread = 1.0f;
    p->offX = p->offY = p->offZ = 0.0f;
    p->yaw = p->pitch = p->roll = 0.0f;
    p->gravity = CAPE_GRAVITY;
    p->backPush = CAPE_BACK_PUSH;
    p->minDist = CAPE_MIN_DIST;
    p->floorOffset = CAPE_MIN_Y_OFFSET;
    p->backSway = 0.3f;
    p->sideSway = 0.15f;
    p->damping = 0.8f;
    p->velClamp = 5.0f;
    p->decel = 0.1f;
    p->r = p->g = p->b = p->a = 255;

    if (!CVarGetInteger(CAPE_CVAR("Custom"), 0)) {
        return;
    }

    p->scale = CVarGetFloat(CAPE_CVAR("Scale"), p->scale);
    p->jointLength = CVarGetFloat(CAPE_CVAR("Length"), p->jointLength);
    p->width = CVarGetFloat(CAPE_CVAR("Width"), p->width);
    p->arcSpan = CVarGetFloat(CAPE_CVAR("ArcSpan"), 180.0f) * CAPE_DEG_TO_RAD;
    p->arcBulge = CVarGetFloat(CAPE_CVAR("ArcBulge"), p->arcBulge);
    p->arcSpread = CVarGetFloat(CAPE_CVAR("ArcSpread"), p->arcSpread);
    p->offX = CVarGetFloat(CAPE_CVAR("OffsetX"), 0.0f);
    p->offY = CVarGetFloat(CAPE_CVAR("OffsetY"), 0.0f);
    p->offZ = CVarGetFloat(CAPE_CVAR("OffsetZ"), 0.0f);
    p->yaw = CVarGetFloat(CAPE_CVAR("Yaw"), 0.0f) * CAPE_DEG_TO_RAD;
    p->pitch = CVarGetFloat(CAPE_CVAR("Pitch"), 0.0f) * CAPE_DEG_TO_RAD;
    p->roll = CVarGetFloat(CAPE_CVAR("Roll"), 0.0f) * CAPE_DEG_TO_RAD;
    p->gravity = CVarGetFloat(CAPE_CVAR("Gravity"), p->gravity);
    p->backPush = CVarGetFloat(CAPE_CVAR("BackPush"), p->backPush);
    p->minDist = CVarGetFloat(CAPE_CVAR("MinDist"), p->minDist);
    p->floorOffset = CVarGetFloat(CAPE_CVAR("FloorOffset"), p->floorOffset);
    p->backSway = CVarGetFloat(CAPE_CVAR("BackSway"), p->backSway);
    p->sideSway = CVarGetFloat(CAPE_CVAR("SideSway"), p->sideSway);
    p->damping = CVarGetFloat(CAPE_CVAR("Damping"), p->damping);
    p->velClamp = CVarGetFloat(CAPE_CVAR("VelClamp"), p->velClamp);
    p->decel = CVarGetFloat(CAPE_CVAR("Decel"), p->decel);
    p->r = (u8)CVarGetInteger(CAPE_CVAR("ColorR"), 255);
    p->g = (u8)CVarGetInteger(CAPE_CVAR("ColorG"), 255);
    p->b = (u8)CVarGetInteger(CAPE_CVAR("ColorB"), 255);
    p->a = (u8)CVarGetInteger(CAPE_CVAR("ColorA"), 255);

    // Guard against values that would divide by zero or invert the cloth.
    if (p->scale < 0.01f) {
        p->scale = 0.01f;
    }
    if (p->jointLength < 0.01f) {
        p->jointLength = 0.01f;
    }
}

// ---------------------------------------------------------------------------
// Strand struct (same as MantStrand from z_en_ganon_mant.h)
// ---------------------------------------------------------------------------
typedef struct {
    Vec3f root;
    Vec3f joints[CAPE_NUM_JOINTS];
    Vec3f rotations[CAPE_NUM_JOINTS];
    Vec3f velocities[CAPE_NUM_JOINTS];
} CapeStrand; // no torn[] needed

// ---------------------------------------------------------------------------
// Static state
// ---------------------------------------------------------------------------
static CapeStrand sCapeStrands[CAPE_NUM_STRANDS];
static u8 sCapeInitialized = 0;
static u8 sCapeFrameTimer = 0;
static u8 sCapeUpdateHasRun = 0;
static f32 sCapeBaseYaw = 0.0f;


// On the first physics tick after Init (scene change, equip toggle, cutscene exit),
// snap every joint of every strand to its current root position so the cape doesn't
// settle from stale world coordinates left over from the previous scene.
static u8 sCapeNeedsRootSnap = 1;

// Shoulder anchors come straight from player->bodyPartsPos (native, maintained every frame).

// ---------------------------------------------------------------------------
// Physics coefficients (from EnGanonMant)
// ---------------------------------------------------------------------------
static f32 sCapeBackSwayCoeff[CAPE_NUM_JOINTS] = {
    0.0f, 1.0f, 0.5f, 0.25f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
};

static f32 sCapeSideSwayCoeff[CAPE_NUM_JOINTS] = {
    0.0f, 1.0f, 0.9f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, 0.0f,
};

static f32 sCapeDistMult[CAPE_NUM_JOINTS] = {
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f,
};


// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------
static void MagicCape_Init(void) {
    if (sCapeInitialized)
        return;

    memset(sCapeStrands, 0, sizeof(sCapeStrands));
    sCapeFrameTimer = 0;
    sCapeUpdateHasRun = 0;
    sCapeNeedsRootSnap = 1;

    // (No blended-texture registration — the material DL from oot.o2r binds gMantTex itself.)

    sCapeInitialized = 1;
}

// ---------------------------------------------------------------------------
// Reset (when cape is unequipped)
// ---------------------------------------------------------------------------
static void MagicCape_Reset(void) {
    if (!sCapeInitialized)
        return;

    // Note: we intentionally do NOT call Gfx_UnregisterBlendedTexture here. The texture
    // registration is established once in Init and persists for the rest of the session.
    // The mask buffer is static so its lifetime is forever; the GPU can keep referencing it
    // safely across re-inits without races.
    sCapeInitialized = 0;
    sCapeNeedsRootSnap = 1;
}



// ---------------------------------------------------------------------------
// Update single strand (adapted from EnGanonMant_UpdateStrand)
// ---------------------------------------------------------------------------
static void MagicCape_UpdateStrand(Vec3f* actorPos, f32 actorRotY, Vec3f* root, Vec3f* pos, Vec3f* nextPos, Vec3f* rot,
                                   Vec3f* vel, s16 strandNum, f32 backSwayMag, f32 sideSwayMag, f32 minY) {
    s16 i;
    f32 x, y, z;
    f32 yaw;
    f32 xDiff, zDiff;
    Vec3f delta;
    Vec3f posStep;
    Vec3f backSwayOffset;
    Vec3f sideSwayOffset;

    for (i = 0; i < CAPE_NUM_JOINTS; i++, pos++, vel++, rot++, nextPos++) {
        if (i == 0) {
            pos->x = root->x;
            pos->y = root->y;
            pos->z = root->z;
        } else {
            // Decelerate
            Math_ApproachZeroF(&vel->x, 1.0f, sCapeP.decel);
            Math_ApproachZeroF(&vel->y, 1.0f, sCapeP.decel);
            Math_ApproachZeroF(&vel->z, 1.0f, sCapeP.decel);

            // Back push + sway
            delta.x = 0;
            delta.y = 0;
            delta.z = (sCapeP.backPush + (sinf((strandNum * (2 * M_PI)) / 2.1f) * backSwayMag)) * sCapeBackSwayCoeff[i];
            Matrix_RotateY(sCapeBaseYaw, MTXMODE_NEW);
            Matrix_MultVec3f(&delta, &backSwayOffset);

            // Side sway
            delta.x = cosf((strandNum * M_PI) / (CAPE_NUM_STRANDS - 1.0f)) * sideSwayMag * sCapeSideSwayCoeff[i];
            delta.z = 0;
            Matrix_MultVec3f(&delta, &sideSwayOffset);

            // Position difference
            x = ((pos->x + vel->x) - (pos - 1)->x) + (backSwayOffset.x + sideSwayOffset.x);
            y = ((pos->y + vel->y) - (pos - 1)->y) + sCapeP.gravity;
            z = ((pos->z + vel->z) - (pos - 1)->z) + (backSwayOffset.z + sideSwayOffset.z);

            // Rotation
            yaw = Math_Atan2F_XY(z, x);
            x = -Math_Atan2F_XY(sqrtf(SQ(x) + SQ(z)), y);
            (rot - 1)->x = x;

            // Constrained position
            delta.x = 0;
            delta.y = 0;
            delta.z = sCapeP.jointLength * sCapeP.scale;
            Matrix_RotateY(yaw, MTXMODE_NEW);
            Matrix_RotateX(x, MTXMODE_APPLY);
            Matrix_MultVec3f(&delta, &posStep);

            // Save old position
            x = pos->x;
            y = pos->y;
            z = pos->z;

            // New position
            pos->x = (pos - 1)->x + posStep.x;
            pos->y = (pos - 1)->y + posStep.y;
            pos->z = (pos - 1)->z + posStep.z;

            // Push away from actor center
            xDiff = pos->x - actorPos->x;
            zDiff = pos->z - actorPos->z;
            if (sqrtf(SQ(xDiff) + SQ(zDiff)) < (sCapeDistMult[i] * sCapeP.minDist)) {
                yaw = Math_Atan2F_XY(zDiff, xDiff);
                delta.z = sCapeP.minDist * sCapeDistMult[i];
                delta.x = 0;
                Matrix_RotateY(yaw, MTXMODE_NEW);
                Matrix_MultVec3f(&delta, &posStep);
                pos->x = actorPos->x + posStep.x;
                pos->z = actorPos->z + posStep.z;
            }

            // Floor constraint
            if (pos->y < minY) {
                pos->y = minY;
            }

            // Velocity (80% damping by default)
            vel->x = (pos->x - x) * sCapeP.damping;
            vel->y = (pos->y - y) * sCapeP.damping;
            vel->z = (pos->z - z) * sCapeP.damping;

            // Clamp velocity
            {
                f32 clamp = sCapeP.velClamp;

                if (vel->x > clamp)
                    vel->x = clamp;
                else if (vel->x < -clamp)
                    vel->x = -clamp;
                if (vel->y > clamp)
                    vel->y = clamp;
                else if (vel->y < -clamp)
                    vel->y = -clamp;
                if (vel->z > clamp)
                    vel->z = clamp;
                else if (vel->z < -clamp)
                    vel->z = -clamp;
            }

            // Update angle
            xDiff = pos->x - nextPos->x;
            zDiff = pos->z - nextPos->z;
            (rot - 1)->y = Math_Atan2F_XY(zDiff, xDiff);
        }
    }
    rot[11].y = rot[10].y;
    rot[11].x = rot[10].x;
}

// ---------------------------------------------------------------------------
// Update vertices (adapted from EnGanonMant_UpdateVertices)
// ---------------------------------------------------------------------------
// MM (Skijer 2026-07-16): the cloth writes its OWN vertex buffer — 2ship resources aren't a
// writable scratch like SoH abused them (the gMant1/2Vtx double-buffer trick never produced pixels
// here). Layout: [strand][joint], filled every frame; the dynamic DL below triangulates it.
static Vtx sCapeVtxBuf[CAPE_NUM_STRANDS * CAPE_NUM_JOINTS];

static void MagicCape_UpdateVertices(void) {
    s16 i, j;
    CapeStrand* strand;
    Vec3f up = { 0.0f, 30.0f, 0.0f };
    Vec3f normal;

    strand = &sCapeStrands[0];
    for (i = 0; i < CAPE_NUM_STRANDS; i++, strand++) {
        for (j = 0; j < CAPE_NUM_JOINTS; j++) {
            Vtx* vtx = &sCapeVtxBuf[(i * CAPE_NUM_JOINTS) + j];

            vtx->n.ob[0] = strand->joints[j].x;
            vtx->n.ob[1] = strand->joints[j].y;
            vtx->n.ob[2] = strand->joints[j].z;
            vtx->n.flag = 0;
            // UVs across the 32x64 mant texture: u by strand, v down the strand (10.5 fixed point).
            vtx->n.tc[0] = (i * ((CAPE_TEX_WIDTH - 1) << 5)) / (CAPE_NUM_STRANDS - 1);
            vtx->n.tc[1] = (j * ((CAPE_TEX_HEIGHT - 1) << 5)) / (CAPE_NUM_JOINTS - 1);
            Matrix_RotateY(strand->rotations[j].y, MTXMODE_NEW);
            Matrix_RotateX(strand->rotations[j].x, MTXMODE_APPLY);
            Matrix_MultVec3f(&up, &normal);
            vtx->n.n[0] = normal.x;
            vtx->n.n[1] = normal.y;
            vtx->n.n[2] = normal.z;
            vtx->n.a = 255;
        }
    }
}

// ---------------------------------------------------------------------------
// Draw cape (adapted from EnGanonMant_DrawCloak + EnGanonMant_Draw)
// ---------------------------------------------------------------------------
static void MagicCape_Draw(Player* player, PlayState* play) {
    // (Availability gate = the ResourceMgr_FileExists texture guard below — OotAssets_Available()'s
    // global mount flag was another possible silent blocker and is redundant with the real check.)
    if (!sCapeInitialized)
        return;
    // Skip cape rendering while riding Epona (and other special states): the player skeleton
    // is in horse pose, shoulder limbs land in unexpected positions, and the cape produces
    // garbage geometry.
    if (player->stateFlags1 & PLAYER_STATE1_ON_HORSE) {
        return;
    }
    // MM (Skijer 2026-07-16): shoulder anchors come straight from bodyPartsPos — MM's skeleton draw
    // maintains them natively every frame (Matrix_MultZero per limb), so the whole PostLimbDraw
    // capture mechanism (and its "both shoulders captured" guard, which was silently blocking the
    // draw) is unnecessary here.
    // Item Editor: refresh the tunables once per drawn frame.
    MagicCape_LoadParams();

    // --- Physics update (runs once per frame in Draw, like original) ---
    if (sCapeUpdateHasRun) {
        Vec3f* rightPos = &player->bodyPartsPos[PLAYER_BODYPART_RIGHT_SHOULDER];
        Vec3f* leftPos = &player->bodyPartsPos[PLAYER_BODYPART_LEFT_SHOULDER];

        f32 xDiff = leftPos->x - rightPos->x;
        f32 yDiff = leftPos->y - rightPos->y;
        f32 zDiff = leftPos->z - rightPos->z;

        Vec3f midpoint;
        midpoint.x = rightPos->x + xDiff * 0.5f;
        midpoint.y = rightPos->y + yDiff * 0.5f;
        midpoint.z = rightPos->z + zDiff * 0.5f;

        f32 yaw = Math_Atan2F_XY(zDiff, xDiff);
        f32 pitch = -Math_Atan2F_XY(sqrtf(SQ(xDiff) + SQ(zDiff)), yDiff);
        f32 diffHalfDist = sqrtf(SQ(xDiff) + SQ(yDiff) + SQ(zDiff)) * 0.5f;

        // Item Editor: yaw/pitch/roll ride on top of the shoulder-derived frame, so the whole
        // cape (root arc included) can be re-aimed without touching the shoulder anchors.
        Matrix_RotateY(yaw + sCapeP.yaw, MTXMODE_NEW);
        Matrix_RotateX(pitch + sCapeP.pitch, MTXMODE_APPLY);
        if (sCapeP.roll != 0.0f) {
            Matrix_RotateZ(sCapeP.roll, MTXMODE_APPLY);
        }
        sCapeBaseYaw = yaw + sCapeP.yaw - M_PI / 2.0f;

        // Movement-based sway
        f32 speed = player->actor.speed;
        f32 backSwayMag = speed * sCapeP.backSway;
        f32 sideSwayMag = speed * sCapeP.sideSway;
        f32 minY = player->actor.world.pos.y + sCapeP.floorOffset;
        f32 halfSpan = diffHalfDist * sCapeP.width * sCapeP.scale;

        for (s16 strandIdx = 0; strandIdx < CAPE_NUM_STRANDS; strandIdx++) {
            Matrix_Push();

            Vec3f strandOffset;
            Vec3f strandDivPos;
            // Root arc: `arcSpan` is the angle the roots are spread over (180 deg = the vanilla
            // semicircle), `arcBulge` how far the arc bows away from the shoulder line (the
            // "parabola" depth) and `arcSpread` its width along that line.
            f32 t = (strandIdx * sCapeP.arcSpan) / (CAPE_NUM_STRANDS - 1);
            strandOffset.x = sinf(t) * halfSpan * sCapeP.arcBulge + sCapeP.offX;
            strandOffset.y = sCapeP.offY;
            strandOffset.z = -cosf(t) * halfSpan * sCapeP.arcSpread + sCapeP.offZ;
            Matrix_MultVec3f(&strandOffset, &strandDivPos);
            sCapeStrands[strandIdx].root.x = midpoint.x + strandDivPos.x;
            sCapeStrands[strandIdx].root.y = midpoint.y + strandDivPos.y;
            sCapeStrands[strandIdx].root.z = midpoint.z + strandDivPos.z;

            // First physics tick after Init: collapse every joint of this strand onto the
            // current root so the cape doesn't have to settle from world (0,0,0) coords left
            // by memset, which produced a violent first frame after every scene transition.
            if (sCapeNeedsRootSnap) {
                for (s32 j = 0; j < CAPE_NUM_JOINTS; j++) {
                    sCapeStrands[strandIdx].joints[j] = sCapeStrands[strandIdx].root;
                    sCapeStrands[strandIdx].velocities[j].x = 0.0f;
                    sCapeStrands[strandIdx].velocities[j].y = 0.0f;
                    sCapeStrands[strandIdx].velocities[j].z = 0.0f;
                }
            }

            s16 nextStrandIdx = strandIdx + 1;
            if (nextStrandIdx >= CAPE_NUM_STRANDS) {
                nextStrandIdx = strandIdx - 1;
            }

            MagicCape_UpdateStrand(&player->actor.world.pos, player->actor.shape.rot.y, &sCapeStrands[strandIdx].root,
                                   sCapeStrands[strandIdx].joints, sCapeStrands[nextStrandIdx].joints,
                                   sCapeStrands[strandIdx].rotations, sCapeStrands[strandIdx].velocities, strandIdx,
                                   backSwayMag, sideSwayMag, minY);

            Matrix_Pop();
        }

        MagicCape_UpdateVertices();
        sCapeUpdateHasRun = 0;
        sCapeNeedsRootSnap = 0;
    }

    // Resolve the mant texture to its RAW DATA POINTER once, BEFORE OPEN_DISPS (2ship's OPEN_DISPS
    // opens a scope block only CLOSE_DISPS may close — no early-outs between them). Passing the OTR
    // PATH to gDPLoadTextureBlock never loaded TMEM here (the cloth sampled whatever texture the
    // scene left bound — the "random scene textures" bug); the RESOLVED pointer is the proven
    // companion-texture pattern (same as Magic Dark's diamond tex).
    static void* sMantTexData = NULL;
    {
        extern u8 ResourceMgr_FileExists(const char* resName);
        extern void* OotAssets_LoadTexOrDList(const char* otrPath);
        static s8 sMantTexState = 0; // 0 unknown, 1 resolved, -1 missing

        if (sMantTexState == 0) {
            sMantTexState = -1;
            if (ResourceMgr_FileExists(gMantTex)) {
                sMantTexData = OotAssets_LoadTexOrDList(gMantTex);
                if (sMantTexData != NULL) {
                    sMantTexState = 1;
                }
            }
        }
        if (sMantTexState < 0) {
            return;
        }
    }

    // --- Render (MM: dynamic DL over our own vertex buffer; only the TEXTURE comes from the
    // companion oot.o2r — resources here aren't writable vertex scratch like SoH used them) ---
    OPEN_DISPS(play->state.gfxCtx);

    // Fully INLINE material (Skijer 2026-07-16): the o2r gMantMaterialDL never produced pixels in
    // 2ship — build the RDP state ourselves and pull only the TEXTURE by OTR path (the proven
    // gDPLoadTextureBlock-with-path mechanism this project already uses for HD retextures).
    // gMantTex = rgba16 32x64 (soh xml). Lit vertex normals * texture.
    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    Matrix_Translate(0.0f, 0.0f, 0.0f, MTXMODE_NEW);

    // Item Editor: a prim alpha below 255 has to go through the XLU list with an XLU render mode —
    // the opaque surface mode discards the blend entirely. Everything else about the two paths is
    // identical, so the command stream is written once through this macro.
    // Pin a clean 1-CYCLE textured-lit state ourselves (Skijer 2026-07-17). SETUPDL_25 leaves the RDP
    // in G_CYC_2CYCLE; a 1-cycle combiner (MODULATEIA) running under 2-cycle makes the SECOND cycle
    // sample a tile we never loaded, and in LUS that pulls whatever texture the scene left bound —
    // the "random scene textures" bug. Forcing G_CYC_1CYCLE + our own render mode + combiner
    // guarantees only our single loaded tile is ever sampled.
#define CAPE_EMIT(DISP, RM1, RM2)                                                                                  \
    do {                                                                                                           \
        s16 strip, j;                                                                                              \
                                                                                                                   \
        gSPMatrix(DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);          \
        gSPClearGeometryMode(DISP++, G_CULL_BOTH); /* cloth is visible from both sides */                           \
        gSPSetGeometryMode(DISP++, G_LIGHTING);                                                                     \
        gDPPipeSync(DISP++);                                                                                        \
        gDPSetCycleType(DISP++, G_CYC_1CYCLE);                                                                      \
        gDPSetRenderMode(DISP++, RM1, RM2);                                                                         \
        gDPSetTextureLUT(DISP++, G_TT_NONE);                                                                        \
        gSPTexture(DISP++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);                                               \
        gDPSetCombineMode(DISP++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);                                            \
        gDPSetPrimColor(DISP++, 0, 0, sCapeP.r, sCapeP.g, sCapeP.b, sCapeP.a);                                      \
        gDPLoadTextureBlock(DISP++, sMantTexData, G_IM_FMT_RGBA, G_IM_SIZ_16b, CAPE_TEX_WIDTH, CAPE_TEX_HEIGHT, 0,  \
                            G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, 5, 6, G_TX_NOLOD, G_TX_NOLOD);    \
        /* 12 strands x 12 joints -> 11 strips: load 2 strands (24 vtx), emit the quads between. */  \
        for (strip = 0; strip < CAPE_NUM_STRANDS - 1; strip++) {                                                    \
            gSPVertex(DISP++, &sCapeVtxBuf[strip * CAPE_NUM_JOINTS], CAPE_NUM_JOINTS * 2, 0);                       \
            for (j = 0; j < CAPE_NUM_JOINTS - 1; j++) {                                                             \
                gSP2Triangles(DISP++, j, j + CAPE_NUM_JOINTS, j + CAPE_NUM_JOINTS + 1, 0, j,                        \
                              j + CAPE_NUM_JOINTS + 1, j + 1, 0);                                                   \
            }                                                                                                       \
        }                                                                                                           \
    } while (0)

    if (sCapeP.a >= 255) {
        CAPE_EMIT(POLY_OPA_DISP, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    } else {
        CAPE_EMIT(POLY_XLU_DISP, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
    }
#undef CAPE_EMIT

    // (Segment 0x0C is no longer touched — the dynamic DL uses direct vertex pointers.)

    CLOSE_DISPS(play->state.gfxCtx);
}

// ---------------------------------------------------------------------------
// Cleanup: called EVERY frame from dispatch, regardless of equipped tunic.
// Handles resetting cape when tunic changes away.
// ---------------------------------------------------------------------------
static void MagicCape_Cleanup(void) {
    // Skijer 2026-07-16: the cape is no longer an ext-equipment slot — the cloth shows whenever the
    // cape is OWNED and not hidden via its kaleido upgrade-cell toggle.
    if (!ExtEquip_CapeVisible() && sCapeInitialized) {
        MagicCape_Reset();
    }
}

// ---------------------------------------------------------------------------
// Main behavior entry (called per frame from dispatch)
// ---------------------------------------------------------------------------
static void MagicCape_Behavior(Player* player, PlayState* play) {
    // Skip while riding Epona — pairs with the same guard in MagicCape_Draw.
    // We don't Reset here; we just stop updating, so the cape resumes naturally on dismount.
    if (player->stateFlags1 & PLAYER_STATE1_ON_HORSE) {
        return;
    }

    // Skip during cutscenes
    if (player->stateFlags1 &
        (PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_LOADING | PLAYER_STATE1_IN_ITEM_CS)) {
        if (sCapeInitialized) {
            MagicCape_Reset();
        }
        return;
    }

    // Initialize if needed
    if (!sCapeInitialized) {
        MagicCape_Init();
    }

    // Skijer 2026-07-16: the old per-frame magic-refund tracker was REMOVED — it double-dipped with
    // the MAGIC_REQ half-cost (items paid half, then got half of that refunded = quarter cost) and
    // used ceil instead of floor. The cape's real effect is MAGIC_REQ at the cost sites, passive
    // while OWNED. This function is cloth physics only now.

    sCapeFrameTimer++;
    sCapeUpdateHasRun = 1;

    // Reset shoulder capture flags for next frame
}
