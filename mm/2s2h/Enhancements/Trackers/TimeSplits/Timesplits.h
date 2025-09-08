#include "GuiWindow.h"
#include <vector>
#include <map>

class TimesplitsWindow : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override;
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override{};
};

typedef enum ExtendedSplitIds {
    SPLIT_KILLED_ODOLWA = 256,
    SPLIT_KILLED_GOHT,
    SPLIT_KILLED_GYORG,
    SPLIT_KILLED_TWINMOLD,
    SPLIT_KILLED_MAJORA,
};

typedef enum SplitSettings {
    SPLIT_HEADERS,
    SPLIT_OPACITY,
    SPLIT_HIGHLIGHT,
};

typedef enum SplitFileActions {
    SPLIT_SAVE,
    SPLIT_LOAD,
};

typedef enum SplitStatus { SPLIT_INACTIVE, SPLIT_ACTIVE, SPLIT_COMPLETE, SPLIT_SKIPPED };

typedef struct {
    uint32_t splitId;
    std::string splitName;
    uint32_t splitCurrentTime;
    uint32_t splitPreviousBest;
    uint8_t splitStatus;
} TimesplitObject;

typedef struct {
    uint32_t timeDisplay;
    ImVec4 colorDisplay;
} SplitTextObject;

typedef struct {
    uint32_t startIndex;
    uint32_t endIndex;
} IndexRangeObject;

extern std::vector<TimesplitObject> splitList;
extern std::vector<TimesplitObject> splitObjectList;
extern std::map<uint32_t, std::vector<uint32_t>> itemSubMenuList;
extern std::map<uint32_t, ImVec4> songColorMap;
extern bool shouldPopUpOpen;
extern TimesplitObject GetSplitObjectById(uint32_t itemId);
extern ImVec4 GetColorTint(uint32_t itemId);
extern const char* GetItemImageById(uint32_t itemId);
extern void SplitsPushImageButtonStyle();
extern void SplitsPopImageButtonStyle();
extern void HandlePopUpContext(uint32_t popupId);
extern void HandleDragAndDrop(size_t i);
extern void UpdateSplitBests();
extern void UpdateSplitSettings(uint32_t settingName);
extern void SkipSplitEntry(uint32_t index);
extern void AddSplitEntryById(uint32_t itemId);
extern void RemoveSplitEntry(uint32_t splitId, uint32_t index);
