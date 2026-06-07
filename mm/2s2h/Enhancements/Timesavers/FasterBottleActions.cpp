#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Timesavers.FasterBottleActions"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// Speed multiplier for bottle animations applied during the "windup" portion of each animation.  Reset to 1.0 before
// the frame where actors spawn to preserve spawn timing.
static constexpr float sBottleAnimSpeedMultiplier = 3.0f;

void RegisterFasterBottleActions() {
    // Action_67: Drinking from bottle.
    // Multiphase animation: start -> drink loop (waits for HP fill) -> end.  Only speed up the start and end
    // animations; the loop phase is gated on health/magic accumulation and should run at normal speed.
    COND_VB_SHOULD(VB_DRINKING_BOTTLE, CVAR, {
        const auto player = va_arg(args, Player*);
        // av2.actionVar2 tracks the drink phase:
        // < 0 = Deku form end sequence
        //   0 = Initial        (start animation playing)
        //   1 = Drink loop     (waiting for HP/magic fill)
        //   2 = End animation  (all other forms)
        //   3 = Final transition
        player->skelAnime.playSpeed = player->av2.actionVar2 != 1 ? sBottleAnimSpeedMultiplier : 1.0f;
    });

    // Action_69: Releasing Fairy from bottle.
    // Actor spawn occurs on frame 37.  Speed up frames <= 28, then restore.
    COND_VB_SHOULD(VB_RELEASING_BOTTLE_FAIRY, CVAR, {
        const auto player = va_arg(args, Player*);
        player->skelAnime.playSpeed = player->skelAnime.curFrame <= 28.0f ? sBottleAnimSpeedMultiplier : 1.0f;
    });

    // Action_70: Emptying bottle contents.
    // Actor spawn occurs on frame 76.  Speed up frames <= 60, then restore.
    COND_VB_SHOULD(VB_EMPTYING_BOTTLE, CVAR, {
        const auto player = va_arg(args, Player*);
        player->skelAnime.playSpeed = player->skelAnime.curFrame <= 60.0f ? sBottleAnimSpeedMultiplier : 1.0f;
    });
}

static RegisterShipInitFunc initFunc(RegisterFasterBottleActions, { CVAR_NAME });