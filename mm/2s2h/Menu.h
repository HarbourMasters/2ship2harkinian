#pragma once

#include <libultraship/libultraship.h>

class BenMenu : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;

private:
    uint32_t menuHeight;
    uint32_t menuWidth;

    void DrawSettingsMenu();
    void DrawEnhancementsMenu();
    void DrawCheatsMenu();

    void DrawAudioSettings(float width);
    void DrawGraphicsSettings(float width);
    void DrawControllerSettings(float width);
};