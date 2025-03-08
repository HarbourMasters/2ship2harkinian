#include "window/gui/Gui.h"
#include "window/gui/GuiWindow.h"

class ItemTrackerWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    typedef struct {
        int16_t panelId;
        const char* panelName;
        std::vector<ItemId> panelContents;
    } ItemTrackerPanel;

    void InitElement() override {};
    void DrawElement() override {};
    void Draw() override;
    void UpdateElement() override {};
};
