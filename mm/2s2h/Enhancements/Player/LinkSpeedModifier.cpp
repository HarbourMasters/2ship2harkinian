#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenPort.h"

extern "C" {
#include "variables.h"
extern Input* sPlayerControlInput;
}

#define CVAR_SPEED_MODIFIER_NAME "gSettings.SpeedModifier.Enable"
#define CVAR_WALK_MODIFIER_NAME "gSettings.SpeedModifier.WalkEnable"
#define CVAR_SWIM_MODIFIER_NAME "gSettings.SpeedModifier.SwimEnable"
#define CVAR_SPEED CVarGetInteger(CVAR_SPEED_MODIFIER_NAME, 0)
#define CVAR_WALK CVarGetInteger(CVAR_WALK_MODIFIER_NAME, 0)
#define CVAR_SWIM CVarGetInteger(CVAR_SWIM_MODIFIER_NAME, 0)

bool speedToggle1;
bool speedToggle2;

void RegisterLinkSpeedModifier() {

    COND_VB_SHOULD(VB_SPEED_MODIFIER_WALK, CVAR_WALK && CVAR_SPEED, {
        f32* speedTarget = va_arg(args, f32*);

        if (CVarGetInteger("gSettings.SpeedModifier.Toggle", 0)) {
            if (speedToggle1) {
                *speedTarget *= CVarGetFloat("gSettings.SpeedModifier.WalkMapping1", 1.0f);
            } else if (speedToggle2) {
                *speedTarget *= CVarGetFloat("gSettings.SpeedModifier.WalkMapping2", 1.0f);
            }
        } else {
            if (CHECK_BTN_ALL(sPlayerControlInput->cur.button, BTN_CUSTOM_MODIFIER1)) {
                *speedTarget *= CVarGetFloat("gSettings.SpeedModifier.WalkMapping1", 1.0f);
            } else if (CHECK_BTN_ALL(sPlayerControlInput->cur.button, BTN_CUSTOM_MODIFIER2)) {
                *speedTarget *= CVarGetFloat("gSettings.SpeedModifier.WalkMapping2", 1.0f);
            }
        }
    });

    COND_VB_SHOULD(VB_SPEED_MODIFIER_SWIM, CVAR_SWIM && CVAR_SPEED, {
        Player* player = GET_PLAYER(gPlayState);
        f32* incrStep = va_arg(args, f32*);
        f32* maxSpeed = va_arg(args, f32*);
        f32* speed = va_arg(args, f32*);
        f32* speedTarget = va_arg(args, f32*);
        f32 swimMod = 1.0f;

        if (CVarGetInteger("gSettings.SpeedModifier.Toggle", 0)) {
            if (speedToggle1) {
                swimMod *= CVarGetFloat("gSettings.SpeedModifier.SwimMapping1", 1.0f);
            } else if (speedToggle2) {
                swimMod *= CVarGetFloat("gSettings.SpeedModifier.SwimMapping2", 1.0f);
            }

            // sControlInput is NULL to prevent inputs while surfacing after obtaining an underwater item so we want
            // to ignore it for that case
        } else if (sPlayerControlInput != NULL) {
            if (CHECK_BTN_ALL(sPlayerControlInput->cur.button, BTN_CUSTOM_MODIFIER1)) {
                swimMod *= CVarGetFloat("gSettings.SpeedModifier.SwimMapping1", 1.0f);
            } else if (CHECK_BTN_ALL(sPlayerControlInput->cur.button, BTN_CUSTOM_MODIFIER2)) {
                swimMod *= CVarGetFloat("gSettings.SpeedModifier.SwimMapping2", 1.0f);
            }
        }

        *maxSpeed *= swimMod;

        Math_AsymStepToF(speed, *speedTarget * 0.8f * swimMod, *incrStep, (fabsf(*speed) * 0.02f) + 0.05f);
    });

    COND_VB_SHOULD(VB_SPEED_MODIFIER_TOGGLE, CVAR_WALK && CVAR_SPEED || CVAR_SWIM && CVAR_SPEED, {
        if (CVarGetInteger("gSettings.SpeedModifier.Toggle", 0)) {
            if (CHECK_BTN_ALL(sPlayerControlInput->press.button, BTN_CUSTOM_MODIFIER1)) {
                speedToggle1 = !speedToggle1;
            }
            if (CHECK_BTN_ALL(sPlayerControlInput->press.button, BTN_CUSTOM_MODIFIER2)) {
                speedToggle2 = !speedToggle2;
            }
        }
    });

    COND_HOOK(OnConsoleLogoUpdate, CVAR_WALK && CVAR_SPEED || CVAR_SWIM && CVAR_SPEED, []() {
        speedToggle1 = 0;
        speedToggle2 = 0;
    });
}

static RegisterShipInitFunc initWalkSpeedFunc(RegisterLinkSpeedModifier,
                                              { CVAR_SPEED_MODIFIER_NAME, CVAR_WALK_MODIFIER_NAME, CVAR_SWIM_MODIFIER_NAME });