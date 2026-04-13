#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_Arms_Hook/z_arms_hook.h"
void ArmsHook_Shoot(ArmsHook*, PlayState*);
}

#define TIME_MOVES_CVAR_NAME "gModes.TimeMovesWhenYouMove"
#define TIME_MOVES_CVAR CVarGetInteger(TIME_MOVES_CVAR_NAME, 0)

#define SUPER_HOT_CVAR_NAME "gModes.SuperHot"
#define SUPER_HOT_CVAR CVarGetInteger(SUPER_HOT_CVAR_NAME, 0)

// Arbitrary speed to determine the offset is unset
#define DEFAULT_TIME_OFFSET -12345
static s32 sStoredTimeOffset = DEFAULT_TIME_OFFSET;

void RegisterTimeMovesWhenYouMove() {
    if (!TIME_MOVES_CVAR && sStoredTimeOffset != DEFAULT_TIME_OFFSET) {
        gSaveContext.save.timeSpeedOffset = sStoredTimeOffset;
        sStoredTimeOffset = DEFAULT_TIME_OFFSET;
    }

    COND_HOOK(ShouldActorUpdate, TIME_MOVES_CVAR && SUPER_HOT_CVAR, [](Actor* actor, bool* should) {
        static bool hookIsFiring = false;
        if (actor->id == ACTOR_ARMS_HOOK) {
            ArmsHook* hook = (ArmsHook*)actor;
            if (hook->actionFunc == ArmsHook_Shoot) {
                hookIsFiring = true;
            } else {
                hookIsFiring = false;
            }
        }

        if (actor->id != ACTOR_EN_ARROW &&
            (actor->id == ACTOR_PLAYER || actor->category == ACTORCAT_BG || actor->category == ACTORCAT_DOOR ||
             actor->category == ACTORCAT_SWITCH || actor->category == ACTORCAT_ITEMACTION)) {
            return;
        }

        Player* player = GET_PLAYER(gPlayState);

        static Actor* lastTalkActor = NULL;
        if (player->talkActor != NULL && player->talkActor != lastTalkActor) {
            lastTalkActor = player->talkActor;
        }

        if (player->speedXZ == 0 && lastTalkActor != actor && !(player->stateFlags1 & PLAYER_STATE1_1) &&
            !(player->stateFlags1 & PLAYER_STATE1_2) && !(player->stateFlags1 & PLAYER_STATE1_20) &&
            !(player->stateFlags1 & PLAYER_STATE1_TALKING) && !(player->stateFlags1 & PLAYER_STATE1_DEAD) &&
            !(player->stateFlags1 & PLAYER_STATE1_100) && !(player->stateFlags1 & PLAYER_STATE1_400) &&
            !(player->stateFlags1 & PLAYER_STATE1_CHARGING_SPIN_ATTACK) &&
            !(player->stateFlags1 & PLAYER_STATE1_ZORA_BOOMERANG_THROWN) &&
            !(player->stateFlags1 & PLAYER_STATE1_10000000) && !(player->stateFlags1 & PLAYER_STATE1_20000000) &&
            !(player->stateFlags2 & PLAYER_STATE2_8) && !(player->stateFlags3 & PLAYER_STATE3_8) &&
            !(player->stateFlags3 & PLAYER_STATE3_2000000) && (!hookIsFiring)) {
            *should = false;
        }
    });

    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, TIME_MOVES_CVAR, [](Actor* actor) {
        Player* player = GET_PLAYER(gPlayState);
        bool timeShouldMove = (player->stateFlags2 & PLAYER_STATE2_USING_OCARINA) || player->speedXZ != 0.0f;

        if (timeShouldMove && sStoredTimeOffset != DEFAULT_TIME_OFFSET) {
            gSaveContext.save.timeSpeedOffset = sStoredTimeOffset;
            sStoredTimeOffset = DEFAULT_TIME_OFFSET;

            // This is for the section above, lets arrows continue flying after they were fired with time frozen
            if (SUPER_HOT_CVAR) {
                player->unk_D57 = 4;
            }
        } else if (!timeShouldMove && sStoredTimeOffset == DEFAULT_TIME_OFFSET) {
            sStoredTimeOffset = gSaveContext.save.timeSpeedOffset;
            gSaveContext.save.timeSpeedOffset = -R_TIME_SPEED;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterTimeMovesWhenYouMove, { TIME_MOVES_CVAR_NAME, SUPER_HOT_CVAR_NAME });
