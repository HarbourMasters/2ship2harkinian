/*
 * File: z_en_bu.c
 * Overlay: ovl_En_Bu
 * Description: Unused dummied-out enemy
 */

#include "z_en_bu.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE)

#define THIS ((EnBu*)thisx)

void EnBu_Init(Actor* thisx, PlayState* play);
void EnBu_Destroy(Actor* thisx, PlayState* play);
void EnBu_Update(Actor* thisx, PlayState* play);
void EnBu_Draw(Actor* thisx, PlayState* play);

void EnBu_DoNothing(EnBu* this, PlayState* play);

ActorInit En_Bu_InitVars = {
    /**/ ACTOR_EN_BU,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnBu),
    /**/ EnBu_Init,
    /**/ EnBu_Destroy,
    /**/ EnBu_Update,
    /**/ EnBu_Draw,
};

void EnBu_Init(Actor* thisx, PlayState* play) {
    EnBu* this = THIS;

    this->actionFunc = EnBu_DoNothing;
}

void EnBu_Destroy(Actor* thisx, PlayState* play) {
}

void EnBu_DoNothing(EnBu* this, PlayState* play) {
}

void EnBu_Update(Actor* thisx, PlayState* play) {
    EnBu* this = THIS;

    Actor_MoveWithGravity(&this->actor);
    this->actionFunc(this, play);
}

void EnBu_Draw(Actor* thisx, PlayState* play) {
    EnBu* this = THIS;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_RotateZS(this->actor.shape.rot.z, MTXMODE_APPLY);
    Matrix_RotateYS(this->actor.shape.rot.y, MTXMODE_APPLY);
    Matrix_RotateXS(this->actor.shape.rot.x, MTXMODE_APPLY);
    Matrix_Scale(0.01f, 0.01f, 0.01f, MTXMODE_APPLY);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_OPA_DISP++, this->displayListPtr);

    CLOSE_DISPS(play->state.gfxCtx);
}
