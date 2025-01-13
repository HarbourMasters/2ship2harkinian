#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
}

#define CVAR_NAME "gEnhancements.Player.InfiniteDekuHopping"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterInfiniteDekuHopping() {
    COND_VB_SHOULD(VB_DEKU_LINK_SPIN_ON_LAST_HOP, CVAR, {
        if (*should) {
            Player* player = GET_PLAYER(gPlayState);
            player->remainingHopsCounter = 5;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterInfiniteDekuHopping, { CVAR_NAME });
