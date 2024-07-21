#pragma once

#ifdef __cplusplus
#include <libultraship/classes.h>

extern "C" {
#endif

void DrawCollisionViewer();

#ifdef __cplusplus
}

class CollisionViewerWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override {};
};

#endif
