#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterClimbSpeed() {
    REGISTER_VB_SHOULD(GI_VB_SET_CLIMB_SPEED, {
        if (CVarGetInteger("gEnhancements.PlayerMovement.ClimbSpeed", 1) >1) {
            f32* speed = static_cast<f32*>(opt);
            *speed *= CVarGetInteger("gEnhancements.PlayerMovement.ClimbSpeed", 1);
        }
    });
}
