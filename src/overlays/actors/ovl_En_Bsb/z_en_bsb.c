/*
 * File: z_en_bsb.c
 * Overlay: ovl_En_Bsb
 * Description: Captain Keeta
 */

#include "z_en_bsb.h"
#include "z64rumble.h"
#include "z64shrink_window.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_UNFRIENDLY | ACTOR_FLAG_10 | ACTOR_FLAG_20 | ACTOR_FLAG_2000000)

#define THIS ((EnBsb*)thisx)

void EnBsb_Init(Actor* thisx, PlayState* play);
void EnBsb_Destroy(Actor* thisx, PlayState* play);
void EnBsb_Update(Actor* thisx, PlayState* play);
void EnBsb_Draw(Actor* thisx, PlayState* play);

void func_80C0B970(EnBsb* this, PlayState* play);
s32 func_80C0BC30(EnBsb* this);
void func_80C0BE1C(EnBsb* this, PlayState* play);
void func_80C0BF2C(EnBsb* this);
void func_80C0BFE8(EnBsb* this, PlayState* play);
void func_80C0C0F4(EnBsb* this, PlayState* play);
void func_80C0C238(EnBsb* this, PlayState* play);
void func_80C0C364(EnBsb* this, PlayState* play);
void func_80C0C484(EnBsb* this, PlayState* play);
void func_80C0C6A8(EnBsb* this, PlayState* play);
void func_80C0C86C(EnBsb* this);
void func_80C0C8EC(EnBsb* this, PlayState* play);
void func_80C0CB3C(EnBsb* this, PlayState* play);
void func_80C0CD04(EnBsb* this, PlayState* play);
void func_80C0CDE4(EnBsb* this, PlayState* play);
void func_80C0CFDC(EnBsb* this, PlayState* play);
void func_80C0D10C(EnBsb* this, PlayState* play);
void func_80C0D27C(EnBsb* this, PlayState* play);
void func_80C0D384(EnBsb* this, PlayState* play);
void func_80C0D51C(EnBsb* this, PlayState* play);
void func_80C0D9B4(EnBsb* this, PlayState* play);
void func_80C0DB18(EnBsb* this, PlayState* play);
void func_80C0E1C0(EnBsb* this, PlayState* play);
void func_80C0E480(EnBsb* this, PlayState* play);
void func_80C0E4FC(EnBsb* this, PlayState* play);
void func_80C0B290(EnBsb* this, s32 arg0);
void func_80C0B31C(PlayState* play, EnBsb* this, Vec3f* pos);
s32 func_80C0B888(EnBsb* this, PlayState* play);
void func_80C0C32C(EnBsb* this);
void func_80C0C430(EnBsb* this);
void func_80C0F544(EnBsb* this, Vec3f* pos, Vec3f* unk2, Vec3f* unk3, f32 unk4, s32 unk5);

#if 0
// static ColliderJntSphElementInit sJntSphElementsInit[7] = {
static ColliderJntSphElementInit D_80C0F8D4[7] = {
    {
        { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_NONE | TOUCH_SFX_NORMAL, BUMP_ON, OCELEM_ON, },
        { 10, { { 1000, 400, 0 }, 40 }, 100 },
    },
    {
        { ELEMTYPE_UNK0, { 0xF7CFFFFF, 0x04, 0x08 }, { 0x00000000, 0x00, 0x00 }, TOUCH_ON | TOUCH_SFX_NORMAL, BUMP_ON, OCELEM_ON, },
        { 9, { { 0, 700, 200 }, 35 }, 100 },
    },
    {
        { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_NONE | TOUCH_SFX_NORMAL, BUMP_ON, OCELEM_ON, },
        { 6, { { 100, 600, 0 }, 35 }, 100 },
    },
    {
        { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_NONE | TOUCH_SFX_NORMAL, BUMP_ON, OCELEM_NONE, },
        { 3, { { 400, 200, 0 }, 40 }, 100 },
    },
    {
        { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_NONE | TOUCH_SFX_NORMAL, BUMP_ON, OCELEM_ON, },
        { 13, { { 700, -100, 0 }, 35 }, 100 },
    },
    {
        { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_NONE | TOUCH_SFX_NORMAL, BUMP_ON, OCELEM_ON, },
        { 16, { { 200, 300, 0 }, 30 }, 100 },
    },
    {
        { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_NONE | TOUCH_SFX_NORMAL, BUMP_ON, OCELEM_ON, },
        { 19, { { 200, 300, 0 }, 30 }, 100 },
    },
};

// static ColliderJntSphInit sJntSphInit = {
static ColliderJntSphInit D_80C0F9D0 = {
    { COLTYPE_HIT6, AT_ON | AT_TYPE_ENEMY, AC_ON | AC_TYPE_PLAYER, OC1_ON | OC1_TYPE_PLAYER, OC2_TYPE_1, COLSHAPE_JNTSPH, },
    ARRAY_COUNT(sJntSphElementsInit), D_80C0F8D4, // sJntSphElementsInit,
};

// static DamageTable sDamageTable = {
static DamageTable D_80C0F9E0 = {
    /* Deku Nut       */ DMG_ENTRY(0, 0x1),
    /* Deku Stick     */ DMG_ENTRY(1, 0xD),
    /* Horse trample  */ DMG_ENTRY(0, 0x0),
    /* Explosives     */ DMG_ENTRY(1, 0xE),
    /* Zora boomerang */ DMG_ENTRY(0, 0xF),
    /* Normal arrow   */ DMG_ENTRY(0, 0xF),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, 0x0),
    /* Hookshot       */ DMG_ENTRY(0, 0xF),
    /* Goron punch    */ DMG_ENTRY(1, 0xD),
    /* Sword          */ DMG_ENTRY(1, 0xD),
    /* Goron pound    */ DMG_ENTRY(1, 0xD),
    /* Fire arrow     */ DMG_ENTRY(0, 0x2),
    /* Ice arrow      */ DMG_ENTRY(0, 0x3),
    /* Light arrow    */ DMG_ENTRY(0, 0x4),
    /* Goron spikes   */ DMG_ENTRY(0, 0xF),
    /* Deku spin      */ DMG_ENTRY(0, 0xC),
    /* Deku bubble    */ DMG_ENTRY(0, 0xF),
    /* Deku launch    */ DMG_ENTRY(1, 0xE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, 0x1),
    /* Zora barrier   */ DMG_ENTRY(1, 0x5),
    /* Normal shield  */ DMG_ENTRY(0, 0x0),
    /* Light ray      */ DMG_ENTRY(0, 0x0),
    /* Thrown object  */ DMG_ENTRY(0, 0xF),
    /* Zora punch     */ DMG_ENTRY(1, 0xD),
    /* Spin attack    */ DMG_ENTRY(1, 0xD),
    /* Sword beam     */ DMG_ENTRY(0, 0x0),
    /* Normal Roll    */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, 0x0),
    /* Unblockable    */ DMG_ENTRY(0, 0x0),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, 0x0),
    /* Powder Keg     */ DMG_ENTRY(1, 0xD),
};

ActorInit En_Bsb_InitVars = {
    /**/ ACTOR_EN_BSB,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_BSB,
    /**/ sizeof(EnBsb),
    /**/ EnBsb_Init,
    /**/ EnBsb_Destroy,
    /**/ EnBsb_Update,
    /**/ EnBsb_Draw,
};

#endif

extern ColliderJntSphElementInit D_80C0F8D4[7];
extern ColliderJntSphInit D_80C0F9D0;
extern DamageTable D_80C0F9E0;
extern Vec3f D_80C0FAA0;
extern Vec3f D_80C0FAAC;
extern Vec3s D_80C0FAB8;
extern Vec3s D_80C0FAC0;

extern AnimationHeader D_06000C50[];
extern UNK_TYPE D_06004894;

extern AnimationHeader* D_80C0FA20[];
extern u8 D_80C0FA84[];

extern AnimationHeader D_060086BC;
extern SkeletonHeader D_0600C3E0;

void func_80C0B290(EnBsb* this, s32 animIndex) {
    this->unk2D8 = animIndex;
    this->unk2C4 = Animation_GetLastFrame(D_80C0FA20[animIndex]);
    Animation_Change(&this->skelAnime, D_80C0FA20[this->unk2D8], 1.0f, 0.0f, this->unk2C4, D_80C0FA84[this->unk2D8],
                     -2.0f);
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0B31C.s")

void EnBsb_Init(Actor* thisx, PlayState* play) {
    EnBsb* this = THIS;
    s32 csId;
    s32 i;

    Actor_SetScale(&this->actor, 0.04f);
    this->unk2CA = 0xFF;
    this->actor.colChkInfo.mass = 0xFF;

    if (this->actor.params & 0x8000) {
        SkelAnime_Init(play, &this->skelAnime, &D_0600C3E0, (AnimationHeader*)&D_06004894, &this->unk_188,
                       &this->unk_206, 0x15);
        this->unk2B0 = this->actor.params & 0xFF;
        func_80C0E3B8(this);
        return;
    }

    this->actor.colChkInfo.damageTable = &D_80C0F9E0;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 0.0f);
    SkelAnime_Init(play, &this->skelAnime, &D_0600C3E0, &D_060086BC, &this->unk_188, &this->unk_206, 0x15);

    this->unk_2B6 = (this->actor.params >> 7) & 0x1F;
    this->unk_2B8 = this->actor.params & 0x7F;
    this->unk_2BA = this->actor.world.rot.z & 0x7F;
    this->actor.colChkInfo.health = 0x18;
    this->actor.hintId = 0x21;
    this->actor.gravity = -2.0f;

    Collider_InitAndSetJntSph(play, &this->unk_F34, &this->actor, &D_80C0F9D0, &this->unk_F54);

    if (this->unk_2B6 == 0x1F) {
        Actor_Kill(&this->actor);
        return;
    }

    csId = this->actor.csId;
    i = 0;

    // clang-format off
    while (csId != CS_ID_NONE) { this->unk_2CC[i] = csId; csId = CutsceneManager_GetAdditionalCsId(csId); i++; }
    // clang-format on

    this->actor.targetMode = 0xA;

    if (WEEKEVENTREG(0x17) & 4) {
        Actor_Kill(&this->actor);
        return;
    }
    func_80C0BF2C(this);
}

void EnBsb_Destroy(Actor* thisx, PlayState* play) {
    EnBsb* this = THIS;

    if (this->unk2B0 == 0) {
        Audio_RestorePrevBgm();
        Collider_DestroyJntSph(play, &this->unk_F34);
    }
    if (CutsceneManager_GetCurrentCsId() == this->unk_2CC[3]) {
        CutsceneManager_Stop(this->unk_2CC[3]);
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0B888.s")

void func_80C0B970(EnBsb* this, PlayState* play) {
    s16 var_a2;

    if ((Animation_OnFrame(&this->skelAnime, 8.0f) != 0) || (Animation_OnFrame(&this->skelAnime, 14.0f) != 0)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_KTIA_WALK);

        var_a2 = (700.0f - this->actor.xzDistToPlayer) * 0.01f;

        if (var_a2 >= 5) {
            var_a2 = 4;
        }

        if (var_a2 > 0) {
            Actor_RequestQuakeAndRumble(&this->actor, play, var_a2, 2);
        }

        if (Animation_OnFrame(&this->skelAnime, 8.0f) != 0) {
            func_80C0B31C(play, this, &this->unk_0304);
            return;
        }

        func_80C0B31C(play, this, &this->unk_02F8);
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0BA58.s")

s32 func_80C0BC30(EnBsb* this) {
    if ((this->actor.world.pos.z < -2180.0f) && (this->actor.world.pos.z > -2470.0f) &&
        (fabsf(this->actor.world.pos.y - this->actor.home.pos.y) < 30.0f)) {
        Vec3s sp38 = D_80C0FAB8;
        Vec3s sp30 = D_80C0FAC0;

        Math_SmoothStepToS(&this->unk_0316.x, sp38.x, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0316.y, sp38.y, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0316.z, sp38.z, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0310.x, sp30.x, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0310.y, sp30.y, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0310.z, sp30.z, 1, 0x7D0, 0);
        return true;
    }

    if (this->unk_0316.z != 0) {
        Math_SmoothStepToS(&this->unk_0316.x, 0, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0316.y, 0, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0316.z, 0, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0310.x, 0, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0310.y, 0, 1, 0x7D0, 0);
        Math_SmoothStepToS(&this->unk_0310.z, 0, 1, 0x7D0, 0);
    }

    return false;
}

void func_80C0BE1C(EnBsb* this, PlayState* play) {
    if (this->unk_111A != 0) {
        Math_ApproachF(&this->unk_1128.x, this->unk_1140.x, 0.5f, 30.0f);
        Math_ApproachF(&this->unk_1128.y, this->unk_1140.y, 0.5f, 30.0f);
        Math_ApproachF(&this->unk_1128.z, this->unk_1140.z, 0.5f, 30.0f);
        Math_ApproachF(&this->unk_1134.x, this->unk_114C.x, 0.5f, 30.0f);
        Math_ApproachF(&this->unk_1134.y, this->unk_114C.y, 0.5f, 30.0f);
        Math_ApproachF(&this->unk_1134.z, this->unk_114C.z, 0.5f, 30.0f);
        Math_ApproachF(&this->unk_1120, this->unk_1124, 0.5f, 10.0f);

        Play_SetCameraAtEye(play, this->unk_111A, &this->unk_1134, &this->unk_1128);
        Play_SetCameraFov(play, this->unk_111A, this->unk_1120);

        ShrinkWindow_Letterbox_SetSizeTarget(0x1B);
    }
}

void func_80C0BF2C(EnBsb* this) {
    s32 i;

    this->unk_F34.elements->dim.modelSphere.radius = 0x6E;
    this->unk_F34.elements->dim.modelSphere.center.x = 0x12C;
    this->unk_F34.elements->dim.modelSphere.center.y = 0x190;
    this->unk_F34.base.colType = 0xC;

    for (i = 0; i < 7; i++) {
        this->unk_F34.elements[i].info.elemType = 2;
    }

    this->actor.flags |= 0x08000000;
    WEEKEVENTREG(0x55) &= 0xBF;
    this->unk2B4 = 0;
    this->actionFunc = func_80C0BFE8;
}

void func_80C0BFE8(EnBsb* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 temp_v0 = ABS_ALT((s16)(this->actor.yawTowardsPlayer - this->actor.shape.rot.y));

    if ((temp_v0 < 0x4300) && !(this->actor.xzDistToPlayer > 300.0f)) {
        if (player->stateFlags2 & 0x08000000) {
            if (this->unk_0290 == 0) {
                Audio_PlaySfx(NA_SE_SY_TRE_BOX_APPEAR);
                this->unk_0290 = 1;
            }
        } else {
            this->unk_0290 = 0;
        }
        if ((play->msgCtx.ocarinaMode == 3) && (play->msgCtx.lastPlayedSong == 0)) {
            play->msgCtx.ocarinaMode = 4;
            func_800BC154(play, &play->actorCtx, &this->actor, 9U);
            func_80C0C0F4(this, play);
        }
    }
}

void func_80C0C0F4(EnBsb* this, PlayState* play) {
    s32 i;

    this->actor.flags |= 0x02000000;
    this->unk2A4 = 0;
    this->unk_F34.elements->dim.modelSphere.radius = 0x28;
    this->unk_F34.elements->dim.modelSphere.center.x = 0x3E8;
    this->unk_F34.elements->dim.modelSphere.center.y = 0x190;
    this->unk_F34.base.colType = 0xA;

    for (i = 0; i < 7; i++) {
        this->unk_F34.elements[i].info.elemType = 0;
    }

    this->unk2AE = 0;

    Actor_PlaySfx(&this->actor, NA_SE_EN_STALTU_UP);

    func_80C0B290(this, 0);

    if (CutsceneManager_IsNext(this->unk_2CC[0]) == 0) {
        CutsceneManager_Queue(this->unk_2CC[0]);
        this->unk2B4 = 1;
        this->actionFunc = func_80C0C238;
    } else {
        CutsceneManager_StartWithPlayerCs(this->unk_2CC[0], &this->actor);

        this->actor.flags &= 0xF7FFFFFF;

        Flags_SetSwitch(play, this->unk_2B8);

        this->unk2A4 = 1;
        this->unk2B4 = 1;
        this->actionFunc = func_80C0C238;
    }
}

void func_80C0C238(EnBsb* this, PlayState* play) {
    func_80C0BC30(this);

    if (this->unk2A4 == 0) {
        if (CutsceneManager_IsNext(this->unk_2CC[0]) == 0) {
            CutsceneManager_Queue(this->unk_2CC[0]);
            return;
        }
        CutsceneManager_StartWithPlayerCs(this->unk_2CC[0], &this->actor);
        this->actor.flags &= 0xF7FFFFFF;
        Flags_SetSwitch(play, this->unk_2B8);
        this->unk2A4 = 1;
    }

    if (this->unk2A4 == 1) {
        if (!(play->gameplayFrames & 1)) {
            func_80C0B31C(play, this, &this->actor.world.pos);
        }
        Math_ApproachF(&this->actor.shape.shadowScale, 25.0f, 1.0f, 2.5f);
        if (this->unk2C4 <= this->skelAnime.curFrame) {
            func_80C0C32C(this);
        }
    }
}

void func_80C0C32C(EnBsb* this) {
    this->unk2A4 = 0;
    func_80C0B290(this, 1);
    this->actionFunc = func_80C0C364;
}

void func_80C0C364(EnBsb* this, PlayState* play) {
    f32 sp2C;
    s16 sp2A = this->actor.yawTowardsPlayer;
    s32 var_v1;

    this->unk_0288 = SubS_GetPathByIndex(play, this->unk_2B6, 0x1F);

    func_80C0BC30(this);

    if (this->unk_0288 != NULL) {
        sp2A = SubS_GetDistSqAndOrientPath(this->unk_0288, this->unk_028C, &this->actor.world.pos, &sp2C);
    }

    this->actor.world.rot.y -= 0x2EE;

    var_v1 = ABS_ALT((s16)(this->actor.world.rot.y - sp2A));

    func_80C0B970(this, play);

    if (var_v1 < 0x1000) {
        this->unk_2BC = this->actor.world.rot.y;
        func_80C0C430(this);
    }
}

void func_80C0C430(EnBsb* this) {
    this->unk2A4 = 0;
    if (this->unk2D8 != 1) {
        func_80C0B290(this, 1);
    }
    this->unk294 = 0x46;
    this->unk2B4 = 2;
    this->actionFunc = func_80C0C484;
}

void func_80C0C484(EnBsb* this, PlayState* play) {
    f32 sp34;
    s16 var_a1;
    s16 temp_v1;

    func_80C0BC30(this);

    if (func_80C0B888(this, play) != 0) {
        this->actor.flags &= 0xFDFFFFFF;
        func_80C0C86C(this);
        return;
    }

    var_a1 = this->actor.yawTowardsPlayer;

    if (this->unk294 == 1) {
        this->actor.flags &= 0xFDFFFFFF;
    }

    if (this->unk_0288 != NULL) {
        var_a1 = SubS_GetDistSqAndOrientPath(this->unk_0288, this->unk_028C, &this->actor.world.pos, &sp34);
    }

    Math_SmoothStepToS(&this->actor.world.rot.y, var_a1, 2, 0x2EE, 5);

    temp_v1 = ABS_ALT((s16)(this->actor.world.rot.y - var_a1));

    if (temp_v1 < 0x1000) {
        this->unk_2BC = this->actor.world.rot.y;
        this->actor.speed = 3.5f;
        if (this->unk_2AF == 0) {
            this->unk_2AF = 1;
            Audio_PlayBgm_StorePrevBgm(3);
        }
    }

    func_80C0B970(this, play);

    if (sp34 < 25.0f) {
        if (this->unk_0288 != NULL) {
            this->unk_028C = this->unk_028C + 1;
            if (this->unk_028C >= this->unk_0288->count) {
                this->unk_028C = this->unk_028C - 1;
                func_80C0DA58(this);
            }
        }
    }
}

void func_80C0C610(EnBsb* this) {
    this->unk2A4 = 0;
    this->actor.speed = 0.0f;

    func_80C0B290(this, 2);
    Actor_PlaySfx(&this->actor, NA_SE_EN_KITA_DAMAGE);

    this->unk294 = 0;

    if (this->unk2DC != 0) {
        this->unk294 = 40;
        Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_FREEZE);
        Actor_SetColorFilter(&this->actor, 0, 255, 0, 40);
    }

    this->unk2B4 = 3;
    this->actionFunc = func_80C0C6A8;
}

void func_80C0C6A8(EnBsb* this, PlayState* play) {
    f32 sp34 = this->skelAnime.curFrame;

    func_80C0B888(this, play);

    if (((this->unk2DC == 0) ||
         ((this->unk2DC != 0) && (this->unk2D8 == 2) && (this->unk2C4 <= sp34) && (this->unk294 == 0))) &&
        ((this->unk_0324 == 0xB) || (this->unk_0324 == 0xA)) && (this->unk_0322 != 0)) {
        Actor_SpawnIceEffects(play, &this->actor, this->unk_0330, 0x11, 2, this->unk_032C, 0.4f);
        this->unk_0322 = 0;
        this->unk_0324 = 0;
    }

    if ((this->unk2D8 == 2) && (this->unk2C4 <= sp34) && (this->unk294 == 0)) {
        if (this->unk2DC == 0) {
            func_80C0B290(this, 3);
            return;
        }
        func_80C0C86C(this);
        return;
    }

    if (this->unk2D8 == 3) {
        Math_SmoothStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 1, 0x7D0, 0);
        if ((Animation_OnFrame(&this->skelAnime, 7.0f) != 0) || (Animation_OnFrame(&this->skelAnime, 15.0f) != 0)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_KTIA_PAUSE_K);
        }

        if (this->unk2C4 <= sp34) {
            this->unk2A4++;
            if (this->unk2A4 >= 2) {
                func_80C0C430(this);
            }
        }
    }
}

void func_80C0C86C(EnBsb* this) {
    this->unk2A4 = 0;
    this->unk2DC = 1;
    this->actor.speed = 2.0f;
    func_80C0B290(this, 4);
    this->unk294 = Rand_S16Offset(0, 0x1E);
    WEEKEVENTREG(0x55) |= 0x40;
    this->unk2B4 = 4;
    this->actionFunc = func_80C0C8EC;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0C8EC.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0CA28.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0CB3C.s")

void func_80C0CCCC(EnBsb* this) {
    func_80C0B290(this, 0x17);
    this->actionFunc = func_80C0CD04;
}

void func_80C0CD04(EnBsb* this, PlayState* play) {
    f32 var_0 = this->skelAnime.curFrame;

    if ((this->unk2D8 == 0x18) && (var_0 >= this->unk2C4)) {
        this->actor.flags &= 0xF7FFFFFF;
        this->actor.gravity = -2.0f;
        this->unk294 = 0xA;
        func_80C0C86C(this);
    } else if (this->unk2D8 == 0x17) {
        func_80C0B290(this, 0x18);
    }
}

void func_80C0CD90(EnBsb* this) {
    this->unk2A4 = 0;
    this->actor.speed = 0.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_KITA_ATTACK_W);
    func_80C0B290(this, 5);
    this->unk2B4 = 6;
    this->actionFunc = func_80C0CDE4;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0CDE4.s")

void func_80C0CF4C(EnBsb* this) {
    this->actor.speed = 0.0f;
    this->unk2A4 = 0;
    Animation_Change(&this->skelAnime, &D_06000C50[0], -1.0f, this->skelAnime.curFrame - 1.0f, 0.0f, 2, 0.0f);
    this->unk294 = 0xA;
    Actor_PlaySfx(&this->actor, NA_SE_EN_KTIA_PAUSE_K);
    this->unk2B4 = 7;
    this->actionFunc = func_80C0CFDC;
}

void func_80C0CFDC(EnBsb* this, PlayState* play) {
    if (this->unk294 == 0) {
        func_80C0C86C(this);
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0D00C.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0D10C.s")

void func_80C0D214(EnBsb* this) {
    this->unk294 = 0x28;
    this->actor.speed = 0.0f;
    Actor_SetColorFilter(&this->actor, 0, 120, 0, 40);
    Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_FREEZE);
    this->unk2B4 = 0xA;
    this->actionFunc = func_80C0D27C;
}

void func_80C0D27C(EnBsb* this, PlayState* play) {
    if (this->unk_0324 == 11) {
        if ((this->unk_0322 != 0) && (this->unk_0322 < 60)) {
            this->unk_0324 = 10;
        }
    }

    if (((this->unk_0324 == 0xB) || (this->unk_0324 == 0xA)) && (this->unk_0322 != 0)) {
        Actor_SpawnIceEffects(play, &this->actor, &this->unk_0330, 0x11, 2, this->unk_032C, 0.4f);
        this->unk_0322 = 0;
        this->unk_0324 = 0;
    }

    if (this->unk294 == 0) {
        func_80C0C86C(this);
    }
}

void func_80C0D334(EnBsb* this) {
    this->actor.speed = 0.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_KITA_LAUGH_K);
    func_80C0B290(this, 6);
    this->unk2B4 = 0xB;
    this->actionFunc = func_80C0D384;
}

void func_80C0D384(EnBsb* this, PlayState* play) {
    f32 var_v0 = this->skelAnime.curFrame;

    if (this->unk2C4 <= var_v0) {
        func_80C0C86C(this);
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0D3C0.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0D51C.s")

void func_80C0D964(EnBsb* this, PlayState* play) {
    this->unk2A4 = 0;
    this->unk2A8 = 0;
    this->actor.textId = 0x1535;
    Message_StartTextbox(play, this->actor.textId, &this->actor);
    this->actionFunc = func_80C0D9B4;
}

void func_80C0D9B4(EnBsb* this, PlayState* play) {
    if ((Message_GetState(&play->msgCtx) == 5) && (Message_ShouldAdvance(play) != 0)) {
        Message_CloseTextbox(play);
        play->nextEntrance = Entrance_CreateFromSpawn(5);
        gSaveContext.nextCutsceneIndex = 0;
        play->transitionTrigger = 0x14;
        play->transitionType = 2;
        gSaveContext.nextTransitionType = 3;
        this->unk_111A = 0;
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0DA58.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0DB18.s")

void func_80C0E178(EnBsb* this) {
    this->actor.flags |= 0x08000000;
    this->unk2AE = 0;
    this->unk2A4 = 0;
    this->actor.flags &= ~1;
    this->unk2B4 = 0xE;
    this->actionFunc = func_80C0E1C0;
    this->actor.speed = 0.0f;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0E1C0.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0E3B8.s")

void func_80C0E480(EnBsb* this, PlayState* play) {
    if (this->unk2C0 != 0.0f) {
        if (this->unk2C0 <= this->actor.world.pos.y) {
            this->unk2C0 = this->actor.world.pos.y - 40.0f;
        }
        this->unk294 = 2;
        this->unk2CA = 0xFF;
        this->actionFunc = func_80C0E4FC;
        this->unk2C0 = this->actor.home.pos.y + (this->actor.world.pos.y - this->unk2C0);
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0E4FC.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0E618.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0E9CC.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/EnBsb_Update.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0EEA0.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0F078.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0F170.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/EnBsb_Draw.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0F544.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0F640.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Bsb/func_80C0F758.s")
