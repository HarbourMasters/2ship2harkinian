#include <libultraship/classes.h>

class ItemTrackerSettingsWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

  protected:
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;
};