/*
 * File: z_en_okuta.c
 * Overlay: ovl_En_Okuta
 * Description: Octorok
 */

#include "z_en_okuta.h"

#include "objects/object_okuta/object_okuta.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_UNFRIENDLY)

#define THIS ((EnOkuta*)thisx)

void EnOkuta_Init(Actor* thisx, PlayState* play);
void EnOkuta_Destroy(Actor* thisx, PlayState* play);
void EnOkuta_Update(Actor* thisx, PlayState* play);
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


#if 0
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

// static ColliderCylinderInit sCylinderInit = {
static ColliderCylinderInit D_808708A0 = {
    { COLTYPE_NONE, AT_ON | AT_TYPE_ENEMY, AC_ON | AC_TYPE_PLAYER, OC1_ON | OC1_TYPE_ALL, OC2_TYPE_2, COLSHAPE_CYLINDER, },
    { ELEMTYPE_UNK4, { 0xF7CFFFFF, 0x00, 0x04 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_ON | TOUCH_SFX_HARD, BUMP_ON, OCELEM_ON, },
    { 13, 20, 0, { 0, 0, 0 } },
};

// static ColliderCylinderInit sCylinderInit = {
static ColliderCylinderInit D_808708CC = {
    { COLTYPE_HIT0, AT_ON | AT_TYPE_ENEMY, AC_ON | AC_TYPE_PLAYER, OC1_ON | OC1_TYPE_ALL, OC2_TYPE_1, COLSHAPE_CYLINDER, },
    { ELEMTYPE_UNK1, { 0xF7CFFFFF, 0x00, 0x04 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_ON | TOUCH_SFX_HARD, BUMP_ON, OCELEM_ON, },
    { 20, 40, -30, { 0, 0, 0 } },
};

// sColChkInfoInit
static CollisionCheckInfoInit D_808708F8 = { 4, 15, 60, 100 };

// static DamageTable sDamageTable = {
static DamageTable D_80870900 = {
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

// static InitChainEntry sInitChain[] = {
static InitChainEntry D_80870920[] = {
    ICHAIN_S8(hintId, TATL_HINT_ID_OCTOROK, ICHAIN_CONTINUE),
    ICHAIN_F32(targetArrowOffset, 6500, ICHAIN_STOP),
};

#endif

extern ColliderCylinderInit D_808708A0;
extern ColliderCylinderInit D_808708CC;
extern CollisionCheckInfoInit D_808708F8;
extern DamageTable D_80870900;
extern InitChainEntry D_80870920[];

extern UNK_TYPE D_0600044C;
extern UNK_TYPE D_06003250;
extern UNK_TYPE D_06003958;
extern UNK_TYPE D_06003B24;
extern UNK_TYPE D_06003EE4;
extern UNK_TYPE D_06004204;
extern AnimationHeader D_0600466C;

extern SkeletonHeader D_060033D0;

void EnOkuta_Init(Actor* thisx, PlayState* play2) {
    EnOkuta* this = THIS;
    PlayState* play = play2;
    WaterBox* sp3C;
    f32 sp38;
    s32 sp34;

    Actor_ProcessInitChain(&this->actor, D_80870920);
    this->unk190 = (this->actor.params >> 8) & 0xFF;
    thisx->params &= 0xFF;
    if ((this->actor.params == 0) || (this->actor.params == 1)) {
        SkelAnime_Init(play, &this->skelAnime, &D_060033D0, &D_0600466C, this->jointTable, this->morphTable, 16);
        Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &D_808708CC);
        CollisionCheck_SetInfo(&this->actor.colChkInfo, &D_80870900, &D_808708F8);
        if ((this->unk190 == 0xFF) || (this->unk190 == 0)) {
            this->unk190 = 1;
        }
        this->actor.floorHeight = BgCheck_EntityRaycastFloor5(&play->colCtx, &this->actor.floorPoly, &sp34,
                                                              &this->actor, &this->actor.world.pos);
        if ((!WaterBox_GetSurface1_2(play, &play->colCtx, this->actor.world.pos.x, this->actor.world.pos.z, &sp38,
                                     &sp3C)) ||
            (sp38 <= this->actor.floorHeight)) {
            Actor_Kill(&this->actor);
        } else {
            this->actor.home.pos.y = sp38;
        }
        if (this->actor.params == 1) {
            this->collider.base.colType = 0xC;
            this->collider.base.acFlags |= 4;
        }
        this->actor.targetMode = 5;
        func_8086E4FC(this);
    } else {
        ActorShape_Init(&this->actor.shape, 1100.0f, ActorShadow_DrawCircle, 18.0f);
        this->actor.flags &= ~1;
        this->actor.flags |= 0x10;
        Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &D_808708A0);
        func_800BC154(play, &play->actorCtx, &this->actor, 6);
        this->unk18E = 0x16;
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
    this->unk18C = 0xA;
    this->unk258 = 0.6f;
    this->unk25C = 0.90000004f;
    this->unk18E = 0x50;
    this->collider.base.colType = 3;
    this->unk254 = 1.0f;
    Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 80);
}

void func_8086E0F0(EnOkuta* this, PlayState* play) {
    if (this->unk18C == 10) {
        this->unk18C = 0;
        this->unk254 = 0.0f;
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

// static Color_RGBA8 D_80870928 = {255, 255, 255, 255};
// static Color_RGBA8 D_8087092C = {150, 150, 150, 255};

extern Color_RGBA8 D_80870928;
extern Color_RGBA8 D_8087092C;

void func_8086E214(Vec3f* pos, Vec3f* velocity, s16 scaleStep, PlayState* play) {
    func_800B0DE0(play, pos, velocity, &gZeroVec3f, &D_80870928, &D_8087092C, 400, scaleStep);
}

void func_8086E27C(EnOkuta* this, PlayState* play) {
    EffectSsGSplash_Spawn(play, &this->actor.home.pos, NULL, NULL, 0, 1300);
}

void func_8086E2C0(EnOkuta* this, PlayState* play) {
    f32 temp_fv0;
    Vec3f sp28;

    temp_fv0 = this->actor.world.pos.y - this->actor.home.pos.y;
    if (((play->gameplayFrames % 7) == 0) && (temp_fv0 < 50.0f) && (temp_fv0 >= -20.0f)) {
        sp28.x = this->actor.world.pos.x;
        sp28.y = this->actor.home.pos.y;
        sp28.z = this->actor.world.pos.z;
        EffectSsGRipple_Spawn(play, &sp28, 250, 650, 0);
    }
}

f32 func_8086E378(EnOkuta* this) {
    f32 temp_fa0;
    f32 temp_fv1;

    temp_fv1 = this->actor.world.pos.y + this->actor.playerHeightRel + 60.0f;
    temp_fa0 = this->actor.home.pos.y;
    if (temp_fa0 < temp_fv1) {
        return temp_fa0;
    }
    return temp_fv1;
}

void func_8086E3B8(EnOkuta* this, PlayState* play) {
    Vec3f pos;
    Vec3f velocity;
    f32 sp3C;
    f32 sp38;

    sp3C = Math_SinS(this->actor.shape.rot.y);
    sp38 = Math_CosS(this->actor.shape.rot.y);
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
        func_8086E214(&pos, &velocity, 0x14, play);
    }
    Actor_PlaySfx(&this->actor, 0x387E);
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
        angle = ABS_ALT((s16) (this->actor.yawTowardsPlayer - this->actor.world.rot.y));

        if ((angle < 0x4000) && (play->bButtonAmmoPlusOne == 0)) {
            func_8086E5E8(this, play);
        }
    }
}

void func_8086E5E8(EnOkuta* this, PlayState* play) {
    this->actor.draw = EnOkuta_Draw;
    this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
    this->actor.flags |= 1;
    Animation_PlayOnce(&this->skelAnime, &D_0600466C);
    func_8086E168(this, play);
    this->actionFunc = func_8086E658;
}

void func_8086E658(EnOkuta* this, PlayState* play) {
    f32 temp_fv0;

    if (SkelAnime_Update(&this->skelAnime) != 0) {
        if ((this->actor.xzDistToPlayer < 160.0f) && (this->actor.params == 0)) {
            func_8086E7A8(this);
        } else {
            func_8086E8E8(this);
        }
    } else {
        temp_fv0 = this->skelAnime.curFrame;
        if (temp_fv0 <= 4.0f) {
            Actor_SetScale(&this->actor, temp_fv0 * 0.25f * 0.01f);
        } else if (Animation_OnFrame(&this->skelAnime, 5.0f) != 0) {
            Actor_SetScale(&this->actor, 0.01f);
        }
    }
    if (Animation_OnFrame(&this->skelAnime, 2.0f) != 0) {
        Actor_PlaySfx(&this->actor, 0x38C2);
    }
    if (Animation_OnFrame(&this->skelAnime, 12.0f) != 0) {
        Actor_PlaySfx(&this->actor, 0x38C3);
    }
    if ((Animation_OnFrame(&this->skelAnime, 3.0f) != 0) || (Animation_OnFrame(&this->skelAnime, 15.0f) != 0)) {
        func_8086E27C(this, play);
    }
}

void func_8086E7A8(EnOkuta* this) {
    Animation_PlayOnce(&this->skelAnime, (AnimationHeader* ) &D_06003B24);
    this->actionFunc = func_8086E7E8;
}

void func_8086E7E8(EnOkuta* this, PlayState* play) {
    f32 temp_fv0;

    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 30.0f);
    if (SkelAnime_Update(&this->skelAnime) != 0) {
        Actor_PlaySfx(&this->actor, 0x38C5);
        func_8086E168(this, play);
        func_8086E4FC(this);
    } else {
        temp_fv0 = this->skelAnime.curFrame;
        if (temp_fv0 >= 4.0f) {
            Actor_SetScale(&this->actor, (6.0f - temp_fv0) * 0.5f * 0.01f);
        }
    }
    if (Animation_OnFrame(&this->skelAnime, 2.0f) != 0) {
        Actor_PlaySfx(&this->actor, 0x38C4);
    }
    if (Animation_OnFrame(&this->skelAnime, 4.0f) != 0) {
        func_8086E27C(this, play);
    }
}

void func_8086E8E8(EnOkuta* this) {
    Animation_PlayLoop(&this->skelAnime, (AnimationHeader* ) &D_06003EE4);
    if (this->actionFunc == func_8086EC00) {
        this->unk18E = 8;
    } else {
        this->unk18E = 0;
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
        if (this->unk18E != 0) {
            this->unk18E--;
        }
    }
    
    if (Animation_OnFrame(&this->skelAnime, 0.5f) != 0) {
        Actor_PlaySfx(&this->actor, 0x38C1);
    }

    if ((this->actor.xzDistToPlayer > 560.0f) || ((this->actor.xzDistToPlayer < 160.0f) && (this->actor.params == 0))) {
        func_8086E7A8(this);
        return;
    }
    
    temp_v0 = Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 3, 0xE38, 0x38E);
 
    if ((ABS_ALT(temp_v0) < 0x38E) && ((this->actor.params == 0 && (this->unk18E == 0) && (this->actor.playerHeightRel < 120.0f)) || ((this->actor.params == 1) && ((this->unk18E == 0) || (this->actor.xzDistToPlayer < 150.0f))))) {
        func_8086EAE0(this, play);
    }
}

void func_8086EAE0(EnOkuta* this, PlayState* play) {

    Animation_PlayOnce(&this->skelAnime, (AnimationHeader* ) &D_0600044C);
    if (this->actionFunc != func_8086EC00) {
        if (this->actor.params == 0) {
            this->unk18E = this->unk190;
        } else {
            this->unk18E = ((560.0f - this->actor.xzDistToPlayer) * 0.005f) + 1.0f;
        }
    }
    if (this->actor.params == 0) {
        this->unk260 = this->actor.playerHeightRel + 20.0f;
        this->unk260 = CLAMP_MIN(this->unk260, 10.0f);
        
        if (this->unk260 > 50.0f) {
            func_8086E27C(this, play);
            Actor_PlaySfx(&this->actor, 0x38C2);
        }
    }
    this->actionFunc = func_8086EC00;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Okuta/func_8086EC00.s")

void func_8086EE8C(EnOkuta* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, (AnimationHeader*)&D_06004204, -5.0f);
    Actor_SetColorFilter(&this->actor, 0x4000, 0xFF, 0, 0xB);
    this->collider.base.acFlags &= 0xFFFE;
    Actor_SetScale(&this->actor, 0.01f);
    Actor_PlaySfx(&this->actor, 0x38C6U);
    this->actionFunc = func_8086EF14;
}

void func_8086EF14(EnOkuta* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime) != 0) {
        if (this->actor.colChkInfo.health == 0) {
            func_8086EF90(this);
        } else {
            this->collider.base.acFlags |= 1;
            func_8086E8E8(this);
        }
    }
    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 5.0f);
}

void func_8086EF90(EnOkuta* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, (AnimationHeader*)&D_06003958, -3.0f);
    this->unk18E = 0;
    this->actor.flags &= ~1;
    this->actionFunc = func_8086EFE8;
}

extern Vec3f D_80870930;
extern Color_RGBA8 D_8087093C;
extern Color_RGBA8 D_80870940;

void func_8086EFE8(EnOkuta* this, PlayState* play) {
    Vec3f velocity;
    Vec3f pos;
    s32 i;

    if (SkelAnime_Update(&this->skelAnime)) {
        this->unk18E++;
    }
    Math_ApproachF(&this->actor.world.pos.y, this->actor.home.pos.y, 0.5f, 5.0f);
    if (this->unk18E == 5) {
        pos.x = this->actor.world.pos.x;
        pos.y = this->actor.world.pos.y + 40.0f;
        pos.z = this->actor.world.pos.z;
        velocity.x = 0.0f;
        velocity.y = -0.5f;
        velocity.z = 0.0f;
        func_8086E214(&pos, &velocity, -0x14, play);
        Actor_PlaySfx(&this->actor, 0x38C7);
    }
    if (Animation_OnFrame(&this->skelAnime, 15.0f)) {
        func_8086E27C(this, play);
        Actor_PlaySfx(&this->actor, 0x38C3);
    }
    if (this->unk18E < 3) {
        Actor_SetScale(&this->actor, ((this->unk18E * 0.25f) + 1.0f) * 0.01f);
    } else if (this->unk18E < 6) {
        Actor_SetScale(&this->actor, (1.5f - ((this->unk18E - 2) * 0.2333f)) * 0.01f);
    } else if (this->unk18E < 11) {
        Actor_SetScale(&this->actor, (((this->unk18E - 5) * 0.04f) + 0.8f) * 0.01f);
    } else {
        if (Math_StepToF(&this->actor.scale.x, 0.0f, 0.0005f) != 0) {
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 30, 0x38C5);
            Item_DropCollectibleRandom(play, &this->actor, &this->actor.world.pos, 0xA0);
            for (i = 0; i < 20; i++) {
                velocity.x = (Rand_ZeroOne() - 0.5f) * 7.0f;
                velocity.y = Rand_ZeroOne() * 7.0f;
                velocity.z = (Rand_ZeroOne() - 0.5f) * 7.0f;
                EffectSsDtBubble_SpawnCustomColor(play, &this->actor.world.pos, &velocity, &D_80870930, &D_8087093C, &D_80870940, (s16) Rand_S16Offset(100, 50), 25, 0);
            }
            Actor_Kill(&this->actor);
        }
        this->actor.scale.y = this->actor.scale.z = this->actor.scale.x;
    }
}

void func_8086F2FC(EnOkuta* this, PlayState* play) {

    this->unk18E = 0xA;
    Actor_SetColorFilter(&this->actor, 0x8000, 0x80FF, 0, 0xA);
    this->actor.child = Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, 0x143, this->actor.world.pos.x, this->actor.world.pos.y + (this->skelAnime.jointTable->y * this->actor.scale.y) + (25.0f * this->headScale.y), this->actor.world.pos.z, 0, this->actor.home.rot.y, 0, 3);
    
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
        this->collider.base.acFlags &= 0xFFFE;
        this->unk18E = 3;
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

    if (this->unk18E != 0) {
        this->unk18E--;
    }
    if (this->unk18E == 0) {
        func_8086E0F0(this, play);
        func_8086EE8C(this);
    }
}

void func_8086F4F4(EnOkuta* this) {
    Animation_Change(&this->skelAnime, (AnimationHeader* ) &D_06004204, 1.0f, 0.0f, Animation_GetLastFrame(&D_06004204) - 3.0f, 2, -3.0f);
    this->unk18E = 0x14;
    this->actionFunc = func_8086F57C;
}

void func_8086F57C(EnOkuta* this, PlayState* play) {

    this->unk18E--;
    Math_ScaledStepToS(&this->actor.shape.rot.x, 0, 0x400);

    if (SkelAnime_Update(&this->skelAnime)) {
        Animation_Change(&this->skelAnime, (AnimationHeader* ) &D_06004204, 1.0f, 0.0f, Animation_GetLastFrame(&D_06004204) - 3.0f, 2, -3.0f);
    }
    
    if (this->unk18E < 10) {
        this->actor.shape.rot.y += (s16) (8192.0f * Math_SinF(this->unk18E * 0.15707964f));
    } else {
        this->actor.shape.rot.y += 0x2000;
    }

    if (this->unk18E == 0) {
        func_8086E8E8(this);
    }
}

void func_8086F694(EnOkuta* this, PlayState* play) {
    Vec3f sp54;
    Player* player;
    Vec3s sp48;

    player = GET_PLAYER(play);
    this->unk18E--;
    if (this->unk18E < 0) {
        this->actor.velocity.y -= 0.5f;
        this->actor.world.rot.x = Math_Atan2S_XY(sqrtf((this->actor.velocity.x * this->actor.velocity.x) + (this->actor.velocity.z * this->actor.velocity.z)), this->actor.velocity.y);
    }

    this->actor.home.rot.z += 0x1554;
    if ((this->actor.bgCheckFlags & 8) || (this->actor.bgCheckFlags & 1) || (this->actor.bgCheckFlags & 0x10) || (this->collider.base.atFlags & 2) || (this->collider.base.acFlags & 2) || (this->collider.base.ocFlags1 & 2) || (this->actor.floorHeight == -32000.0f)) {
        if (player->currentShield == 1 && (this->collider.base.atFlags & 2) && (this->collider.base.atFlags & 0x10) && (this->collider.base.atFlags & 4)) {
                    this->collider.base.atFlags &= 0xFFE9;
                    this->collider.base.atFlags |= 8;
                    this->collider.info.toucher.dmgFlags = 0x400000;
                    this->collider.info.toucher.damage = 2;
                    Matrix_MtxFToYXZRot(&player->shieldMf, &sp48, 0);
                    this->actor.world.rot.y = sp48.y + 0x8000;
                    this->unk18E = 22;
        } else {
            sp54.x = this->actor.world.pos.x;
            sp54.y = this->actor.world.pos.y + 11.0f;
            sp54.z = this->actor.world.pos.z;
            EffectSsHahen_SpawnBurst(play, &sp54, 6.0f, 0, 1, 2, 15, 5, 10, (Gfx* ) &D_06003250);
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 0x14, 0x38C0);
            if ((this->collider.base.atFlags & 2) && (this->collider.base.atFlags & 0x10) && !(this->collider.base.atFlags & 4) && (this->actor.params == 0x11)) {
                func_800B8D98(play, &this->actor, 8.0f, this->actor.world.rot.y, 6.0f);
            }
            Actor_Kill(&this->actor);
        }
    } else if (this->unk18E == -300) {
        Actor_Kill(&this->actor);
    }
}

void func_8086F8FC(EnOkuta* this) {
    f32 curFrame;

    curFrame = this->skelAnime.curFrame;
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

    if ((this->collider.base.acFlags & 2) && ((this->collider.base.acFlags = (u8) (this->collider.base.acFlags & 0xFFFD), (this->unk18C != 0xA)) || !(this->collider.info.acHitInfo->toucher.dmgFlags & 0xDB0B3))) {
        Actor_SetDropFlag(&this->actor, &this->collider.info);
        func_8086E0F0(this, play);
        if (this->actor.colChkInfo.damageEffect == 3) {
            func_8086F2FC(this, play);
        } else {
            if (this->actor.colChkInfo.damageEffect == 4) {
                this->unk254 = 4.0f;
                this->unk258 = 0.6f;
                this->unk18C = 0x14;
                Actor_Spawn(&play->actorCtx, play, 0xA2, this->collider.info.bumper.hitPos.x, this->collider.info.bumper.hitPos.y, this->collider.info.bumper.hitPos.z, 0, 0, 0, 4);
            }
            
            if (!Actor_ApplyDamage(&this->actor)) {
                Enemy_StartFinishingBlow(play, &this->actor);
            }
            func_8086EE8C(this);
        }
    }
}

extern s16 D_808708EE;
extern s16 D_808708EC;

void EnOkuta_Update(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnOkuta* this = THIS;
    s32 pad[2];

    if (this->actor.params == 0) {
        func_8086FCA4(this, play);
    } else {
        if ((this->collider.base.atFlags & 2) || (this->collider.base.acFlags & 2)) {
            if (this->collider.base.atFlags & 2) {
                func_800B8D98(play, &this->actor, 8.0f, this->actor.world.rot.y, 6.0f);
            }
            func_8086F4F4(this);
        }
    }
    this->actionFunc(this, play);
    if (this->actionFunc != func_8086F434) {
        func_8086F8FC(this);
        this->collider.dim.height = ((D_808708EE * this->headScale.y) - this->collider.dim.yShift) * this->actor.scale.y * 100.0f;
        Collider_UpdateCylinder(&this->actor, &this->collider);
        if ((this->actionFunc == func_8086E658) || (this->actionFunc == func_8086E7E8)) {
            this->collider.dim.pos.y = this->actor.world.pos.y + (this->skelAnime.jointTable->y * this->actor.scale.y);
            this->collider.dim.radius = D_808708EC * this->actor.scale.x * 100.0f;
        }
        if (this->actor.draw) {
            if (this->actor.params == 1) {
                CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
            }
            if (this->collider.base.acFlags & 1) {
                CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
            }
            CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
            func_8086E2C0(this, play);
        }
        Actor_SetFocus(&this->actor, 15.0f);
        if (this->unk254 > 0.0f) {
            if (this->unk18C != 10) {
                Math_StepToF(&this->unk254, 0.0f, 0.05f);
                this->unk258 = (this->unk254 + 1.0f) * 0.3f;
                this->unk258 = CLAMP_MAX(this->unk258, 0.6f);
            } else if (Math_StepToF(&this->unk25C, 0.6f, 0.015000001f) == 0) {
                Actor_PlaySfx_Flagged(&this->actor, 0x20B2);
            }
        }
    }
}

void func_808700C0(Actor* thisx, PlayState* play) {
    s32 pad;
    EnOkuta* this = THIS;
    Player* player = GET_PLAYER(play);
    Vec3f sp38;
    s32 sp34;

    sp34 = false;
    if (!(player->stateFlags1 & 0x300002C2)) {
        this->actionFunc(this, play);
        Actor_MoveWithoutGravity(&this->actor);
        Math_Vec3f_Copy(&sp38, &this->actor.world.pos);
        Actor_UpdateBgCheckInfo(play, &this->actor, 10.0f, 15.0f, 30.0f, 7);
        if ((this->actor.bgCheckFlags & 8) && SurfaceType_IsIgnoredByProjectiles(&play->colCtx, this->actor.wallPoly, this->actor.wallBgId)) {
            sp34 = true;
            this->actor.bgCheckFlags &= 0xFFF7;
        }
        if ((this->actor.bgCheckFlags & 1) && SurfaceType_IsIgnoredByProjectiles(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId)) {
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



#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Okuta/func_80870254.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Okuta/func_808704DC.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Okuta/func_808705C8.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Okuta/EnOkuta_Draw.s")
