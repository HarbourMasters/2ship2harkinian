#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "z64save.h"
}

#define CVAR_NAME "gEnhancements.Timesavers.FasterBottleEmpty"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterFasterEmptyBottle() {
    COND_VB_SHOULD(VB_EMPTYING_BOTTLE, CVAR, {
        Player* player = va_arg(args, Player*);
        if (player->skelAnime.curFrame <= 60.0f) {
            player->skelAnime.playSpeed = 3.0f;
        } else {
            player->skelAnime.playSpeed = 1.0f;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterFasterEmptyBottle, { CVAR_NAME });
