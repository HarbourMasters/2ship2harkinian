#include "MiscBehavior.h"
#include "gu.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

typedef enum { TRAP_BLAST, TRAP_MAX } TrapTypes;

void Rando::MiscBehavior::OfferTrapItem() {
    if (!gPlayState) {
        return;
    }

    Player* player = GET_PLAYER(gPlayState);

    int roll = rand() % TRAP_MAX;
    switch (roll) {
        case TRAP_BLAST:
            GameInteractor::Instance->events.emplace_back(GIEventSpawnActor{ .actorId = ACTOR_EN_BOM,
                                                                             .posX = player->actor.world.pos.x,
                                                                             .posY = player->actor.world.pos.y,
                                                                             .posZ = player->actor.world.pos.z,
                                                                             .rotX = 1,
                                                                             .params = 0 });
            break;
        default:
            break;
    }
}
