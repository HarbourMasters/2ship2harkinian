/*
 * File: z_en_muto.c
 * Overlay: ovl_En_Muto
 * Description: Mutoh, leader of the carpenters
 */

#include "z_en_muto.h"
#include "objects/object_toryo/object_toryo.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY)

#define THIS ((EnMuto*)thisx)

void EnMuto_Init(Actor* thisx, PlayState* play);
void EnMuto_Destroy(Actor* thisx, PlayState* play);
void EnMuto_Update(Actor* thisx, PlayState* play2);
void EnMuto_Draw(Actor* thisx, PlayState* play);

void EnMuto_ChangeAnim(EnMuto* this, s32 animIndex);
void EnMuto_SetHeadRotation(EnMuto* this);
void EnMuto_SetupIdle(EnMuto* this);
void EnMuto_Idle(EnMuto* this, PlayState* play);
void EnMuto_SetupDialogue(EnMuto* this, PlayState* play);
void EnMuto_InDialogue(EnMuto* this, PlayState* play);
s32 EnMuto_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx);

ActorInit En_Muto_InitVars = {
    /**/ ACTOR_EN_MUTO,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_TORYO,
    /**/ sizeof(EnMuto),
    /**/ EnMuto_Init,
    /**/ EnMuto_Destroy,
    /**/ EnMuto_Update,
    /**/ EnMuto_Draw,
};

static u16 sTextIds[] = { 0x2ABD, 0x2ABB, 0x0624, 0x0623, 0x2AC6 };

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        TOUCH_NONE | TOUCH_SFX_NORMAL,
        BUMP_NONE,
        OCELEM_ON,
    },
    { 20, 60, 0, { 0, 0, 0 } },
};

void EnMuto_Init(Actor* thisx, PlayState* play) {
    EnMuto* this = THIS;

    this->actor.colChkInfo.mass = MASS_IMMOVABLE;
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 40.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &object_toryo_Skel_007150, &object_toryo_Anim_000E50, this->jointTable,
                       this->morphTable, 17);

    this->isInMayorsRoom = this->actor.params;
    if (!this->isInMayorsRoom) {
        this->shouldSetHeadRotation = true;
        this->textIdIndex = 2;
        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_60_80)) {
            this->textIdIndex = 3;
        }

        if ((gSaveContext.save.day != 3) || !gSaveContext.save.isNight) {
            Actor_Kill(&this->actor);
        }
    } else {
        this->collider.dim.radius = 30;
        this->collider.dim.height = 60;
        this->collider.dim.yShift = 0;

        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_63_80) || ((gSaveContext.save.day == 3) && gSaveContext.save.isNight)) {
            Actor_Kill(&this->actor);
        }
    }

    this->actor.targetMode = TARGET_MODE_6;
    this->actor.gravity = -3.0f;
    Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &sCylinderInit);
    EnMuto_SetupIdle(this);
}

void EnMuto_Destroy(Actor* thisx, PlayState* play) {
    EnMuto* this = THIS;

    Collider_DestroyCylinder(play, &this->collider);
}

void EnMuto_ChangeAnim(EnMuto* this, s32 animIndex) {
    static AnimationHeader* sAnimations[] = { &object_toryo_Anim_000E50, &object_toryo_Anim_000E50 };
    static u8 sAnimationModes[] = { ANIMMODE_LOOP, ANIMMODE_ONCE };

    this->animIndex = animIndex;
    this->frameIndex = Animation_GetLastFrame(&sAnimations[animIndex]->common);
    Animation_Change(&this->skelAnime, sAnimations[this->animIndex], 1.0f, 0.0f, this->frameIndex,
                     sAnimationModes[this->animIndex], -4.0f);
}

void EnMuto_SetHeadRotation(EnMuto* this) {
    s32 yawRotToTarget = ABS_ALT(BINANG_SUB(this->yawTowardsTarget, this->actor.world.rot.y));

    this->headRotTarget.y = 0;
    if ((this->actor.xzDistToPlayer < 200.0f) && (yawRotToTarget < 0x4E20)) {
        this->headRotTarget.y = BINANG_SUB(this->yawTowardsTarget, this->actor.world.rot.y);
        if (this->headRotTarget.y > 0x2710) {
            this->headRotTarget.y = 0x2710;
        } else if (this->headRotTarget.y < -0x2710) {
            this->headRotTarget.y = -0x2710;
        }
    }
}

void EnMuto_SetupIdle(EnMuto* this) {
    EnMuto_ChangeAnim(this, 0);
    this->isInDialogue = false;
    this->actionFunc = EnMuto_Idle;
}

void EnMuto_Idle(EnMuto* this, PlayState* play) {
    Player* player;
    this->actor.textId = sTextIds[this->textIdIndex];

    if (!this->isInMayorsRoom) {
        player = GET_PLAYER(play);
        if (player->transformation == PLAYER_FORM_DEKU) {
            if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_88_08)) {
                this->actor.textId = 0x62C;
            } else {
                this->actor.textId = 0x62B;
            }
        }
    }

    if (1) {} // Needed to match

    if (!this->isInMayorsRoom && (Player_GetMask(play) == PLAYER_MASK_KAFEIS_MASK)) {
        this->actor.textId = 0x2363;
    }

    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        EnMuto_SetupDialogue(this, play);
        return;
    }

    if (!this->isInMayorsRoom) {
        s16 yawRotToPlayer = ABS_ALT(BINANG_SUB(this->actor.yawTowardsPlayer, this->actor.world.rot.y));

        this->yawTowardsTarget = this->actor.yawTowardsPlayer;
        if (yawRotToPlayer > 0x2890) {
            return;
        }
    } else {
        this->textIdIndex = 0;
        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_60_08)) {
            this->textIdIndex = 1;
        }
        if (Player_GetMask(play) == PLAYER_MASK_COUPLE) {
            this->textIdIndex = 4;
        }

        if (this->cutsceneState == 1) {
            EnMuto_SetupDialogue(this, play);
            return;
        }
    }

    Actor_OfferTalk(&this->actor, play, 80.0f);
}

void EnMuto_SetupDialogue(EnMuto* this, PlayState* play) {
    if (!this->isInMayorsRoom) {
        this->yawTowardsTarget = 0;
    }

    if (this->targetActor != NULL) {
        this->shouldSetHeadRotation = true;
        this->cutsceneState = 1;
        Actor_ChangeFocus(this->targetActor, play, this->targetActor);
    }

    this->isInDialogue = true;
    this->actionFunc = EnMuto_InDialogue;
}

void EnMuto_InDialogue(EnMuto* this, PlayState* play) {
    if (!this->isInMayorsRoom) {
        this->yawTowardsTarget = this->actor.yawTowardsPlayer;
        if ((Message_GetState(&play->msgCtx) == TEXT_STATE_5) && Message_ShouldAdvance(play)) {
            Message_CloseTextbox(play);

            if (this->actor.textId == 0x62C) {
                SET_WEEKEVENTREG(WEEKEVENTREG_88_08);
            }
            if (this->actor.textId == 0x624) {
                SET_WEEKEVENTREG(WEEKEVENTREG_60_80);
            }

            this->textIdIndex = 3;

            EnMuto_SetupIdle(this);
        }
        return;
    }

    if (this->targetActor == &this->actor) {
        this->yawTowardsTarget = this->actor.world.rot.y;
        if (this->skelAnime.playSpeed == 0.0f) {
            this->skelAnime.playSpeed = 1.0f;
        }
    } else {
        f32 frameIndex = this->skelAnime.curFrame;

        this->yawTowardsTarget = Math_Vec3f_Yaw(&this->actor.world.pos, &this->targetActor->world.pos);
        if (this->frameIndex <= frameIndex) {
            this->skelAnime.playSpeed = 0.0f;
        }
    }

    if ((play->msgCtx.currentTextId == 0x2AC6) || (play->msgCtx.currentTextId == 0x2AC7) ||
        (play->msgCtx.currentTextId == 0x2AC8)) {
        this->skelAnime.playSpeed = 0.0f;
        this->yawTowardsTarget = this->actor.yawTowardsPlayer;
        this->skelAnime.curFrame = 30.0f;
    }
    if (play->msgCtx.currentTextId == 0x2ACF) {
        this->skelAnime.playSpeed = 0.0f;
    }

    if (this->cutsceneState == 2) {
        EnMuto_SetupIdle(this);
    }
}

void EnMuto_Update(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnMuto* this = THIS;

    SkelAnime_Update(&this->skelAnime);

    if (this->shouldSetHeadRotation) {
        EnMuto_SetHeadRotation(this);
    }

    if (this->isInMayorsRoom && (gSaveContext.save.day == 3) && gSaveContext.save.isNight) {
        Actor_Kill(&this->actor);
        return;
    }

    this->actionFunc(this, play);

    DECR(this->unusedCounter);

    Actor_SetScale(&this->actor, 0.01f);
    this->actor.shape.rot.y = this->actor.world.rot.y;
    Actor_SetFocus(&this->actor, 60.0f);
    Actor_MoveWithGravity(&this->actor);

    Math_SmoothStepToS(&this->headRot.y, this->headRotTarget.y, 1, 0xBB8, 0);
    Math_SmoothStepToS(&this->headRot.x, this->headRotTarget.x, 1, 0x3E8, 0);
    Math_SmoothStepToS(&this->waistRot.y, this->waistRotTarget.y, 1, 0xBB8, 0);

    Actor_UpdateBgCheckInfo(play, &this->actor, 20.0f, 20.0f, 50.0f,
                            UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4 | UPDBGCHECKINFO_FLAG_8 |
                                UPDBGCHECKINFO_FLAG_10);

    this->actor.uncullZoneForward = 500.0f;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
}

s32 EnMuto_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnMuto* this = THIS;

    if (limbIndex == 1) {
        rot->x += this->waistRot.y;
    }

    if (limbIndex == 15) {
        rot->x += this->headRot.y;
        rot->z += this->headRot.x;
    }

    return false;
}

void EnMuto_Draw(Actor* thisx, PlayState* play) {
    EnMuto* this = THIS;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
                          EnMuto_OverrideLimbDraw, NULL, &this->actor);
}
