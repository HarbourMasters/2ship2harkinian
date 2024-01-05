/*
 * File: z_dm_tag.c
 * Overlay: ovl_Dm_Tag
 * Description:
 */

#include "z_dm_tag.h"

#define FLAGS (ACTOR_FLAG_10)

#define THIS ((DmTag*)thisx)

void DmTag_Init(Actor* thisx, PlayState* play);
void DmTag_Destroy(Actor* thisx, PlayState* play);
void DmTag_Update(Actor* thisx, PlayState* play);

void func_80C229AC(DmTag* this, PlayState* play);
void DmTag_DoNothing(DmTag* this, PlayState* play);
void func_80C229FC(DmTag* this, PlayState* play);

ActorInit Dm_Tag_InitVars = {
    /**/ ACTOR_DM_TAG,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(DmTag),
    /**/ DmTag_Init,
    /**/ DmTag_Destroy,
    /**/ DmTag_Update,
    /**/ NULL,
};

MsgScript D_80C22BF0[] = {
    /* 0x0000 0x03 */ MSCRIPT_BEGIN_TEXT(0x28F0),
    /* 0x0003 0x01 */ MSCRIPT_AWAIT_TEXT(),
    /* 0x0004 0x01 */ MSCRIPT_PAUSE(),
    /* 0x0005 0x03 */ MSCRIPT_BRANCH_ON_CALLBACK_OPTIONAL(0x0),
    /* 0x0008 0x01 */ MSCRIPT_DONE(),
};

MsgScript D_80C22BFC[] = {
    /* 0x0000 0x03 */ MSCRIPT_BRANCH_ON_CALLBACK_OPTIONAL(0x0),
    /* 0x0003 0x03 */ MSCRIPT_BEGIN_TEXT(0x28F1),
    /* 0x0006 0x01 */ MSCRIPT_AWAIT_TEXT(),
    /* 0x0007 0x03 */ MSCRIPT_BRANCH_ON_CALLBACK_OPTIONAL(0x0),
    /* 0x000A 0x03 */ MSCRIPT_BEGIN_TEXT(0x28F2),
    /* 0x000D 0x01 */ MSCRIPT_AWAIT_TEXT(),
    /* 0x000E 0x03 */ MSCRIPT_BRANCH_ON_CALLBACK_OPTIONAL(0x0),
    /* 0x0011 0x03 */ MSCRIPT_BEGIN_TEXT(0x28F3),
    /* 0x0014 0x01 */ MSCRIPT_AWAIT_TEXT(),
    /* 0x0015 0x03 */ MSCRIPT_BRANCH_ON_CALLBACK_OPTIONAL(0x0),
    /* 0x0018 0x03 */ MSCRIPT_BEGIN_TEXT(0x28F4),
    /* 0x001B 0x01 */ MSCRIPT_AWAIT_TEXT(),
    /* 0x001C 0x03 */ MSCRIPT_BRANCH_ON_CALLBACK_OPTIONAL(0x0),
    /* 0x001F 0x03 */ MSCRIPT_BEGIN_TEXT(0x28F5),
    /* 0x0022 0x01 */ MSCRIPT_AWAIT_TEXT(),
    /* 0x0023 0x03 */ MSCRIPT_BRANCH_ON_CALLBACK_OPTIONAL(0x0),
    /* 0x0026 0x03 */ MSCRIPT_BEGIN_TEXT(0x28F6),
    /* 0x0029 0x01 */ MSCRIPT_AWAIT_TEXT(),
    /* 0x002A 0x01 */ MSCRIPT_PAUSE(),
    /* 0x002B 0x03 */ MSCRIPT_WEEK_EVENT_REG_SET(0x55, 0x04),
    /* 0x002E 0x03 */ MSCRIPT_BRANCH_ON_CALLBACK_OPTIONAL(0x0),
    /* 0x0031 0x01 */ MSCRIPT_DONE(),
};

MsgScript D_80C22C30[] = {
    /* 0x0000 0x03 */ MSCRIPT_BEGIN_TEXT(0x28EF),
    /* 0x0003 0x01 */ MSCRIPT_AWAIT_TEXT(),
    /* 0x0004 0x01 */ MSCRIPT_DONE(),
};

Actor* func_80C22350(DmTag* this, PlayState* play, u8 actorCat, s16 actorId) {
    Actor* foundActor = NULL;

    while (true) {
        foundActor = SubS_FindActor(play, foundActor, actorCat, actorId);

        if ((foundActor == NULL) || (((this != (DmTag*)foundActor)) && (foundActor->update != NULL))) {
            break;
        }

        if (foundActor->next == NULL) {
            foundActor = NULL;
            break;
        }
        foundActor = foundActor->next;
    }
    return foundActor;
}

s32 func_80C22400(DmTag* this, s16 csId) {
    s32 ret = false;

    if (CutsceneManager_GetCurrentCsId() == CS_ID_GLOBAL_TALK) {
        CutsceneManager_Stop(CS_ID_GLOBAL_TALK);
        CutsceneManager_Queue(csId);
    } else if (CutsceneManager_IsNext(csId)) {
        CutsceneManager_StartWithPlayerCs(csId, &this->actor);
        ret = true;
    } else {
        CutsceneManager_Queue(csId);
    }
    return ret;
}

s16 func_80C2247C(DmTag* this, s32 numCutscenes) {
    s32 i;
    s16 csId = this->actor.csId;

    for (i = 0; i < numCutscenes; i++) {
        csId = CutsceneManager_GetAdditionalCsId(csId);
    }

    return csId;
}

s32 func_80C224D8(Actor* thisx, PlayState* play) {
    DmTag* this = THIS;
    Actor* sp30;
    Actor* sp2C;
    s16 csId = this->actor.csId;
    s32 ret = false;

    sp30 = func_80C22350(this, play, ACTORCAT_NPC, ACTOR_EN_AN);
    sp2C = func_80C22350(this, play, ACTORCAT_NPC, ACTOR_EN_AH);

    switch (this->unk_1A4) {
        case 0:
            if (func_80C22400(this, csId)) {
                if ((sp2C != NULL) && (sp2C->update != NULL)) {
                    Camera_SetTargetActor(Play_GetCamera(play, CutsceneManager_GetCurrentSubCamId(csId)), sp2C);
                }
                this->unk_1A4++;
                ret = true;
            }
            break;

        case 1:
            if ((sp30 != NULL) && (sp30->update != NULL)) {
                Camera_SetTargetActor(Play_GetCamera(play, CutsceneManager_GetCurrentSubCamId(func_80C2247C(this, 0))),
                                      sp30);
            }
            this->unk_1A4++;
            ret = true;
            break;

        case 2:
            CutsceneManager_Stop(func_80C2247C(this, 0));
            if (func_80C22400(this, func_80C2247C(this, 1))) {
                this->unk_1A4++;
                ret = true;
            }
            break;

        case 3:
            CutsceneManager_Stop(func_80C2247C(this, 1));
            if (func_80C22400(this, func_80C2247C(this, 2))) {
                this->unk_1A4++;
                ret = true;
            }
            break;

        case 4:
            CutsceneManager_Stop(func_80C2247C(this, 2));
            if (func_80C22400(this, func_80C2247C(this, 3))) {
                this->unk_1A4++;
                ret = true;
            }
            break;

        case 5:
            CutsceneManager_Stop(func_80C2247C(this, 3));
            if (func_80C22400(this, func_80C2247C(this, 4))) {
                this->unk_1A4++;
                ret = true;
            }
            break;

        case 6:
            Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
            play->nextEntrance = ENTRANCE(STOCK_POT_INN, 5);
            gSaveContext.nextCutsceneIndex = 0;
            play->transitionTrigger = TRANS_TRIGGER_START;
            play->transitionType = TRANS_TYPE_FADE_BLACK;
            gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK_SLOW;
            this->unk_1A4++;
            break;

        default:
            break;
    }
    return ret;
}

s32 func_80C227E8(Actor* thisx, PlayState* play) {
    DmTag* this = THIS;

    if (this->unk_1A4 == 0) {
        Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_WAIT);
        play->nextEntrance = ENTRANCE(STOCK_POT_INN, 4);
        gSaveContext.nextCutsceneIndex = 0;
        play->transitionTrigger = TRANS_TRIGGER_START;
        play->transitionType = TRANS_TYPE_FADE_BLACK;
        gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK_SLOW;
        this->unk_1A4++;
    }
    return false;
}

MsgScript* func_80C22880(DmTag* this, PlayState* play) {
    s32 time;

    switch (this->unk_18E) {
        case 1:
            time = SCHEDULE_TIME_NOW;
            if ((time >= SCHEDULE_TIME(21, 30)) && (time < SCHEDULE_TIME(23, 0))) {
                if (gSaveContext.save.day == 2) {
                    this->msgEventCallback = func_80C227E8;
                    return D_80C22BF0;
                }
            }
            return D_80C22C30;

        case 2:
            this->msgEventCallback = func_80C224D8;
            return D_80C22BFC;

        default:
            break;
    }

    return NULL;
}

s32 func_80C2291C(DmTag* this, PlayState* play) {
    s32 ret = false;

    if (((this->unk_18C & SUBS_OFFER_MODE_MASK) != SUBS_OFFER_MODE_NONE) &&
        Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        this->unk_18C |= 8;
        SubS_SetOfferMode(&this->unk_18C, SUBS_OFFER_MODE_NONE, SUBS_OFFER_MODE_MASK);
        this->msgEventScript = func_80C22880(this, play);
        this->actionFunc = func_80C229FC;
        ret = true;
    }
    return ret;
}

void func_80C229AC(DmTag* this, PlayState* play) {
    SubS_SetOfferMode(&this->unk_18C, SUBS_OFFER_MODE_ONSCREEN, SUBS_OFFER_MODE_MASK);
    this->actor.flags |= ACTOR_FLAG_TARGETABLE;
}

void DmTag_DoNothing(DmTag* this, PlayState* play) {
}

void func_80C229FC(DmTag* this, PlayState* play) {
    if (MsgEvent_RunScript(&this->actor, play, this->msgEventScript, this->msgEventCallback, &this->msgEventArg4)) {
        this->actionFunc = func_80C229AC;
    }
}

void DmTag_Init(Actor* thisx, PlayState* play) {
    DmTag* this = (DmTag*)thisx;
    Player* player = GET_PLAYER(play);

    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_85_04)) {
        Actor_Kill(&this->actor);
        return;
    }

    if ((play->sceneId == SCENE_YADOYA) && (play->curSpawn == 4)) {
        player->stateFlags1 |= PLAYER_STATE1_20;
        this->unk_18E = 2;
        this->unk_18C = 0;
        SubS_SetOfferMode(&this->unk_18C, SUBS_OFFER_MODE_AUTO, SUBS_OFFER_MODE_MASK);
        this->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
        this->actionFunc = DmTag_DoNothing;
    } else if (this->actor.room == 2) {
        Actor_Kill(&this->actor);
    } else {
        this->actor.targetMode = TARGET_MODE_1;
        this->unk_18E = 1;
        this->unk_18C = 0;
        this->actionFunc = func_80C229AC;
    }
}

void DmTag_Destroy(Actor* thisx, PlayState* play) {
}

void DmTag_Update(Actor* thisx, PlayState* play) {
    DmTag* this = THIS;

    func_80C2291C(this, play);
    this->actionFunc(this, play);
    SubS_Offer(&this->actor, play, 40.0f, fabsf(this->actor.playerHeightRel) + 1.0f, PLAYER_IA_NONE,
               this->unk_18C & SUBS_OFFER_MODE_MASK);
    Actor_SetFocus(&this->actor, 0.0f);
}
