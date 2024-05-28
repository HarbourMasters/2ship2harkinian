#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "InstantRecall.h"
#include "src/overlays/actors/ovl_En_Boom/z_en_boom.h"

extern "C" {
extern PlayState* gPlayState;
}

static HOOK_ID onActorUpdateHookId = 0;

void Player_ReturnBoomerangs() {
    Player* player = GET_PLAYER(gPlayState);

    EnBoom* boomerangs = (EnBoom*)player->boomerangActor;

    if (boomerangs != NULL) {
        // Set the positions of both boomerangs
        boomerangs->actor.world.pos = player->actor.world.pos;
        if (boomerangs->actor.child != NULL) {
            boomerangs->actor.child->world.pos = player->actor.world.pos;
        }

        // Kill both boomerangs
        Actor_Kill(&boomerangs->actor);
        if (boomerangs->actor.child != NULL) {
            Actor_Kill(boomerangs->actor.child);
        }
    }
}

void RegisterInstantRecall() {
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorUpdate>(onActorUpdateHookId);
    onActorUpdateHookId = 0;
    
    onActorUpdateHookId = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(
        ACTOR_EN_BOOM, [](Actor* outerActor) {
            if (CVarGetInteger("gEnhancements.PlayerActions.InstantRecall", 0)) {
                if (gPlayState->state.input->cur.button == BTN_B) {
                    Player_ReturnBoomerangs();
                }
            }
        });
}
