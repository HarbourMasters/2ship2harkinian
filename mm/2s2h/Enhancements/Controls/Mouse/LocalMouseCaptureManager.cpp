#include "LocalMouseCaptureManager.h"

#include <fast/Fast3dWindow.h>
#include "ship/Context.h"
#include "Mouse.h"

void LocalMouseCaptureManager::ToggleMouseCaptureOverride() {
    Mouse_ForceToggleCapture();
}

void LocalMouseCaptureManager::UpdateMouseCapture() {
    Mouse_UpdateCaptureByState();
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow());
    if (wnd->IsMouseCaptured()) {
        SetCursorVisibleTicks(GetCursorVisibilityTime() * wnd->GetTargetFps());
    }
}
