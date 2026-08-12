/*
 * File: z_en_hammergeist.c
 * Description: Molmauk (formerly Hammergeist), an enemy with an ice hammer and a fire hammer.
 * Authors: @syeo501 (Model) | @trueffel (Code) — 2ship/MM port by Skijer's NEI.
 *
 * ---- 2ship/MM port notes (this file is unity-#included into trutefel_enemies.cpp) ----------------
 *  - Compiled as C++ inside an extern "C" block: `this` -> `self`, explicit casts everywhere.
 *  - OoT func_80034BA0 / func_80034CC4 (draw flex skeleton with env alpha, OPA/XLU) do not exist
 *    in MM — re-implemented locally as EnHammergeist_DrawFlexAlphaOpa/Xlu on top of MM's
 *    SkelAnime_DrawFlex (which is why the limb callbacks keep the Gfx**-tailed OverrideLimbDraw /
 *    PostLimbDraw signatures).
 *  - The hammer/face material DLs branch into segment 0x0C (gsSPDisplayList(0xC000000)); the
 *    original relied on the scene binding it. Bound defensively to a no-op DL here (same trick as
 *    boss_remains' allies) so a stray branch just returns.
 *  - OoT->MM API mapping: Actor_SetPlayerKnockbackDamage->func_800B8D50 (last arg = damage),
 *    Actor_SetPlayerKnockbackNoDamage->func_800B8D98, Camera_RequestQuake->Actor_RequestQuake,
 *    player->isBurning/flameTimers->bodyIsBurning/bodyFlameTimers, func_8002843C (sparkle aura)
 *    ->EffectSsKirakira_SpawnDispersed, EffectSsEnIce_SpawnFlyingVec3s->local ice-chunk helper on
 *    MM's EffectSsEnIce_Spawn (Bg_Icicle idiom), Audio_StopSfxById->AudioSfx_StopById.
 *  - Sfx substitutions: NA_SE_EN_GOMA_JR_FREEZE->NA_SE_EN_COMMON_FREEZE,
 *    NA_SE_EN_ANUBIS_FIRE->NA_SE_EV_FLAME_OF_FIRE. Pitched Audio_PlaySfxGeneral calls ->
 *    Audio_PlaySfx_AtPosWithFreq.
 */

#include "z_en_hammergeist.h"

#include "z64.h"
#include "functions.h"
#include "macros.h"
#include "variables.h"

extern u8 ResourceMgr_FileExists(const char* resName);

// z-targetable, hostile, update + draw outside cull zone (OoT FLAG_0|2|4|5)
#define HAMMERGEIST_FLAGS \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_DRAW_CULLING_DISABLED)

void EnHammergeist_Init(Actor* thisx, PlayState* play);
void EnHammergeist_Destroy(Actor* thisx, PlayState* play);
void EnHammergeist_Update(Actor* thisx, PlayState* play);
void EnHammergeist_Draw(Actor* thisx, PlayState* play);

s32 EnHammergeist_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx,
                                   Gfx** gfx);
void EnHammergeist_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx, Gfx** gfx);
// Sets body parts in fire and body transparency
void EnHammergeist_DeadPostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx, Gfx** gfx);

void EnHammergeist_UpdateBgCheck(EnHammergeist* self, PlayState* play);
void EnHammergeist_Movement(EnHammergeist* self, PlayState* play);
void EnHammergeist_CheckDamage(EnHammergeist* self, PlayState* play);

void EnHammergeist_SetupDoNothing(EnHammergeist* self, PlayState* play);
void EnHammergeist_DoNothing(EnHammergeist* self, PlayState* play);
void EnHammergeist_SetupApproachPlayer(EnHammergeist* self, PlayState* play);
void EnHammergeist_ApproachPlayer(EnHammergeist* self, PlayState* play);
void EnHammergeist_SetupDamage(EnHammergeist* self, PlayState* play);
void EnHammergeist_Damage(EnHammergeist* self, PlayState* play);
void EnHammergeist_SetupStunned(EnHammergeist* self, PlayState* play);
void EnHammergeist_Stunned(EnHammergeist* self, PlayState* play);
void EnHammergeist_SetupDie(EnHammergeist* self, PlayState* play);
void EnHammergeist_Die(EnHammergeist* self, PlayState* play);
void EnHammergeist_SetupExplosion(EnHammergeist* self, PlayState* play);
void EnHammergeist_Explosion(EnHammergeist* self, PlayState* play); // 2 Heart Damage
void EnHammergeist_SetupInfuse(EnHammergeist* self, PlayState* play);
void EnHammergeist_Infuse(EnHammergeist* self, PlayState* play);
void EnHammergeist_SetupHeavySlam(EnHammergeist* self, PlayState* play);
void EnHammergeist_HeavySlam(EnHammergeist* self, PlayState* play); // 3 Heart Damage
void EnHammergeist_SetupSlamL(EnHammergeist* self, PlayState* play);
void EnHammergeist_SlamL(EnHammergeist* self, PlayState* play);
void EnHammergeist_SetupSlamR(EnHammergeist* self, PlayState* play);
void EnHammergeist_SlamR(EnHammergeist* self, PlayState* play); // 1 Heart Damage (1 1/2 if infused)
void EnHammergeist_SetupFlex(EnHammergeist* self, PlayState* play);
void EnHammergeist_Flex(EnHammergeist* self, PlayState* play);

static ColliderCylinderInit sHammergeistCylinderInit = {
    {
        COL_MATERIAL_METAL,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_PLAYER,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK1,
        { 0x00000000, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 }, // MM accept-all bumper mask (z_en_skb.c), not OoT's 0xFFCFFFFF
        ATELEM_NONE,
        ACELEM_ON | ACELEM_HOOKABLE,
        OCELEM_ON,
    },
    { 40, 90, 0, { 0, 0, 0 } },
};

// Hammer touchers: OoT used DMG_HAMMER (an OoT dmg bit) — meaningless vs the MM player, so the
// generic enemy-attack mask is used; damage stays 0x10 (0x18 when infused, set at runtime).
static ColliderCylinderInit sHammerLeftCylinderInit = {
    {
        COL_MATERIAL_HIT5,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0xF7CFFFFF, 0x00, 0x10 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { 40, 80, 0, { 0, 0, 0 } },
};

static ColliderCylinderInit sHammerRightCylinderInit = {
    {
        COL_MATERIAL_HIT5,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0xF7CFFFFF, 0x00, 0x10 },
        { 0x00000000, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { 40, 80, 0, { 0, 0, 0 } },
};

static ColliderJntSphElementInit sHammergeistJntSphElementsInit[1] = {
    {
        {
            ELEM_MATERIAL_UNK0,
            { 0xF7CFFFFF, 0x00, 0x20 },
            { 0x00000000, 0x00, 0x00 },
            ATELEM_ON | ATELEM_SFX_NONE,
            ACELEM_ON,
            OCELEM_NONE,
        },
        { 0, { { 0, 0, 900 }, 0 }, 100 },
    },
};

// For the hammer explosion
static ColliderJntSphInit sHammergeistJntSphInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ALL,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_JNTSPH,
    },
    1,
    sHammergeistJntSphElementsInit,
};

typedef enum {
    /* 0 */ HAMMERGEIST_ANIMATION_IDLE,
    /* 1 */ HAMMERGEIST_ANIMATION_WALK,
    /* 2 */ HAMMERGEIST_ANIMATION_DAMAGE,
    /* 3 */ HAMMERGEIST_ANIMATION_DIE,
    /* 4 */ HAMMERGEIST_ANIMATION_EXPLOSION,
    /* 5 */ HAMMERGEIST_ANIMATION_INFUSE,
    /* 6 */ HAMMERGEIST_ANIMATION_SLAM_HEAVY,
    /* 7 */ HAMMERGEIST_ANIMATION_SLAM_L,
    /* 8 */ HAMMERGEIST_ANIMATION_SLAM_R,
    /* 9 */ HAMMERGEIST_ANIMATION_FLEX
} EnHammergeistAnimation;

static AnimationInfo sHammergeistAnimationInfo[] = {
    { &gHammergeistSkelIdleAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP_INTERP, 3.0f },
    { &gHammergeistSkelWalkAnim, 2.0f, 0.0f, -1.0f, ANIMMODE_LOOP_PARTIAL, 3.0f },
    { &gHammergeistSkelDamageAnim, 3.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 3.0f },
    { &gHammergeistSkelDieAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 3.0f },
    { &gHammergeistSkelExplosionAnim, 2.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 3.0f },
    { &gHammergeistSkelInfuseAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 3.0f },
    { &gHammergeistSkelSlamheavyAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 3.0f },
    { &gHammergeistSkelSlamlAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 3.0f },
    { &gHammergeistSkelSlamrAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 3.0f },
    { &gHammergeistSkelFlexAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 3.0f },
};

typedef enum {
    /* 0 */ HAMMERGEIST_FACE_NORMAL,
    /* 1 */ HAMMERGEIST_FACE_LAUGH,
    /* 2 */ HAMMERGEIST_FACE_MOUTH_OPEN
} EnHammergeistFace;

typedef enum {
    /* 0 */ HAMMERGEIST_FIRE_HAMMER_NORMAL,
    /* 1 */ HAMMERGEIST_FIRE_HAMMER_FIRE_1,
    /* 2 */ HAMMERGEIST_FIRE_HAMMER_FIRE_2
} EnHammergeistFireHammer;

typedef enum {
    /* 0 */ HAMMERGEIST_ICE_HAMMER_NORMAL,
    /* 1 */ HAMMERGEIST_ICE_HAMMER_ICE_1,
    /* 2 */ HAMMERGEIST_ICE_HAMMER_ICE_2
} EnHammerGeistIceHammer;

// OTR path strings — resolved by the gSPSegment shim at draw time.
static const char* const sHammergeistFaceTextures[] = {
    gHammergeistSkel_normal_ci8,
    gHammergeistSkel_laugh_ci8,
    gHammergeistSkel_mouth_open_ci8,
};

// Very small texture differences so that the hammer doesn't just look the same the whole time
static const char* const sFireHammerTextures[] = {
    gHammergeistSkel_metal2_rgba16,
    gHammergeistSkel_hammerfire_1_rgba16,
    gHammergeistSkel_hammerfire_2_rgba16,
};

static const char* const sIceHammerTextures[] = {
    gHammergeistSkel_metal2_rgba16,
    gHammergeistSkel_hammerice_1_rgba16,
    gHammergeistSkel_hammerice_2_rgba16,
};

typedef enum {
    /*  0 */ ENHAMMERGEIST_DMGEFF_NONE,
    /*  1 */ ENHAMMERGEIST_DMGEFF_STUN,
    /*  6 */ ENHAMMERGEIST_DMGEFF_ICE_MAGIC = 6,
    /* 13 */ ENHAMMERGEIST_DMGEFF_LIGHT_MAGIC = 13,
    /* 14 */ ENHAMMERGEIST_DMGEFF_FIRE
} EnHammergeistDamageEffect;

/**
 * MM damage table (MM weapon order — rebuilt from the OoT original preserving intent):
 * stun columns (nut/boomerang/hookshot) -> 0 + STUN, explosives 0 (he shrugs them off, as in the
 * original), sword 1, spins 3, arrows 2, fire arrow 2 + FIRE, ice arrow 3 + ICE (OoT's ice-magic
 * damage), light arrow 4 + LIGHT, Goron punch/pound = OoT hammer 2.
 */
static DamageTable sHammergeistDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_STUN),
    /* Deku Stick     */ DMG_ENTRY(2, ENHAMMERGEIST_DMGEFF_NONE),
    /* Horse trample  */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* Explosives     */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* Zora boomerang */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_STUN),
    /* Normal arrow   */ DMG_ENTRY(2, ENHAMMERGEIST_DMGEFF_NONE),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* Hookshot       */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_STUN),
    /* Goron punch    */ DMG_ENTRY(2, ENHAMMERGEIST_DMGEFF_NONE),
    /* Sword          */ DMG_ENTRY(1, ENHAMMERGEIST_DMGEFF_NONE),
    /* Goron pound    */ DMG_ENTRY(2, ENHAMMERGEIST_DMGEFF_NONE),
    /* Fire arrow     */ DMG_ENTRY(2, ENHAMMERGEIST_DMGEFF_FIRE),
    /* Ice arrow      */ DMG_ENTRY(3, ENHAMMERGEIST_DMGEFF_ICE_MAGIC),
    /* Light arrow    */ DMG_ENTRY(4, ENHAMMERGEIST_DMGEFF_LIGHT_MAGIC),
    /* Goron spikes   */ DMG_ENTRY(2, ENHAMMERGEIST_DMGEFF_NONE),
    /* Deku spin      */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* Deku bubble    */ DMG_ENTRY(1, ENHAMMERGEIST_DMGEFF_NONE),
    /* Deku launch    */ DMG_ENTRY(2, ENHAMMERGEIST_DMGEFF_NONE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_STUN),
    /* Zora barrier   */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_STUN),
    /* Normal shield  */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* Thrown object  */ DMG_ENTRY(1, ENHAMMERGEIST_DMGEFF_NONE),
    /* Zora punch     */ DMG_ENTRY(1, ENHAMMERGEIST_DMGEFF_NONE),
    /* Spin attack    */ DMG_ENTRY(3, ENHAMMERGEIST_DMGEFF_NONE),
    /* Sword beam     */ DMG_ENTRY(2, ENHAMMERGEIST_DMGEFF_NONE),
    /* Normal Roll    */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(0, ENHAMMERGEIST_DMGEFF_NONE),
};

// MM field order: health, cylRadius, cylHeight, cylYShift, mass (differs from OoT).
static CollisionCheckInfoInit2 sHammergeistColChkInit = {
    /* health    */ 16,
    /* cylRadius */ 35,
    /* cylHeight */ 55,
    /* cylYShift */ 0,
    /* mass      */ MASS_HEAVY,
};

// The material DLs contain gsSPDisplayList(0xC000000) branches; bind segment 0x0C to a no-op so
// they return immediately (spiritual_stone_statue.c / remains_ally idiom).
static Gfx sHammergeistSeg0xC_Noop[] = {
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
};

void EnHammergeist_SetupAction(EnHammergeist* self, EnHammergeistActionFunc actionFunc) {
    self->actionFunc = actionFunc;
}

void EnHammergeist_ChangeAnimation(EnHammergeist* self, s32 index) {
    Actor_ChangeAnimationByInfo(&self->skelAnime, sHammergeistAnimationInfo, index);
}

void EnHammergeist_ChangeFace(EnHammergeist* self, s16 faceIndex) {
    self->faceIndex = faceIndex;
}

/**
 * OoT func_80034BA0 replacement: draw the flex skeleton on the OPA list with an env-alpha
 * setup. Uses SkelAnime_DrawFlex so the Gfx**-tailed limb callbacks work like the original's.
 */
static void EnHammergeist_DrawFlexAlphaOpa(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                                           PostLimbDraw postLimbDraw, Actor* actor, s16 alpha) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, alpha);
    POLY_OPA_DISP = SkelAnime_DrawFlex(play, skelAnime->skeleton, skelAnime->jointTable, skelAnime->dListCount,
                                       overrideLimbDraw, postLimbDraw, actor, POLY_OPA_DISP);

    CLOSE_DISPS(play->state.gfxCtx);
}

/**
 * OoT func_80034CC4 replacement: same, on the XLU list (used for the transparent death fade).
 */
static void EnHammergeist_DrawFlexAlphaXlu(PlayState* play, SkelAnime* skelAnime, OverrideLimbDraw overrideLimbDraw,
                                           PostLimbDraw postLimbDraw, Actor* actor, s16 alpha) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);
    gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, alpha);
    POLY_XLU_DISP = SkelAnime_DrawFlex(play, skelAnime->skeleton, skelAnime->jointTable, skelAnime->dListCount,
                                       overrideLimbDraw, postLimbDraw, actor, POLY_XLU_DISP);

    CLOSE_DISPS(play->state.gfxCtx);
}

/**
 * OoT EffectSsEnIce_SpawnFlyingVec3s replacement: MM only has EffectSsEnIce_Spawn (no built-in
 * fling), so each chunk gets a random outward velocity (Bg_Icicle's shatter idiom).
 */
static void EnHammergeist_SpawnIceChunk(PlayState* play, Vec3s* posS) {
    static Color_RGBA8 sIcePrimColor = { 150, 150, 150, 250 };
    static Color_RGBA8 sIceEnvColor = { 235, 245, 255, 255 };
    static Vec3f sIceAccel = { 0.0f, -1.0f, 0.0f };
    Vec3f pos;
    Vec3f velocity;

    pos.x = posS->x + Rand_CenteredFloat(10.0f);
    pos.y = posS->y + Rand_CenteredFloat(10.0f);
    pos.z = posS->z + Rand_CenteredFloat(10.0f);

    velocity.x = Rand_CenteredFloat(7.0f);
    velocity.y = (Rand_ZeroOne() * 4.0f) + 6.0f;
    velocity.z = Rand_CenteredFloat(7.0f);

    EffectSsEnIce_Spawn(play, &pos, (Rand_ZeroOne() * 0.3f) + 0.2f, &velocity, &sIceAccel, &sIcePrimColor,
                        &sIceEnvColor, 30);
}

// Very small texture differences so that the hammers don't just look the same the whole time
void EnHammergeist_HammerAppearance(EnHammergeist* self, PlayState* play) {
    if (self->rightHammerInfused) {
        if (self->fireHammerIndex == HAMMERGEIST_FIRE_HAMMER_NORMAL) {
            self->fireHammerIndex = HAMMERGEIST_FIRE_HAMMER_FIRE_1;
        }
        if (play->gameplayFrames % 16 == 0) {
            self->fireHammerIndex = self->fireHammerIndex == HAMMERGEIST_FIRE_HAMMER_FIRE_1
                                        ? HAMMERGEIST_FIRE_HAMMER_FIRE_2
                                        : HAMMERGEIST_FIRE_HAMMER_FIRE_1;
        }
    } else {
        if (self->fireHammerIndex != HAMMERGEIST_FIRE_HAMMER_NORMAL) {
            self->fireHammerIndex = HAMMERGEIST_FIRE_HAMMER_NORMAL;
        }
    }

    if (self->leftHammerInfused) {
        if (self->iceHammerIndex == HAMMERGEIST_ICE_HAMMER_NORMAL) {
            self->iceHammerIndex = HAMMERGEIST_ICE_HAMMER_ICE_1;
        }
        if (play->gameplayFrames % 16 == 0) {
            self->iceHammerIndex = self->iceHammerIndex == HAMMERGEIST_ICE_HAMMER_ICE_1 ? HAMMERGEIST_ICE_HAMMER_ICE_2
                                                                                        : HAMMERGEIST_ICE_HAMMER_ICE_1;
        }
    } else {
        if (self->iceHammerIndex != HAMMERGEIST_ICE_HAMMER_NORMAL) {
            self->iceHammerIndex = HAMMERGEIST_ICE_HAMMER_NORMAL;
        }
    }
}

void EnHammergeist_InitAndSetCollision(EnHammergeist* self, PlayState* play) {
    Collider_InitCylinder(play, &self->collider);
    Collider_SetCylinder(play, &self->collider, &self->actor, &sHammergeistCylinderInit);

    Collider_InitCylinder(play, &self->hammerLeftCollider);
    Collider_SetCylinder(play, &self->hammerLeftCollider, &self->actor, &sHammerLeftCylinderInit);

    Collider_InitCylinder(play, &self->hammerRightCollider);
    Collider_SetCylinder(play, &self->hammerRightCollider, &self->actor, &sHammerRightCylinderInit);

    Collider_InitJntSph(play, &self->explosionCollider);
    Collider_SetJntSph(play, &self->explosionCollider, &self->actor, &sHammergeistJntSphInit,
                       &self->explosionColliderItems[0]);

    CollisionCheck_SetInfo2(&self->actor.colChkInfo, &sHammergeistDamageTable, &sHammergeistColChkInit);
}

void EnHammergeist_UpdateCollision(EnHammergeist* self, PlayState* play) {
    if (DECR(self->hurtboxCooldown) == 0 && self->actionFunc != EnHammergeist_Die) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &self->collider.base);
        CollisionCheck_SetOC(play, &play->colChkCtx, &self->collider.base);
    }
}

void EnHammergeist_UpdateHammerCollider(EnHammergeist* self, PlayState* play) {
    // MM player hit effects: 1 = fire, 2 = freeze (verified against z_en_firefly.c) — same
    // values the OoT original used. The infused hammers just hit harder + carry the effect.
    if (self->leftHammerInfused) { // More damage and ice effect
        self->hammerLeftCollider.elem.atDmgInfo.effect = 2; // Ice
        self->hammerLeftCollider.elem.atDmgInfo.damage = 0x18;
    } else {
        self->hammerLeftCollider.elem.atDmgInfo.effect = 0;
        self->hammerLeftCollider.elem.atDmgInfo.damage = 0x10;
    }

    if (self->rightHammerInfused) { // More damage and fire effect
        self->hammerRightCollider.elem.atDmgInfo.effect = 1; // Fire
        self->hammerRightCollider.elem.atDmgInfo.damage = 0x18;
    } else {
        self->hammerRightCollider.elem.atDmgInfo.effect = 0;
        self->hammerRightCollider.elem.atDmgInfo.damage = 0x10;
    }

    // If the hammers explode with ice and fire together, the explosion causes more damage
    if (self->leftHammerInfused && self->rightHammerInfused) {
        self->explosionColliderItems[0].base.atDmgInfo.damage = 0x40; // 4 Heart Damage
    } else {
        self->explosionColliderItems[0].base.atDmgInfo.damage = 0x20; // 2 Heart Damage
    }
}

void EnHammergeist_DefuseLeftHammer(EnHammergeist* self, PlayState* play) {
    s32 i;

    self->leftHammerInfused = false;

    for (i = 0; i <= 7; i++) { // The pushing ice energy gets visualized by ice fragments
        EnHammergeist_SpawnIceChunk(play, &self->hammerLeftCollider.dim.pos);
    }
}

void EnHammergeist_DefuseRightHammer(EnHammergeist* self, PlayState* play) {
    s32 i;

    self->rightHammerInfused = false;

    for (i = 0; i <= 7; i++) { // The pushing fire energy gets visualized as a big flame
        EffectSsEnFire_SpawnVec3s(play, &self->actor, &self->hammerRightCollider.dim.pos, 400, 0, 0, -1);
    }
}

void EnHammergeist_Init(Actor* thisx, PlayState* play) {
    EnHammergeist* self = (EnHammergeist*)thisx;

    // O2R gate: without trutefel-enemies.o2r the skeleton's limb DL paths can't resolve.
    if (!ResourceMgr_FileExists("__OTR__objects/trutefel/object_hammergeist/gHammergeistSkel_body_mesh_layer_Opaque")) {
        Actor_Kill(&self->actor);
        return;
    }

    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_IDLE);
    ActorShape_Init(&self->actor.shape, 0.0f, ActorShadow_DrawCircle, 80.0f);
    Actor_SetScale(&self->actor, 0.015f);

    thisx->gravity = -1.0f;
    self->explosionTimer = 20;
    self->infuseTimer = 20;
    self->slamTimer = 20;
    self->heavySlamTimer = 60;
    self->leftHammerInfused = false;
    self->rightHammerInfused = false;
    self->playerHit = false;
    self->alpha = 255;

    EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_NORMAL);

    EnHammergeist_InitAndSetCollision(self, play);
    SkelAnime_InitFlex(play, &self->skelAnime, &gHammergeistSkel, NULL, self->jointTable, self->morphTable,
                       GHAMMERGEISTSKEL_NUM_LIMBS);
    EnHammergeist_SetupDoNothing(self, play);
}

void EnHammergeist_Destroy(Actor* thisx, PlayState* play) {
    EnHammergeist* self = (EnHammergeist*)thisx;

    SkelAnime_Free(&self->skelAnime, play);
    Collider_DestroyCylinder(play, &self->collider);
    Collider_DestroyCylinder(play, &self->hammerLeftCollider);
    Collider_DestroyCylinder(play, &self->hammerRightCollider);
    Collider_DestroyJntSph(play, &self->explosionCollider);
}

void EnHammergeist_Update(Actor* thisx, PlayState* play) {
    EnHammergeist* self = (EnHammergeist*)thisx;

    self->actionFunc(self, play);

    Actor_MoveWithGravity(thisx);
    EnHammergeist_UpdateBgCheck(self, play);

    Collider_UpdateCylinder(&self->actor, &self->collider);
    Collider_UpdateCylinder(&self->actor, &self->hammerLeftCollider);
    Collider_UpdateCylinder(&self->actor, &self->hammerRightCollider);

    EnHammergeist_UpdateCollision(self, play);
    EnHammergeist_UpdateHammerCollider(self, play);
    EnHammergeist_HammerAppearance(self, play);

    Actor_TrackPlayer(play, &self->actor, &self->headRot, &self->upperBodyRot, self->actor.focus.pos);
}

void EnHammergeist_Draw(Actor* thisx, PlayState* play) {
    EnHammergeist* self = (EnHammergeist*)thisx;

    Collider_UpdateSpheres(0, &self->explosionCollider);

    OPEN_DISPS(play->state.gfxCtx);

    if (self->alpha == 255) { // Alive
        gSPSegment(POLY_OPA_DISP++, 0x08, (uintptr_t)sFireHammerTextures[self->fireHammerIndex]);
        gSPSegment(POLY_OPA_DISP++, 0x09, (uintptr_t)sIceHammerTextures[self->iceHammerIndex]);
        gSPSegment(POLY_OPA_DISP++, 0x0A, (uintptr_t)sHammergeistFaceTextures[self->faceIndex]);
        gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)sHammergeistSeg0xC_Noop);

        EnHammergeist_DrawFlexAlphaOpa(play, &self->skelAnime, EnHammergeist_OverrideLimbDraw,
                                       EnHammergeist_PostLimbDraw, thisx, 255);
    } else {                    // Dead
        if (self->alpha != 0) { // Molmauk loses his transparency over time
            gSPSegment(POLY_XLU_DISP++, 0x08, (uintptr_t)sFireHammerTextures[self->fireHammerIndex]);
            gSPSegment(POLY_XLU_DISP++, 0x09, (uintptr_t)sIceHammerTextures[self->iceHammerIndex]);
            gSPSegment(POLY_XLU_DISP++, 0x0A, (uintptr_t)sHammergeistFaceTextures[self->faceIndex]);
            gSPSegment(POLY_XLU_DISP++, 0x0C, (uintptr_t)sHammergeistSeg0xC_Noop);
            EnHammergeist_DrawFlexAlphaXlu(play, &self->skelAnime, NULL, EnHammergeist_DeadPostLimbDraw, thisx,
                                           self->alpha);
        }

        if (self->fireTimer != 0) { // Molmauk is burning down when dying
            thisx->colorFilterTimer++;
            self->fireTimer--;
            if (self->fireTimer % 4 == 0) {
                EffectSsEnFire_SpawnVec3s(play, thisx, &self->firePos[self->fireTimer >> 2], 250, 0, 0,
                                          (self->fireTimer >> 2));
            }
        }
    }
    CLOSE_DISPS(play->state.gfxCtx);
}

s32 EnHammergeist_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx,
                                   Gfx** gfx) {
    EnHammergeist* self = (EnHammergeist*)thisx;

    switch (limbIndex) {
        // Rotate head towards player
        case GHAMMERGEISTSKEL_HEAD_LIMB:
            if (self->actionFunc == EnHammergeist_ApproachPlayer) {
                rot->z += self->headRot.y;
                rot->x += self->headRot.x;
            }
            break;
    }

    return false;
}

static Vec3f sHammergeistZeroVec = { 0.0f, 0.0f, 0.0f };

void EnHammergeist_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx, Gfx** gfx) {
    static Vec3f fireEffPos;
    static Vec3f iceEffPos;
    static Vec3f effVelocity = { 0.0f, 0.0f, 0.0f };
    static Vec3f effAccel = { 0.0f, 0.0f, 0.0f };
    static Color_RGBA8 fireAuraPrimColor = { 255, 255, 100, 255 };
    static Color_RGBA8 fireAuraEnvColor = { 255, 50, 0, 0 };
    static Color_RGBA8 iceAuraPrimColor = { 100, 200, 255, 255 };
    static Color_RGBA8 iceAuraEnvColor = { 0, 0, 255, 0 };
    EnHammergeist* self = (EnHammergeist*)thisx;
    MtxF mtx;

    Matrix_Get(&mtx); // This is for positioning the hammer effects and AT colliders

    switch (limbIndex) {
        case GHAMMERGEISTSKEL_HEAD_LIMB:
            Matrix_MultVec3f(&sHammergeistZeroVec, &self->actor.focus.pos);
            break;

        // Positioning code for the ice effect on the left hammer
        case GHAMMERGEISTSKEL_HAMMERL_LIMB:
            self->hammerLeftCollider.dim.pos.x = mtx.xw;
            self->hammerLeftCollider.dim.pos.y = (mtx.yw - 40.0f);
            self->hammerLeftCollider.dim.pos.z = mtx.zw;

            iceEffPos.x = mtx.xw;
            iceEffPos.y = mtx.yw + 30.0f;
            iceEffPos.z = mtx.zw;
            break;

        // Positioning code for the fire effect on the right hammer
        case GHAMMERGEISTSKEL_HAMMERR_LIMB:
            self->hammerRightCollider.dim.pos.x = mtx.xw;
            self->hammerRightCollider.dim.pos.y = (mtx.yw - 40.0f);
            self->hammerRightCollider.dim.pos.z = mtx.zw;

            fireEffPos.x = mtx.xw;
            fireEffPos.y = mtx.yw + 30.0f;
            fireEffPos.z = mtx.zw;
            break;
    }

    // Fire effect (OoT func_8002843C == kirakira sparkle burst)
    if (self->rightHammerInfused) {
        EffectSsKirakira_SpawnDispersed(play, &fireEffPos, &effVelocity, &effAccel, &fireAuraPrimColor,
                                        &fireAuraEnvColor, 500, 10);
    }

    // Ice effect
    if (self->leftHammerInfused) {
        EffectSsKirakira_SpawnDispersed(play, &iceEffPos, &effVelocity, &effAccel, &iceAuraPrimColor, &iceAuraEnvColor,
                                        500, 10);
    }
}

// Flames on all his body parts when dying
void EnHammergeist_DeadPostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx, Gfx** gfx) {
    EnHammergeist* self = (EnHammergeist*)thisx;
    s32 idx = -1;
    Vec3f modifiedVec = { 300.0f, 0.0f, 0.0f };
    Vec3f destPos;

    if (self->fireTimer != 0) {
        switch (limbIndex) {
            case GHAMMERGEISTSKEL_HEAD_LIMB:
                idx = 0;
                break;

            case GHAMMERGEISTSKEL_HAMMERL_LIMB:
                idx = 1;
                break;

            case GHAMMERGEISTSKEL_HAMMERR_LIMB:
                idx = 2;
                break;

            case GHAMMERGEISTSKEL_BODY_LIMB:
                idx = 3;
                break;

            case GHAMMERGEISTSKEL_HAND_L_LIMB:
                idx = 4;
                break;

            case GHAMMERGEISTSKEL_HAND_R_LIMB:
                idx = 5;
                break;

            case GHAMMERGEISTSKEL_FOOT_L_LIMB:
                idx = 6;
                break;

            case GHAMMERGEISTSKEL_FOOT_R_LIMB:
                idx = 7;
                break;

            case GHAMMERGEISTSKEL_ARM_L_LIMB:
                idx = 8;
                break;

            case GHAMMERGEISTSKEL_ARM_R_LIMB:
                idx = 9;
                break;
        }
    }

    if (idx >= 0) { // this is straight off copied ReDead code
        Matrix_MultVec3f(&modifiedVec, &destPos);
        self->firePos[idx].x = destPos.x;
        self->firePos[idx].y = destPos.y;
        self->firePos[idx].z = destPos.z;
    }
}

void EnHammergeist_UpdateBgCheck(EnHammergeist* self, PlayState* play) {
    Actor_UpdateBgCheckInfo(play, &self->actor, self->actor.colChkInfo.cylHeight, self->actor.colChkInfo.cylRadius,
                            self->actor.colChkInfo.cylHeight,
                            (UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4 | UPDBGCHECKINFO_FLAG_8 |
                             UPDBGCHECKINFO_FLAG_10));
}

// Move towards Link, stand still if right infront of him
void EnHammergeist_Movement(EnHammergeist* self, PlayState* play) {
    SkelAnime_Update(&self->skelAnime);

    if (self->actor.xzDistToPlayer <= 75.0f) {
        if (self->skelAnime.animation != &gHammergeistSkelIdleAnim) {
            EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_IDLE);
        }
        Math_ApproachS(&self->actor.world.rot.y, self->actor.yawTowardsPlayer, 3, 2000);
        Math_ApproachS(&self->actor.shape.rot.y, self->actor.world.rot.y, 2, 3000);
        self->actor.speed = 0.0f;
    } else {
        if (self->skelAnime.animation != &gHammergeistSkelWalkAnim) {
            EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_WALK);
        }
        if ((self->skelAnime.curFrame >= 10.0f && self->skelAnime.curFrame <= 20.0f) ||
            (self->skelAnime.curFrame >= 38.0f && self->skelAnime.curFrame <= 45.0f)) {
            self->actor.speed = 0.0f;
            if (self->skelAnime.curFrame == 10.0f || self->skelAnime.curFrame == 38.0f) {
                Actor_PlaySfx(&self->actor, NA_SE_EN_AMOS_WALK);
            }
        } else {
            Math_ApproachF(&self->actor.speed, 5.0f / 1.5f, 0.5f, 1.5f);
            Math_ApproachS(&self->actor.world.rot.y, self->actor.yawTowardsPlayer, 3, 2000);
            Math_ApproachS(&self->actor.shape.rot.y, self->actor.world.rot.y, 2, 3000);
        }
    }
}

void EnHammergeist_CheckDamage(EnHammergeist* self, PlayState* play) {
    if (self->collider.base.acFlags & AC_HIT) {
        self->collider.base.acFlags &= ~AC_HIT;
        self->hurtboxCooldown = 10;
        self->actor.speed = 0.0f;

        if (self->actor.colChkInfo.damageEffect != ENHAMMERGEIST_DMGEFF_STUN) {
            EnHammergeist_SetupDamage(self, play);
        } else {
            // Stunning effect because of e.g. a deku nut
            Actor_SetColorFilter(&self->actor, COLORFILTER_COLORFLAG_BLUE, 120, COLORFILTER_BUFFLAG_OPA, 60);
            Actor_ApplyDamage(&self->actor);
            EnHammergeist_SetupStunned(self, play);
        }

        if (self->actor.colChkInfo.health == 0) {
            EnHammergeist_SetupDie(self, play);
        }
    }
    if ((self->actor.bgCheckFlags & BGCHECKFLAG_WATER) && self->actionFunc != EnHammergeist_Die) {
        // Currently, the Hammergeist dies if he falls into a water box
        EnHammergeist_SetupDie(self, play);
    }
}

void EnHammergeist_SetupDoNothing(EnHammergeist* self, PlayState* play) {
    self->actor.speed = 0.0f;
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_IDLE);
    EnHammergeist_SetupAction(self, EnHammergeist_DoNothing);
}

void EnHammergeist_DoNothing(EnHammergeist* self, PlayState* play) {
    SkelAnime_Update(&self->skelAnime);

    // Player noticed, get active
    if (self->actor.xzDistToPlayer < 800.0f) {
        EnHammergeist_SetupApproachPlayer(self, play);
    }
}

void EnHammergeist_SetupApproachPlayer(EnHammergeist* self, PlayState* play) {
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_WALK);
    EnHammergeist_SetupAction(self, EnHammergeist_ApproachPlayer);
}

void EnHammergeist_ApproachPlayer(EnHammergeist* self, PlayState* play) {
    EnHammergeist_Movement(self, play);

    if (self->actor.xzDistToPlayer > 1500.0f) {
        EnHammergeist_SetupDoNothing(self, play);
    }

    if (self->actor.xzDistToPlayer < 120.0f) {
        if (DECR(self->slamTimer) == 0) {
            self->slamTimer = 30;
            if (Rand_ZeroOne() < 0.6f) {
                if (play->gameplayFrames % 2 == 0) {
                    // Either hit with the left hammer
                    EnHammergeist_SetupSlamL(self, play);
                } else {
                    // Or the right hammer
                    EnHammergeist_SetupSlamR(self, play);
                }
            }
        }
    }

    if (!self->leftHammerInfused && !self->rightHammerInfused) {
        if (DECR(self->infuseTimer) == 0) {
            self->infuseTimer = 40;
            if (Rand_ZeroOne() < 0.2f) {
                // 20% chance
                EnHammergeist_SetupInfuse(self, play);
            }
        }
    }

    if (self->actor.xzDistToPlayer < 170.0f && self->actor.xzDistToPlayer > 60.0f) {
        if (DECR(self->explosionTimer) == 0) {
            self->explosionTimer = 20;
            if (Rand_ZeroOne() < 0.3f) {
                // 30% chance
                EnHammergeist_SetupExplosion(self, play);
            }
        }
    }
    if (DECR(self->heavySlamCooldown) == 0) {
        if (DECR(self->heavySlamTimer) == 0) {
            self->heavySlamTimer = 60;
            if (Rand_ZeroOne() < 0.2f) {
                // 20% chance
                EnHammergeist_SetupHeavySlam(self, play);
            }
        }
    }
}

void EnHammergeist_SetupDamage(EnHammergeist* self, PlayState* play) {
    self->genericAnimationTimer = 5;
    EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_NORMAL);
    Actor_SetColorFilter(&self->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 8);
    Actor_ApplyDamage(&self->actor);
    // OoT played this with pitch 0.25 (deep grunt) — MM positional pitched wrapper (MM's AtPos
    // wrappers want the actor's PROJECTED position, not world pos — see z_shot_sun.c).
    Audio_PlaySfx_AtPosWithFreq(&self->actor.projectedPos, NA_SE_EN_STALKID_DAMAGE, 0.25f);
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_DAMAGE);
    EnHammergeist_SetupAction(self, EnHammergeist_Damage);
}

void EnHammergeist_Damage(EnHammergeist* self, PlayState* play) {
    if (SkelAnime_Update(&self->skelAnime)) {
        if (DECR(self->genericAnimationTimer) == 0) {
            // Molmauk might take revenge for getting hit
            if (Rand_ZeroOne() < 0.4f && self->noHitAgain == false) {
                // 40% chance
                self->noHitAgain = true;
                if (play->gameplayFrames % 2 == 0) {
                    // either left slam
                    EnHammergeist_SetupSlamL(self, play);
                } else {
                    // or right slam
                    EnHammergeist_SetupSlamR(self, play);
                }
            } else {
                self->noHitAgain = false;
                EnHammergeist_SetupDoNothing(self, play);
            }
        }
    }
}

void EnHammergeist_SetupStunned(EnHammergeist* self, PlayState* play) {
    self->actor.speed = 0.0f;
    Actor_PlaySfx(&self->actor, NA_SE_EN_COMMON_FREEZE); // MM's common enemy stun/freeze sfx
    Animation_PlayOnceSetSpeed(&self->skelAnime, &gHammergeistSkelIdleAnim, 0.0f);
    Actor_SetColorFilter(&self->actor, COLORFILTER_COLORFLAG_BLUE, 120, COLORFILTER_BUFFLAG_OPA, 60);
    EnHammergeist_SetupAction(self, EnHammergeist_Stunned);
}

void EnHammergeist_Stunned(EnHammergeist* self, PlayState* play) {
    EnHammergeist_CheckDamage(self, play);
    if (self->actor.colorFilterTimer == 0) {
        EnHammergeist_SetupDoNothing(self, play);
    }
}

void EnHammergeist_SetupDie(EnHammergeist* self, PlayState* play) {
    EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_NORMAL);
    self->actor.shape.shadowDraw = NULL;
    if (self->leftHammerInfused) {
        EnHammergeist_DefuseLeftHammer(self, play);
    }
    if (self->rightHammerInfused) {
        EnHammergeist_DefuseRightHammer(self, play);
    }
    self->actor.speed = 0.0f;
    self->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED; // Molmauk not targetable anymore
    Actor_SetColorFilter(&self->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 80);
    self->fireTimer = 40;
    Enemy_StartFinishingBlow(play, &self->actor);
    // NA_SE_EN_ANUBIS_FIRE doesn't exist in MM — burst of flame sfx instead.
    Actor_PlaySfx(&self->actor, NA_SE_EV_FLAME_OF_FIRE);
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_DIE);
    EnHammergeist_SetupAction(self, EnHammergeist_Die);
}

void EnHammergeist_Die(EnHammergeist* self, PlayState* play) {
    // Molmauk loses his transparency when dying
    if (self->alpha != 0) {
        if (play->gameplayFrames % 2 == 0) {
            self->alpha -= 5;
        }
    }
    if (SkelAnime_Update(&self->skelAnime)) {
        if (DECR(self->fireTimer) == 0 && self->actor.colorFilterTimer == 0) {
            Actor_Kill(&self->actor);
        }
    }
}

void EnHammergeist_SetupExplosion(EnHammergeist* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->genericAnimationTimer = 33;
    self->explosionRadiusIncrease = false;
    self->playerHit = false;
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_EXPLOSION);
    EnHammergeist_SetupAction(self, EnHammergeist_Explosion);
    self->actor.world.rot.y = self->actor.yawTowardsPlayer;
    self->actor.shape.rot.y = self->actor.world.rot.y;
}

void EnHammergeist_Explosion(EnHammergeist* self, PlayState* play) {
    Vec3f effPos = self->actor.world.pos;
    Vec3f effVel = { 0.0f, 0.0f, 0.0f };
    Vec3f effAcc = { 0.0f, 0.0f, 0.0f };

    if (SkelAnime_Update(&self->skelAnime)) {
        if (DECR(self->genericAnimationTimer) == 0) {
            if (self->playerHit == true) {
                // Player got hit, emote on him
                self->playerHit = false;
                EnHammergeist_SetupFlex(self, play);
            } else {
                EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_NORMAL);
                EnHammergeist_SetupDoNothing(self, play);
            }
        }
    }

    if (self->explosionCollider.base.atFlags & AT_HIT) {
        self->explosionCollider.base.atFlags &= ~AT_HIT;
        self->playerHit = true;
        // knockback without damage (the collider already delivered the damage)
        func_800B8D98(play, &self->actor, 10.0f, self->actor.shape.rot.y, 5.0f);
        Player_PlaySfx(GET_PLAYER(play), NA_SE_PL_BODY_HIT);
    }

    if (self->explosionRadiusIncrease == true) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &self->explosionCollider.base);
        self->explosionCollider.elements[0].dim.modelSphere.radius += 15;
        self->explosionCollider.elements[0].dim.worldSphere.radius =
            self->explosionCollider.elements[0].dim.modelSphere.radius;
        if (self->explosionCollider.elements[0].dim.worldSphere.radius >= 150) {
            self->explosionCollider.elements[0].dim.modelSphere.radius = 0;
            self->explosionCollider.elements[0].dim.worldSphere.radius = 0;
            self->explosionRadiusIncrease = false;
        }
    }

    if (self->skelAnime.curFrame == 30.0f) {
        self->explosionRadiusIncrease = true;
    }

    if (self->skelAnime.curFrame == 40.0f) {
        self->explosionRadiusIncrease = true;
        if (self->leftHammerInfused) {
            EnHammergeist_DefuseLeftHammer(self, play);
        }
        if (self->rightHammerInfused) {
            EnHammergeist_DefuseRightHammer(self, play);
        }
        EffectSsBomb2_SpawnLayered(play, &effPos, &effVel, &effAcc, 100, 30);
        Actor_PlaySfx(&self->actor, NA_SE_IT_BOMB_EXPLOSION);
        Actor_RequestQuake(play, 11, 8); // OoT Camera_RequestQuake(mainCam, 2, 11, 8)
    }

    // Molmauk is attackable
    if (self->skelAnime.curFrame >= 41.0f) {
        EnHammergeist_CheckDamage(self, play);
    }
}

void EnHammergeist_SetupInfuse(EnHammergeist* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->genericAnimationTimer = 10;
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_INFUSE);
    EnHammergeist_SetupAction(self, EnHammergeist_Infuse);
}

void EnHammergeist_Infuse(EnHammergeist* self, PlayState* play) {
    s32 i;
    Vec3s newIcePos = self->hammerLeftCollider.dim.pos;
    newIcePos.y += 70; // ice fragments needed a better offset

    if (self->skelAnime.curFrame == 9.0f) {
        EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_MOUTH_OPEN);
    }

    if (self->skelAnime.curFrame == 15.0f) {
        self->rightHammerInfused = true;
        for (i = 0; i <= 7; i++) { // Big flame appears
            EffectSsEnFire_SpawnVec3s(play, &self->actor, &self->hammerRightCollider.dim.pos, 400, 0, 0, -1);
        }
        EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_NORMAL);
    }

    if (self->skelAnime.curFrame == 31.0f) {
        EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_MOUTH_OPEN);
    }

    if (self->skelAnime.curFrame == 37.0f) {
        self->leftHammerInfused = true;
        for (i = 0; i <= 7; i++) { // Ice fragments appear
            EnHammergeist_SpawnIceChunk(play, &newIcePos);
        }
        EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_LAUGH);
    }

    if (SkelAnime_Update(&self->skelAnime)) {
        EnHammergeist_SetupDoNothing(self, play);
    }
}

void EnHammergeist_SetupHeavySlam(EnHammergeist* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->genericAnimationTimer = 10;
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_SLAM_HEAVY);
    EnHammergeist_SetupAction(self, EnHammergeist_HeavySlam);
}

void EnHammergeist_HeavySlam(EnHammergeist* self, PlayState* play) {
    if (SkelAnime_Update(&self->skelAnime)) {
        self->heavySlamCooldown = 600; // Heavy slam cooldown
        if (DECR(self->genericAnimationTimer) == 0) {
            EnHammergeist_SetupFlex(self, play);
        }
    }

    // Frame window right before the hit where the heavy slam can be prevented
    if (self->skelAnime.curFrame >= 38.0f && self->skelAnime.curFrame <= 45.0f) {
        EnHammergeist_CheckDamage(self, play);
    }

    if (self->skelAnime.curFrame == 50.0f) {
        s32 i;

        if (self->leftHammerInfused) {
            EnHammergeist_DefuseLeftHammer(self, play);
        }
        if (self->rightHammerInfused) {
            EnHammergeist_DefuseRightHammer(self, play);
        }

        Audio_PlaySfx_AtPos(&GET_PLAYER(play)->actor.projectedPos, NA_SE_EV_WALL_BROKEN);
        for (i = 0; i < 10; i++) { // it just needed to be more powerful!
            Actor_SpawnFloorDustRing(play, &self->actor, &self->actor.world.pos, i * 100.0f, 4, 4.0f, i * 500, i * 110,
                                     true);
        }
        if (self->actor.xzDistToPlayer < 800.0f) { // The energy caused by the ground hit makes Link fly away
            // knockback WITH damage (0x30) — MM's func_800B8D50 takes the damage as the last arg
            func_800B8D50(play, &self->actor, 20.0f, GET_PLAYER(play)->actor.world.rot.y + 0x8000, 10.0f, 0x30);
        }
    }
}

void EnHammergeist_SetupSlamL(EnHammergeist* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->actor.world.rot.y = self->actor.yawTowardsPlayer;
    self->actor.shape.rot.y = self->actor.world.rot.y;
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_SLAM_L);
    EnHammergeist_SetupAction(self, EnHammergeist_SlamL);
}

void EnHammergeist_SlamL(EnHammergeist* self, PlayState* play) {
    if (SkelAnime_Update(&self->skelAnime)) {
        EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_NORMAL);
        EnHammergeist_SetupDoNothing(self, play);
    }

    if (self->skelAnime.curFrame >= 20.0f) {
        EnHammergeist_CheckDamage(self, play);
    }

    // Sound of the hammer when hitting the floor
    if (self->skelAnime.curFrame == 15.0f) {
        Actor_PlaySfx(&self->actor, NA_SE_IT_HAMMER_HIT);
    }

    if (self->leftHammerInfused && self->skelAnime.curFrame == 17.0f) {
        EnHammergeist_DefuseLeftHammer(self, play);
    }

    if (self->hammerLeftCollider.base.atFlags & AT_HIT) {
        self->hammerLeftCollider.base.atFlags &= ~AT_HIT;
        if (self->leftHammerInfused) {
            EnHammergeist_DefuseLeftHammer(self, play);
        } else {
            func_800B8D98(play, &self->actor, 0.0f, self->actor.shape.rot.y, 0.0f);
        }
        Player_PlaySfx(GET_PLAYER(play), NA_SE_PL_BODY_HIT);
    }

    // The frame window where the left hammer causes damage
    if (self->skelAnime.curFrame >= 10.0f && self->skelAnime.curFrame <= 18.0f) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &self->hammerLeftCollider.base);
    }
}

void EnHammergeist_SetupSlamR(EnHammergeist* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->actor.world.rot.y = self->actor.yawTowardsPlayer;
    self->actor.shape.rot.y = self->actor.world.rot.y;
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_SLAM_R);
    EnHammergeist_SetupAction(self, EnHammergeist_SlamR);
}

void EnHammergeist_SlamR(EnHammergeist* self, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 i;

    if (SkelAnime_Update(&self->skelAnime)) {
        EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_NORMAL);
        EnHammergeist_SetupDoNothing(self, play);
    }

    if (self->skelAnime.curFrame >= 20.0f) {
        EnHammergeist_CheckDamage(self, play);
    }

    // Sound of the hammer when hitting the floor
    if (self->skelAnime.curFrame == 15.0f) {
        Actor_PlaySfx(&self->actor, NA_SE_IT_HAMMER_HIT);
    }

    if (self->rightHammerInfused && self->skelAnime.curFrame == 17.0f) {
        EnHammergeist_DefuseRightHammer(self, play);
    }

    if (self->hammerRightCollider.base.atFlags & AT_HIT) {
        self->hammerRightCollider.base.atFlags &= ~AT_HIT;
        if (self->rightHammerInfused) {
            EnHammergeist_DefuseRightHammer(self, play);
            func_800B8D98(play, &self->actor, 0.0f, self->actor.shape.rot.y, 0.0f);
            // MM player burn fields (OoT isBurning/flameTimers)
            if (player->bodyIsBurning == false) {
                for (i = 0; i < PLAYER_BODYPART_MAX; i++) {
                    player->bodyFlameTimers[i] = Rand_S16Offset(0, 200);
                }
                player->bodyIsBurning = true;
            }
        } else {
            func_800B8D98(play, &self->actor, 0.0f, self->actor.shape.rot.y, 0.0f);
        }
        Player_PlaySfx(GET_PLAYER(play), NA_SE_PL_BODY_HIT);
    }

    // The frame window where the right hammer causes damage
    if (self->skelAnime.curFrame >= 10.0f && self->skelAnime.curFrame <= 18.0f) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &self->hammerRightCollider.base);
    }
}

void EnHammergeist_SetupFlex(EnHammergeist* self, PlayState* play) {
    self->actor.speed = 0.0f;
    self->genericAnimationTimer = 10;
    EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_LAUGH);
    // Flexing voice — pitch 0.7 like the OoT original.
    Audio_PlaySfx_AtPosWithFreq(&self->actor.projectedPos, NA_SE_EN_FANTOM_VOICE, 0.7f);
    EnHammergeist_ChangeAnimation(self, HAMMERGEIST_ANIMATION_FLEX);
    EnHammergeist_SetupAction(self, EnHammergeist_Flex);
}

// Molmauk is distracted when flexing and can be attacked
void EnHammergeist_Flex(EnHammergeist* self, PlayState* play) {
    EnHammergeist_CheckDamage(self, play);
    if (SkelAnime_Update(&self->skelAnime)) {
        if (DECR(self->genericAnimationTimer) == 0) {
            EnHammergeist_ChangeFace(self, HAMMERGEIST_FACE_NORMAL);
            EnHammergeist_SetupDoNothing(self, play);
        }
    }
}

// GAMEPLAY_KEEP object: limb DLs resolve standalone by OTR path (no scene object bank needed).
ActorProfile EnHammergeist_Profile = {
    /**/ ACTOR_EN_HAMMERGEIST,
    /**/ ACTORCAT_ENEMY,
    /**/ HAMMERGEIST_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnHammergeist),
    /**/ EnHammergeist_Init,
    /**/ EnHammergeist_Destroy,
    /**/ EnHammergeist_Update,
    /**/ EnHammergeist_Draw,
};
