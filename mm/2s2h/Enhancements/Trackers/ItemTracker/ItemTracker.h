#include <vector>
#include <imgui.h>
#include "ship/window/gui/GuiWindow.h"

typedef enum {
    ITEM_BOTTLE_1 = 0x100,
    ITEM_BOTTLE_2,
    ITEM_BOTTLE_3,
    ITEM_BOTTLE_4,
    ITEM_BOTTLE_5,
    ITEM_BOTTLE_6,
} TrackerBottleSlots;

typedef enum {
    ITEM_SKULL_TOKEN_SWAMP = 0x106,
    ITEM_SKULL_TOKEN_OCEAN,
} TrackerTokenSlots;

typedef struct {
    ImTextureID textureId;
    ImVec4 textureColor;
    ImVec2 textureDimensions;
} TrackerImageObject;

typedef struct {
    std::string windowName;
    int32_t columnLength;
    float windowScale;
    float windowOpacity;
    std::vector<int16_t> itemList;
} TrackerItemListObject;

TrackerImageObject GetTextureObject(int16_t itemId);
extern std::string GetItemTrackerItemName(int16_t itemId);
extern bool shouldWindowSplit;

class ItemTrackerWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;
    void Draw() override;

    std::vector<TrackerItemListObject> namedItemWindows;

  protected:
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};
};
