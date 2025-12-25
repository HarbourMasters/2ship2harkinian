#include "Mouse.h"

#include "ship/Context.h"
#include "ShipInit.hpp"
#include "GameInteractor/GameInteractor.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include <libultraship/bridge/consolevariablebridge.h>

static MouseCoords current;

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

void Mouse_UpdateCaptureByState() {
    // checks:
    // - forced via F2
    // - game start (forced)
    // - in pause (kaleido)
    // - in menu
    // - fullscreen mode
}

void HandlePauseCapture(PauseContext* pauseCtx) {
    static bool buf = false;
    static bool paused = false;

    switch (pauseCtx->state) {
        case PAUSE_STATE_MAIN: {
            if (paused) { return; }
            paused = true;

            std::shared_ptr<Ship::Window> window = Ship::Context::GetInstance()->GetWindow();
            bool current = window->IsMouseCaptured();
            buf = current;
            if (current) {
                window->SetMouseCapture(false);
            }
            return;
        }
        case PAUSE_STATE_UNPAUSE_SETUP:
            Ship::Context::GetInstance()->GetWindow()->SetMouseCapture(buf);
            paused = false;
            return;
    }
}

void RegisterMouseRelatedHooks() {
    COND_HOOK(
        OnKaleidoUpdate,
        true,
        HandlePauseCapture
    );
}

// OnGameStateMainStart (capture toggle on start?)

static RegisterShipInitFunc initFunc(RegisterMouseRelatedHooks, {});

#ifdef __cplusplus
} // extern "C"
#endif
