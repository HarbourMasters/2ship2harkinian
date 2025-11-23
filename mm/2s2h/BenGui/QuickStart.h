#ifndef QUICKSTART_H
#define QUICKSTART_H

#include <ship/window/gui/GuiWindow.h>
#include "UIWidgets.hpp"

typedef enum {
    QUICK_START_NONE = -1,
    QUICK_START_PRESET_DEFAULT,
    QUICK_START_PRESET_ENHANCED,
    QUICK_START_RANDO_BASE,
    QUICK_START_RANDO_ALL,
    QUICK_START_RANDO_GLITCHLESS,
    QUICK_START_RANDO_NOLOGIC,
} QuickStartSettings;

typedef struct {
    QuickStartSettings shuffleSet;
    QuickStartSettings logicOption;
} RandoQuickStartOptions;

typedef struct {
    int16_t questID;
    QuickStartSettings presetOption;
    RandoQuickStartOptions rando;
} QuickStartSelection;

namespace Ship {
// void RegisterQuickStartMenu();

class QuickStart : public GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void UpdateElement() override{};
    void Draw() override;
};
} // namespace Ship

#endif // QUICKSTART_H