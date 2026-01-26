#include "Mouse.h"

#include "ship/Context.h"
#include "ShipInit.hpp"
#include "GameInteractor/GameInteractor.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include <libultraship/bridge/consolevariablebridge.h>

static MouseCoords current;
static MouseCaptureGameState gameState;

#ifdef __cplusplus
extern "C" {
#endif

void Mouse_Update() {
    Ship::Coords coords = Ship::Context::GetInstance()->GetWindow()->GetMouseDelta();
    current.x = coords.x;
    current.y = coords.y;
}

MouseCoords Mouse_GetDelta() {
    return current;
}

MouseCoords Mouse_GetPos() {
    Ship::Coords coords = Ship::Context::GetInstance()->GetWindow()->GetMousePos();
    return { coords.x, coords.y };
}

void Mouse_SetCursorPos(s32 x, s32 y) {
    Ship::Context::GetInstance()->GetWindow()->SetMousePos({ x, y });
}

bool Mouse_IsCaptured() {
    return Ship::Context::GetInstance()->GetWindow()->IsMouseCaptured();
}

bool InferCaptureFromState(MouseCaptureGameState state) {
    // TODO: forced on app start?
    bool capture;
    bool inBenMenu = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetMenuOrMenubarVisible();

    if (state.isCaptureForced) {
        capture = state.forcedCaptureState;
    } else if (inBenMenu || state.inKaleido) {
        capture = false;
    } else if (!state.gameStarted) {
        capture = false;
    } else {
        capture = true;
    }
    return capture;
}

void Mouse_ForceToggleCapture() {
    if (gameState.isCaptureForced) {
        MouseCaptureGameState unforcedState = gameState;
        unforcedState.isCaptureForced = false;
        if (gameState.forcedCaptureState == InferCaptureFromState(unforcedState)) {
            gameState.forcedCaptureState = !gameState.forcedCaptureState;
        } else {
            gameState.isCaptureForced = false;
        }
    } else {
        gameState.isCaptureForced = true;
        gameState.forcedCaptureState = !Ship::Context::GetInstance()->GetWindow()->IsMouseCaptured();
    }
    Mouse_UpdateCaptureByState();
}

void Mouse_UpdateCaptureByState() {
    bool capture = InferCaptureFromState(gameState);

    if (
        gameState.isCaptureForced
        || !capture
        || Ship::Context::GetInstance()->GetWindow()->GetMouseCaptureManager()->ShouldAutoCaptureMouse()
    ) {
        Ship::Context::GetInstance()->GetWindow()->SetMouseCapture(capture);
    }
}

void HandleKaleidoCapture(PauseContext* pauseCtx) {
    bool prev = gameState.inKaleido;
    switch (pauseCtx->state) {
        case PAUSE_STATE_MAIN: {
            gameState.inKaleido = true;
            break;
        }
        case PAUSE_STATE_UNPAUSE_SETUP: {
            gameState.inKaleido = false;
            break;
        }
    }
    if (prev != gameState.inKaleido) {
        Mouse_UpdateCaptureByState();
    }
}

void RegisterMouseRelatedHooks() {
    COND_HOOK(
        OnKaleidoUpdate,
        true,
        HandleKaleidoCapture
    );
    COND_HOOK(
        OnSaveLoad,
        true,
        [](s16 fileNum) {
            gameState.gameStarted = true;
            Mouse_UpdateCaptureByState();
        }
    );
    COND_HOOK(
        OnConsoleLogoUpdate,
        true,
        []() {
            if (gameState.gameStarted) {
                gameState.gameStarted = false;
                gameState.inKaleido = false;
                Mouse_UpdateCaptureByState();
            }
        }
    );
}

// OnGameStateMainStart (capture toggle on start?)

static RegisterShipInitFunc initFunc(RegisterMouseRelatedHooks, {});

#ifdef __cplusplus
} // extern "C"
#endif
