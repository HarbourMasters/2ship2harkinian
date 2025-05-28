#include "public/bridge/consolevariablebridge.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
}

#define CVAR_NAME_FASTER_SCENE_TRANSITIONS "gEnhancements.Timesavers.FasterSceneTransitions"
#define CVAR_NAME_PAUSE_SAVE "gEnhancements.Saving.PauseSave"
#define CVAR_NAME_DEBUG_MODE "gDeveloperTools.DebugEnabled"

#define CVAR_FASTER_SCENE_TRANSITIONS CVarGetInteger(CVAR_NAME_FASTER_SCENE_TRANSITIONS, 0)
#define CVAR_PAUSE_SAVE CVarGetInteger(CVAR_NAME_PAUSE_SAVE, 0)
#define CVAR_DEBUG_MODE CVarGetInteger(CVAR_NAME_DEBUG_MODE, 0)

/*
 * Certain 2ship features circumvent sequence player state manipulation: debug warping, loading into a dungeon via
 * pause save, and faster scene transitions for example. Something about these different features causes
 * SEQ_PLAYER_BGM_MAIN to have the wrong isSeqPlayerInit and enabled flags. A result of this is that the active main BGM
 * sequence is seen as NA_BGM_DISABLED even if BGM is playing. Attempts to store and restore BGM from this point will
 * play silence, as seen with mini-boss battles.
 *
 * Rather than get in the weeds of tracking audio state information between different enhancements, a simple workaround
 * is to call AudioScript_SequencePlayerDisable in an OnSceneInit hook. This forces SEQ_PLAYER_BGM_MAIN to reset its
 * state information.
 *
 * Enabled when any of the enhancements in question are enabled.
 */

void RegisterFixBgmReplay() {
    COND_HOOK(
        OnSceneInit, CVAR_FASTER_SCENE_TRANSITIONS || CVAR_PAUSE_SAVE || CVAR_DEBUG_MODE,
        [](s8 sceneId, s8 spawnNum) { AudioScript_SequencePlayerDisable(&gAudioCtx.seqPlayers[SEQ_PLAYER_BGM_MAIN]); });
}

static RegisterShipInitFunc initFunc(RegisterFixBgmReplay, { CVAR_NAME_FASTER_SCENE_TRANSITIONS, CVAR_NAME_PAUSE_SAVE,
                                                             CVAR_NAME_DEBUG_MODE });
