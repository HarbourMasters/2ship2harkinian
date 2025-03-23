#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "variables.h"
#include "functions.h"

#include "include/z64player.h"
}

void Swim_Ability(Player* player) {
    if (gPlayState->sceneId == SCENE_BOWLING) {
        return;
    }

    if (player->stateFlags1 & PLAYER_STATE1_8000000) {
        gPlayState->nextEntrance = gSaveContext.respawn->entrance;
        gPlayState->transitionTrigger = TRANS_TRIGGER_START;
        gPlayState->transitionType = TRANS_TYPE_FADE_BLACK_FAST;
        player->stateFlags1 |= PLAYER_STATE1_200;
        Audio_PlaySfx(NA_SE_SY_DEKUNUTS_JUMP_FAILED);
    }
}

void Rando::ActorBehavior::InitPlayerBehavior() {
    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, IS_RANDO, [](Actor* actor) {
        if (!RANDO_SAVE_OPTIONS[RO_SHUFFLE_SWIM]) {
            return;
        }

        if (!Flags_GetRandoInf(RANDO_INF_OBTAINED_SWIM)) {
            Swim_Ability(GET_PLAYER(gPlayState));
        }
    });
}