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
    ITEM_WOODFALL_STRAY_FAIRY,
    ITEM_SNOWHEAD_STRAY_FAIRY,
    ITEM_GREAT_BAY_STRAY_FAIRY,
    ITEM_STONE_TOWER_STRAY_FAIRY,
    ITEM_WOODFALL_DUNGEON_MAP,
    ITEM_WOODFALL_DUNGEON_COMPASS,
    ITEM_WOODFALL_KEY_SMALL,
    ITEM_WOODFALL_KEY_BOSS,
    ITEM_SNOWHEAD_DUNGEON_MAP,
    ITEM_SNOWHEAD_DUNGEON_COMPASS,
    ITEM_SNOWHEAD_KEY_SMALL,
    ITEM_SNOWHEAD_KEY_BOSS,
    ITEM_GREAT_BAY_DUNGEON_MAP,
    ITEM_GREAT_BAY_DUNGEON_COMPASS,
    ITEM_GREAT_BAY_KEY_SMALL,
    ITEM_GREAT_BAY_KEY_BOSS,
    ITEM_STONE_TOWER_DUNGEON_MAP,
    ITEM_STONE_TOWER_DUNGEON_COMPASS,
    ITEM_STONE_TOWER_KEY_SMALL,
    ITEM_STONE_TOWER_KEY_BOSS,
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
