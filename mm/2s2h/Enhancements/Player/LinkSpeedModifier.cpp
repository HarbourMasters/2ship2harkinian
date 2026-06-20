#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenPort.h"

extern "C" {
#include "variables.h"
extern Input* sPlayerControlInput;
}

#define CVAR_SPEED_MODIFIER_MODE_NAME "gCheats.SpeedModifier.Mode"
#define CVAR_SPEED_MODIFIER_TOGGLE_NAME "gCheats.SpeedModifier.Toggle"
#define CVAR_SPEED_MODIFIER_VALUE_NAME "gCheats.SpeedModifier.Value"
#define CVAR_SPEED_MODIFIER_BTN_NAME "gCheats.SpeedModifier.Btn"
#define CVAR_SPEED_MODIFIER_DOESNT_CHANGE_JUMP_NAME "gCheats.SpeedModifier.DoesntChangeJump"
#define CVAR_SPEED_MODIFIER_MODE CVarGetInteger(CVAR_SPEED_MODIFIER_MODE_NAME, 0)
#define CVAR_SPEED_MODIFIER_TOGGLE CVarGetInteger(CVAR_SPEED_MODIFIER_TOGGLE_NAME, 0)
#define CVAR_SPEED_MODIFIER_VALUE CVarGetFloat(CVAR_SPEED_MODIFIER_VALUE_NAME, 1.0f)
#define CVAR_SPEED_MODIFIER_BTN CVarGetInteger(CVAR_SPEED_MODIFIER_BTN_NAME, BTN_CUSTOM_MODIFIER1)
#define CVAR_SPEED_MODIFIER_DOESNT_CHANGE_JUMP CVarGetInteger(CVAR_SPEED_MODIFIER_DOESNT_CHANGE_JUMP_NAME, 0)

bool btnHeldOrToggled = false;

static bool IsSpeedModifierActive() {
    return CVAR_SPEED_MODIFIER_MODE == 1 || btnHeldOrToggled;
}

void RegisterLinkSpeedModifier() {
    // Reset in case they disabled while toggled
    btnHeldOrToggled = false;

    COND_VB_SHOULD(VB_SPEED_MODIFIER_WALK, CVAR_SPEED_MODIFIER_MODE, {
        f32* speedTarget = va_arg(args, f32*);

        if (IsSpeedModifierActive()) {
            *speedTarget *= CVAR_SPEED_MODIFIER_VALUE;
        }
    });

    COND_VB_SHOULD(VB_SPEED_MODIFIER_SWIM, CVAR_SPEED_MODIFIER_MODE, {
        f32* incrStep = va_arg(args, f32*);
        f32* maxSpeed = va_arg(args, f32*);
        f32* speed = va_arg(args, f32*);
        f32* speedTarget = va_arg(args, f32*);
        f32 swimMod = 1.0f;

        // sControlInput is NULL to prevent inputs while surfacing after obtaining an underwater item so we want
        // to ignore it for that case
        if (sPlayerControlInput == NULL) {
            return;
        }

        if (IsSpeedModifierActive()) {
            swimMod *= CVAR_SPEED_MODIFIER_VALUE;
            *maxSpeed *= swimMod;
            Math_AsymStepToF(speed, *speedTarget * 0.8f * swimMod, *incrStep, (fabsf(*speed) * 0.02f) + 0.05f);
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_SPEED_MODIFIER_JUMP, CVAR_SPEED_MODIFIER_MODE && CVAR_SPEED_MODIFIER_DOESNT_CHANGE_JUMP, {
        f32* speedXZ = va_arg(args, f32*);

        if (IsSpeedModifierActive() && CVAR_SPEED_MODIFIER_VALUE != 0.0f) {
            *speedXZ /= CVAR_SPEED_MODIFIER_VALUE;
        }
    });

    COND_HOOK(OnPassPlayerInputs, CVAR_SPEED_MODIFIER_MODE >= 2, [](Input* input) {
        const s32 modMask = CVAR_SPEED_MODIFIER_BTN;

        if (modMask == 0) {
            btnHeldOrToggled = false;
            return;
        }

        if (CVAR_SPEED_MODIFIER_MODE == 2) {
            btnHeldOrToggled = CHECK_BTN_ANY(input->cur.button, modMask);
        } else if (CHECK_BTN_ANY(input->press.button, modMask)) {
            btnHeldOrToggled = !btnHeldOrToggled;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterLinkSpeedModifier, { CVAR_SPEED_MODIFIER_MODE_NAME });