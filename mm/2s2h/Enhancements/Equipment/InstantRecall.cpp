#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "src/overlays/actors/ovl_En_Boom/z_en_boom.h"
}

#define CVAR_NAME "gEnhancements.PlayerActions.InstantRecall"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void Player_ReturnBoomerangs() {
    Player* player = GET_PLAYER(gPlayState);

    if (player == NULL) {
        return;
    }

    EnBoom* firstBoomerang = (EnBoom*)player->boomerangActor;

    // Kill both boomerangs as long as they are not carrying an actor
    if (firstBoomerang != NULL) {
        if (firstBoomerang->unk_1C8 == NULL) {
            Actor_Kill(&firstBoomerang->actor);
        }
        EnBoom* secondBoomerang = (EnBoom*)firstBoomerang->actor.child;
        if (secondBoomerang != NULL && secondBoomerang->unk_1C8 == NULL) {
            Actor_Kill(&secondBoomerang->actor);
        }
    }
}

void RegisterInstantRecall() {
    COND_ID_HOOK(OnActorUpdate, ACTOR_EN_BOOM, CVAR, [](Actor* actor) {
        if (CHECK_BTN_ALL(gPlayState->state.input->press.button, BTN_B)) {
            Player_ReturnBoomerangs();
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterInstantRecall, { CVAR_NAME });
