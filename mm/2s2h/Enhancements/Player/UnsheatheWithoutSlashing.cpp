#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Player.UnsheatheWithoutSlashing"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterUnsheatheWithoutSlashing() {
    COND_VB_SHOULD(VB_USE_HELD_ITEM_AFTER_CHANGE, CVAR, {
        Player* player = va_arg(args, Player*);
        if ((player->heldItemAction == PLAYER_IA_SWORD_KOKIRI) || (player->heldItemAction == PLAYER_IA_SWORD_RAZOR) ||
            (player->heldItemAction == PLAYER_IA_SWORD_GILDED)) {
            *should = false;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterUnsheatheWithoutSlashing, { CVAR_NAME });
