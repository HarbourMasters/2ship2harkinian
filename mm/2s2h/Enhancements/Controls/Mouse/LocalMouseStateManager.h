#pragma once

#include <ship/window/MouseStateManager.h>

class LocalMouseStateManager : public Ship::MouseStateManager {
    void ToggleMouseCaptureOverride() override;
    void UpdateMouseCapture() override;
};
