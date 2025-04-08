#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Minifrog/z_en_minifrog.h"
void EnMinifrog_TurnToPlayer(EnMinifrog* enMinifrog);
void EnMinifrog_Jump(EnMinifrog* enMinifrog);
void EnMinifrog_JumpTimer(EnMinifrog* enMinifrog);
}

RandoCheckId GetFrogCheck(s16 index) {
    switch (index) {
        case 1:
            return RC_WOODFALL_TEMPLE_GEKKO_FROG;
        case 2:
            return RC_GREAT_BAY_TEMPLE_GEKKO_FROG;
        case 3:
            return RC_SOUTHERN_SWAMP_FROG;
        case 4:
            return RC_CLOCK_TOWN_LAUNDRY_FROG;
    }
    return RC_UNKNOWN;
}

void MiniFrog_DrawCustom(Actor* thisx, PlayState* play) {
    EnMinifrog* enMinifrog = (EnMinifrog*)thisx;
    RandoCheckId frogCheck = GetFrogCheck(enMinifrog->frogIndex);
    if (frogCheck == RC_UNKNOWN) {
        return;
    }

    RandoItemId frogItem = RANDO_SAVE_CHECKS[frogCheck].randoItemId;

    Matrix_Translate(0.0f, 25.0f, 0.0f, MTXMODE_APPLY);
    Rando::DrawItem(Rando::ConvertItem(frogItem, frogCheck), thisx);
}

void MiniFrog_UpdateCustom(Actor* thisx, PlayState* play) {
    EnMinifrog* enMinifrog = (EnMinifrog*)thisx;
    RandoCheckId frogCheck = GetFrogCheck(enMinifrog->frogIndex);

    EnMinifrog_TurnToPlayer(enMinifrog);
    EnMinifrog_Jump(enMinifrog);
    EnMinifrog_JumpTimer(enMinifrog);

    Actor_MoveWithGravity(&enMinifrog->actor);
    Actor_UpdateBgCheckInfo(play, &enMinifrog->actor, 25.0f, 12.0f, 0.0f,
                            UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4 | UPDBGCHECKINFO_FLAG_8 |
                                UPDBGCHECKINFO_FLAG_10);
    enMinifrog->actor.focus.rot.y = enMinifrog->actor.shape.rot.y;

    if ((thisx->xzDistToPlayer <= 30.0f) && (fabsf(thisx->playerHeightRel) <= fabsf(80.0f))) {
        RANDO_SAVE_CHECKS[frogCheck].eligible = true;
        Actor_Kill(&enMinifrog->actor);
        return;
    }
}

void Rando::ActorBehavior::InitEnMinifrogBehavior() {
    COND_VB_SHOULD(VB_SPAWN_FROG, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS], {
        EnMinifrog* enMinifrog = va_arg(args, EnMinifrog*);
        RandoCheckId frogCheck = GetFrogCheck(enMinifrog->frogIndex);
        if (frogCheck == RC_UNKNOWN) {
            return;
        }
        *should = RANDO_SAVE_CHECKS[frogCheck].cycleObtained;
    });

    COND_ID_HOOK(OnActorInit, ACTOR_EN_MINIFROG, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS], [](Actor* actor) {
        EnMinifrog* enMinifrog = (EnMinifrog*)actor;

        if (EN_FROG_IS_RETURNED(&enMinifrog->actor)) {
            return;
        }

        RandoCheckId frogCheck = GetFrogCheck(enMinifrog->frogIndex);
        if (frogCheck == RC_UNKNOWN) {
            return;
        }

        actor->draw = MiniFrog_DrawCustom;
        actor->update = MiniFrog_UpdateCustom;
        actor->shape.shadowDraw = NULL;
        actor->flags &= ~ACTOR_FLAG_TARGETABLE;
        Actor_SetScale(&enMinifrog->actor, 0.4f);
    });
}
