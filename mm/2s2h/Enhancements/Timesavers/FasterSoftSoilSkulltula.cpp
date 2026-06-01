#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/actors/ovl_Obj_Bean/z_obj_bean.h"
#include "overlays/actors/ovl_En_Mushi2/z_en_mushi2.h"
}

#define CVAR_NAME "gEnhancements.Timesavers.FasterSoftSoilSkulltula"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterFasterSoftSoilSkulltula() {
    // During initialization, only the first bug to claim the soil keeps tracking toward it (func_80A6024 returns
    // true).  The other two have unk_34C cleared and free-roam immediately, available for rebottling.  The tracking
    // bug has its guidance factor maxed so it heads straight for the soil.
    COND_VB_SHOULD(VB_BUG_TRACK_SOFT_SOIL, CVAR, { *should = false; });

    // When the single tracking bug finishes burrowing, force the count to 3 so ObjBean's vanilla check (unk_1E0 >= 3)
    // passes and spawns the Gold Skulltula.
    COND_VB_SHOULD(VB_SOFT_SOIL_BUG_BURROWED, CVAR, {
        const auto soil = va_arg(args, ObjBean*);
        soil->unk_1E0 = 3;
    });

    // Freeze the life timer for freed bugs while the Gold Skulltula cutscene is active so they survive until the
    // player regains control and can rebottle them.
    COND_VB_SHOULD(VB_BUG_DECREMENT_LIFE_TIMER, CVAR, {
        const auto bug = va_arg(args, EnMushi2*);
        if (!bug->unk_34C && Player_InCsMode(gPlayState)) {
            *should = false;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterFasterSoftSoilSkulltula, { CVAR_NAME });