#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "global.h"

extern "C" u8 Champion_AllowsMidairAim(Player* player);

static void RegisterChampionHooks() {
    REGISTER_VB_SHOULD(VB_PLAYER_ALLOW_MIDAIR_AIM, {
        Player* player = va_arg(args, Player*);
        if (Champion_AllowsMidairAim(player)) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initChampionHooks(RegisterChampionHooks);
