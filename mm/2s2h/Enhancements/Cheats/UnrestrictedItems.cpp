#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gCheats.UnrestrictedItems"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterUnrestrictedItems() {
    COND_VB_SHOULD(VB_ITEM_BE_RESTRICTED, CVAR, { *should = false; });

    // Prevent Deku Hookshot crash from float overflow/failed raycasts
    COND_VB_SHOULD(VB_DEKU_COMMON_HEAD_OVERRIDE_HELD_ACTOR, CVAR, {
        Actor* heldActor = va_arg(args, Actor*);
        if (heldActor != NULL && heldActor->id == ACTOR_ARMS_HOOK) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_DEKU_COMMON_UPPER_LIMB_OVERRIDE_HELD_ACTOR, CVAR, {
        Actor* heldActor = va_arg(args, Actor*);
        if (heldActor != NULL && heldActor->id == ACTOR_ARMS_HOOK) {
            *should = false;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterUnrestrictedItems, { CVAR_NAME });
