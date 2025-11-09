#include <vector>
#include <imgui.h>
#include "ship/window/gui/GuiWindow.h"

typedef struct {
    ImTextureID textureId;
    float fade;
} TrackerImageObject;

typedef struct {
    std::string windowName;
    int32_t columnLength;
    float windowScale;
    std::vector<int16_t> itemList;
} TrackerItemListObject;

class ItemTrackerWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;
    void Draw() override;

    TrackerItemListObject mainItemWindow;
    std::vector<TrackerItemListObject> namedItemWindows;

  protected:
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};
};
