#include "window/gui/Gui.h"
#include "window/gui/GuiWindow.h"
#include <vector>

typedef enum {
    TRACKER_INVENTORY,
    TRACKER_MASKS,
    TRACKER_QUEST,
    TRACKER_SONGS,
    TRACKER_DUNGEON,
    TRACKER_STRAY_FAIRIES,
};

typedef enum {
    SECTION_HIDDEN,
    SECTION_MAIN,
    SECTION_SUB,
    SECTION_SEPARATE,
};

void UpdateTrackerWindows();
void UpdateTrackerSettings();
extern std::vector<std::pair<const char*, const char*>> itemTrackerPanelOptions;
extern std::vector<std::pair<const char*, const char*>> itemTrackerSettingsOptions;

class ItemTrackerWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    typedef struct {
        int16_t panelId;
        const char* panelName;
        int16_t panelWidth;
        std::vector<int16_t> panelContents;
    } ItemTrackerPanel;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override{};
};
