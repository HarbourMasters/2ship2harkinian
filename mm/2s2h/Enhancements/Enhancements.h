#ifndef ENHANCEMENTS_H
#define ENHANCEMENTS_H

#include "Cheats/MoonJump.h"
#include "Cheats/Infinite.h"
#include "Masks/FierceDeityAnywhere.h"
#include "Minigames/AlwaysWinDoggyRace.h"
#include "TimeSavers/TimeSavers.h"

enum AlwaysWinDoggyRaceOptions {
    ALWAYS_WIN_DOGGY_RACE_OFF,
    ALWAYS_WIN_DOGGY_RACE_MASKOFTRUTH,
    ALWAYS_WIN_DOGGY_RACE_ALWAYS,
};

#ifdef __cplusplus
extern "C" {
#endif

void InitEnhancements();

#ifdef __cplusplus
}
#endif

#endif // ENHANCEMENTS_H
