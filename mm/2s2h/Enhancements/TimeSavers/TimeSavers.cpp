#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterTimeSaversHooks() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(GI_VB_PLAY_ENTRANCE_CS, [](GIVanillaBehavior _, bool* should, void* __) {
        if (CVarGetInteger("gEnhancements.TimeSavers.SkipEntranceCutscenes", 0)) {
            *should = false;
        }
    });
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(GI_VB_SHOW_TITLE_CARD, [](GIVanillaBehavior _, bool* should, void* __) {
        if (CVarGetInteger("gEnhancements.TimeSavers.HideTitleCards", 0)) {
            *should = false;
        }
    });
}
