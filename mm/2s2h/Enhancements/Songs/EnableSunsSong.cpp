#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

extern "C" {
#include <z64ocarina.h>
}

#define CVAR_NAME CVAR_ENHANCEMENT("Songs.EnableSunsSong")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterEnableSunsSong() {
    COND_VB_SHOULD(VB_SONG_AVAILABLE_TO_PLAY, CVAR, {
        uint8_t* songIndex = va_arg(args, uint8_t*);
        // If the currently played song is Sun's Song, set it to be available to be played.
        if (*songIndex == OCARINA_SONG_SUNS) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterEnableSunsSong, { CVAR_NAME });
