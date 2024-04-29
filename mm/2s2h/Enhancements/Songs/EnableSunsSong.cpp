#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
    #include <z64save.h>
    extern SaveContext gSaveContext;
    extern u32 gBitFlags[32];
}

void RegisterEnableSunsSong() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s8 sceneId, s8 spawnNum) {
        if (CVarGetInteger("gEnhancements.Songs.EnableSunsSong", 0)) {
            SET_QUEST_ITEM(QUEST_SONG_SUN);
        } else {
            REMOVE_QUEST_ITEM(QUEST_SONG_SUN);
        }
    });
}
