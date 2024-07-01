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

    virtual uint8_t canTakeAssignment(ItemId item);
    virtual uint8_t assignmentTriggered(Input* input);
    virtual uint8_t activateItem(Input* input, uint8_t buttonState);
    virtual uint8_t tradeItem(Input* input);
    
    virtual ArbitraryItemDrawParams getDrawParams(PlayState *play);
};

struct CarouselItemSlotLister : public ArbitraryItemSlotLister {
    uint16_t equipButtonIntent = 0;

    int16_t selectedIndex = 0;
    int16_t previousSelectedIndex = 0;
    int32_t rectLeft = 75;
    int32_t rectTop = 10;
    
    std::vector<std::shared_ptr<CarouselItemSlotManager>> carouselSlots = {
        std::shared_ptr<CarouselItemSlotManager>{ new CarouselItemSlotManager(1, this) },
        std::shared_ptr<CarouselItemSlotManager>{ new CarouselItemSlotManager(2, this) },
        std::shared_ptr<CarouselItemSlotManager>{ new CarouselItemSlotManager(3, this) }
    };

    std::chrono::high_resolution_clock::time_point lastSlotSwap = std::chrono::high_resolution_clock::now();

    CarouselItemSlotLister(uint16_t equipButtonIntent);
    virtual ArbitraryItemEquipSet getEquipSlots(Input* input);
    virtual void initItemEquips(ItemEquips* equips);
};

#endif