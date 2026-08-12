/*
 * File: z_en_miniblin.c
 * Description: Miniblin, similar to the bokoblins in The Wind Waker. Tries stealing a red rupee from the player.
 * Authors: @syeo501 (Model) @trueffel (Code) — 2ship/MM port by Skijer's NEI.
 *
 * ---- 2ship/MM port notes (this file is unity-#included into trutefel_enemies.cpp) ----------------
 *  - Compiled as C++ inside an extern "C" block: NEVER name a local `this` -> the typed pointer is
 *    `self`; all pointer->uintptr_t / const-dropping conversions are cast explicitly.
 *  - Eye textures are OTR path char arrays; 2ship's gSPSegment shim (mm/src/code/stubs.c:149)
 *    resolves them, so there is NO Lib_SegmentedToVirtual wrapping here.
 *  - OoT->MM API mapping: Actor_MoveXZGravity->Actor_MoveWithGravity,
 *    Animation_ChangeByInfo->Actor_ChangeAnimationByInfo, Matrix_NewMtx->MATRIX_FINALIZE_AND_LOAD,
 *    Matrix_RotateX/Y/Z->Matrix_RotateXF/YF/ZF, targetMode->attentionRangeType,
 *    Audio_PlaySfxGeneral->Audio_PlaySfx_AtPosWithFreq, UPDBGCHECKINFO_FLAG_0|2|3|4 (OoT bits
 *    0,2,3,4) -> MM UPDBGCHECKINFO_FLAG_1|4|8|10 (same bits, hex-named).
 *  - Sfx substitutions: NA_SE_EV_NALE_MAGIC->NA_SE_EN_KOUME_MAGIC (magic sting),
 *    NA_SE_EN_GOMA_JR_FREEZE->NA_SE_EN_COMMON_FREEZE (MM's common enemy stun/freeze).
 */

#include "z_en_miniblin.h"

#include "z64.h"
#include "functions.h"
#include "macros.h"
#include "variables.h"
#include "objects/gameplay_keep/gameplay_keep.h" // MM's own gRupeeRedTex / gRupeeDL (OTR paths)

// O2R gate (mm/mods/items/objects/object_net.c idiom): if the trutefel-enemies.o2r archive is not
// installed the skeleton's limb DLs can't resolve, so the actor kills itself instead of crashing.
extern u8 ResourceMgr_FileExists(const char* resName);

// z-targetable, hostile, update outside cull zone, hookshot pulls the actor (OoT FLAG_0|2|4|9)
#define MINIBLIN_FLAGS \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_HOOKSHOT_PULLS_ACTOR)

void EnMiniblin_Init(Actor* thisx, PlayState* play);
void EnMiniblin_Destroy(Actor* thisx, PlayState* play);
void EnMiniblin_Update(Actor* thisx, PlayState* play);
void EnMiniblin_Draw(Actor* thisx, PlayState* play);

s32 EnMiniblin_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx);
void EnMiniblin_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx);

void EnMiniblin_CheckDamage(EnMiniblin* self, PlayState* play);
void EnMiniblin_UpdateBgCheck(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupDoNothing(EnMiniblin* self, PlayState* play);
void EnMiniblin_DoNothing(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupApproachPlayer(EnMiniblin* self, PlayState* play);
void EnMiniblin_ApproachPlayer(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupTailAttack(EnMiniblin* self, PlayState* play);
void EnMiniblin_TailAttack(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupStunned(EnMiniblin* self, PlayState* play);
void EnMiniblin_Stunned(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupFlee(EnMiniblin* self, PlayState* play);
void EnMiniblin_Flee(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupDamage(EnMiniblin* self, PlayState* play);
void EnMiniblin_Damage(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupLaugh(EnMiniblin* self, PlayState* play);
void EnMiniblin_Laugh(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupDisappear(EnMiniblin* self, PlayState* play);
void EnMiniblin_Disappear(EnMiniblin* self, PlayState* play);
void EnMiniblin_SetupDie(EnMiniblin* self, PlayState* play);
void EnMiniblin_Die(EnMiniblin* self, PlayState* play);

// Body hurtbox. Bumper mask 0xF7CFFFFF = MM's vanilla "accept every player attack" mask (see
// z_en_skb.c) — MM's dmg bit layout differs from OoT's, so OoT's 0xFFCFFFFF is NOT reused.
static ColliderCylinderInit sMiniblinCylinderInit = {
    {
        COL_MATERIAL_HIT5,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_PLAYER,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK1,
        { 0x00000000, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_NONE,
        ACELEM_ON | ACELEM_HOOKABLE,
        OCELEM_ON,
    },
    { 20, 45, 0, { 0, 0, 0 } },
};

// Tail attack quad. Toucher dmgFlags: generic enemy-attack mask (OoT used the raw 0x20000000
// bit, which means something else in MM's table).
static ColliderQuadInit sMiniblinQuadInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_QUAD,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0xF7CFFFFF, 0x00, 0x08 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL | ATELEM_UNK7,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

typedef enum {
    /* 0 */ MINIBLIN_ANIMATION_IDLE,
    /* 1 */ MINIBLIN_ANIMATION_JUMP,
    /* 2 */ MINIBLIN_ANIMATION_TAILATTACK,
    /* 3 */ MINIBLIN_ANIMATION_DAMAGE,
    /* 4 */ MINIBLIN_ANIMATION_LAUGH,
    /* 5 */ MINIBLIN_ANIMATION_BOMBTHROW,
    /* 6 */ MINIBLIN_ANIMATION_DEATH
} EnMiniblinAnimation;

// MM's AnimationInfo has the exact same layout as OoT's (animation, playSpeed, startFrame,
// frameCount, mode, morphFrames) and MM's Actor_ChangeAnimationByInfo == OoT's
// Animation_ChangeByInfo. The ANIMMODE_* interp/partial modes all exist in MM.
static AnimationInfo sMiniblinAnimationInfo[] = {
    { &gMiniblinSkelIdleAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, 3.0f },
    { &gMiniblinSkelJumpAnim, 4.0f, 0.0f, -1.0f, ANIMMODE_LOOP_INTERP, 3.0f },
    { &gMiniblinSkelTailattackAnim, 1.5f, 0.0f, -1.0f, ANIMMODE_ONCE, 0.0f },
    { &gMiniblinSkelDamageAnim, 2.0f, 0.0f, -1.0f, ANIMMODE_ONCE, 0.0f },
    { &gMiniblinSkelLaughAnim, 1.5f, 0.0f, -1.0f, ANIMMODE_ONCE, 0.0f },
    { &gMiniblinSkelBombthrowAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, 0.0f },
    { &gMiniblinSkelDeathAnim, 3.0f, 0.0f, -1.0f, ANIMMODE_ONCE, 0.0f },
};

typedef enum {
    /* 0 */ MINIBLIN_EYES_NORMAL,
    /* 1 */ MINIBLIN_EYES_HALFCLOSED,
    /* 2 */ MINIBLIN_EYES_CLOSED,
    /* 3 */ MINIBLIN_EYES_LAUGH,
    /* 4 */ MINIBLIN_EYES_HIT
} EnMiniblinEyeList;

// OTR path strings — resolved by the gSPSegment shim at draw time, NOT dereferenced here.
static const char* const sMiniblinEyeTextures[] = {
    gMiniblinSkel_eye_normal_rgba16, gMiniblinSkel_eye_halfclosed_rgba16, gMiniblinSkel_eye_closed_rgba16,
    gMiniblinSkel_eye_laugh_rgba16,  gMiniblinSkel_eye_hit_rgba16,
};

typedef enum {
    /*  0 */ ENMINIBLIN_DMGEFF_NONE,
    /*  1 */ ENMINIBLIN_DMGEFF_STUN,
    /*  6 */ ENMINIBLIN_DMGEFF_ICE_MAGIC = 6,
    /* 13 */ ENMINIBLIN_DMGEFF_LIGHT_MAGIC = 13,
    /* 14 */ ENMINIBLIN_DMGEFF_FIRE
} EnMiniblinDamageEffect;

/**
 * MM damage table (32 columns, MM weapon order — rebuilt from the OoT original preserving intent):
 *   deku nut / boomerang-class (Zora boomerang) / hookshot / stun columns -> 0 dmg + STUN;
 *   swords 1 (Sword col covers Kokiri-class), spins/beams 2-3, explosives (incl. Powder Keg) 2,
 *   arrows 2; fire arrow keeps 2 + FIRE, ice 4 + ICE, light 4 + LIGHT (OoT's separate magic
 *   columns don't exist in MM, so the elemental effects ride the arrows);
 *   Goron punch/pound = OoT hammer 2.
 */
static DamageTable sMiniblinDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_STUN),
    /* Deku Stick     */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* Horse trample  */ DMG_ENTRY(1, ENMINIBLIN_DMGEFF_NONE),
    /* Explosives     */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* Zora boomerang */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_STUN),
    /* Normal arrow   */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_NONE),
    /* Hookshot       */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_STUN),
    /* Goron punch    */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* Sword          */ DMG_ENTRY(1, ENMINIBLIN_DMGEFF_NONE),
    /* Goron pound    */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* Fire arrow     */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_FIRE),
    /* Ice arrow      */ DMG_ENTRY(4, ENMINIBLIN_DMGEFF_ICE_MAGIC),
    /* Light arrow    */ DMG_ENTRY(4, ENMINIBLIN_DMGEFF_LIGHT_MAGIC),
    /* Goron spikes   */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* Deku spin      */ DMG_ENTRY(1, ENMINIBLIN_DMGEFF_NONE),
    /* Deku bubble    */ DMG_ENTRY(1, ENMINIBLIN_DMGEFF_NONE),
    /* Deku launch    */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_STUN),
    /* Zora barrier   */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_STUN),
    /* Normal shield  */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_NONE),
    /* Thrown object  */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* Zora punch     */ DMG_ENTRY(1, ENMINIBLIN_DMGEFF_NONE),
    /* Spin attack    */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* Sword beam     */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
    /* Normal Roll    */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, ENMINIBLIN_DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(2, ENMINIBLIN_DMGEFF_NONE),
};

// MM CollisionCheckInfoInit2 field ORDER differs from OoT: health, cylRadius, cylHeight,
// cylYShift, mass. Positional (no designated initializers — this TU compiles as C++).
static CollisionCheckInfoInit2 sMiniblinColChkInit = {
    /* health    */ 4,
    /* cylRadius */ 25,
    /* cylHeight */ 35,
    /* cylYShift */ 0,
    /* mass      */ MASS_HEAVY,
};

void EnMiniblin_SetupAction(EnMiniblin* self, EnMiniblinActionFunc actionFunc) {
    self->actionFunc = actionFunc;
}

void EnMiniblin_ChangeAnimation(EnMiniblin* self, s32 index) {
    Actor_ChangeAnimationByInfo(&self->skelAnime, sMiniblinAnimationInfo, index);
}

void EnMiniblin_ChangeEyes(EnMiniblin* self, s16 eyeIndex) {
    self->eyeIndex = eyeIndex;
}

void EnMiniblin_UpdateEyes(EnMiniblin* self) {
    // Eye blinking logic
    if (self->eyeIndex <= MINIBLIN_EYES_CLOSED) {
        if (DECR(self->blinkTimer) == 0) {
            self->eyeIndex++;
            if (self->eyeIndex >= 2) {
                self->blinkTimer = Rand_S16Offset(30, 30);
                self->eyeIndex = 0;
            }
        }
    }
}

void EnMiniblin_InitAndSetCollision(EnMiniblin* self, PlayState* play) {
    Collider_InitCylinder(play, &self->collider);
    Collider_SetCylinder(play, &self->collider, &self->actor, &sMiniblinCylinderInit);
    Collider_InitQuad(play, &self->quad);
    Collider_SetQuad(play, &self->quad, &self->actor, &sMiniblinQuadInit);
    CollisionCheck_SetInfo2(&self->actor.colChkInfo, &sMiniblinDamageTable, &sMiniblinColChkInit);
}

void EnMiniblin_Init(Actor* thisx, PlayState* play) {
    EnMiniblin* self = (EnMiniblin*)thisx;

    // O2R gate: without the asset archive, the flex skeleton's limb DL paths can't resolve.
    if (!ResourceMgr_FileExists("__OTR__objects/trutefel/object_miniblin/gMiniblinSkel_body_mesh_layer_Opaque")) {
        Actor_Kill(&self->actor);
        return;
    }

    ActorShape_Init(&self->actor.shape, 0.0f, ActorShadow_DrawCircle, 100.0f);
    Actor_SetScale(&self->actor, 0.0035f);
    EnMiniblin_ChangeEyes(self, MINIBLIN_EYES_NORMAL);
    thisx->attentionRangeType = ATTENTION_RANGE_3; // OoT targetMode = 3
    thisx->gravity = -1.0f;

    EnMiniblin_InitAndSetCollision(self, play);
    SkelAnime_InitFlex(play, &self->skelAnime, &gMiniblinSkel, NULL, self->jointTable, self->morphTable,
                       GMINIBLINSKEL_NUM_LIMBS);
    EnMiniblin_ChangeAnimation(self, MINIBLIN_ANIMATION_IDLE);
    EnMiniblin_SetupDoNothing(self, play);
}

void EnMiniblin_Destroy(Actor* thisx, PlayState* play) {
    EnMiniblin* self = (EnMiniblin*)thisx;

    Collider_DestroyCylinder(play, &self->collider);
    Collider_DestroyQuad(play, &self->quad);
}

void EnMiniblin_Update(Actor* thisx, PlayState* play) {
    EnMiniblin* self = (EnMiniblin*)thisx;

    EnMiniblin_CheckDamage(self, play);
    self->actionFunc(self, play);

    Actor_MoveWithGravity(&self->actor);
    EnMiniblin_UpdateBgCheck(self, play);
    EnMiniblin_UpdateEyes(self);

    if (self->actionFunc != EnMiniblin_Die) { // No need for colliders if the Miniblin is dead
        Collider_UpdateCylinder(&self->actor, &self->collider);

        if (DECR(self->hurtboxCooldown) == 0 && self->actionFunc != EnMiniblin_TailAttack &&
            self->actionFunc != EnMiniblin_Laugh && self->actionFunc != EnMiniblin_Disappear) {
            // Miniblin can only take damage by the player if not already hit or doing specific animations
            CollisionCheck_SetAC(play, &play->colChkCtx, &self->collider.base);
        }

        CollisionCheck_SetOC(play, &play->colChkCtx, &self->collider.base);
    }

    if (self->actionFunc == EnMiniblin_TailAttack) {
        // Miniblin can only damage the player when in attack mode
        CollisionCheck_SetAT(play, &play->colChkCtx, &self->quad.base);
    }
}

void EnMiniblin_Draw(Actor* thisx, PlayState* play) {
    EnMiniblin* self = (EnMiniblin*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    // Different eye textures — OTR path handed to the gSPSegment shim (no SegmentedToVirtual).
    gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)sMiniblinEyeTextures[self->eyeIndex]);

    SkelAnime_DrawFlexOpa(play, self->skelAnime.skeleton, self->skelAnime.jointTable, self->skelAnime.dListCount,
                          EnMiniblin_OverrideLimbDraw, EnMiniblin_PostLimbDraw, &self->actor);

    CLOSE_DISPS(play->state.gfxCtx);
}

s32 EnMiniblin_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    return false;
}

static Vec3f sMiniblinTailQuadVertex[4] = {
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 8000.0f, 0.0f },
    { 0.0f, 0.0f, 5000.0f },
    { 0.0f, 8000.0f, 5000.0f },
};

static Vec3f sMiniblinZeroVec = { 0.0f, 0.0f, 0.0f };

void EnMiniblin_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    EnMiniblin* self = (EnMiniblin*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    switch (limbIndex) {
        case GMINIBLINSKEL_TAILEND_LIMB: // The tail of the Miniblin can attack the player
            Matrix_MultVec3f(&sMiniblinTailQuadVertex[0], &self->quad.dim.quad[0]);
            Matrix_MultVec3f(&sMiniblinTailQuadVertex[1], &self->quad.dim.quad[1]);
            Matrix_MultVec3f(&sMiniblinTailQuadVertex[2], &self->quad.dim.quad[2]);
            Matrix_MultVec3f(&sMiniblinTailQuadVertex[3], &self->quad.dim.quad[3]);
            Collider_SetQuadVertices(&self->quad, &self->quad.dim.quad[0], &self->quad.dim.quad[1],
                                     &self->quad.dim.quad[2], &self->quad.dim.quad[3]);

            if (self->aboutToSteal == true) {
                // The miniblin stole a rupee of the player. Display the rupee on his tail.
                // MM's gameplay_keep rupee draws like OoT's: segment 8 = color texture, then gRupeeDL
                // (see z_en_item00.c EnItem00_DrawRupee) — both are OTR paths here.
                Matrix_Push();

                Matrix_Scale(3.0f, 3.0f, 3.0f, MTXMODE_APPLY);
                Matrix_RotateXF(2.0f, MTXMODE_APPLY);
                Matrix_RotateYF(1.4f, MTXMODE_APPLY);
                Matrix_RotateZF(3.0f, MTXMODE_APPLY);
                Matrix_Translate(-500.0f, -700.0f, 450.0f, MTXMODE_APPLY);

                MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

                gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)gRupeeRedTex);
                gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gRupeeDL);
                Matrix_Pop();
            }
            break;
        case GMINIBLINSKEL_HAND_L_LIMB: // If the miniblin stole a rupee, he runs away with it in his left hand
            if (self->rupeeStolen == true) {
                Matrix_Push();

                Matrix_Scale(3.0f, 3.0f, 3.0f, MTXMODE_APPLY);
                Matrix_RotateXF(2.0f, MTXMODE_APPLY);
                Matrix_RotateYF(1.4f, MTXMODE_APPLY);
                Matrix_RotateZF(3.0f, MTXMODE_APPLY);
                Matrix_Translate(-500.0f, 200.0f, 100.0f, MTXMODE_APPLY);

                MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);

                gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)gRupeeRedTex);
                gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gRupeeDL);
                Matrix_Pop();
            }
            break;
        case GMINIBLINSKEL_BODY_LIMB: // This is just for fixing the (Tatl) target position
            Matrix_MultVec3f(&sMiniblinZeroVec, &self->actor.focus.pos);
            break;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void EnMiniblin_CheckDamage(EnMiniblin* self, PlayState* play) {
    if (self->collider.base.acFlags & AC_HIT) {
        self->collider.base.acFlags &= ~AC_HIT;
        self->hurtboxCooldown = 20;
        self->actor.speed = 0.0f;

        if (self->actor.colChkInfo.damageEffect != ENMINIBLIN_DMGEFF_STUN) {
            EnMiniblin_SetupDamage(self, play);
        } else {
            // Stunning effect because of e.g. a deku nut
            Actor_SetColorFilter(&self->actor, COLORFILTER_COLORFLAG_BLUE, 120, COLORFILTER_BUFFLAG_OPA, 60);
            Actor_ApplyDamage(&self->actor);
            EnMiniblin_SetupStunned(self, play);
        }

        if (self->actor.colChkInfo.health == 0) {
            EnMiniblin_SetupDie(self, play);
        }
    }
    if ((self->actor.bgCheckFlags & BGCHECKFLAG_WATER) && self->actionFunc != EnMiniblin_Die) {
        // Currently, the miniblin dies if he falls into a water box
        EnMiniblin_SetupDie(self, play);
    }
}

void EnMiniblin_UpdateBgCheck(EnMiniblin* self, PlayState* play) {
    Actor_UpdateBgCheckInfo(play, &self->actor, self->actor.colChkInfo.cylHeight, self->actor.colChkInfo.cylRadius,
                            self->actor.colChkInfo.cylHeight,
                            (UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4 | UPDBGCHECKINFO_FLAG_8 |
                             UPDBGCHECKINFO_FLAG_10));
}

void EnMiniblin_SetupDoNothing(EnMiniblin* self, PlayState* play) {
    self->actor.speed = 0.0f;
    EnMiniblin_ChangeAnimation(self, MINIBLIN_ANIMATION_IDLE);
    EnMiniblin_ChangeEyes(self, MINIBLIN_EYES_NORMAL);
    EnMiniblin_SetupAction(self, EnMiniblin_DoNothing);
}

void EnMiniblin_DoNothing(EnMiniblin* self, PlayState* play) {
    // Idling around
    SkelAnime_Update(&self->skelAnime);
    if (self->actor.xzDistToPlayer < 280.0f) {
        // Miniblin spots the player
        EnMiniblin_SetupApproachPlayer(self, play);
    }
}

void EnMiniblin_SetupApproachPlayer(EnMiniblin* self, PlayState* play) {
    EnMiniblin_ChangeAnimation(self, MINIBLIN_ANIMATION_JUMP);
    EnMiniblin_SetupAction(self, EnMiniblin_ApproachPlayer);
}

void EnMiniblin_ApproachPlayer(EnMiniblin* self, PlayState* play) {
    SkelAnime_Update(&self->skelAnime);
    if (Animation_OnFrame(&self->skelAnime, 17.0f)) {
        // Optimal frame for playing the sound effect as he touches the ground
        Actor_PlaySfx(&self->actor, NA_SE_EN_TEKU_WALK);
    }

    if (self->skelAnime.curFrame < 18.0f) {
        // The miniblin shouldn't rotate or move when the feet are clearly on the ground
        Math_ApproachF(&self->actor.speed, 20.0f / 3.0f, 0.5f, 2.0f);
        Math_ApproachS(&self->actor.world.rot.y, self->actor.yawTowardsPlayer, 3, 2000);
        Math_ApproachS(&self->actor.shape.rot.y, self->actor.world.rot.y, 2, 3000);
    } else {
        self->actor.speed = 0.0f;
    }

    if (self->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        // Since the miniblin jumps towards the player, stopping his velocity as
        // soon as he touches the ground looks more natural
        self->actor.velocity.y = 0.0f;
    }

    if (self->actor.xzDistToPlayer < 35.0f) {
        // The tail can now hit the player
        EnMiniblin_SetupTailAttack(self, play);
    }

    if (self->actor.xzDistToPlayer > 280.0f) {
        // Player is too far away to still follow him
        EnMiniblin_SetupDoNothing(self, play);
    }
}

void EnMiniblin_SetupTailAttack(EnMiniblin* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->timer = 3;
    EnMiniblin_ChangeAnimation(self, MINIBLIN_ANIMATION_TAILATTACK);
    EnMiniblin_SetupAction(self, EnMiniblin_TailAttack);
}

void EnMiniblin_TailAttack(EnMiniblin* self, PlayState* play) {
    if (self->quad.base.atFlags & AT_HIT) {
        // NA_SE_EV_NALE_MAGIC has no MM equivalent — Koume's magic sting is the closest match.
        Actor_PlaySfx(&self->actor, NA_SE_EN_KOUME_MAGIC);
        if (gSaveContext.save.saveInfo.playerData.rupees >= 20 && self->rupeeStolen == false) {
            // Miniblin only steals rupees if the player has enough or if he didn't already steal one

            if (Rand_ZeroOne() < 0.4f) {
                // ~40% chance for the Miniblin to steal a rupee

                Rupees_ChangeBy(-20); // currently only steals a red rupee. This could be randomized
                self->aboutToSteal = true;
            }
        }
    }
    if (SkelAnime_Update(&self->skelAnime)) {
        if (DECR(self->timer) == 0) {
            if (self->aboutToSteal == true) {
                self->rupeeStolen = true;
                self->aboutToSteal = false;
            }
            EnMiniblin_SetupFlee(self, play);
        }
    }
}

void EnMiniblin_SetupStunned(EnMiniblin* self, PlayState* play) {
    self->actor.speed = 0.0f;
    // NA_SE_EN_GOMA_JR_FREEZE doesn't exist in MM — common enemy freeze/stun sfx instead.
    Actor_PlaySfx(&self->actor, NA_SE_EN_COMMON_FREEZE);
    Animation_PlayOnceSetSpeed(&self->skelAnime, &gMiniblinSkelIdleAnim, 0.0f);
    Actor_SetColorFilter(&self->actor, COLORFILTER_COLORFLAG_BLUE, 120, COLORFILTER_BUFFLAG_OPA, 60);
    EnMiniblin_SetupAction(self, EnMiniblin_Stunned);
}

void EnMiniblin_Stunned(EnMiniblin* self, PlayState* play) {
    if (self->actor.colorFilterTimer == 0) {
        if (self->rupeeStolen == true) {
            // Miniblin continues to try fleeing if he already has a rupee
            EnMiniblin_SetupFlee(self, play);
        } else {
            // Miniblin will still try to get a rupee of the player.
            // EnMiniblin_DoNothing will immediately switch to
            // EnMiniblin_ApproachPlayer if the player is in the near
            EnMiniblin_SetupDoNothing(self, play);
        }
    }
}

void EnMiniblin_SetupFlee(EnMiniblin* self, PlayState* play) {
    self->timer = 100;
    EnMiniblin_ChangeAnimation(self, MINIBLIN_ANIMATION_JUMP);
    // OoT Audio_PlaySfxGeneral(pitch 1.5) -> MM's positional pitched wrapper (MM's AtPos
    // wrappers want the actor's PROJECTED position, not world pos — see z_shot_sun.c).
    Audio_PlaySfx_AtPosWithFreq(&self->actor.projectedPos, NA_SE_VO_IN_LOST, 1.5f);
    EnMiniblin_SetupAction(self, EnMiniblin_Flee);
}

void EnMiniblin_Flee(EnMiniblin* self, PlayState* play) {
    SkelAnime_Update(&self->skelAnime);
    if (Animation_OnFrame(&self->skelAnime, 17.0f)) {
        Actor_PlaySfx(&self->actor, NA_SE_EN_TEKU_WALK);
    }

    if (self->skelAnime.curFrame < 18.0f) {
        Math_ApproachF(&self->actor.speed, 25.0f / 3.0f, 0.5f, 2.0f);
        // opposite direction of the yaw towards player
        Math_ApproachS(&self->actor.world.rot.y, self->actor.yawTowardsPlayer + 0x8000, 3, 2000);
        Math_ApproachS(&self->actor.shape.rot.y, self->actor.world.rot.y, 2, 3000);
    } else {
        self->actor.speed = 0.0f;
    }

    if (self->rupeeStolen == true) {
        if (DECR(self->timer) == 0) {
            // The Miniblin had enough time fleeing
            EnMiniblin_SetupLaugh(self, play);
        }
    }

    if (self->actor.xzDistToPlayer > 150.0f || (self->actor.bgCheckFlags & BGCHECKFLAG_WALL)) {
        // If the miniblin didn't get a rupee, he will try getting back to the player in order to steal one
        if (self->rupeeStolen == false) {
            // this can also switch immediately to EnMiniblin_ApproachPlayer if the player is in the near
            EnMiniblin_SetupDoNothing(self, play);
        }
    }
}

void EnMiniblin_SetupDamage(EnMiniblin* self, PlayState* play) {
    self->damageTimer = 3;
    EnMiniblin_ChangeAnimation(self, MINIBLIN_ANIMATION_DAMAGE);
    EnMiniblin_ChangeEyes(self, MINIBLIN_EYES_HIT);
    Actor_SetColorFilter(&self->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 8);
    Actor_ApplyDamage(&self->actor);
    Actor_PlaySfx(&self->actor, NA_SE_EN_STALKID_DAMAGE);
    EnMiniblin_SetupAction(self, EnMiniblin_Damage);
}

void EnMiniblin_Damage(EnMiniblin* self, PlayState* play) {
    if (SkelAnime_Update(&self->skelAnime)) {
        if (DECR(self->damageTimer) == 0) { // timer for seeing the Miniblin taking damage
            if (self->rupeeStolen == true) {
                // Miniblin already has a rupee and continues fleeing
                EnMiniblin_SetupFlee(self, play);
            } else {
                // Miniblin will continue trying to get a rupee
                EnMiniblin_SetupDoNothing(self, play);
            }
        }
    }
}

void EnMiniblin_SetupLaugh(EnMiniblin* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->actor.shape.rot.y = self->actor.yawTowardsPlayer; // Miniblin rotates to the player and laughs in his face
    EnMiniblin_ChangeAnimation(self, MINIBLIN_ANIMATION_LAUGH);
    EnMiniblin_ChangeEyes(self, MINIBLIN_EYES_LAUGH);
    // OoT played this with pitch 3.5 + volume boost; MM's WithFreq wrapper keeps the pitch
    // (the extra volume-scale argument has no MM equivalent).
    Audio_PlaySfx_AtPosWithFreq(&self->actor.projectedPos, NA_SE_EN_STAL_WARAU, 3.5f);
    EnMiniblin_SetupAction(self, EnMiniblin_Laugh);
}

void EnMiniblin_Laugh(EnMiniblin* self, PlayState* play) {
    if (SkelAnime_Update(&self->skelAnime)) {
        // The Miniblin successfully stole a rupee and despawns
        EnMiniblin_SetupDisappear(self, play);
    }
}

void EnMiniblin_SetupDisappear(EnMiniblin* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED; // Actor not targetable anymore
    self->timer = 12;
    EnMiniblin_SetupAction(self, EnMiniblin_Disappear);
}

void EnMiniblin_Disappear(EnMiniblin* self, PlayState* play) {
    Math_StepToF(&self->actor.scale.x, 0.0f, 0.00034f); // Miniblin shrinks in his scale while despawning
    self->actor.scale.y = self->actor.scale.z = self->actor.scale.x;
    if (DECR(self->timer) == 0) {
        Actor_Kill(&self->actor);
    }
}

void EnMiniblin_SetupDie(EnMiniblin* self, PlayState* play) {
    self->timer = 12;
    self->deathTimer = 12;
    self->actor.speed = 0.0f;
    self->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED; // Miniblin not targetable anymore
    self->actor.shape.shadowAlpha = 0;
    Actor_PlaySfx(&self->actor, NA_SE_EN_STALKID_DEAD);
    Enemy_StartFinishingBlow(play, &self->actor);
    EnMiniblin_ChangeAnimation(self, MINIBLIN_ANIMATION_DEATH);
    EnMiniblin_ChangeEyes(self, MINIBLIN_EYES_CLOSED);
    EnMiniblin_SetupAction(self, EnMiniblin_Die);
}

void EnMiniblin_Die(EnMiniblin* self, PlayState* play) {
    if (SkelAnime_Update(&self->skelAnime)) {
        if (DECR(self->timer) == 0) {
            if (self->deathTimer != 0) {
                self->deathTimer--;
            }
            Math_StepToF(&self->actor.scale.x, 0.0f, 0.00034f); // Miniblin shrinks in his scale while dying
            self->actor.scale.y = self->actor.scale.z = self->actor.scale.x;
            if (self->deathTimer == 0) {
                if (self->rupeeStolen == true) {
                    // The player gets his rupee back if the Miniblin had one stolen
                    Item_DropCollectible(play, &self->actor.world.pos, ITEM00_RUPEE_RED);
                }
                // The Miniblin might also drop some random collectibles (0xE0 is a valid MM
                // random-drop param — see z_boss_05.c)
                Item_DropCollectibleRandom(play, &self->actor, &self->actor.world.pos, 0xE0);
                Actor_Kill(&self->actor);
            }
        }
    }
}

// GAMEPLAY_KEEP object: the flex skeleton's limb DLs are OTR paths resolved standalone at draw
// time, so no scene object bank is needed (same route as boss_remains' allies).
ActorProfile EnMiniblin_Profile = {
    /**/ ACTOR_EN_MINIBLIN,
    /**/ ACTORCAT_ENEMY,
    /**/ MINIBLIN_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnMiniblin),
    /**/ EnMiniblin_Init,
    /**/ EnMiniblin_Destroy,
    /**/ EnMiniblin_Update,
    /**/ EnMiniblin_Draw,
};
