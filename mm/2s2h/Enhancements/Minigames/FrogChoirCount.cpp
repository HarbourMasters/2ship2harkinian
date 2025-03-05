#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/actors/ovl_En_Minifrog/z_en_minifrog.h"

// void func_8095345C(EnHs* enHs, PlayState* play);
}

#define CVAR_NAME "gEnhancements.Minigames.FrogChoirCount"
#define CVAR CVarGetInteger(CVAR_NAME, 5)

u8 SavedFrogs() {
    u8 saved = 1;
    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_32_40)) {
        saved++;
    }
    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_32_80)) {
        saved++;
    }
    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_33_01)) {
        saved++;
    }
    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_33_02)) {
        saved++;
    }
    return saved;
}

void RegisterFrogChoirCount() {
    // will require a scene reload
    COND_VB_SHOULD(VB_FROG_SAVED, CVAR < 5, {
        if (SavedFrogs() >= CVAR) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterFrogChoirCount, { CVAR_NAME });
