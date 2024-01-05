/*
 * File: z_en_talk.c
 * Overlay: ovl_En_Talk
 * Description: Green Target Spot (e.g., Indigo-Go's Poster)
 */

#include "z_en_talk.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY)

#define THIS ((EnTalk*)thisx)

void EnTalk_Init(Actor* thisx, PlayState* play);
void EnTalk_Destroy(Actor* thisx, PlayState* play);
void EnTalk_Update(Actor* thisx, PlayState* play);
void func_80BDE058(EnTalk* this, PlayState* play);
void func_80BDE090(EnTalk* this, PlayState* play);

ActorInit En_Talk_InitVars = {
    /**/ ACTOR_EN_TALK,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnTalk),
    /**/ EnTalk_Init,
    /**/ EnTalk_Destroy,
    /**/ EnTalk_Update,
    /**/ NULL,
};

void EnTalk_Init(Actor* thisx, PlayState* play) {
    EnTalk* this = THIS;
    s8 targetMode = this->actor.home.rot.x - 0x1;

    if ((targetMode >= TARGET_MODE_0) && (targetMode < TARGET_MODE_7)) {
        this->actor.targetMode = targetMode;
    }

    Actor_SetScale(&this->actor, 1.0f);
    this->actionFunc = func_80BDE090;
    this->actor.textId = ENTALK_GET_TEXT_ID(&this->actor);
}

void EnTalk_Destroy(Actor* thisx, PlayState* play) {
}

void func_80BDE058(EnTalk* this, PlayState* play) {
    if (Actor_TextboxIsClosing(&this->actor, play)) {
        this->actionFunc = func_80BDE090;
    }
}

void func_80BDE090(EnTalk* this, PlayState* play) {
    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        this->actionFunc = func_80BDE058;
        return;
    }

    if (((this->actor.xzDistToPlayer < 40.0f) && Player_IsFacingActor(&this->actor, 0x3000, play)) ||
        this->actor.isLockedOn) {
        Actor_OfferTalk(&this->actor, play, 120.0f);
    }
}

void EnTalk_Update(Actor* thisx, PlayState* play) {
    EnTalk* this = THIS;

    this->actionFunc(this, play);
}
