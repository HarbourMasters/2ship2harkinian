#include <vector>
#include <imgui.h>
#include "ship/window/gui/GuiWindow.h"

typedef struct {
    ImTextureID textureId;
    float fade;
} TrackerImageObject;

typedef struct {
    std::string windowName;
    std::vector<int16_t> itemList;
} TrackerItemListObject;

class ItemTrackerWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;
    void Draw() override;

    std::vector<int16_t> mainItemWindow;
    std::vector<TrackerItemListObject> namedItemWindows;
    //std::vector<std::vector<int16_t>> subItemWindows;

  protected:
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};
};
