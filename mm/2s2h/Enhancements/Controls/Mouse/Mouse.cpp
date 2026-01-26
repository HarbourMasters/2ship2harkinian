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

void Mouse_ForceToggleCapture() {
    if (gameState.isCaptureForced) {
        gameState.isCaptureForced = false;
    } else {
        gameState.isCaptureForced = true;
        gameState.forcedCaptureState = !Ship::Context::GetInstance()->GetWindow()->IsMouseCaptured();
    }
    Mouse_UpdateCaptureByState();
}

void Mouse_UpdateCaptureByState() {
    // checks:
    // - [x] forced via F2
    // - [ ] game start
    // - [x] in pause (kaleido)
    // - [x] in menu
    // - [ ] fullscreen
    // - [ ] forced on app start
    bool capture;
    bool inBenMenu = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetMenuOrMenubarVisible();

    // FIXME: stub
    gameState.gameStarted = true;

    if (gameState.isCaptureForced) {
        capture = gameState.forcedCaptureState;
    } else if (Ship::Context::GetInstance()->GetWindow()->IsFullscreen()) {
        capture = !inBenMenu;
    } else if (inBenMenu || gameState.inKaleido) {
        capture = false;
    } else if (!gameState.gameStarted) {
        capture = false;
    } else {
        capture = true;
    }
    Ship::Context::GetInstance()->GetWindow()->SetMouseCapture(capture);
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
}

// OnGameStateMainStart (capture toggle on start?)

static RegisterShipInitFunc initFunc(RegisterMouseRelatedHooks, {});

#ifdef __cplusplus
} // extern "C"
#endif
