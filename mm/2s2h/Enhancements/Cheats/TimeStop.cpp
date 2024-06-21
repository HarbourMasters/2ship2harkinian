#include <libultraship/libultraship.h>
#include "BenPort.h"
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "Enhancements/Enhancements.h"

extern "C" {
#include <variables.h>
#include <functions.h>
}

void RegisterTimeStopInTemples() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s16 sceneId, s8 spawnNum) {
        uint8_t selectedOption = CVarGetInteger("gCheats.TempleTimeStop", 0);

        switch (selectedOption) {
            case TIME_STOP_TEMPLES_DUNGEONS:
                switch (gPlayState->sceneId) {
                    // Swamp Spider House
                    case SCENE_KINSTA1:
                    // Ocean Spider House
                    case SCENE_KINDAN2:
                    // Pirates' Fortress
                    case SCENE_KAIZOKU:
                    // Beneath the Well
                    case SCENE_REDEAD:
                    // Ancient Castle of Ikana
                    case SCENE_CASTLE:
                    // Igos du Ikana's Lair
                    case SCENE_IKNINSIDE:
                    // Secret Shrine
                    case SCENE_RANDOM:
                        R_TIME_SPEED = 0;
                        break;
                    default:
                        break;
                }
            case TIME_STOP_TEMPLES:
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
                break;
            case TIME_STOP_OFF:
                break;
            default:
                break;
        }
    });
}
