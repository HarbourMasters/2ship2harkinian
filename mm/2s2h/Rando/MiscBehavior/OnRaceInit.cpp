#include "MiscBehavior.h"
#include "PresetManager/PresetManager.h"

extern "C" {
#include "variables.h"
}

static bool isRaceInitialized = false;

void Rando::MiscBehavior::OnRaceFileInit() {
    COND_HOOK(OnSaveLoad, RANDO_SAVE_OPTIONS[RO_LOGIC] == RO_LOGIC_VOYAGE_3,
              [](s16 fileNum) { isRaceInitialized = false; });

    COND_HOOK(OnSceneInit, IS_RANDO, [](s8 sceneId, s8 spawnNum) {
        if (!isRaceInitialized) {
            if (RANDO_SAVE_OPTIONS[RO_LOGIC] == RO_LOGIC_VOYAGE_3) {
                PresetManager_ApplyPreset(voyage3PresetJ);
            }
            isRaceInitialized = true;
        }
    });
}
