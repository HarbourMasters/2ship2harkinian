#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
    #include <z64ocarina.h>
}

void RegisterEnableSunsSong() {
    REGISTER_VB_SHOULD(GI_VB_SONG_AVAILABLE_TO_PLAY, {
        // If the enhancement is on, and the currently played song is Sun's Song, set it to be available to be played.
        if (CVarGetInteger("gEnhancements.Songs.EnableSunsSong", 0) && (uint8_t)opt == OCARINA_SONG_SUNS) {
            *should = true;
        }
    });
}
