#ifndef BenMenuBar_h
#define BenMenuBar_h

#include "window/gui/GuiMenuBar.h"
#include "window/gui/GuiElement.h"
#include "DeveloperTools/SaveEditor.h"
#include "DeveloperTools/ActorViewer.h"
#include "DeveloperTools/CollisionViewer.h"

namespace BenGui {
class BenMenuBar : public LUS::GuiMenuBar {
  public:
    using LUS::GuiMenuBar::GuiMenuBar;
  protected:
    void DrawElement() override;
    void InitElement() override {};
    void UpdateElement() override {};
};
} // namespace BenGui

#endif // BenMenuBar_h