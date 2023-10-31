/*
 * File: z_en_dt.c
 * Overlay: ovl_En_Dt
 * Description: Mayor Dotour
 */

#include "z_en_dt.h"
#include "objects/object_dt/object_dt.h"
#include "overlays/actors/ovl_En_Muto/z_en_muto.h"
#include "overlays/actors/ovl_En_Baisen/z_en_baisen.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY)

#define THIS ((EnDt*)thisx)

void EnDt_Init(Actor* thisx, PlayState* play);
void EnDt_Destroy(Actor* thisx, PlayState* play);
void EnDt_Update(Actor* thisx, PlayState* play);
void EnDt_Draw(Actor* thisx, PlayState* play);

void func_80BE9C74(EnDt* this);
void func_80BE9CE8(EnDt* this, s32 arg1);
void func_80BE9D9C(EnDt* this);
void func_80BE9DF8(EnDt* this);
void func_80BE9E94(EnDt* this, PlayState* play);
void func_80BE9EF8(EnDt* this, PlayState* play);
void func_80BEA088(EnDt* this, PlayState* play);
void func_80BEA254(EnDt* this, PlayState* play);
void func_80BEA394(EnDt* this, PlayState* play);
void func_80BEA8F0(EnDt* this, PlayState* play);
void func_80BEAB44(EnDt* this, PlayState* play);
void func_80BEABF8(EnDt* this, PlayState* play);
void func_80BEAC84(EnDt* this, PlayState* play);
void func_80BEAD2C(EnDt* this, PlayState* play);
void func_80BEADB8(EnDt* this);
void func_80BEADD4(EnDt* this, PlayState* play);
s32 EnDt_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx);

extern ColliderCylinderInit D_80BEB29C;
extern u16 D_80BEB1D0[];
extern s16 D_80BEB268[];
extern s16 D_80BEB26A[];
extern s32 D_80BEB2C8[];
extern u8 D_80BEB2E0[];
extern s32 D_80BEB348[];
extern s32 D_80BEB35C[];

#if 0
ActorInit En_Dt_InitVars = {
    /**/ ACTOR_EN_DT,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ OBJECT_DT,
    /**/ sizeof(EnDt),
    /**/ EnDt_Init,
    /**/ EnDt_Destroy,
    /**/ EnDt_Update,
    /**/ EnDt_Draw,
};

// static ColliderCylinderInit sCylinderInit = {
static ColliderCylinderInit D_80BEB29C = {
    { COLTYPE_NONE, AT_NONE, AC_NONE, OC1_ON | OC1_TYPE_ALL, OC2_TYPE_2, COLSHAPE_CYLINDER, },
    { ELEMTYPE_UNK0, { 0x00000000, 0x00, 0x00 }, { 0xF7CFFFFF, 0x00, 0x00 }, TOUCH_NONE | TOUCH_SFX_NORMAL, BUMP_NONE, OCELEM_ON, },
    { 25, 70, 0, { 0, 0, 0 } },
};

#endif

void EnDt_Init(Actor* thisx, PlayState* play) {
    EnDt* this = THIS;
    s32 csId;
    s32 i;

    this->actor.colChkInfo.mass = 0xFF;
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 19.0f);
    SkelAnime_InitFlex(play, &this->skelanime, &object_dt_Skel_00B0CC, &object_dt_Anim_00112C, &this->unk_188,
                       &this->unk_1E2, 0xF);
    this->actor.targetMode = 6;
    this->unk_274 = 0;
    this->unk_278 = 0;

    Collider_InitAndSetCylinder(play, &this->collider, &this->actor, &D_80BEB29C);

    if ((gSaveContext.save.day == 3) && (gSaveContext.save.isNight != 0)) {
        func_80BEAC84(this, play);
    } else {
        //! FAKE:
        EnDt* var_s1;

        csId = this->actor.csId;

        i = 0;
        // clang-format off
        while (csId != CS_ID_NONE) { this->csIdList[i] = csId; csId = CutsceneManager_GetAdditionalCsId(csId); i++; };
        // clang-format on
    }
    this->actionFunc = func_80BE9E94;
}

void EnDt_Destroy(Actor* thisx, PlayState* play) {
    EnDt* this = THIS;
    Collider_DestroyCylinder(play, &this->collider);
}

void func_80BE9C74(EnDt* this) {
    s32 var_v1 = ABS_ALT((s16)(this->actor.yawTowardsPlayer - this->actor.world.rot.y));

    this->unk_28A.y = 0;

    if (var_v1 < 0x4E20) {
        this->unk_28A.y = this->actor.yawTowardsPlayer - this->actor.world.rot.y;
        if (this->unk_28A.y > 0x2710) {
            this->unk_28A.y = 0x2710;
        } else if (this->unk_28A.y < -0x2710) {
            this->unk_28A.y = -0x2710;
        }
    }
}

void func_80BE9CE8(EnDt* this, s32 arg1) {
    f32 var_fv1 = -4.0f;

    this->unk_246 = arg1;

    if ((this->unk_246 == 2) || (this->unk_246 == 5)) {
        var_fv1 = 0.0f;
    }

    this->unk_250 = Animation_GetLastFrame(D_80BEB2C8[this->unk_246]);
    Animation_Change(&this->skelanime, D_80BEB2C8[this->unk_246], 1.0f, 0.0f, this->unk_250, D_80BEB2E0[this->unk_246],
                     var_fv1);
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Dt/func_80BE9D9C.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Dt/func_80BE9DF8.s")

void func_80BE9E94(EnDt* this, PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_NPC].first;

    while (actor != NULL) {
        if (actor->id == ACTOR_EN_MUTO) {
            this->unk_274 = (EnMuto*)actor;
        } else if (actor->id == ACTOR_EN_BAISEN) {
            this->unk_278 = (EnBaisen*)actor;
        }
        actor = actor->next;
    }
    func_80BE9EF8(this, play);
}

void func_80BE9EF8(EnDt* this, PlayState* play) {
    this->unk_256 = 0;

    if (!(gSaveContext.save.saveInfo.weekEventReg[60] & 0x10)) {
        if (gSaveContext.save.saveInfo.weekEventReg[63] & 0x80) {
            this->unk_256 = 21;
            this->unk_280 = 5;
            func_80BE9D9C(this);
            this->actor.textId = D_80BEB1D0[this->unk_256];
            Message_StartTextbox(play, this->actor.textId, &this->actor);
            func_800B7298(play, &this->actor, 7);
            this->unk_254 = 1;
            this->actionFunc = func_80BEA394;
        } else {
            goto block_5;
        }
    } else {
        if (gSaveContext.save.saveInfo.weekEventReg[63] & 0x80) {
            this->unk_256 = 23;
            this->unk_280 = 5;
            this->unk_290 = 1;
            func_80BE9D9C(this);
            Message_BombersNotebookQueueEvent(play, 7);
        }
    block_5:
        if (this->unk_280 == 0) {
            if (gSaveContext.save.saveInfo.weekEventReg[60] & 8) {
                this->unk_256 = 9;
                this->unk_280 = 2;
            }
            func_80BE9D9C(this);
            if (!(gSaveContext.save.saveInfo.weekEventReg[60] & 8)) {
                this->unk_280 = 1;
            }
        }
        this->actor.textId = D_80BEB1D0[this->unk_256];
        if (!(gSaveContext.save.saveInfo.weekEventReg[63] & 0x80)) {
            func_80BE9DF8(this);
        }
        this->unk_254 = 1;
        this->actionFunc = func_80BEA088;
    }
}

void func_80BEA088(EnDt* this, PlayState* play) {
    EnMuto* mutoh = NULL;
    EnBaisen* viscen = NULL;

    if (Actor_ProcessTalkRequest(&this->actor, &play->state)) {
        func_80BEA254(this, play);
        return;
    }

    if (!(gSaveContext.save.saveInfo.weekEventReg[63] & 0x80)) {
        if (this->unk_274 != NULL) {
            if (this->unk_278 != NULL) {
                mutoh = this->unk_274;
                viscen = this->unk_278;
            }
        }
        this->unk_240 = 0;
        this->unk_256 = 0;
        if (Player_GetMask(play) == PLAYER_MASK_COUPLE) {
            this->unk_256 = 0xB;
            this->unk_240 = 1;
            if ((this->unk_274 != NULL) && (this->unk_278 != NULL)) {
                mutoh->textIdIndex = 4;
                viscen->textIdIndex = 6;
            }
        } else if (gSaveContext.save.saveInfo.weekEventReg[60] & 8) {
            this->unk_256 = 9;
        }
    }

    this->actor.textId = D_80BEB1D0[this->unk_256];

    if (!(gSaveContext.save.saveInfo.weekEventReg[0x3F] & 0x80) && (this->unk_274 != NULL) && (this->unk_278 != NULL) &&
        ((mutoh->cutsceneState == 1) || (viscen->unk2AC == 1))) {
        func_80BEA254(this, play);
    }

    if ((gSaveContext.save.saveInfo.weekEventReg[0x3F] & 0x80) && (Player_GetMask(play) == 2)) {
        this->actor.textId = 0x2368;
    }
    Actor_OfferTalk(&this->actor, play, 150.0f);
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Dt/func_80BEA254.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Dt/func_80BEA394.s")

void func_80BEA8F0(EnDt* this, PlayState* play) {
    f32 curFrame = this->skelanime.curFrame;
    s16 subCamId;
    s32 i;
    s32 j;

    if (this->unk_244 != 0) {
        if (this->unk_244 == 1) {
            if ((this->unk_256 == 8) || (this->unk_280 == 4)) {
                func_80BE9D9C(this);
            }
            this->unk_280++;
            if (this->unk_280 >= 6) {
                this->unk_280 = 5;
            }
        }
    } else if (curFrame >= this->unk_250) {
        j = D_80BEB26A[this->unk_26E];

        func_80BE9D9C(this);
        Message_CloseTextbox(play);

        this->actor.textId = D_80BEB1D0[this->unk_256];

        Message_StartTextbox(play, this->actor.textId, &this->actor);

        if (this->unk_256 == 8) {
            Message_BombersNotebookQueueEvent(play, 7);
        }

        for (i = 0; i < 24; i++) {
            if ((play->msgCtx.currentTextId == D_80BEB268[i]) || (this->actor.textId == D_80BEB268[i])) {
                this->unk_26E = i;
            }
        }

        i = D_80BEB26A[this->unk_26E];

        if ((this->unk_270 == 2) && (i != j)) {
            this->unk_270 = 1;
            CutsceneManager_Stop(this->csIdList[j]);
            CutsceneManager_Queue(this->csIdList[i]);
        }

        func_80BE9DF8(this);
        Actor_ChangeFocus(&this->actor, play, this->targetActor);
        subCamId = CutsceneManager_GetCurrentSubCamId(this->csIdList[i]);
        Camera_SetTargetActor(Play_GetCamera(play, subCamId), this->targetActor);
        this->actionFunc = func_80BEA394;
    }
}

void func_80BEAAF8(EnDt* this, PlayState* play) {
    Actor_OfferGetItem(this, play, GI_HEART_PIECE, 300.0f, 300.0f);
    this->unk_254 = 3;
    this->actionFunc = func_80BEAB44;
}

void func_80BEAB44(EnDt* this, PlayState* play) {
    if (Actor_HasParent(&this->actor, play) != 0) {
        this->unk_256 = 22;
        this->actor.parent = NULL;
        this->actor.textId = D_80BEB1D0[this->unk_256];
        Actor_OfferTalkExchange(&this->actor, play, 400.0f, 400.0f, PLAYER_IA_MINUS1);
        gSaveContext.save.saveInfo.weekEventReg[60] |= 0x10;
        this->actionFunc = func_80BEABF8;
    } else {
        Actor_OfferGetItem(&this->actor, play, GI_HEART_PIECE, 300.0f, 300.0f);
    }
}

void func_80BEABF8(EnDt* this, PlayState* play) {
    SkelAnime_Update(&this->skelanime);
    if (Actor_ProcessTalkRequest(&this->actor, &play->state) != 0) {
        Message_BombersNotebookQueueEvent(play, 0x26);
        Message_BombersNotebookQueueEvent(play, 7);
        this->actionFunc = func_80BEA394;
    } else {
        Actor_OfferTalkExchange(&this->actor, play, 400.0f, 400.0f, PLAYER_IA_MINUS1);
    }
}

void func_80BEAC84(EnDt* this, PlayState* play) {
    func_80BE9CE8(this, 3);
    this->unk_248 = 1;
    this->unk_256 = 24;
    Message_BombersNotebookQueueEvent(play, 7);

    if (gSaveContext.save.saveInfo.weekEventReg[60] & 0x40) {
        this->unk_256 = 26;
    }

    this->actor.textId = D_80BEB1D0[this->unk_256];
    this->unk_24C = 3;
    this->unk_248 = 1;
    this->unk_254 = 4;
    this->actionFunc = func_80BEAD2C;
}

void func_80BEAD2C(EnDt* this, PlayState* play) {
    func_80BE9C74(this);
    if (Actor_ProcessTalkRequest(&this->actor, play) != 0) {
        func_80BEADB8(this);
        return;
    }

    if ((gSaveContext.save.saveInfo.weekEventReg[63] & 0x80) && (Player_GetMask(play) == 2)) {
        this->actor.textId = 0x2368;
    }
    Actor_OfferTalk(&this->actor, play, 150.0f);
}

void func_80BEADB8(EnDt* this) {
    this->unk_254 = 5;
    this->actionFunc = func_80BEADD4;
}

void func_80BEADD4(EnDt* this, PlayState* play) {
    func_80BE9C74(this);
    if ((Message_GetState(&play->msgCtx) == 5) && (Message_ShouldAdvance(play) != 0)) {
        Message_CloseTextbox(play);
        if (!(gSaveContext.save.saveInfo.weekEventReg[60] & 0x40)) {
            this->unk_256 = 25;
            Message_ContinueTextbox(play, D_80BEB1D0[this->unk_256]);
            gSaveContext.save.saveInfo.weekEventReg[60] |= 0x40;
        } else {
            func_80BEAC84(this, play);
        }
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_En_Dt/EnDt_Update.s")

s32 EnDt_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnDt* this = THIS;

    if (limbIndex == 12) {
        rot->y += -1 * this->unk_284.y;
    }
    return false;
}

void EnDt_Draw(Actor* thisx, PlayState* play) {
    EnDt* this = THIS;
    s32 index = 0;

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x08, Lib_SegmentedToVirtual(D_80BEB348[this->unk_24C]));

    if (this->unk_24C < 3) {
        index = this->unk_24C;
    }

    gSPSegment(POLY_OPA_DISP++, 0x09, Lib_SegmentedToVirtual(D_80BEB35C[index]));
    CLOSE_DISPS(play->state.gfxCtx);
    SkelAnime_DrawFlexOpa(play, this->skelanime.skeleton, this->skelanime.jointTable, this->skelanime.dListCount,
                          EnDt_OverrideLimbDraw, NULL, &this->actor);
}
