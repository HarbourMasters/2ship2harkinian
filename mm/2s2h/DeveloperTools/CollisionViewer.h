#pragma once

#include <libultraship/libultraship.h>

#ifdef __cplusplus
extern "C"
#endif
void DrawCollisionViewer();

#ifdef __cplusplus
class CollisionViewerWindow : public LUS::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override {};
};

#endif