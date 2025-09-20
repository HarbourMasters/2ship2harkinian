#include "MiscBehavior.h"

extern "C" {
#include "functions.h"
#include "variables.h"
#include "overlays/gamestates/ovl_daytelop/z_daytelop.h"
}

void Rando::MiscBehavior::OnSceneInit(s16 sceneId, s8 spawnNum) {
    InterfaceContext* interfaceCtx = &gPlayState->interfaceCtx;

    // Remove mask restrictions for Clock Tower rooftop
    if (sceneId == SCENE_OKUJOU) {
        interfaceCtx->restrictions.masks = 0;
    }

    // ClockShuffle daytelop handling removed
}
