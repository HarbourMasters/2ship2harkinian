#include <libultraship/libultraship.h>
#include "BenPort.h"
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include <variables.h>
#include <functions.h>
}

void RegisterTimeStopInTemples() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s16 sceneId, s8 spawnNum) {
        if (CVarGetInteger("gCheats.TempleTimeStop", 0)) {

            switch (gPlayState->sceneId) {
                // Woodfall Temple + Odolwa
                case SCENE_MITURIN:
                case SCENE_MITURIN_BS:
                // Snowhead Temple + Goht
                case SCENE_HAKUGIN:
                case SCENE_HAKUGIN_BS:
                // Great Bay Temple + Gyorg
                case SCENE_SEA:
                case SCENE_SEA_BS:
                // Stone Tower Temple (+ inverted) + Twinmold
                case SCENE_INISIE_N:
                case SCENE_INISIE_R:
                case SCENE_INISIE_BS:
                    R_TIME_SPEED = 0;
                    break;
                default:
                    break;
            }
        }
    });
}
