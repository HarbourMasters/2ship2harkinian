/*
 * File: z_en_okuta.c
 * Overlay: ovl_En_Okuta
 * Description: Octorok
 */

#include "z_en_okuta.h"

#include "objects/object_okuta/object_okuta.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_UNFRIENDLY)

#define THIS ((EnOkuta*)thisx)

void EnOkuta_Init(Actor* thisx, PlayState* play2);
void EnOkuta_Destroy(Actor* thisx, PlayState* play);
void EnOkuta_Update(Actor* thisx, PlayState* play2);
void EnOkuta_Draw(Actor* thisx, PlayState* play);

void func_8086E4FC(EnOkuta* this);
void func_8086E52C(EnOkuta* this, PlayState* play);
void func_8086E5E8(EnOkuta* this, PlayState* play);
void func_8086E658(EnOkuta* this, PlayState* play);
void func_8086E7A8(EnOkuta* this);
void func_8086E7E8(EnOkuta* this, PlayState* play);
void func_8086E8E8(EnOkuta* this);
void func_8086E948(EnOkuta* this, PlayState* play);
void func_8086EAE0(EnOkuta* this, PlayState* play);
void func_8086EC00(EnOkuta* this, PlayState* play);
void func_8086EF14(EnOkuta* this, PlayState* play);
void func_8086EF90(EnOkuta* this);
void func_8086EFE8(EnOkuta* this, PlayState* play);
void func_8086F434(EnOkuta* this, PlayState* play);
void func_8086F4B0(EnOkuta* this, PlayState* play);
void func_8086F57C(EnOkuta* this, PlayState* play);
void func_8086F694(EnOkuta* this, PlayState* play);
void func_808700C0(Actor* thisx, PlayState* play);
s32 func_808704DC(PlayState* play, s32 arg1, Gfx** dList, Vec3f* pos, Vec3s* rot, struct Actor* thisx);
void func_808705C8(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx);

static Gfx D_80870870[2] = { { { 0xFA000000, 0xC89BFF } }, { { 0xDF000000, 0 } } };

ActorInit En_Okuta_InitVars = {
    /**/ ACTOR_EN_OKUTA,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_OKUTA,
    /**/ sizeof(EnOkuta),
    /**/ EnOkuta_Init,
    /**/ EnOkuta_Destroy,
    /**/ EnOkuta_Update,
    /**/ EnOkuta_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK4,
        { 0xF7CFFFFF, 0x00, 0x04 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_HARD,
        BUMP_ON,
        OCELEM_ON,
    },
    { 13, 20, 0, { 0, 0, 0 } },
};

static ColliderCylinderInit sCylinderInit2 = {
    {
        COLTYPE_HIT0,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK1,
        { 0xF7CFFFFF, 0x00, 0x04 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_HARD,
        BUMP_ON,
        OCELEM_ON,
    },
    { 20, 40, -30, { 0, 0, 0 } },
};

static CollisionCheckInfoInit sColChkInfoInit = { 4, 15, 60, 100 };

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, 0x0),
    /* Deku Stick     */ DMG_ENTRY(1, 0x0),
    /* Horse trample  */ DMG_ENTRY(0, 0x0),
    /* Explosives     */ DMG_ENTRY(1, 0x0),
    /* Zora boomerang */ DMG_ENTRY(1, 0x0),
    /* Normal arrow   */ DMG_ENTRY(1, 0x0),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, 0x0),
    /* Hookshot       */ DMG_ENTRY(1, 0x0),
    /* Goron punch    */ DMG_ENTRY(1, 0x0),
    /* Sword          */ DMG_ENTRY(1, 0x0),
    /* Goron pound    */ DMG_ENTRY(0, 0x0),
    /* Fire arrow     */ DMG_ENTRY(1, 0x0),
    /* Ice arrow      */ DMG_ENTRY(2, 0x3),
    /* Light arrow    */ DMG_ENTRY(2, 0x4),
    /* Goron spikes   */ DMG_ENTRY(1, 0x0),
    /* Deku spin      */ DMG_ENTRY(1, 0x0),
    /* Deku bubble    */ DMG_ENTRY(1, 0x0),
    /* Deku launch    */ DMG_ENTRY(2, 0x0),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, 0x0),
    /* Zora barrier   */ DMG_ENTRY(0, 0x0),
    /* Normal shield  */ DMG_ENTRY(0, 0x0),
    /* Light ray      */ DMG_ENTRY(0, 0x0),
    /* Thrown object  */ DMG_ENTRY(1, 0x0),
    /* Zora punch     */ DMG_ENTRY(1, 0x0),
    /* Spin attack    */ DMG_ENTRY(1, 0x0),
    /* Sword beam     */ DMG_ENTRY(0, 0x0),
    /* Normal Roll    */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, 0x0),
    /* Unblockable    */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, 0x0),
    /* Powder Keg     */ DMG_ENTRY(1, 0x0),
};

static InitChainEntry sInitChain[] = {
    ICHAIN_S8(hintId, TATL_HINT_ID_OCTOROK, ICHAIN_CONTINUE),
    ICHAIN_F32(targetArrowOffset, 6500, ICHAIN_STOP),
};

static Color_RGBA8 D_80870928 = { 255, 255, 255, 255 };

static Color_RGBA8 D_8087092C = { 150, 150, 150, 255 };

static Vec3f D_80870930 = { 0.0f, -0.5f, 0.0f };

static Color_RGBA8 D_8087093C = { 255, 255, 255, 255 };

static Color_RGBA8 D_80870940 = { 150, 150, 150, 0 };

static s8 D_80870944[16] = { -1, 0, -1, -1, 1, -1, -1, 2, -1, -1, 3, -1, -1, 4, 6, 5 };

static Vec3f D_80870954[3] = {
    { 1500.0f, 1000.0f, 0.0f },
    { -1500.0f, 1000.0f, 0.0f },
    { 0.0f, 1500.0f, -2000.0f },
};

static s32 D_80870978[] = { 0, 0 };

void EnOkuta_Init(Actor* thisx, PlayState* play2) {
    EnOkuta* this = THIS;
    PlayState* play = play2;
    WaterBox* waterbox;
    f32 sp38;
    s32 sp34;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    this->unk_190 = (this->actor.params >> 8) & 0xFF;
    thisx->params &= 0xFF;

    if ((this->actor.params == 0) || (this->actor.params == 1)) {
        SkelAnime_Init(play, &this->skelAnime, &gOctorokSkel, &gOctorokAppearAnim, this->jointTable, this->morphTable,
                       16);
        Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &sCylinderInit2);
        CollisionCheck_SetInfo(&this->actor.colChkInfo, &sDamageTable, &sColChkInfoInit);

        if ((this->unk_190 == 0xFF) || (this->unk_190 == 0)) {
            this->unk_190 = 1;
        }

        this->actor.floorHeight = BgCheck_EntityRaycastFloor5(&play->colCtx, &this->actor.floorPoly, &sp34,
                                                              &this->actor, &this->actor.world.pos);
        if ((!WaterBox_GetSurface1_2(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z, &sp38,
                                     &waterbox)) ||
            (sp38 <= this->actor.floorHeight)) {
            Actor_Kill(&this->actor);
        } else {
            this->actor.home.pos.y = sp38;
        }

        if (this->actor.params == 1) {
            this->collider.base.colType = 0xC;
            this->collider.base.acFlags |= AC_HARD;
        }

        this->actor.targetMode = 5;
        func_8086E4FC(this);
    } else {
        ActorShape_Init(&this->actor.shape, 1100.0f, ActorShadow_DrawCircle, 18.0f);
        this->actor.flags &= ~1;
        this->actor.flags |= 0x10;
        Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
        func_800BC154(play, &play->actorCtx, &this->actor, 6);
        this->unk_18E = 0x16;
        this->actor.shape.rot.y = 0;
        this->actor.world.rot.x = -this->actor.shape.rot.x;
        this->actor.shape.rot.x = 0;
        this->actionFunc = func_8086F694;
        this->actor.update = func_808700C0;
        this->actor.speed = 10.0f;
    }
}

void EnOkuta_Destroy(Actor* thisx, PlayState* play) {
    EnOkuta* this = THIS;
    Collider_DestroyCylinder(play, &this->collider);
}

void func_8086E084(EnOkuta* this) {
    this->unk_18C = 0xA;
    this->unk_258 = 0.6f;
    this->unk_25C = 0.90000004f;
    this->unk_18E = 0x50;
    this->collider.base.colType = 3;
    this->unk_254 = 1.0f;
    Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 80);
}

void func_8086E0F0(EnOkuta* this, PlayState* play) {
    if (this->unk_18C == 10) {
        this->unk_18C = 0;
        this->unk_254 = 0.0f;
        Actor_SpawnIceEffects(play, &this->actor, this->bodyPartsPos, 0xA, 2, 0.3f, 0.2f);
        this->collider.base.colType = 0;
    }
}

void func_8086E168(EnOkuta* this, PlayState* play) {
    s32 i;

    for (i = 0; i < 10; i++) {
        EffectSsBubble_Spawn(play, &this->actor.world.pos, -10.0f, 10.0f, 30.0f, 0.25f);
    }
}

void func_8086E214(Vec3f* pos, Vec3f* velocity, s16 scaleStep, PlayState* play) {
    func_800B0DE0(play, pos, velocity, &gZeroVec3f, &D_80870928, &D_8087092C, 400, scaleStep);
}

void func_8086E27C(EnOkuta* this, PlayState* play) {
    EffectSsGSplash_Spawn(play, &this->actor.home.pos, NULL, NULL, 0, 1300);
}

void func_8086E2C0(EnOkuta* this, PlayState* play) {
    f32 temp_fv0 = this->actor.world.pos.y - this->actor.home.pos.y;
    Vec3f pos;

    if (((play->gameplayFrames % 7) == 0) && (temp_fv0 < 50.0f) && (temp_fv0 >= -20.0f)) {
        pos.x = this->actor.world.pos.x;
        pos.y = this->actor.home.pos.y;
        pos.z = this->actor.world.pos.z;
        EffectSsGRipple_Spawn(play, &pos, 250, 650, 0);
    }
}

f32 func_8086E378(EnOkuta* this) {
    f32 temp_fv1 = this->actor.world.pos.y + this->actor.playerHeightRel + 60.0f;
    f32 temp_fa0 = this->actor.home.pos.y;

    if (temp_fa0 < temp_fv1) {
        return temp_fa0;
    } else {
        return temp_fv1;
    }
}

void func_8086E3B8(EnOkuta* this, PlayState* play) {
    Vec3f pos;
    Vec3f velocity;
    f32 sp3C = Math_SinS(this->actor.shape.rot.y);
    f32 sp38 = Math_CosS(this->actor.shape.rot.y);

    pos.x = this->actor.world.pos.x + (25.0f * sp3C);
    pos.y = this->actor.world.pos.y - 6.0f;
    pos.z = this->actor.world.pos.z + (25.0f * sp38);

    if (Actor_Spawn(&play->actorCtx, play, 8, pos.x, pos.y, pos.z, this->actor.shape.rot.x, this->actor.shape.rot.y,
                    this->actor.shape.rot.z, this->actor.params + 0x10) != NULL) {
        pos.x = this->actor.world.pos.x + (40.0f * sp3C);
        pos.z = this->actor.world.pos.z + (40.0f * sp38);
        pos.y = this->actor.world.pos.y;
        velocity.x = 1.5f * sp3C;
        velocity.y = 0.0f;
        velocity.z = 1.5f * sp38;
        func_8086E214(&pos, &velocity, 20, play);
    }
    Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_THROW);
}

void func_8086E4FC(EnOkuta* this) {
    this->actor.draw = NULL;
    this->actor.flags &= ~1;
    this->actionFunc = func_8086E52C;
    this->actor.world.pos.y = this->actor.home.pos.y;
}

void func_8086E52C(EnOkuta* this, PlayState* play) {
    s32 angle;

    this->actor.world.pos.y = this->actor.home.pos.y;

    if ((this->actor.xzDistToPlayer < 480.0f) && (this->actor.xzDistToPlayer > 200.0f)) {
        if (this->actor.params == 0) {
            func_8086E5E8(this, play);
            return;
        }

        angle = ABS_ALT((s16)(this->actor.yawTowardsPlayer - this->actor.world.rot.y));

        if ((angle < 0x4000) && (play->bButtonAmmoPlusOne == 0)) {
            func_8086E5E8(this, play);
        }
    }
}

void func_8086E5E8(EnOkuta* this, PlayState* play) {
    this->actor.draw = EnOkuta_Draw;
    this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
    this->actor.flags |= 1;
    Animation_PlayOnce(&this->skelAnime, &gOctorokAppearAnim);
    func_8086E168(this, play);
    this->actionFunc = func_8086E658;
}

void func_8086E658(EnOkuta* this, PlayState* play) {
    s32 pad;

    if (SkelAnime_Update(&this->skelAnime) != 0) {
        if ((this->actor.xzDistToPlayer < 160.0f) && (this->actor.params == 0)) {
            func_8086E7A8(this);
        } else {
            func_8086E8E8(this);
        }
    } else {
        if (this->skelAnime.curFrame <= 4.0f) {
            Actor_SetScale(&this->actor, this->skelAnime.curFrame * 0.25f * 0.01f);
        } else if (Animation_OnFrame(&this->skelAnime, 5.0f) != 0) {
            Actor_SetScale(&this->actor, 0.01f);
        }
    }

    if (Animation_OnFrame(&this->skelAnime, 2.0f) != 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_JUMP);
    }

    if (Animation_OnFrame(&this->skelAnime, 12.0f) != 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_LAND);
    }

    if ((Animation_OnFrame(&this->skelAnime, 3.0f) != 0) || (Animation_OnFrame(&this->skelAnime, 15.0f) != 0)) {
        func_8086E27C(this, play);
    }
}

void func_8086E7A8(EnOkuta* this) {
    Animation_PlayOnce(&this->skelAnime, &gOctorokHideAnim);
    this->actionFunc = func_8086E7E8;
}

void func_8086E7E8(EnOkuta* this, PlayState* play) {
    s32 pad;

    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 30.0f);

    if (SkelAnime_Update(&this->skelAnime) != 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_WATER_MID);
        func_8086E168(this, play);
        func_8086E4FC(this);
    } else {
        if (this->skelAnime.curFrame >= 4.0f) {
            Actor_SetScale(&this->actor, (6.0f - this->skelAnime.curFrame) * 0.5f * 0.01f);
        }
    }

    if (Animation_OnFrame(&this->skelAnime, 2.0f) != 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_SINK);
    }

    if (Animation_OnFrame(&this->skelAnime, 4.0f) != 0) {
        func_8086E27C(this, play);
    }
}

void func_8086E8E8(EnOkuta* this) {
    Animation_PlayLoop(&this->skelAnime, &gOctorokFloatAnim);
    if (this->actionFunc == func_8086EC00) {
        this->unk_18E = 8;
    } else {
        this->unk_18E = 0;
    }
    this->actionFunc = func_8086E948;
}

void func_8086E948(EnOkuta* this, PlayState* play) {
    s16 temp_v0;

    if (this->actor.params == 0) {
        this->actor.world.pos.y = this->actor.home.pos.y;
    } else {
        this->actor.world.pos.y = func_8086E378(this);
    }

    SkelAnime_Update(&this->skelAnime);

    if (Animation_OnFrame(&this->skelAnime, 0.0f) != 0) {
        if (this->unk_18E != 0) {
            this->unk_18E--;
        }
    }

    if (Animation_OnFrame(&this->skelAnime, 0.5f) != 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_WATER_SLW);
    }

    if ((this->actor.xzDistToPlayer > 560.0f) || ((this->actor.xzDistToPlayer < 160.0f) && (this->actor.params == 0))) {
        func_8086E7A8(this);
        return;
    }

    temp_v0 = Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 3, 0xE38, 0x38E);

    if ((ABS_ALT(temp_v0) < 0x38E) &&
        ((this->actor.params == 0 && (this->unk_18E == 0) && (this->actor.playerHeightRel < 120.0f)) ||
         ((this->actor.params == 1) && ((this->unk_18E == 0) || (this->actor.xzDistToPlayer < 150.0f))))) {
        func_8086EAE0(this, play);
    }
}

void func_8086EAE0(EnOkuta* this, PlayState* play) {
    Animation_PlayOnce(&this->skelAnime, &gOctorokShootAnim);

    if (this->actionFunc != func_8086EC00) {
        if (this->actor.params == 0) {
            this->unk_18E = this->unk_190;
        } else {
            this->unk_18E = ((560.0f - this->actor.xzDistToPlayer) * 0.005f) + 1.0f;
        }
    }

    if (this->actor.params == 0) {
        this->unk_260 = this->actor.playerHeightRel + 20.0f;
        this->unk_260 = CLAMP_MIN(this->unk_260, 10.0f);

        if (this->unk_260 > 50.0f) {
            func_8086E27C(this, play);
            Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_JUMP);
        }
    }
    this->actionFunc = func_8086EC00;
}

void func_8086EC00(EnOkuta* this, PlayState* play) {
    f32 curFrame;
    Player* player;
    Vec3f sp34;
    s16 pitchTarget;

    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 3, 0x71C);

    if (SkelAnime_Update(&this->skelAnime) != 0) {
        if (this->unk_18E != 0) {
            this->unk_18E--;
        }
        if (this->unk_18E == 0) {
            if ((this->actor.params != 1) || (this->actor.xzDistToPlayer > 150.f)) {
                func_8086E8E8(this);
            } else {

                func_8086EAE0(this, play);
            }
        } else {
            func_8086EAE0(this, play);
        }
    } else {
        if (this->actor.params == 0) {
            curFrame = this->skelAnime.curFrame;
            if (curFrame < 13.0f) {
                this->actor.world.pos.y = (Math_SinF(0.2617889f * curFrame) * this->unk_260) + this->actor.home.pos.y;
            }
            if ((this->unk_260 > 50.0f) && Animation_OnFrame(&this->skelAnime, 13.0f)) {
                func_8086E27C(this, play);
                Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_LAND);
            }
        } else { // clang-format off
                this->actor.world.pos.y = func_8086E378(this);
                curFrame = this->skelAnime.curFrame; if (curFrame < 13.0f) {
                // clang-format on
                player = GET_PLAYER(play);
                sp34.x = player->actor.world.pos.x;
                sp34.y = player->actor.world.pos.y + 20.0f;
                sp34.z = player->actor.world.pos.z;
                pitchTarget = Actor_WorldPitchTowardPoint(&this->actor, &sp34);
                pitchTarget = CLAMP(pitchTarget, -0x2AAA, 0);
                this->actor.shape.rot.x = Math_SinF(0.2617889f * curFrame) * pitchTarget;
            }
        }

        if (Animation_OnFrame(&this->skelAnime, 6.0f) != 0) {
            func_8086E3B8(this, play);
        }
    }

    if ((this->actor.params == 0) && (this->actor.xzDistToPlayer < 160.0f)) {
        func_8086E7A8(this);
    }
}

void func_8086EE8C(EnOkuta* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, &gOctorokHitAnim, -5.0f);
    Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 255, 0, 11);
    this->collider.base.acFlags &= ~AC_ON;
    Actor_SetScale(&this->actor, 0.01f);
    Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_DEAD1);
    this->actionFunc = func_8086EF14;
}

void func_8086EF14(EnOkuta* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime) != 0) {
        if (this->actor.colChkInfo.health == 0) {
            func_8086EF90(this);
        } else {
            this->collider.base.acFlags |= AC_ON;
            func_8086E8E8(this);
        }
    }
    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 5.0f);
}

void func_8086EF90(EnOkuta* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, &gOctorokDieAnim, -3.0f);
    this->unk_18E = 0;
    this->actor.flags &= ~1;
    this->actionFunc = func_8086EFE8;
}

void func_8086EFE8(EnOkuta* this, PlayState* play) {
    Vec3f velocity;
    Vec3f pos;
    s32 i;

    if (SkelAnime_Update(&this->skelAnime)) {
        this->unk_18E++;
    }

    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 5.0f);

    if (this->unk_18E == 5) {
        pos.x = this->actor.world.pos.x;
        pos.y = this->actor.world.pos.y + 40.0f;
        pos.z = this->actor.world.pos.z;
        velocity.x = 0.0f;
        velocity.y = -0.5f;
        velocity.z = 0.0f;
        func_8086E214(&pos, &velocity, -0x14, play);
        Actor_PlaySfx(&this->actor, NA_SE_EN_OCTAROCK_DEAD2);
    }

    if (Animation_OnFrame(&this->skelAnime, 15.0f)) {
        func_8086E27C(this, play);
        Actor_PlaySfx(&this->actor, NA_SE_EN_DAIOCTA_LAND);
    }

    if (this->unk_18E < 3) {
        Actor_SetScale(&this->actor, ((this->unk_18E * 0.25f) + 1.0f) * 0.01f);
    } else if (this->unk_18E < 6) {
        Actor_SetScale(&this->actor, (1.5f - ((this->unk_18E - 2) * 0.2333f)) * 0.01f);
    } else if (this->unk_18E < 11) {
        Actor_SetScale(&this->actor, (((this->unk_18E - 5) * 0.04f) + 0.8f) * 0.01f);
    } else {
        if (Math_StepToF(&this->actor.scale.x, 0.0f, 0.0005f) != 0) {
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 30, 0x38C5);
            Item_DropCollectibleRandom(play, &this->actor, &this->actor.world.pos, 0xA0);
            for (i = 0; i < 20; i++) {
                velocity.x = (Rand_ZeroOne() - 0.5f) * 7.0f;
                velocity.y = Rand_ZeroOne() * 7.0f;
                velocity.z = (Rand_ZeroOne() - 0.5f) * 7.0f;
                EffectSsDtBubble_SpawnCustomColor(play, &this->actor.world.pos, &velocity, &D_80870930, &D_8087093C,
                                                  &D_80870940, (s16)Rand_S16Offset(100, 50), 25, 0);
            }
            Actor_Kill(&this->actor);
        }
        this->actor.scale.y = this->actor.scale.z = this->actor.scale.x;
    }
}

void func_8086F2FC(EnOkuta* this, PlayState* play) {
    this->unk_18E = 0xA;

    Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_GRAY, 0x80FF, COLORFILTER_BUFFLAG_OPA, 0xA);

    this->actor.child = Actor_SpawnAsChild(
        &play->actorCtx, &this->actor, play, ACTOR_OBJ_ICEBLOCK, this->actor.world.pos.x,
        this->actor.world.pos.y + (this->skelAnime.jointTable->y * this->actor.scale.y) + (25.0f * this->headScale.y),
        this->actor.world.pos.z, 0, this->actor.home.rot.y, 0, 3);

    if (this->actor.child != NULL) {
        this->actor.flags &= ~1;
        this->actor.flags |= 0x10;
        this->actor.child->csId = this->actor.csId;
        this->actionFunc = func_8086F434;
        return;
    }

    func_8086E084(this);

    if (!Actor_ApplyDamage(&this->actor)) {
        Enemy_StartFinishingBlow(play, &this->actor);
        this->collider.base.acFlags &= ~AC_ON;
        this->unk_18E = 3;
    }

    this->actionFunc = func_8086F4B0;
}

void func_8086F434(EnOkuta* this, PlayState* play) {
    this->actor.colorFilterTimer = 0xA;
    if (!this->actor.child || !this->actor.child->update) {
        this->actor.flags |= 1;
        if (Math_StepToF(&this->actor.world.pos.y, this->actor.home.pos.y, 10.0f) != 0) {
            this->actor.flags &= ~0x10;
            func_8086E8E8(this);
        }
    }
}

void func_8086F4B0(EnOkuta* this, PlayState* play) {

    if (this->unk_18E != 0) {
        this->unk_18E--;
    }

    if (this->unk_18E == 0) {
        func_8086E0F0(this, play);
        func_8086EE8C(this);
    }
}

void func_8086F4F4(EnOkuta* this) {
    Animation_Change(&this->skelAnime, &gOctorokHitAnim, 1.0f, 0.0f, Animation_GetLastFrame(&gOctorokHitAnim) - 3.0f, 2,
                     -3.0f);
    this->unk_18E = 20;
    this->actionFunc = func_8086F57C;
}

void func_8086F57C(EnOkuta* this, PlayState* play) {
    this->unk_18E--;

    Math_ScaledStepToS(&this->actor.shape.rot.x, 0, 0x400);

    if (SkelAnime_Update(&this->skelAnime)) {
        Animation_Change(&this->skelAnime, &gOctorokHitAnim, 1.0f, 0.0f,
                         Animation_GetLastFrame(&gOctorokHitAnim) - 3.0f, 2, -3.0f);
    }

    if (this->unk_18E < 10) {
        this->actor.shape.rot.y += (s16)(8192.0f * Math_SinF(this->unk_18E * 0.15707964f));
    } else {
        this->actor.shape.rot.y += 0x2000;
    }

    if (this->unk_18E == 0) {
        func_8086E8E8(this);
    }
}

void func_8086F694(EnOkuta* this, PlayState* play) {
    Vec3f sp54;
    Player* player = GET_PLAYER(play);
    Vec3s sp48;

    this->unk_18E--;

    if (this->unk_18E < 0) {
        this->actor.velocity.y -= 0.5f;
        this->actor.world.rot.x =
            Math_Atan2S_XY(sqrtf(SQ(this->actor.velocity.x) + SQ(this->actor.velocity.z)), this->actor.velocity.y);
    }

    this->actor.home.rot.z += 0x1554;

    if ((this->actor.bgCheckFlags & 8) || (this->actor.bgCheckFlags & 1) || (this->actor.bgCheckFlags & 0x10) ||
        (this->collider.base.atFlags & AT_HIT) || (this->collider.base.acFlags & AC_HIT) ||
        (this->collider.base.ocFlags1 & 2) || (this->actor.floorHeight == -32000.0f)) {
        if (player->currentShield == 1 && (this->collider.base.atFlags & AT_HIT) &&
            (this->collider.base.atFlags & AT_TYPE_ENEMY) && (this->collider.base.atFlags & AT_BOUNCED)) {
            this->collider.base.atFlags &= 0xFFE9;
            this->collider.base.atFlags |= AT_TYPE_PLAYER;
            this->collider.info.toucher.dmgFlags = 0x400000;
            this->collider.info.toucher.damage = 2;
            Matrix_MtxFToYXZRot(&player->shieldMf, &sp48, 0);
            this->actor.world.rot.y = sp48.y + 0x8000;
            this->unk_18E = 22;
        } else {
            sp54.x = this->actor.world.pos.x;
            sp54.y = this->actor.world.pos.y + 11.0f;
            sp54.z = this->actor.world.pos.z;
            EffectSsHahen_SpawnBurst(play, &sp54, 6.0f, 0, 1, 2, 15, 5, 10, gOctorokProjectileDL);
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 0x14, 0x38C0);
            if ((this->collider.base.atFlags & AT_HIT) && (this->collider.base.atFlags & AT_TYPE_ENEMY) &&
                !(this->collider.base.atFlags & AT_BOUNCED) && (this->actor.params == 0x11)) {
                func_800B8D98(play, &this->actor, 8.0f, this->actor.world.rot.y, 6.0f);
            }
            Actor_Kill(&this->actor);
        }
    } else if (this->unk_18E == -300) {
        Actor_Kill(&this->actor);
    }
}

void func_8086F8FC(EnOkuta* this) {
    f32 curFrame = this->skelAnime.curFrame;

    if (this->actionFunc == func_8086E658) {
        if (curFrame < 8.0f) {
            this->headScale.x = this->headScale.y = this->headScale.z = 1.0f;
        } else if (curFrame < 10.0f) {
            this->headScale.x = this->headScale.z = 1.0f;
            this->headScale.y = (((curFrame - 7.0f) * 0.4f) + 1.0f);
        } else if (curFrame < 14.0f) {
            this->headScale.x = this->headScale.z = ((curFrame - 9.0f) * 0.075f) + 1.0f;
            this->headScale.y = 1.8f - ((curFrame - 9.0f) * 0.25f);
        } else {
            this->headScale.x = this->headScale.z = 1.3f - ((curFrame - 13.0f) * 0.05f);
            this->headScale.y = ((curFrame - 13.0f) * 0.0333f) + 0.8f;
        }
    } else if (this->actionFunc == func_8086E7E8) {
        if (curFrame < 3.0f) {
            this->headScale.y = 1.0f;
        } else if (curFrame < 4.0f) {
            this->headScale.y = (curFrame - 2.0f) + 1.0f;
        } else {
            this->headScale.y = 2.0f - ((curFrame - 3.0f) * 0.333f);
        }
        this->headScale.x = this->headScale.z = 1.0f;
    } else if (this->actionFunc == func_8086EC00) {
        if (curFrame < 5.0f) {
            this->headScale.x = this->headScale.y = this->headScale.z = (curFrame * 0.125f) + 1.0f;
        } else if (curFrame < 7.0f) {
            this->headScale.x = this->headScale.y = this->headScale.z = 1.5f - ((curFrame - 4.0f) * 0.35f);
        } else if (curFrame < 17.0f) {
            this->headScale.x = this->headScale.z = ((curFrame - 6.0f) * 0.05f) + 0.8f;
            this->headScale.y = 0.8f;
        } else {
            this->headScale.x = this->headScale.z = 1.3f - ((curFrame - 16.0f) * 0.1f);
            this->headScale.y = ((curFrame - 16.0f) * 0.0666f) + 0.8f;
        }
    } else if (this->actionFunc == func_8086E948) {
        this->headScale.x = this->headScale.z = 1.0f;
        this->headScale.y = (Math_SinF(0.19634955f * curFrame) * 0.2f) + 1.0f;
    } else {
        this->headScale.x = this->headScale.y = this->headScale.z = 1.0f;
    }
}

void func_8086FCA4(EnOkuta* this, PlayState* play) {
    if ((this->collider.base.acFlags & AC_HIT) &&
        ((this->collider.base.acFlags = (this->collider.base.acFlags & ~AC_HIT), (this->unk_18C != 0xA)) ||
         !(this->collider.info.acHitInfo->toucher.dmgFlags & 0xDB0B3))) {
        Actor_SetDropFlag(&this->actor, &this->collider.info);
        func_8086E0F0(this, play);

        if (this->actor.colChkInfo.damageEffect == 3) {
            func_8086F2FC(this, play);
        } else {
            if (this->actor.colChkInfo.damageEffect == 4) {
                this->unk_254 = 4.0f;
                this->unk_258 = 0.6f;
                this->unk_18C = 0x14;
                Actor_Spawn(&play->actorCtx, play, 0xA2, this->collider.info.bumper.hitPos.x,
                            this->collider.info.bumper.hitPos.y, this->collider.info.bumper.hitPos.z, 0, 0, 0, 4);
            }

            if (!Actor_ApplyDamage(&this->actor)) {
                Enemy_StartFinishingBlow(play, &this->actor);
            }
            func_8086EE8C(this);
        }
    }
}

void EnOkuta_Update(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnOkuta* this = THIS;
    s32 pad[2];

    if (this->actor.params == 0) {
        func_8086FCA4(this, play);
    } else {
        if ((this->collider.base.atFlags & AT_HIT) || (this->collider.base.acFlags & AC_HIT)) {
            if (this->collider.base.atFlags & AT_HIT) {
                func_800B8D98(play, &this->actor, 8.0f, this->actor.world.rot.y, 6.0f);
            }
            func_8086F4F4(this);
        }
    }

    this->actionFunc(this, play);

    if (this->actionFunc != func_8086F434) {
        func_8086F8FC(this);
        this->collider.dim.height = ((sCylinderInit2.dim.height * this->headScale.y) - this->collider.dim.yShift) *
                                    this->actor.scale.y * 100.0f;
        Collider_UpdateCylinder(&this->actor, &this->collider);

        if ((this->actionFunc == func_8086E658) || (this->actionFunc == func_8086E7E8)) {
            this->collider.dim.pos.y = this->actor.world.pos.y + (this->skelAnime.jointTable->y * this->actor.scale.y);
            this->collider.dim.radius = sCylinderInit2.dim.radius * this->actor.scale.x * 100.0f;
        }

        if (this->actor.draw) {
            if (this->actor.params == 1) {
                CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
            }
            if (this->collider.base.acFlags & AC_ON) {
                CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
            }
            CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
            func_8086E2C0(this, play);
        }

        Actor_SetFocus(&this->actor, 15.0f);

        if (this->unk_254 > 0.0f) {
            if (this->unk_18C != 10) {
                Math_StepToF(&this->unk_254, 0.0f, 0.05f);
                this->unk_258 = (this->unk_254 + 1.0f) * 0.3f;
                this->unk_258 = CLAMP_MAX(this->unk_258, 0.6f);
            } else if (Math_StepToF(&this->unk_25C, 0.6f, 0.015000001f) == 0) {
                Actor_PlaySfx_Flagged(&this->actor, NA_SE_EV_ICE_FREEZE - SFX_FLAG);
            }
        }
    }
}

void func_808700C0(Actor* thisx, PlayState* play) {
    s32 pad;
    EnOkuta* this = THIS;
    Player* player = GET_PLAYER(play);
    Vec3f sp38;
    s32 sp34 = false;

    if (!(player->stateFlags1 & 0x300002C2)) {
        this->actionFunc(this, play);
        Actor_MoveWithoutGravity(&this->actor);
        Math_Vec3f_Copy(&sp38, &this->actor.world.pos);
        Actor_UpdateBgCheckInfo(play, &this->actor, 10.0f, 15.0f, 30.0f, 7);

        if ((this->actor.bgCheckFlags & 8) &&
            SurfaceType_IsIgnoredByProjectiles(&play->colCtx, this->actor.wallPoly, this->actor.wallBgId)) {
            sp34 = true;
            this->actor.bgCheckFlags &= 0xFFF7;
        }

        if ((this->actor.bgCheckFlags & 1) &&
            SurfaceType_IsIgnoredByProjectiles(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId)) {
            sp34 = true;
            this->actor.bgCheckFlags &= 0xFFFE;
        }

        if (sp34 && !(this->actor.bgCheckFlags & 9)) {
            Math_Vec3f_Copy(&this->actor.world.pos, &sp38);
        }

        Collider_UpdateCylinder(&this->actor, &this->collider);

        this->actor.flags |= 0x01000000;

        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    }
}

s32 func_80870254(EnOkuta* this, f32 curFrame, Vec3f* scale) {
    if (this->actionFunc == func_8086E948) {
        scale->z = scale->y = 1.0f;
        scale->x = (Math_SinF(0.19634955f * curFrame) * 0.4f) + 1.0f;
    } else if (this->actionFunc == func_8086EC00) {
        if (curFrame < 5.0f) {
            scale->z = 1.0f;
            scale->x = scale->y = (curFrame * 0.25f) + 1.0f;
        } else if (curFrame < 7.0f) {
            scale->z = (curFrame - 4.0f) * 0.5f + 1.0f;
            scale->x = scale->y = 2.0f - (curFrame - 4.0f) * 0.5f;
        } else {
            scale->z = 2.0f - ((curFrame - 6.0f) * 0.0769f);
            scale->x = scale->y = 1.0f;
        }
    } else if (this->actionFunc == func_8086EFE8) {
        if ((curFrame >= 35.0f) || (curFrame < 25.0f)) {
            return 0;
        }
        if (curFrame < 27.0f) {
            scale->z = 1.0f;
            scale->x = scale->y = ((curFrame - 24.0f) * 0.5f) + 1.0f;
        } else if (curFrame < 30.0f) {
            scale->z = (curFrame - 26.0f) * 0.333f + 1.0f;
            scale->x = scale->y = 2.0f - (curFrame - 26.0f) * 0.333f;
        } else {
            scale->z = 2.0f - ((curFrame - 29.0f) * 0.2f);
            scale->x = scale->y = 1.0f;
        }
    } else {
        return false;
    }
    return true;
}

s32 func_808704DC(PlayState* play, s32 arg1, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnOkuta* this = THIS;
    f32 curFrame;
    Vec3f scale;
    s32 doScale = false;

    curFrame = this->skelAnime.curFrame;

    if (this->actionFunc == func_8086EFE8) {
        curFrame += this->unk_18E;
    }

    if (arg1 == 0xE) {
        if ((this->headScale.x != 1.0f) || (this->headScale.y != 1.0f) || (this->headScale.z != 1.0f)) {
            Math_Vec3f_Copy(&scale, &this->headScale);
            doScale = true;
        }
    } else if (arg1 == 0xF) {
        doScale = func_80870254(this, curFrame, &scale);
    }

    if (doScale) {
        Matrix_Scale(scale.x, scale.y, scale.z, MTXMODE_APPLY);
    }

    return 0;
}

void func_808705C8(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    EnOkuta* this = THIS;
    s32 bodyPartIndex = D_80870944[limbIndex];
    Vec3f* var_s0;
    Vec3f* var_s1;
    s32 i;

    if (bodyPartIndex != BODYPART_NONE) {
        if (bodyPartIndex == 5) {
            Matrix_MultVecX(1500.0f, &this->bodyPartsPos[bodyPartIndex]);
        } else if (bodyPartIndex == 6) {
            Matrix_MultVecY(2800.0f, &this->bodyPartsPos[bodyPartIndex]);
            bodyPartIndex++;
            for (i = 0; i < ARRAY_COUNT(D_80870954); i++) {
                Matrix_MultVec3f(&D_80870954[i], &this->bodyPartsPos[bodyPartIndex + i]);
            }
        } else {
            Matrix_MultZero(&this->bodyPartsPos[bodyPartIndex]);
        }
    }
}

void EnOkuta_Draw(Actor* thisx, PlayState* play) {
    EnOkuta* this = THIS;
    s32 pad;
    Gfx* gfxPtr;

    OPEN_DISPS(play->state.gfxCtx);

    gfxPtr = POLY_OPA_DISP;

    gSPDisplayList(&gfxPtr[0], gSetupDLs[SETUPDL_25]);

    if (this->actor.params < 0x10) {
        if (this->actor.params == 0) {
            gSPSegment(&gfxPtr[1], 0x08, D_801AEFA0);
        } else {
            gSPSegment(&gfxPtr[1], 0x08, D_80870870);
        }
        POLY_OPA_DISP = &gfxPtr[2];
        SkelAnime_DrawOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, func_808704DC, func_808705C8,
                          &this->actor);
    } else {
        Matrix_Mult(&play->billboardMtxF, MTXMODE_APPLY);
        Matrix_RotateZS(this->actor.home.rot.z, MTXMODE_APPLY);
        gSPMatrix(&gfxPtr[1], Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(&gfxPtr[2], gOctorokProjectileDL);
        POLY_OPA_DISP = &gfxPtr[3];
    }

    CLOSE_DISPS(play->state.gfxCtx);

    Actor_DrawDamageEffects(play, &this->actor, this->bodyPartsPos, 0xA, this->unk_258 * this->actor.scale.y * 100.0f,
                            this->unk_25C, this->unk_254, this->unk_18C);
}
