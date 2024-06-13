#ifndef ENHANCEMENTS_H
#define ENHANCEMENTS_H

#include "Camera/CameraInterpolationFixes.h"
#include "Camera/DebugCam.h"
#include "Camera/FreeLook.h"
#include "Cheats/MoonJump.h"
#include "Cheats/Infinite.h"
#include "Dialogue/Dialogue.h"
#include "Graphics/TextBasedClock.h"
#include "Cheats/Cheats.h"
#include "Cheats/UnbreakableRazorSword.h"
#include "Cheats/UnrestrictedItems.h"
#include "Cycle/EndOfCycle.h"
#include "Equipment/SkipMagicArrowEquip.h"
#include "Masks/BlastMaskKeg.h"
#include "Masks/FierceDeityAnywhere.h"
#include "Masks/NoBlastMaskCooldown.h"
#include "Masks/FastTransformation.h"
#include "Minigames/AlwaysWinDoggyRace.h"
#include "Cutscenes/Cutscenes.h"
#include "Restorations/FlipHopVariable.h"
#include "Restorations/PowerCrouchStab.h"
#include "Restorations/SideRoll.h"
#include "Restorations/TatlISG.h"
#include "Graphics/PlayAsKafei.h"
#include "Player/Player.h"
#include "Songs/EnableSunsSong.h"
#include "Saving/SavingEnhancements.h"
#include "Graphics/DisableBlackBars.h"
#include "Modes/TimeMovesWhenYouMove.h"

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
