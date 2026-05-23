#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenPort.h"
#include <math.h>

#include <SDL2/SDL.h>
#include <imgui.h>
#include <ship/utils/StringHelper.h>
#include <ship/window/gui/IconsFontAwesome4.h>

extern "C" {
#include "variables.h"
}

#define CVAR_NAME "gEnhancements.Player.ExtKeyboardControls"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define CVAR_ESS_NAME "gEnhancements.Player.ExtKeyboardEss"
#define CVAR_ESS CVarGetInteger(CVAR_ESS_NAME, 0)
#define CVAR_ESS_BTN_NAME "gEnhancements.Player.ExtKeyboardEssBtn"
#define CVAR_ESS_BTN CVarGetInteger(CVAR_ESS_BTN_NAME, BTN_CUSTOM_MODIFIER1)

#define CVAR_SPIN_NAME "gEnhancements.Player.ExtKeyboardSpin"
#define CVAR_SPIN CVarGetInteger(CVAR_SPIN_NAME, 0)
#define CVAR_SPIN_BTN_NAME "gEnhancements.Player.ExtKeyboardSpinBtn"
#define CVAR_SPIN_BTN CVarGetInteger(CVAR_SPIN_BTN_NAME, BTN_CUSTOM_MODIFIER2)

#define CVAR_NOTCH_NAME "gEnhancements.Player.ExtKeyboardNotchFix"
#define CVAR_NOTCH CVarGetInteger(CVAR_NOTCH_NAME, 0)

#define CVAR_HALF_STICK_NAME "gEnhancements.Player.ExtKeyboardHalfStick"
#define CVAR_HALF_STICK CVarGetInteger(CVAR_HALF_STICK_NAME, 0)
#define CVAR_HALF_LEFT_KEY "gEnhancements.Player.ExtKeyboardHalfLeftKey"
#define CVAR_HALF_RIGHT_KEY "gEnhancements.Player.ExtKeyboardHalfRightKey"
#define CVAR_HALF_UP_KEY "gEnhancements.Player.ExtKeyboardHalfUpKey"
#define CVAR_HALF_DOWN_KEY "gEnhancements.Player.ExtKeyboardHalfDownKey"

#define ESS_MAGNITUDE 17
#define HALF_STICK_VALUE 35

static inline int clampStick(int val) {
    return (val > 127) ? 127 : (val < -128) ? -128 : val;
}

static inline void clampMagnitude(int* x, int* y, int maxMag) {
    float fx = (float)*x;
    float fy = (float)*y;
    float mag = sqrtf(fx * fx + fy * fy);
    if (mag > (float)maxMag) {
        float scale = (float)maxMag / mag;
        *x = (int)(fx * scale);
        *y = (int)(fy * scale);
    }
}

void RegisterExtendedKeyboardControls() {
    COND_HOOK(OnPassPlayerInputs, CVAR, [](Input* input) {
        // Half Stick: keyboard keys that add +/-35 to each axis.
        // Uses raw SDL scancodes because the button system doesn't support analog injection.
        if (CVAR_HALF_STICK) {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            int sx = input->cur.stick_x;
            int sy = input->cur.stick_y;
            int origSx = sx;
            int origSy = sy;
            int32_t kL = CVarGetInteger(CVAR_HALF_LEFT_KEY, 0);
            int32_t kR = CVarGetInteger(CVAR_HALF_RIGHT_KEY, 0);
            int32_t kU = CVarGetInteger(CVAR_HALF_UP_KEY, 0);
            int32_t kD = CVarGetInteger(CVAR_HALF_DOWN_KEY, 0);

            if (kL > 0 && keys[kL])
                sx -= HALF_STICK_VALUE;
            if (kR > 0 && keys[kR])
                sx += HALF_STICK_VALUE;
            if (kU > 0 && keys[kU])
                sy += HALF_STICK_VALUE;
            if (kD > 0 && keys[kD])
                sy -= HALF_STICK_VALUE;

            if (sx != origSx || sy != origSy) {
                clampMagnitude(&sx, &sy, 72);
                input->cur.stick_x = (s8)clampStick(sx);
                input->cur.stick_y = (s8)clampStick(sy);
                input->rel.stick_x = input->cur.stick_x;
                input->rel.stick_y = input->cur.stick_y;
            }
        }

        // Notch Correction: zeros X-axis on the A-press frame when input is a near-exact diagonal,
        // so down-diagonals produce drifited backflips instead of sidehops.
        if (CVAR_NOTCH && CHECK_BTN_ALL(input->press.button, BTN_A)) {
            int x = input->cur.stick_x;
            int y = input->cur.stick_y;
            int ax = (x > 0) ? x : -x;
            int ay = (y > 0) ? y : -y;
            if (y < 0 && ax > 10 && ay > 10 && (ax - ay > -10 && ax - ay < 10)) {
                input->cur.stick_x = 0;
            }
        }

        // ESS: normalize stick to 17/127. Runs last so it overrides half-stick/notch.
        if (CVAR_ESS && CHECK_BTN_ALL(input->cur.button, CVAR_ESS_BTN)) {
            s8 x = input->cur.stick_x;
            s8 y = input->cur.stick_y;
            if (x != 0 || y != 0) {
                float mag = sqrtf((float)(x * x + y * y));
                input->cur.stick_x = (s8)((x / mag) * ESS_MAGNITUDE);
                input->cur.stick_y = (s8)((y / mag) * ESS_MAGNITUDE);
                input->rel.stick_x = input->cur.stick_x;
                input->rel.stick_y = input->cur.stick_y;
            }
        }
    });

    COND_VB_SHOULD(VB_PLAYER_CAN_SPIN_ATTACK, CVAR, {
        if (CVAR_SPIN && gPlayState != NULL) {
            Input* input = CONTROLLER1(&gPlayState->state);
            if (CHECK_BTN_ALL(input->cur.button, CVAR_SPIN_BTN)) {
                *should = true;
            }
        }
    });
}

void ExtendedKeyboardControls_RenderHalfStickKeysWidget() {
    struct KeyBinding {
        const char* label;
        const char* cvar;
    };
    static const KeyBinding bindings[] = {
        { "Half Left", "gEnhancements.Player.ExtKeyboardHalfLeftKey" },
        { "Half Right", "gEnhancements.Player.ExtKeyboardHalfRightKey" },
        { "Half Up", "gEnhancements.Player.ExtKeyboardHalfUpKey" },
        { "Half Down", "gEnhancements.Player.ExtKeyboardHalfDownKey" },
    };
    static int pendingBindIndex = -1;

    for (int i = 0; i < 4; i++) {
        int32_t scancode = CVarGetInteger(bindings[i].cvar, 0);
        const char* keyName = scancode > 0 ? SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)scancode)) : "None";

        ImGui::Text("%s:", bindings[i].label);
        ImGui::SameLine();

        auto popupId = StringHelper::Sprintf("##extKeyBind%d", i);
        if (pendingBindIndex == i) {
            ImGui::Button(StringHelper::Sprintf("Press a key...%s", popupId.c_str()).c_str());
            int numkeys;
            const Uint8* keys = SDL_GetKeyboardState(&numkeys);
            for (int k = 4; k < numkeys; k++) {
                if (keys[k]) {
                    CVarSetInteger(bindings[i].cvar, k);
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                    pendingBindIndex = -1;
                    break;
                }
            }
        } else {
            if (ImGui::Button(StringHelper::Sprintf("%s%s", keyName, popupId.c_str()).c_str())) {
                pendingBindIndex = i;
            }
            if (scancode > 0) {
                ImGui::SameLine();
                if (ImGui::Button(StringHelper::Sprintf("%s##extKeyClear%d", ICON_FA_TIMES, i).c_str())) {
                    CVarSetInteger(bindings[i].cvar, 0);
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                }
            }
        }
    }
}

static RegisterShipInitFunc initFunc(RegisterExtendedKeyboardControls, { CVAR_NAME, CVAR_ESS_NAME, CVAR_SPIN_NAME,
                                                                         CVAR_NOTCH_NAME, CVAR_HALF_STICK_NAME });
