#include <libultraship/classes.h>
typedef enum {
    TRACKER_WINDOW_FLOATING,
    TRACKER_WINDOW_WINDOW,
} TrackerWindowType;

typedef enum {
    SECTION_DISPLAY_HIDDEN,
    SECTION_DISPLAY_MAIN_WINDOW,
    SECTION_DISPLAY_SEPARATE,
} ItemTrackerDisplayType;

typedef enum {
    SECTION_INVENTORY,
    SECTION_MASKS,
    SECTION_SONGS,
    SECTION_DUNGEON,
    SECTION_MAX,
} ItemTrackerSection;

typedef enum {
    DrawCurrent,
    DrawCurCapacity,
    DrawMaxCapacity,
    DrawCapacityMax,
}ItemTrackerCapacityMode;

class ItemTrackerWindow : public Ship::GuiWindow {
    typedef struct AmmoInfo {
        uint8_t cur;
        uint8_t curCap;
        uint8_t maxCap;
    } AmmoInfo;

  public:
    using GuiWindow::GuiWindow;
    ItemTrackerWindow(const std::string& consoleVariable, const std::string& name);
    ~ItemTrackerWindow();
    ImVec4* GetBgColorPtr();
    float* GetIconSizePtr();
    float* GetIconSpacingPtr();
    float* GetTextSizePtr();
    float* GetTextOffsetPtr();
    int* GetWindowTypePtr();
    bool* GetIsDraggablePtr();
    bool* GetOnlyShowPausedPtr();
    uint8_t* GetDrawModePtr(ItemTrackerSection type);
    bool* GetCapacityModePtr(ItemTrackerCapacityMode mode);

  protected:
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};

  private:
    void BeginFloatingWindows(const char* name, ImGuiWindowFlags flags = 0);
    void DrawItemsInRows(int columns = 6);
    int DrawItems(int columns, int startAt);
    int DrawMasks(int columns, int startAt);
    int DrawSongs(int columns, int startAt);
    int DrawDungeonItemsVert(int columns, int startAt);
    void DrawAmmoCount(int itemId, const ImVec2& iconPos);
    bool HasAmmoCount(int itemId);
    AmmoInfo GetAmmoInfo(int itemId);

    ImVec4 mBgColor;
    float mIconSize = 36.0f;
    float mIconSpacing = 12.0f;
    float mTextSize = 13.0f;
    float mTextOffset = 0.0f;
    int mWindowType = TrackerWindowType::TRACKER_WINDOW_FLOATING;
    bool mIsDraggable = false;
    bool mOnlyDrawPaused = false;
    std::array<bool, DrawCapacityMax> mCapacityModes;
    std::array<uint8_t, SECTION_MAX> mItemDrawModes;
};