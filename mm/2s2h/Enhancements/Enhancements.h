#ifndef ENHANCEMENTS_H
#define ENHANCEMENTS_H

#include "Camera/DebugCam.h"
#include "Camera/FreeLook.h"
#include "Cheats/MoonJump.h"
#include "Cheats/Infinite.h"
#include "Graphics/TextBasedClock.h"
#include "Cheats/UnbreakableRazorSword.h"
#include "Cycle/EndOfCycle.h"
#include "Masks/FierceDeityAnywhere.h"
#include "Masks/NoBlastMaskCooldown.h"
#include "Masks/FastTransformation.h"
#include "Minigames/AlwaysWinDoggyRace.h"
#include "Cutscenes/Cutscenes.h"
#include "Restorations/PowerCrouchStab.h"
#include "Restorations/SideRoll.h"
#include "Restorations/TatlISG.h"
#include "Graphics/PlayAsKafei.h"
#include "PlayerMovement/ClimbSpeed.h"
#include "Songs/EnableSunsSong.h"

enum AlwaysWinDoggyRaceOptions {
    ALWAYS_WIN_DOGGY_RACE_OFF,
    ALWAYS_WIN_DOGGY_RACE_MASKOFTRUTH,
    ALWAYS_WIN_DOGGY_RACE_ALWAYS,
};

enum ClockTypeOptions {
    CLOCK_TYPE_ORIGINAL,
    CLOCK_TYPE_TEXT_BASED,
};

#ifdef __cplusplus
extern "C" {
#endif

void InitEnhancements();

#ifdef __cplusplus
}
#endif

#endif // ENHANCEMENTS_H
