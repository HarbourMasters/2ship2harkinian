#ifndef _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_SLOT_STATE
#define _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_SLOT_STATE
#include <stdint.h>
#include <z64save.h>
#include <string>

struct SlotState {
    int32_t posLeft = 0;
    int32_t posTop = 0;
    float scale = 1;
    float rgb[4] = { 255.0 / 255.0, 240.0 / 255.0, 0.0 / 255.0, 1.0 };
    float transparency = 1;

    int32_t getWidth();
    int32_t getHeight();

    SlotState blend(SlotState& other, float ratio);
    SlotState parent(SlotState& child);
    ArbitraryItemDrawParams toDrawParams(int32_t hudAlpha);

    virtual void saveCVars(std::string savePath);
    virtual void loadCVars(std::string savePath);
};

#endif