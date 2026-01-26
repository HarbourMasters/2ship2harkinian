#pragma once

#include <ship/window/MouseCaptureManager.h>

class LocalMouseCaptureManager : public Ship::MouseCaptureManager {
    void ToggleMouseCaptureOverride() override;
    void UpdateMouseCapture() override;
};
