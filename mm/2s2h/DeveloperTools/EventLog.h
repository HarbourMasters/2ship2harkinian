#pragma once

#include <libultraship/libultraship.h>

class EventLogWindow : public LUS::GuiWindow {
    public:
        using GuiWindow::GuiWindow;

        void InitElement() override;
        void DrawElement() override;
        void UpdateElement() override {};
};