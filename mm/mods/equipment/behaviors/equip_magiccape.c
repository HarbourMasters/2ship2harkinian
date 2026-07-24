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
            Math_ApproachZeroF(&vel->x, 1.0f, 0.1f);
            Math_ApproachZeroF(&vel->y, 1.0f, 0.1f);
            Math_ApproachZeroF(&vel->z, 1.0f, 0.1f);

            // Back push + sway
            delta.x = 0;
            delta.y = 0;
            delta.z = (CAPE_BACK_PUSH + (sinf((strandNum * (2 * M_PI)) / 2.1f) * backSwayMag)) * sCapeBackSwayCoeff[i];
            Matrix_RotateY(sCapeBaseYaw, MTXMODE_NEW);
            Matrix_MultVec3f(&delta, &backSwayOffset);

            // Side sway
            delta.x = cosf((strandNum * M_PI) / (CAPE_NUM_STRANDS - 1.0f)) * sideSwayMag * sCapeSideSwayCoeff[i];
            delta.z = 0;
            Matrix_MultVec3f(&delta, &sideSwayOffset);

            // Position difference
            x = ((pos->x + vel->x) - (pos - 1)->x) + (backSwayOffset.x + sideSwayOffset.x);
            y = ((pos->y + vel->y) - (pos - 1)->y) + CAPE_GRAVITY;
            z = ((pos->z + vel->z) - (pos - 1)->z) + (backSwayOffset.z + sideSwayOffset.z);

            // Rotation
            yaw = Math_Atan2F_XY(z, x);
            x = -Math_Atan2F_XY(sqrtf(SQ(x) + SQ(z)), y);
            (rot - 1)->x = x;

            // Constrained position
            delta.x = 0;
            delta.y = 0;
            delta.z = CAPE_JOINT_LENGTH;
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
            if (sqrtf(SQ(xDiff) + SQ(zDiff)) < (sCapeDistMult[i] * CAPE_MIN_DIST)) {
                yaw = Math_Atan2F_XY(zDiff, xDiff);
                delta.z = CAPE_MIN_DIST * sCapeDistMult[i];
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

            // Velocity (80% damping)
            vel->x = (pos->x - x) * 0.8f;
            vel->y = (pos->y - y) * 0.8f;
            vel->z = (pos->z - z) * 0.8f;

            // Clamp velocity
            if (vel->x > 5.0f)
                vel->x = 5.0f;
            else if (vel->x < -5.0f)
                vel->x = -5.0f;
            if (vel->y > 5.0f)
                vel->y = 5.0f;
            else if (vel->y < -5.0f)
                vel->y = -5.0f;
            if (vel->z > 5.0f)
                vel->z = 5.0f;
            else if (vel->z < -5.0f)
                vel->z = -5.0f;

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

        Matrix_RotateY(yaw, MTXMODE_NEW);
        Matrix_RotateX(pitch, MTXMODE_APPLY);
        sCapeBaseYaw = yaw - M_PI / 2.0f;

        // Movement-based sway
        f32 speed = player->actor.speed;
        f32 backSwayMag = speed * 0.3f;
        f32 sideSwayMag = speed * 0.15f;
        f32 minY = player->actor.world.pos.y + CAPE_MIN_Y_OFFSET;

        for (s16 strandIdx = 0; strandIdx < CAPE_NUM_STRANDS; strandIdx++) {
            Matrix_Push();

            Vec3f strandOffset;
            Vec3f strandDivPos;
            strandOffset.x = sinf((strandIdx * M_PI) / (CAPE_NUM_STRANDS - 1)) * diffHalfDist;
            strandOffset.y = 0;
            strandOffset.z = -cosf((strandIdx * M_PI) / (CAPE_NUM_STRANDS - 1)) * diffHalfDist;
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
    gSPMatrix(POLY_OPA_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPClearGeometryMode(POLY_OPA_DISP++, G_CULL_BOTH); // cloth is visible from both sides
    gSPSetGeometryMode(POLY_OPA_DISP++, G_LIGHTING);

    // Pin a clean 1-CYCLE textured-lit state ourselves (Skijer 2026-07-17). SETUPDL_25 leaves the RDP
    // in G_CYC_2CYCLE; a 1-cycle combiner (MODULATEIA) running under 2-cycle makes the SECOND cycle
    // sample a tile we never loaded, and in LUS that pulls whatever texture the scene left bound —
    // the "random scene textures" bug. Forcing G_CYC_1CYCLE + our own render mode + combiner
    // guarantees only our single loaded tile is ever sampled.
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCycleType(POLY_OPA_DISP++, G_CYC_1CYCLE);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetTextureLUT(POLY_OPA_DISP++, G_TT_NONE);
    gSPTexture(POLY_OPA_DISP++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATERGBA, G_CC_MODULATERGBA);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, 255);
    gDPLoadTextureBlock(POLY_OPA_DISP++, sMantTexData, G_IM_FMT_RGBA, G_IM_SIZ_16b, CAPE_TEX_WIDTH, CAPE_TEX_HEIGHT, 0,
                        G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, 5, 6, G_TX_NOLOD, G_TX_NOLOD);

    // Triangulate strand pairs: 12 strands x 12 joints -> 11 strips of 11 quads. Each strip loads
    // its two strands (24 vtx) and emits the quads between them.
    {
        s16 strip, j;

        for (strip = 0; strip < CAPE_NUM_STRANDS - 1; strip++) {
            gSPVertex(POLY_OPA_DISP++, &sCapeVtxBuf[strip * CAPE_NUM_JOINTS], CAPE_NUM_JOINTS * 2, 0);
            for (j = 0; j < CAPE_NUM_JOINTS - 1; j++) {
                gSP2Triangles(POLY_OPA_DISP++, j, j + CAPE_NUM_JOINTS, j + CAPE_NUM_JOINTS + 1, 0, j,
                              j + CAPE_NUM_JOINTS + 1, j + 1, 0);
            }
        }
    }

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
