#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/ShipInit.hpp"
#include <stdarg.h>

#define CVAR_NAME "gEnhancements.Minigames.GibdoTradeSequence"
#define CVAR CVarGetInteger(CVAR_NAME, GIBDO_TRADE_SEQUENCE_VANILLA)

extern "C" {
#include "functions.h"
#include "variables.h"
}

void RegisterGibdoTradeSequenceOptions() {
    COND_VB_SHOULD(VB_GIBDO_TRADE_SEQUENCE_SUFFICIENT_QUANTITY_PRESENTED, CVAR != GIBDO_TRADE_SEQUENCE_VANILLA, {
        ItemId requestedItemId = va_arg(args, ItemId);
        if (AMMO(requestedItemId) >= 1) {
            *should = true;
        }
    });

    COND_VB_SHOULD(VB_GIBDO_TRADE_SEQUENCE_ACCEPT_RED_POTION, CVAR == GIBDO_TRADE_SEQUENCE_MM3D, {
        PlayerItemAction requestedItemAction = va_arg(args, PlayerItemAction);
        PlayerItemAction presentedItemAction = va_arg(args, PlayerItemAction);

        *should = (requestedItemAction == PLAYER_IA_BOTTLE_POTION_BLUE) &&
                  (presentedItemAction == PLAYER_IA_BOTTLE_POTION_RED);
    });

    COND_VB_SHOULD(VB_GIBDO_TRADE_SEQUENCE_TAKE_MORE_THAN_ONE_ITEM, CVAR != GIBDO_TRADE_SEQUENCE_VANILLA,
                   { *should = false; });

    COND_VB_SHOULD(VB_GIBDO_TRADE_SEQUENCE_DO_TRADE, CVAR == GIBDO_TRADE_SEQUENCE_NO_TRADE, { *should = false; });
}

static RegisterShipInitFunc initFunc(RegisterGibdoTradeSequenceOptions, { CVAR_NAME });