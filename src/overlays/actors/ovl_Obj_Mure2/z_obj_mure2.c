/*
 * File: z_obj_mure2.c
 * Overlay: ovl_Obj_Mure2
 * Description: Circle of rocks spawner
 */

#include "z_obj_mure2.h"

#define FLAGS 0x00000000

#define THIS ((ObjMure2*)thisx)

void ObjMure2_Init(Actor* thisx, PlayState* play);
void ObjMure2_Update(Actor* thisx, PlayState* play);

void func_809613C4(ObjMure2* this, PlayState* play);
void func_809613FC(ObjMure2* this, PlayState* play);
void func_80961490(ObjMure2* this, PlayState* play);

void func_809613B0(ObjMure2*);
void func_809613E8(ObjMure2*);
void func_8096104C(ObjMure2*, PlayState*);
void func_8096147C(ObjMure2*);
void func_809611BC(ObjMure2*, PlayState*);
void func_809612BC(ObjMure2*);

extern f32 D_80961590[];
extern f32 D_8096159C[];
extern s16 D_809615A8[];

#if 0
ActorInit Obj_Mure2_InitVars = {
    ACTOR_OBJ_MURE2,
    ACTORCAT_PROP,
    FLAGS,
    GAMEPLAY_KEEP,
    sizeof(ObjMure2),
    (ActorFunc)ObjMure2_Init,
    (ActorFunc)Actor_Noop,
    (ActorFunc)ObjMure2_Update,
    (ActorFunc)NULL,
};

// static InitChainEntry sInitChain[] = {
static InitChainEntry D_809615F4[] = {
    ICHAIN_F32(uncullZoneForward, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneScale, 2100, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 100, ICHAIN_STOP),
};

#endif

extern InitChainEntry D_809615F4[];

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_Obj_Mure2/func_80960CF0.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_Obj_Mure2/func_80960E0C.s")

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_Obj_Mure2/func_80960F0C.s")

void func_80961018(s16* arg0, ObjMure2* arg1) {
    s32 value = arg1->actor.params & 3;
    s32 temp_a2 = (arg1->actor.params >> 8) & 0x1F;

    if (value == 2) {
        *arg0 = temp_a2 * 0x10;
        return;
    }

    *arg0 = temp_a2 << 8;
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_Obj_Mure2/func_8096104C.s")

void func_809611BC(ObjMure2* arg0, PlayState* arg1) {
    s16 var_v0 = D_809615A8[arg0->actor.params & 3];
    s32 var_s2 = 0;

    if (var_v0 > 0) {
        do {
            if (!((arg0->unk_178 >> var_s2) & 1)) {
                if (arg0->unk_148[var_s2] != NULL) {
                    if (Actor_HasParent(arg0->unk_148[var_s2], arg1) != 0) {
                        arg0->unk_178 |= 1 << var_s2;
                    } else {
                        Actor_Kill(arg0->unk_148[var_s2]);
                    }
                    arg0->unk_148[var_s2] = NULL;
                }
            } else {
                arg0->unk_148[var_s2] = NULL;
            }

            var_v0 = D_809615A8[arg0->actor.params & 3];
        } while (++var_s2 < var_v0);
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/overlays/ovl_Obj_Mure2/func_809612BC.s")

void ObjMure2_Init(Actor* thisx, PlayState* play) {
    ObjMure2* this = THIS;
    Actor_ProcessInitChain(&this->actor, D_809615F4);
    if (play->csCtx.state != 0) {
        this->actor.uncullZoneForward += 1200.0f;
    }
    func_809613B0(this);
}

void func_809613B0(ObjMure2* this) {
    this->actionFunc = func_809613C4;
}

void func_809613C4(ObjMure2 *this, PlayState *play) {
    func_809613E8(this);
}

void func_809613E8(ObjMure2* this) {
    this->actionFunc = func_809613FC;
}

void func_809613FC(ObjMure2* this, PlayState* play) {
    if (Math3D_XZLengthSquared(this->actor.projectedPos.x, this->actor.projectedPos.z) <
        D_80961590[this->actor.params & 3] * this->unk_17C) {
        this->actor.flags |= 0x10;
        func_8096104C(this, play);
        func_8096147C(this);
    }
}

void func_8096147C(ObjMure2* this) {
    this->actionFunc = func_80961490;
}

void func_80961490(ObjMure2* this, PlayState* play) {
    func_809612BC(this);

    if ((D_8096159C[this->actor.params & 3] * this->unk_17C) <=
        Math3D_XZLengthSquared(this->actor.projectedPos.x, this->actor.projectedPos.z)) {
        this->actor.flags &= ~0x10;
        func_809611BC(this, play);
        func_809613E8(this);
    }
}

void ObjMure2_Update(Actor* thisx, PlayState* play) {
    ObjMure2* this = THIS;
    if (play->csCtx.state == 0) {
        this->unk_17C = 1.0f;
    } else {
        this->unk_17C = 4.0f;
    }
    this->actionFunc(this, play);
}
