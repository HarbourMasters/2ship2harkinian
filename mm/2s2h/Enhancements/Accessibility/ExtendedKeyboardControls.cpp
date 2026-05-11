#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenPort.h"
#include <math.h>

#include <SDL2/SDL.h>

extern "C" {
#include "variables.h"
}

// ===== CVars =====

#define CVAR_NAME "gEnhancements.Player.ExtKeyboardControls"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// ESS and Quick Spin use the engine's button system (works with keyboard, mouse, or gamepad).
// Mapped via WIDGET_CVAR_BTN_SELECTOR in BenMenu — defaults to Modifier 1 and 2.
#define CVAR_ESS_BTN_NAME "gEnhancements.Player.ExtKeyboardEssBtn"
#define CVAR_ESS_BTN CVarGetInteger(CVAR_ESS_BTN_NAME, BTN_CUSTOM_MODIFIER1)
#define CVAR_SPIN_BTN_NAME "gEnhancements.Player.ExtKeyboardSpinBtn"
#define CVAR_SPIN_BTN CVarGetInteger(CVAR_SPIN_BTN_NAME, BTN_CUSTOM_MODIFIER2)

// Diagonal notch correction — separate toggle since it changes gameplay behavior.
#define CVAR_NOTCH_NAME "gEnhancements.Player.ExtKeyboardNotchFix"
#define CVAR_NOTCH CVarGetInteger(CVAR_NOTCH_NAME, 0)

// Half-stick keys are raw SDL scancodes (keyboard-only, 0 = unbound).
// These don't go through the engine's button system because they inject
// analog stick values, which the button mapping system doesn't support.
#define CVAR_HALF_LEFT_KEY "gEnhancements.Player.ExtKeyboardHalfLeftKey"
#define CVAR_HALF_RIGHT_KEY "gEnhancements.Player.ExtKeyboardHalfRightKey"
#define CVAR_HALF_UP_KEY "gEnhancements.Player.ExtKeyboardHalfUpKey"
#define CVAR_HALF_DOWN_KEY "gEnhancements.Player.ExtKeyboardHalfDownKey"

// ===== Constants =====

// 17/127 is the ESS position — the exact stick magnitude for Extended Superslide.
#define ESS_MAGNITUDE 17

// ~35/127 gives useful diagonal angles when combined with full-axis keyboard input.
// Keyboard normally only gives 0 or full (±69 after octagonal bounding) on each axis.
#define HALF_STICK_VALUE 35

// ===== Helpers =====

// Clamp to s8 range after half-stick arithmetic. Defensive — keyboard max is ~69
// and half-stick adds 35, so 104 fits, but this guards against edge cases.
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

// ===== Registration =====

void RegisterExtendedKeyboardControls() {
    // Input hook: ESS modifier, half-stick, and notch correction.
    // All three modify the Input struct before Player_ProcessControlStick runs,
    // so the engine sees the adjusted values naturally.
    // Evaluation order: Half Stick → Notch Correction → ESS.
    // Half stick is lowest priority (base input adjustment).
    // Notch correction modifies the combined result on the A-press frame.
    // ESS is highest priority (normalizes the final result to 17/127).
    // Quick spin doesn't modify stick — it uses a VB hook below.
    COND_HOOK(OnPassPlayerInputs, CVAR, [](Input* input) {
        // 1. Half Stick — keyboard keys that add ±35 to each axis.
        // On keyboard, stick input is binary (0 or full). These keys let you
        // hit intermediate angles useful for diagonal movement setups.
        // Uses raw SDL scancodes because the engine's button system doesn't
        // support analog stick injection.
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

        // 2. Diagonal Notch Correction — keyboard produces perfect 45° diagonals
        // that always resolve to a sidehop on down-left/down-right.
        // This zeros out X on the frame A is pressed so the engine sees pure-down (backflip).
        // Without this you'd need an awkward down to down-diagonal input to
        // get a down-diagonal backflip.
        //
        // Only fires on near-exact diagonals (both axes active, magnitudes within
        // 10 of each other). Analog sticks never hit exact 45° so this is
        // effectively keyboard-only.
        if (CVAR_NOTCH && CHECK_BTN_ALL(input->press.button, BTN_A)) {
            s8 x = input->cur.stick_x;
            s8 y = input->cur.stick_y;
            s8 ax = (x > 0) ? x : -x;
            s8 ay = (y > 0) ? y : -y;
            if (y < 0 && ax > 10 && ay > 10 && (ax - ay > -10 && ax - ay < 10)) {
                input->cur.stick_x = 0;
            }
        }

        // 3. ESS Position — hold the ESS button to normalize the stick to 17/127.
        // Highest priority: runs last so it normalizes the final combined input
        // from half-stick, notch correction, etc. Both cur and rel must be set —
        // Lib_GetControlStickData reads rel for magnitude and cur for angle.
        if (CHECK_BTN_ALL(input->cur.button, CVAR_ESS_BTN)) {
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

    // Instant Quick Spin / JS Cancel — hold button to force spin attack on any B press.
    //
    // Normally a quick spin requires rotating the control stick through 4+ angles
    // in the frames before pressing B (Player_CanSpinAttack checks the
    // controlStickSpinAngles ring buffer). This is impractical on keyboard.
    //
    // We use VB_PLAYER_CAN_SPIN_ATTACK to override the rotation check result
    // when the spin button is held. The engine handles everything else — animation,
    // damage, sword beam checks — identically to a real stick-rotation spin.
    // Also enables JS (jumpslash) cancel to regain control faster
    // which uses the same spin input.
    COND_VB_SHOULD(VB_PLAYER_CAN_SPIN_ATTACK, CVAR, {
        if (gPlayState != NULL) {
            Input* input = CONTROLLER1(&gPlayState->state);
            if (CHECK_BTN_ALL(input->cur.button, CVAR_SPIN_BTN)) {
                *should = true;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterExtendedKeyboardControls, { CVAR_NAME, CVAR_NOTCH_NAME });
