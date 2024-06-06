#pragma once

#include <libultraship/libultraship.h>
#include <2s2h/BenGui/UIWidgets.hpp>

namespace BenGui {
class BenMenu : public Ship::GuiWindow {
public:
    using Ship::GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;

private:
    uint32_t windowHeight;
    uint32_t windowWidth;
    UIWidgets::SidebarEntry audioSettingsEntry;
    UIWidgets::MainMenuEntry settingsEntry;
};
}