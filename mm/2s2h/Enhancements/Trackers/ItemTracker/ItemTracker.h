#include <array>
#include <imgui.h>
#include "ship/window/gui/GuiWindow.h"

typedef struct {
    ImTextureID textureId;
    float fade;
} TrackerImageObject;

class ItemTrackerWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;
    void Draw() override;

  protected:
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};
};
