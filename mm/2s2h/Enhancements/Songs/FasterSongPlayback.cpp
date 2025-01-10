#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
extern u8 sPlaybackState;
}

#define CVAR_NAME "gEnhancements.Songs.FasterSongPlayback"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define NOT_OCARINA_ACTION_BALAD_WIND_FISH                                       \
    (gPlayState->msgCtx.ocarinaAction < OCARINA_ACTION_PROMPT_WIND_FISH_HUMAN || \
     gPlayState->msgCtx.ocarinaAction > OCARINA_ACTION_PROMPT_WIND_FISH_DEKU)

void RegisterFasterSongPlayback() {
    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, CVAR, [](Actor* actor) {
        if (gPlayState->msgCtx.msgMode >= MSGMODE_SONG_PLAYED && gPlayState->msgCtx.msgMode <= MSGMODE_17 &&
            !gPlayState->csCtx.state && NOT_OCARINA_ACTION_BALAD_WIND_FISH) {
            if (gPlayState->msgCtx.stateTimer > 1) {
                gPlayState->msgCtx.stateTimer = 1;
            }
            gPlayState->msgCtx.ocarinaSongEffectActive = 0;
            sPlaybackState = 0;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterFasterSongPlayback, { CVAR_NAME });
