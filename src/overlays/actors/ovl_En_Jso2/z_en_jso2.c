/*
 * File: z_en_jso2.c
 * Overlay: ovl_En_Jso2
 * Description: Garo Master
 */

#include "z_en_jso2.h"

#define FLAGS                                                                                            \
    (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_UNFRIENDLY | ACTOR_FLAG_10 | ACTOR_FLAG_20 | ACTOR_FLAG_100000 | \
     ACTOR_FLAG_80000000)

#define THIS ((EnJso2*)thisx)

void EnJso2_Init(Actor* thisx, PlayState* play);
void EnJso2_Destroy(Actor* thisx, PlayState* play);
void EnJso2_Update(Actor* thisx, PlayState* play);
void EnJso2_Draw(Actor* thisx, PlayState* play);

void func_80A778F8(EnJso2* this, PlayState* play);
void func_80A785E4(EnJso2* this, PlayState* play);
void func_80A78868(EnJso2* this, PlayState* play);
void func_80A78ACC(EnJso2* this, PlayState* play);
void func_80A78B70(EnJso2* this, PlayState* play);
void func_80A78C7C(EnJso2* this, PlayState* play);
void func_80A78F04(EnJso2* this, PlayState* play);
void func_80A79038(EnJso2* this, PlayState* play);
void func_80A7919C(EnJso2* this, PlayState* play);
void func_80A79364(EnJso2* this, PlayState* play);
void func_80A794C8(EnJso2* this, PlayState* play);
void func_80A79600(EnJso2* this, PlayState* play);
void func_80A7980C(EnJso2* this, PlayState* play);
void func_80A798C8(EnJso2* this, PlayState* play);
void func_80A79A84(EnJso2* this, PlayState* play);
void func_80A79BA0(EnJso2* this, PlayState* play);
void func_80A7A124(EnJso2* this, PlayState* play);
void func_80A7A2EC(EnJso2* this, PlayState* play);
void func_80A78868(EnJso2* this, PlayState* play);
void func_80A776E0(EnJso2* this, s32 animIndex);
void func_80A79300(EnJso2* this);
void func_80A78C08(EnJso2* this);
void func_80A78F80(EnJso2* this, PlayState* play);
void func_80A790E4(EnJso2* this, PlayState* play);
void func_80A79864(EnJso2* this);
void func_80A79B60(EnJso2* this);
void func_80A7A0D0(EnJso2* this);
void func_80A79450(EnJso2* this);
void func_80A78B04(EnJso2* this);
void func_80A78E8C(EnJso2* this);

#if 0
// static DamageTable sDamageTable = {
static DamageTable D_80A7B4F0 = {
    /* Deku Nut       */ DMG_ENTRY(0, 0x1),
    /* Deku Stick     */ DMG_ENTRY(1, 0xF),
    /* Horse trample  */ DMG_ENTRY(0, 0x0),
    /* Explosives     */ DMG_ENTRY(1, 0xF),
    /* Zora boomerang */ DMG_ENTRY(1, 0xF),
    /* Normal arrow   */ DMG_ENTRY(1, 0xF),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, 0x0),
    /* Hookshot       */ DMG_ENTRY(0, 0x1),
    /* Goron punch    */ DMG_ENTRY(1, 0xF),
    /* Sword          */ DMG_ENTRY(1, 0xF),
    /* Goron pound    */ DMG_ENTRY(1, 0xF),
    /* Fire arrow     */ DMG_ENTRY(2, 0x2),
    /* Ice arrow      */ DMG_ENTRY(2, 0x3),
    /* Light arrow    */ DMG_ENTRY(2, 0x4),
    /* Goron spikes   */ DMG_ENTRY(1, 0xF),
    /* Deku spin      */ DMG_ENTRY(0, 0x1),
    /* Deku bubble    */ DMG_ENTRY(1, 0xF),
    /* Deku launch    */ DMG_ENTRY(2, 0xF),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, 0x1),
    /* Zora barrier   */ DMG_ENTRY(0, 0x5),
    /* Normal shield  */ DMG_ENTRY(0, 0x0),
    /* Light ray      */ DMG_ENTRY(0, 0x0),
    /* Thrown object  */ DMG_ENTRY(1, 0xF),
    /* Zora punch     */ DMG_ENTRY(1, 0xF),
    /* Spin attack    */ DMG_ENTRY(1, 0xF),
    /* Sword beam     */ DMG_ENTRY(0, 0x0),
    /* Normal Roll    */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, 0x0),
    /* Unblockable    */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, 0x0),
    /* Powder Keg     */ DMG_ENTRY(1, 0xF),
};

ActorInit En_Jso2_InitVars = {
    /**/ ACTOR_EN_JSO2,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_JSO,
    /**/ sizeof(EnJso2),
    /**/ EnJso2_Init,
    /**/ EnJso2_Destroy,
    /**/ EnJso2_Update,
    /**/ EnJso2_Draw,
};

// static ColliderCylinderInit sCylinderInit = {
static ColliderCylinderInit D_80A7B608 = {
    { COLTYPE_NONE, AT_ON | AT_TYPE_ENEMY, AC_ON | AC_HARD | AC_TYPE_PLAYER, OC1_ON | OC1_TYPE_ALL, OC2_TYPE_1, COLSHAPE_CYLINDER, },
    { ELEMTYPE_UNK0, { 0xF7CFFFFF, 0x08, 0x04 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_ON | TOUCH_SFX_NORMAL, BUMP_ON, OCELEM_ON, },
    { 22, 70, 0, { 0, 0, 0 } },
};

// static ColliderQuadInit sQuadInit = {
static ColliderQuadInit D_80A7B634 = {
    { COLTYPE_NONE, AT_ON | AT_TYPE_ENEMY, AC_NONE, OC1_NONE, OC2_NONE, COLSHAPE_QUAD, },
    { ELEMTYPE_UNK0, { 0xF7CFFFFF, 0x09, 0x10 }, { 0x00000000, 0x00, 0x00 }, TOUCH_ON | TOUCH_SFX_NORMAL | TOUCH_UNK7, BUMP_NONE, OCELEM_NONE, },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

#endif

extern DamageTable D_80A7B4F0;
extern ColliderCylinderInit D_80A7B608;
extern ColliderQuadInit D_80A7B634;

extern UNK_TYPE D_06002ED8;
extern UNK_TYPE D_060081F4;
extern AnimationHeader* D_80A7B684[];
extern u8 D_80A7B6DC[];

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Jso2/EnJso2_Init.s")

void EnJso2_Destroy(Actor* thisx, PlayState* play) {
    EnJso2* this = THIS;

    Collider_DestroyCylinder(play, &this->unkEF4);
    Collider_DestroyQuad(play, &this->unkF40);
    Collider_DestroyQuad(play, &this->unkFC0);
    Effect_Destroy(play, this->unk380);
    Effect_Destroy(play, this->unk384);
    Audio_SetMainBgmVolume(0x7FU, 0U);
    Audio_RestorePrevBgm();
}

void func_80A776E0(EnJso2* this, s32 animIndex) {
    f32 morphFrames;

    this->unk374 = Animation_GetLastFrame(D_80A7B684[animIndex]);
    this->unk1040 = animIndex;
    morphFrames = 1.0f;

    if (animIndex == 8) {
        morphFrames = 2.0f;
    }
    Animation_Change(&this->skelAnime, D_80A7B684[animIndex], morphFrames, 0.0f, this->unk374, D_80A7B6DC[animIndex],
                     -2.0f);
}

void func_80A77790(EnJso2* this, PlayState* play) {
    if (this->unk1048 != 0) {
        Math_ApproachF(&this->unk1054.x, this->unk1078.x, this->unk294, this->unk298);
        Math_ApproachF(&this->unk1054.y, this->unk1078.y, this->unk294, this->unk298);
        Math_ApproachF(&this->unk1054.z, this->unk1078.z, this->unk294, this->unk298);
        Math_ApproachF(&this->unk1060.x, this->unk1084.x, this->unk294, this->unk298);
        Math_ApproachF(&this->unk1060.y, this->unk1084.y, this->unk294, this->unk298);
        Math_ApproachF(&this->unk1060.z, this->unk1084.z, this->unk294, this->unk298);
        Math_ApproachF(&this->unk104C, this->unk1050, 0.3f, 10.0f);
        Play_SetCameraAtEye(play, this->unk1048, &this->unk1060, &this->unk1054);
        Play_SetCameraFov(play, this->unk1048, this->unk104C);
    }
}

s32 func_80A77880(PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == 5) && (Message_ShouldAdvance(play) != 0)) {
        Message_CloseTextbox(play);
        return 1;
    } else {
        return 0;
    }
}

void func_80A778D8(EnJso2* this) {
    this->unk284 = 0;
    this->unk36C = 2;
    this->actionFunc = func_80A778F8;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Jso2/func_80A778F8.s")

void func_80A78588(EnJso2* this) {
    this->unk36C = 2;
    this->unkEF4.base.acFlags |= 4;
    this->actor.flags &= ~(ACTOR_FLAG_100000);
    func_80A776E0(this, 13);
    this->actionFunc = func_80A785E4;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Jso2/func_80A785E4.s")

void func_80A787FC(EnJso2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    func_80A776E0(this, 8);

    this->unk286 = player->actor.shape.rot.y;
    this->unk288 = 0x258;
    this->unk284 = 3;
    this->unk370 = 0;
    this->unkEF4.base.acFlags |= 4;
    this->actionFunc = func_80A78868;
}

void func_80A78868(EnJso2* this, PlayState* play) {
    f32 sp44 = this->skelAnime.curFrame;
    Player* player = GET_PLAYER(play);
    Vec3f sp34;
    SkelAnime* sp28 = &this->skelAnime;

    Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_MOVING - SFX_FLAG);

    if (sp44 < this->unk374) {
        SkelAnime_Update(&this->skelAnime);
    } else if (this->actor.bgCheckFlags & 1) {
        SkelAnime_Update(&this->skelAnime);
    }

    if (Animation_OnFrame(&this->skelAnime, 6.0f) != 0) {
        this->actor.velocity.y = 10.0f;
        if (!(play->gameplayFrames & 1)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_CRYING);
        }
    }

    if (Animation_OnFrame(sp28, 12.0f) != 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_SKIP);
        this->actor.speed = 0.0f;
        if (Rand_ZeroFloat(1.0f) < 0.5f) {
            this->unk288 = -this->unk288;
        }
    }

    if (this->unk28E == 0) {
        this->actor.speed = 0.0f;
        func_80A78B04(this);
        return;
    }

    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xA, 0xFA0, 0x14);
    Math_ApproachF(&this->actor.speed, 5.0f, 0.3f, 2.0f);

    this->unk286 += this->unk288;

    sp34.x = (Math_SinS(this->unk286) * 200.0f) + player->actor.world.pos.x;
    sp34.y = this->actor.world.pos.y;
    sp34.z = (Math_CosS(this->unk286) * 200.0f) + player->actor.world.pos.z;

    Math_SmoothStepToS(&this->actor.world.rot.y, Math_Vec3f_Yaw(&this->actor.world.pos, &sp34), 0xA, 0xFA0, (s16)0x14);
}

void func_80A78A70(EnJso2* this) {
    func_80A776E0(this, 5);
    this->actor.world.rot.y = this->actor.yawTowardsPlayer;
    Actor_PlaySfx(&this->actor, NA_SE_IT_SHIELD_BOUND);
    this->unkEF4.base.acFlags |= 4;
    this->unk284 = 4;
    this->actionFunc = func_80A78ACC;
}

void func_80A78ACC(EnJso2* this, PlayState* play) {
    f32 temp = this->skelAnime.curFrame;

    if (temp >= this->unk374) {
        func_80A787FC(this, play);
    }
}

void func_80A78B04(EnJso2* this) {
    func_80A776E0(this, 0);
    this->actor.world.rot.y = -this->actor.yawTowardsPlayer;
    this->unkEF4.base.acFlags |= 4;
    this->unk284 = 5;
    this->actionFunc = func_80A78B70;
    this->actor.speed = 10.0f;
    this->actor.velocity.y = 20.0f;
}

void func_80A78B70(EnJso2* this, PlayState* play) {
    this->actor.world.rot.x += 0x1770;
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xA, 0xFA0, (s16)0x14);
    if (!(this->actor.velocity.y > 0.0f) && (this->actor.bgCheckFlags & 1)) {
        this->actor.world.rot.x = 0;
        this->actor.velocity.y = 0.0f;
        this->actor.speed = 0.0f;
        this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
        this->actor.world.rot.y = this->actor.yawTowardsPlayer;
        func_80A78C08(this);
    }
}

void func_80A78C08(EnJso2* this) {
    this->unk28A = 0x28;
    this->unkEF4.base.acFlags |= 4;
    this->actor.speed = 15.0f;
    this->actor.velocity.y = 13.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_ENTRY);
    Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_DASH_2);
    this->unk284 = 6;
    this->actionFunc = func_80A78C7C;
}

void func_80A78C7C(EnJso2* this, PlayState* play) {
    f32 temp_fv1 = this->skelAnime.curFrame;
    s16 temp_v0_2;
    Vec3f sp2C;

    if ((this->unkF40.base.atFlags & AT_BOUNCED) || (this->unkFC0.base.atFlags & 4)) {
        this->unkF40.base.atFlags &= 0xFFF9;
        this->unkFC0.base.atFlags &= 0xFFF9;
        Matrix_RotateYS(this->actor.yawTowardsPlayer, MTXMODE_NEW);
        Matrix_MultVecZ(-10.0f, &sp2C);
        Math_Vec3f_Copy(&this->unkE58, &sp2C);
        this->unk368 = 1;
        this->unk28A = 0;
        AudioSfx_SetChannelIO(&this->actor.projectedPos, 0x39BE, 0);
        func_80A79864(this);
        return;
    }

    if ((this->actor.velocity.y < 0.0f) && (this->actor.bgCheckFlags & 1)) {
        if (Rand_ZeroOne() < ((gRegEditor->data[0x976] * 0.1f) + 0.7f)) {
            this->actor.velocity.y = 13.0f;
        } else {
            AudioSfx_SetChannelIO(&this->actor.projectedPos, 0x39BE, 0);
            this->unk368 = 1;
            this->unk370 = 1;
            this->actor.speed = 0.0f;
            func_80A78E8C(this);
            return;
        }
    }

    if (!(temp_fv1 < this->unk374)) {
        temp_v0_2 = ABS_ALT((s16)(this->actor.yawTowardsPlayer - this->actor.shape.rot.y));

        if (((this->unk28A == 0) || (this->actor.xzDistToPlayer < 120.0f)) || (temp_v0_2 > 0x4300)) {
            AudioSfx_SetChannelIO(&this->actor.projectedPos, 0x39BE, 0);
            Math_ApproachZeroF(&this->actor.speed, 0.3f, 3.0f);
            func_80A790E4(this, play);
        }
    }
}

void func_80A78E8C(EnJso2* this) {
    func_80A776E0(this, 0);
    this->unk28A = 0x14;
    this->actor.speed = 0.0f;
    this->actor.gravity = 0.0f;
    this->actor.velocity.y = 10.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_JUMP);
    this->actor.flags |= ACTOR_FLAG_CANT_LOCK_ON;
    this->unk284 = 0xF;
    this->actionFunc = func_80A78F04;
}

void func_80A78F04(EnJso2* this, PlayState* play) {
    this->actor.shape.rot.y -= 0x1D4C;
    Math_ApproachZeroF(&this->unk378, 0.3f, 0.01f);
    Math_ApproachZeroF(&this->actor.shape.shadowScale, 0.3f, 3.0f);
    if (this->unk28A == 0) {
        func_80A78F80(this, play);
    }
}

void func_80A78F80(EnJso2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Math_Vec3f_Copy(&this->actor.world.pos, &player->actor.world.pos);
    this->actor.world.pos.y += 300.0f + BREG(0x34);
    this->actor.velocity.y = 0.0f;
    this->actor.gravity = BREG(0x35) + -3.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_FALL);
    this->unk284 = 0x10;
    this->actionFunc = func_80A79038;
}

void func_80A79038(EnJso2* this, PlayState* play) {
    Math_ApproachF(&this->unk378, 0.042f, 0.3f, 0.03f);
    Math_ApproachF(&this->actor.shape.shadowScale, 17.0f, 0.4f, 4.0f);
    if (this->actor.bgCheckFlags & 1) {
        this->unk370 = 0;
        this->unk378 = 0.042f;
        this->actor.shape.shadowScale = 17.0f;
        this->actor.flags &= ~(ACTOR_FLAG_CANT_LOCK_ON);
        func_80A790E4(this, play);
    }
}

void func_80A790E4(EnJso2* this, PlayState* play) {
    func_80A776E0(this, 1);
    Actor_SpawnFloorDustRing(play, &this->actor, &this->actor.world.pos, this->actor.shape.shadowScale, 1, 8.0f, 0x1F4,
                             0xA, 1);
    Math_ApproachZeroF(&this->actor.speed, 0.3f, 3.0f);
    this->unk371 = 0;
    Actor_PlaySfx(&this->actor, NA_SE_IT_SWORD_SWING_HARD);
    this->unk284 = 7;
    this->unkEF4.base.acFlags &= ~(AC_HARD);
    this->actionFunc = func_80A7919C;
}

void func_80A7919C(EnJso2* this, PlayState* play) {
    Vec3f sp44;
    f32 sp40;

    sp40 = this->skelAnime.curFrame;
    Math_ApproachZeroF(&this->actor.speed, 0.5f, 5.0f);
    if (!(play->gameplayFrames & 7)) {
        Actor_SpawnFloorDustRing(play, &this->actor, &this->actor.world.pos, this->actor.shape.shadowScale, 1, 8.0f,
                                 (s16)0x1F4, (s16)0xA, (u8)1);
    }
    if ((this->unkF40.base.atFlags & 2) || (this->unkFC0.base.atFlags & AT_HIT) != 0) {
        this->unk371 = 1;
        this->unkF40.base.atFlags &= 0xFFFD;
        this->unkFC0.base.atFlags &= 0xFFFD;
    }
    if ((((u8)this->unkF40.base.atFlags) & 4) || (((u8)this->unkFC0.base.atFlags) & 4)) {
        this->unkF40.base.atFlags &= 0xFFF9;
        this->unkFC0.base.atFlags &= 0xFFF9;
        Matrix_RotateYS(this->actor.yawTowardsPlayer, MTXMODE_NEW);
        Matrix_MultVecZ(-10.0f, &sp44);
        Math_Vec3f_Copy(&this->unkE58, &sp44);
        this->unk368 = 1;
        func_80A79864(this);
    } else if (this->unk374 <= sp40) {
        this->unk368 = 1;
        this->actor.speed = 0.0f;
        func_80A79450(this);
    }
}

void func_80A79300(EnJso2* this) {
    func_80A776E0(this, 0xE);
    this->unk371 = 0;
    Actor_PlaySfx(&this->actor, NA_SE_IT_SWORD_SWING_HARD);
    this->unk368 = 0;
    this->unk28E = 0xF;
    this->unk284 = 8;
    this->actionFunc = func_80A79364;
    this->actor.speed = 12.0f;
}

void func_80A79364(EnJso2* this, PlayState* play) {
    this->actor.shape.rot.y -= 0x1770;
    Actor_SpawnFloorDustRing(play, &this->actor, &this->actor.world.pos, this->actor.shape.shadowScale, 1, 4.0f, 0x12C,
                             5, 1);
    Math_SmoothStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 1, 0xFA0, 0x14);

    if ((this->unk28E == 0) || ((this->unkF40.base.atFlags & 2) != 0) || (this->unkF40.base.atFlags & AT_BOUNCED) ||
        ((this->unkFC0.base.atFlags & AT_HIT) != 0) || (this->unkFC0.base.atFlags & AT_BOUNCED)) {
        this->unkF40.base.atFlags &= ~(AT_BOUNCED | AT_HIT);
        this->unkFC0.base.atFlags &= ~(AT_BOUNCED | AT_HIT);
        func_80A79864(this);
    }
}

void func_80A79450(EnJso2* this) {
    if (this->unk371 != 0) {
        func_80A776E0(this, 2);
        this->unk290 = 20;
    } else {
        func_80A776E0(this, 12);
        this->unk290 = 40;
    }
    this->unkEF4.base.acFlags &= ~(AC_HARD);
    this->unk284 = 9;
    this->actionFunc = func_80A794C8;
}

void func_80A794C8(EnJso2* this, PlayState* play) {
    if (this->unk290 == 0) {
        this->unk28E = Rand_S16Offset(30, 30);
        this->unk371 = 0;
        this->unkEF4.base.acFlags |= 4;
        func_80A787FC(this, play);
    }
}

void func_80A79524(EnJso2* this) {
    Vec3f vec;

    AudioSfx_SetChannelIO(&this->actor.projectedPos, NA_SE_EN_ANSATSUSYA_DASH_2, 0U);
    func_80A776E0(this, 4);

    this->unk290 = 0x1E;
    this->unkEF4.base.acFlags &= ~(AC_HARD);
    this->actor.speed = 0.0f;

    Matrix_RotateYS(this->actor.yawTowardsPlayer, MTXMODE_NEW);
    Matrix_MultVecZ(-10.0f, &vec);
    Math_Vec3f_Copy(&this->unkE58, &vec);

    if (((this->unk2A2 == 0xB) || (this->unk2A2 == 0xA)) && (this->unk2A0 == 0)) {
        this->unk2A0 = 0;
        this->unk2A2 = 0;
    }

    if ((this->unk2A2 != 0xB) && (this->unk2A2 != 0xA)) {
        this->unk290 = 40;
    }

    this->unk284 = 0xA;
    this->actionFunc = func_80A79600;
}

void func_80A79600(EnJso2* this, PlayState* play) {
    if (this->unk2A2 == 0xB) {
        if ((this->unk2A0 != 0) && (this->unk2A0 < 0x3C)) {
            this->unk2A2 = 0xA;
        }
    }

    if ((this->unk290 == 0) && (this->unk2A0 == 0)) {
        if ((this->unk2A2 == 0xB) || (this->unk2A2 == 0xA)) {
            Actor_SpawnIceEffects(play, &this->actor, &this->unk2D4, 0xC, 2, 0.7f, 0.4f);
            this->unk2A0 = 0;
            this->unk2A2 = 0;
        }
        func_80A79864(this);
    }
}

void func_80A796BC(EnJso2* this, PlayState* play) {
    Vec3f vec;

    AudioSfx_SetChannelIO(&this->actor.projectedPos, NA_SE_EN_ANSATSUSYA_DASH_2, 0);
    func_80A776E0(this, 4);

    this->unk371 = 0;
    this->actor.velocity.y = 10.0f;
    this->actor.speed = 0.0f;

    Matrix_RotateYS(this->actor.yawTowardsPlayer, MTXMODE_NEW);
    Matrix_MultVecZ(-20.0f, &vec);
    Math_Vec3f_Copy(&this->unkE58, &vec);

    if (((this->unk2A2 == 0xB) || (this->unk2A2 == 0xA)) && (this->unk2A0 != 0)) {
        Actor_SpawnIceEffects(play, &this->actor, &this->unk2D4, 0xC, 2, 0.7f, 0.4f);
        this->unk2A0 = 0;
        this->unk2A2 = 0;
    }

    CollisionCheck_GreenBlood(play, NULL, &this->actor.focus.pos);
    CollisionCheck_GreenBlood(play, NULL, &this->actor.focus.pos);
    CollisionCheck_GreenBlood(play, NULL, &this->actor.focus.pos);

    Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 0xFF, 0, 8);
    Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_DAMAGE);
    this->unk284 = 0xB;
    this->actionFunc = func_80A7980C;
}

void func_80A7980C(EnJso2* this, PlayState* play) {
    if (!(this->actor.velocity.y > 0.0f) && (this->actor.colorFilterTimer == 0) && (this->actor.bgCheckFlags & 1)) {
        func_80A79300(this);
    }
}

void func_80A79864(EnJso2* this) {
    func_80A776E0(this, 3);
    this->actor.world.rot.y *= -1;
    this->unk370 = 0;
    this->unk284 = 0xC;
    this->actionFunc = func_80A798C8;
    this->actor.speed = 7.0f;
    this->actor.velocity.y = 20.0f;
}

void func_80A798C8(EnJso2* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xA, 0xBB8, 0x14);

    if (this->actor.bgCheckFlags & 1) {
        this->actor.speed = 0.0f;
    }

    if ((this->unk374 <= curFrame) && (this->actor.bgCheckFlags & 1)) {
        this->actor.world.rot.x = 0;
        this->actor.velocity.y = 0.0f;
        this->actor.speed = 0.0f;
        this->actor.world.rot.y = this->actor.shape.rot.y;
        this->unk28E = Rand_S16Offset(10, 10);
        func_80A787FC(this, play);
    }
}

void func_80A7998C(EnJso2* this, PlayState* play) {
    AudioSfx_SetChannelIO(&this->actor.projectedPos, NA_SE_EN_ANSATSUSYA_DASH_2, 0U);
    func_80A776E0(this, 4);

    if (((this->unk2A2 == 0xB) || (this->unk2A2 == 0xA)) && (this->unk2A0 == 0)) {
        this->unk2A2 = 0;
    }

    this->actor.shape.rot.z = 0;
    this->actor.flags |= ACTOR_FLAG_CANT_LOCK_ON;
    this->actor.flags &= ~(ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_UNFRIENDLY);
    this->actor.speed = 0.0f;
    this->unk368 = 1;
    this->unk290 = 0x1E;
    this->unk36C = 2;
    this->actor.world.rot.z = this->actor.shape.rot.z;
    this->actor.shape.rot.x = this->actor.shape.rot.z;
    this->actor.world.rot.x = this->actor.shape.rot.z;

    Enemy_StartFinishingBlow(play, &this->actor);
    Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_DEAD);
    Math_Vec3f_Copy(&this->unkE58, &gZeroVec3f);

    this->unk284 = 0xD;
    this->actionFunc = func_80A79A84;
}

void func_80A79A84(EnJso2* this, PlayState* play) {
    Math_SmoothStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 0xA, 0xFA0, 0x14);

    if ((this->unk2A2 == 0xB) || (this->unk2A2 == 0xA)) {
        if (this->unk2A0 != 0) {
            Actor_SpawnIceEffects(play, &this->actor, &this->unk2D4, 0xC, 2, 0.7f, 0.4f);
            this->unk2A0 = 0;
            this->unk2A2 = 0;
        } else {
            return;
        }
    }

    if (this->unk290 == 0) {
        this->unk1050 = 60.0f;
        this->unk104C = 60.0f;
        if (this->unk29C == 0) {
            func_80A79B60(this);
            return;
        }
        func_80A7A0D0(this);
    }
}

void func_80A79B60(EnJso2* this) {
    this->unk1046 = 0;
    this->unk1044 = 0;
    this->unk1048 = 0;
    this->actor.flags |= ACTOR_FLAG_100000;
    this->unk290 = 0x1E;
    this->unk284 = 0xE;
    this->actionFunc = func_80A79BA0;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Jso2/func_80A79BA0.s")

void func_80A7A0D0(EnJso2* this) {
    this->unk1044 = 0;
    Audio_SetMainBgmVolume(0, 0xA);
    func_80A776E0(this, 19);
    this->unk284 = 0xE;
    this->actionFunc = func_80A7A124;
}

void func_80A7A124(EnJso2* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xA, 0xFA0, 0x14);

    if ((this->unk1040 == 0x13) && (this->unk374 <= curFrame)) {
        this->unk104A = 0;
        func_80A776E0(this, 20);
    }

    if ((this->unk1040 == 0x14) && (this->unk374 <= curFrame)) {
        this->unk104A += 1;
        if (this->unk104A >= 2) {
            this->unk104A = 0;
            func_80A776E0(this, 0x15);
        }
    }

    if ((this->unk1040 == 0x15) && (this->unk374 <= curFrame)) {
        if (this->unk2D0 == NULL) {
            this->unk2D0 = Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, 0x1D9, this->unk2C4.x,
                                              this->unk2C4.y, this->unk2C4.z, 0, 0, 0, 4);
        } else if (this->unk104A >= 0xA) {
            if (this->unk2D0 != NULL) {
                this->unk2D0->world.rot.z = 1;
                this->unk2B4 = (s32)1;
                this->actionFunc = func_80A7A2EC;
                return;
            }
        } else {
            this->unk104A++;
        }
    }

    if (this->unk2D0 != NULL) {
        this->unk2D0->world.pos.x = this->unk2C4.x;
        this->unk2D0->world.pos.y = this->unk2C4.y;
        this->unk2D0->world.pos.z = this->unk2C4.z;
    }
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->unkEF4.base);
}

void func_80A7A2EC(EnJso2* this, PlayState* play) {
    Math_SmoothStepToS(&this->unk366, 0, 1, 0xF, 0x32);
    Math_ApproachZeroF(&this->actor.shape.shadowScale, 0.3f, 3.0f);
    if (this->unk366 < 2) {
        Actor_Kill(&this->actor);
    }
}

void func_80A7A360(EnJso2* this, PlayState* play) {
    s32 var_a3 = false;

    if ((this->unk284 != 0xB) && (this->unk284 != 0xC) && (this->unk284 != 0xD) && (this->unk284 != 0xE) &&
        this->unkEF4.base.acFlags & AT_HIT) {
        this->unkEF4.base.acFlags &= ~(AT_HIT);
        if ((this->actor.colChkInfo.damageEffect == 1) || (this->actor.colChkInfo.damageEffect == 5)) {
            this->actor.world.rot.x = this->actor.shape.rot.x = 0;
            if (((this->unk2A2 != 0xB) && (this->unk2A2 != 0xA)) || (this->unk2A0 == 0)) {
                Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_FREEZE);
                if (this->actor.colChkInfo.damageEffect == 5) {
                    this->unk2A0 = 40;
                    this->unk2A2 = 32;
                }
                Actor_SetColorFilter(&this->actor, 0, 255, 0, 40);
                func_80A79524(this);
            }
        } else {
            switch (this->unk284) {
                case 2:
                case 3:
                case 4:
                    this->actor.speed = 0.0f;
                    func_80A78A70(this);
                    break;
                case 7:
                case 9:
                case 10:
                    switch (this->actor.colChkInfo.damageEffect) {
                        case 15:
                            var_a3 = true;
                            break;
                        case 2:
                            this->unk2A0 = 40;
                            this->unk2A2 = 0;
                            var_a3 = true;
                            break;

                        case 4:
                            if (((this->unk2A2 != 0xB) && (this->unk2A2 != 0xA)) || (this->unk2A0 == 0)) {
                                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, this->actor.focus.pos.x,
                                            this->actor.focus.pos.y, this->actor.focus.pos.z, 0, 0, 0, 4);
                                this->unk2A0 = 20;
                                this->unk2A2 = 20;
                                var_a3 = true;
                            }
                            break;

                        case 3:
                            if (((this->unk2A2 != 0xB) && (this->unk2A2 != 0xA)) || (this->unk2A0 == 0)) {
                                Actor_ApplyDamage(&this->actor);
                                this->unk2A0 = 0x50;
                                this->unk2A2 = 0xB;
                                this->unk2A4 = 0.0f;
                                this->unk2A8 = 1.5f;
                            }

                            if (this->actor.colChkInfo.health <= 0) {
                                func_80A7998C(this, play);
                            } else {
                                func_80A79524(this);
                            }
                            break;

                        default:
                            break;
                    }
                default:
                    break;
            }

            if (var_a3) {
                Actor_ApplyDamage(&this->actor);
                if (this->actor.colChkInfo.health > 0) {
                    func_80A796BC(this, play);
                } else {
                    func_80A7998C(this, play);
                }
            }
        }
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Jso2/EnJso2_Update.s")

s32 func_80A7AA48(s32 arg0, s32 arg1, s32* arg2, s32 arg3, u8 arg4, EnJso2* thisx) {
    if (thisx->unk36C == 2) {
        if ((arg1 == 4) && (thisx->unk284 != 0xE)) {
            *arg2 = 0;
        }
        if (arg1 == 6) {
            *arg2 = 0;
        }
    }
    return 0;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Jso2/func_80A7AA9C.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Jso2/EnJso2_Draw.s")
