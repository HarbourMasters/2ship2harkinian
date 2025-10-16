#include "window/gui/Gui.h"
#include "window/gui/GuiWindow.h"
#include <array>

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
