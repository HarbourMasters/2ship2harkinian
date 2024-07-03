#ifndef _2SHIP_ENHANCEMENT_CAROUSEL_ITEM_SLOTS
#define _2SHIP_ENHANCEMENT_CAROUSEL_ITEM_SLOTS

#include "ArbitraryItemSlots.h"
#include "intent-control-types.h"
#include <algorithm>

struct CarouselItemSlotLister;

struct CarouselItemSlotManager : public ArbitraryItemSlotManager {
    CarouselItemSlotLister* lister;
    CarouselItemSlotManager(uint16_t id, CarouselItemSlotLister* lister);
    int32_t getLeftOffset(int16_t index);
    bool isSelectedSlot();
    int16_t scrollPosition = 0;

    virtual uint8_t canTakeAssignment(ItemId item);
    virtual uint8_t assignmentTriggered(Input* input);
    virtual uint8_t activateItem(Input* input, uint8_t buttonState);
    virtual uint8_t tradeItem(Input* input);
    
    virtual ArbitraryItemDrawParams getDrawParams(PlayState *play);
};

struct CarouselItemSlotLister : public ArbitraryItemSlotLister {
    uint16_t equipButtonIntent = 0;
    uint32_t processedInputOnFrame = 0;
    bool prevWasPaused = false;
    /**
     * Direction of the carousel in radians
     */
    float carouselDirectionAngle = 0;

    uint8_t slotCount = 3;
    int16_t selectedIndex = 0;
    int16_t previousSelectedIndex = 0;
    int32_t rectLeft = 288;
    int32_t rectTop = 190;
    float rgb[3] = {255.0 / 255.0, 240.0 / 255.0, 0.0 / 255.0};
    
    std::vector<std::shared_ptr<CarouselItemSlotManager>> carouselSlots = {};

    std::chrono::high_resolution_clock::time_point lastSlotSwap = std::chrono::high_resolution_clock::now();

    CarouselItemSlotLister(uint16_t equipButtonIntent);
    void resetSlotCount(uint8_t count);
    virtual ArbitraryItemEquipSet getEquipSlots(PlayState *play, Input* input);
    virtual void initItemEquips(ItemEquips* equips);
};

#endif