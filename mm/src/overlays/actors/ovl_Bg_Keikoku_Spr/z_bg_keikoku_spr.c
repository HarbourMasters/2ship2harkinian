/*
 * File: z_bg_keikoku_spr.c
 * Overlay: ovl_Bg_Keikoku_Spr
 * Description: Termina Field Fountain Water
 */

#include "z_bg_keikoku_spr.h"
#include "objects/object_keikoku_obj/object_keikoku_obj.h"

#define FLAGS 0x00000000

#define THIS ((BgKeikokuSpr*)thisx)

void BgKeikokuSpr_Init(Actor* thisx, PlayState* play);
void BgKeikokuSpr_Destroy(Actor* thisx, PlayState* play);
void BgKeikokuSpr_Update(Actor* thisx, PlayState* play);
void BgKeikokuSpr_Draw(Actor* thisx, PlayState* play);

ActorInit Bg_Keikoku_Spr_InitVars = {
    /**/ ACTOR_BG_KEIKOKU_SPR,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ OBJECT_KEIKOKU_OBJ,
    /**/ sizeof(BgKeikokuSpr),
    /**/ BgKeikokuSpr_Init,
    /**/ BgKeikokuSpr_Destroy,
    /**/ BgKeikokuSpr_Update,
    /**/ BgKeikokuSpr_Draw,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_F32(uncullZoneForward, 3000, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneScale, 200, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 400, ICHAIN_CONTINUE),
    ICHAIN_VEC3F_DIV1000(scale, 20, ICHAIN_STOP),
};

void BgKeikokuSpr_Init(Actor* thisx, PlayState* play) {
    BgKeikokuSpr* this = THIS;

    Actor_ProcessInitChain(&this->actor, sInitChain);
}

void BgKeikokuSpr_Destroy(Actor* thisx, PlayState* play) {
}

void BgKeikokuSpr_Update(Actor* thisx, PlayState* play) {
}

void BgKeikokuSpr_Draw(Actor* thisx, PlayState* play) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    AnimatedMat_Draw(play, Lib_SegmentedToVirtual(object_keikoku_obj_Matanimheader_0001F8));
    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, object_keikoku_obj_DL_000100);

    AnimatedMat_Draw(play, Lib_SegmentedToVirtual(object_keikoku_obj_Matanimheader_0003F8));
    gSPDisplayList(POLY_XLU_DISP++, object_keikoku_obj_DL_000300);

    AnimatedMat_Draw(play, Lib_SegmentedToVirtual(object_keikoku_obj_Matanimheader_0005F8));
    gSPDisplayList(POLY_XLU_DISP++, object_keikoku_obj_DL_000500);

    CLOSE_DISPS(play->state.gfxCtx);
}
