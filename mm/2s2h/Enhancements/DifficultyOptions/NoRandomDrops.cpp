#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.DifficultyOptions.NoRandomDrops"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define DISABLE_VB_DROP(vb) COND_VB_SHOULD(vb, CVAR, { *should = false; })

void RegisterNoRandomDrops() {
    DISABLE_VB_DROP(VB_DROP_COLLECTIBLE);
    DISABLE_VB_DROP(VB_GRASS_DROP_COLLECTIBLE);
    DISABLE_VB_DROP(VB_POT_DROP_COLLECTIBLE);
    DISABLE_VB_DROP(VB_SNOWBALL_DROP_COLLECTIBLE);
    DISABLE_VB_DROP(VB_TREE_DROP_COLLECTIBLE);
    DISABLE_VB_DROP(VB_OBJ_MURE3_DROP_COLLECTIBLE);
    DISABLE_VB_DROP(VB_BARREL_OR_CRATE_DROP_COLLECTIBLE);
}

static RegisterShipInitFunc initFunc(RegisterNoRandomDrops, { CVAR_NAME });
