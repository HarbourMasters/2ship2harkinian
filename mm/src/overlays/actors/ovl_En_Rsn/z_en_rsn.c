/*
 * File: z_en_rsn.c
 * Overlay: ovl_En_Rsn
 * Description: Bomb Shop Man in the credits
 */

#include "z_en_rsn.h"
#include "objects/object_rs/object_rs.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_10 | ACTOR_FLAG_2000000)

#define THIS ((EnRsn*)thisx)

void EnRsn_Init(Actor* thisx, PlayState* play);
void EnRsn_Destroy(Actor* thisx, PlayState* play);
void EnRsn_Update(Actor* thisx, PlayState* play);
void EnRsn_Draw(Actor* thisx, PlayState* play);

void func_80C25D84(EnRsn* this, PlayState* play);

ActorInit En_Rsn_InitVars = {
    /**/ ACTOR_EN_RSN,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_RS,
    /**/ sizeof(EnRsn),
    /**/ EnRsn_Init,
    /**/ EnRsn_Destroy,
    /**/ EnRsn_Update,
    /**/ EnRsn_Draw,
};

static AnimationInfo sAnimationInfo[] = { { &gBombShopkeeperSwayAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, 0.0f } };

void func_80C25D40(EnRsn* this) {
    Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, 0);
    this->actionFunc = func_80C25D84;
}

void func_80C25D84(EnRsn* this, PlayState* play) {
}

void EnRsn_Init(Actor* thisx, PlayState* play) {
    EnRsn* this = THIS;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 20.0f);
    SkelAnime_InitFlex(play, &this->skelAnime, &gBombShopkeeperSkel, &gBombShopkeeperWalkAnim, NULL, NULL, 0);
    this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
    func_80C25D40(this);
}

void EnRsn_Destroy(Actor* thisx, PlayState* play) {
    EnRsn* this = THIS;

    SkelAnime_Free(&this->skelAnime, play);
}

void EnRsn_Update(Actor* thisx, PlayState* play) {
    EnRsn* this = THIS;

    this->actionFunc(this, play);
    Actor_MoveWithGravity(&this->actor);
    SkelAnime_Update(&this->skelAnime);
    Actor_TrackPlayer(play, &this->actor, &this->unk1D8, &this->unk1DE, this->actor.focus.pos);
}

s32 EnRsn_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnRsn* this = THIS;

    if (limbIndex == BOMB_SHOPKEEPER_LIMB_RIGHT_HAND) {
        Matrix_RotateXS(this->unk1D8.y, MTXMODE_APPLY);
    }
    return false;
}

void EnRsn_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx) {
    EnRsn* this = THIS;
    Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };

    if (limbIndex == BOMB_SHOPKEEPER_LIMB_RIGHT_HAND) {
        Matrix_MultVec3f(&zeroVec, &this->actor.focus.pos);
    }
}

void EnRsn_Draw(Actor* thisx, PlayState* play) {
    EnRsn* this = THIS;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL37_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(gBombShopkeeperEyeTex));
    SkelAnime_DrawFlexOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
                          EnRsn_OverrideLimbDraw, EnRsn_PostLimbDraw, &this->actor);

    CLOSE_DISPS(play->state.gfxCtx);
}
