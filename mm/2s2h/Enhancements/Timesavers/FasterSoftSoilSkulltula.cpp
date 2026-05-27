#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/actors/ovl_Obj_Bean/z_obj_bean.h"
}

#define CVAR_NAME "gEnhancements.Timesavers.FasterSoftSoilSkulltula"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterFasterSoftSoilSkulltula() {
    // During initialization, only the first bug to claim the soil keeps tracking toward it (func_80A6A024 returns
    // true).  The other two have unk_34C cleared and free-roam immediately, available for rebottling.
    COND_VB_SHOULD(VB_BUG_TRACK_SOFT_SOIL, CVAR, { *should = false; });

    // When the single tracking bug finishes burrowing, force the count to 3 so ObjBean's vanilla check (unk_1E0 >= 3)
    // passes and spawns the Gold Skulltula.
    COND_VB_SHOULD(VB_SOFT_SOIL_BUG_BURROWED, CVAR, {
        const auto soil = va_arg(args, ObjBean*);
        soil->unk_1E0 = 3;
    });
}

static RegisterShipInitFunc initFunc(RegisterFasterSoftSoilSkulltula, { CVAR_NAME });