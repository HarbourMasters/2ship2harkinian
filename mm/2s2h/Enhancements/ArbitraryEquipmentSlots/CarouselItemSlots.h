#ifndef _2SHIP_ENHANCEMENT_CAROUSEL_ITEM_SLOTS
#define _2SHIP_ENHANCEMENT_CAROUSEL_ITEM_SLOTS

#include "ArbitraryItemSlots.h"
#include "intent-control-types.h"
#include <algorithm>

struct CarouselItemSlotLister;

struct CarouselItemSlotManager : public ArbitraryItemSlotManager {
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
    uint16_t swapLeftIntent = 0;
    uint16_t swapRightIntent = 0;
    uint32_t processedInputOnFrame = 0;
    bool prevWasPaused = false;
    /**
     * Direction of the carousel in radians
     */
    float carouselDirectionAngle = M_PI / 2;

    uint8_t slotCount = 3;
    int16_t selectedIndex = 0;
    int16_t previousSelectedIndex = 0;
    int32_t restPositionLeft = 330;
    int32_t restPositionTop = 18;

    bool activePositionDifferent = true;
    int32_t activePositionLeft = 330;
    int32_t activePositionTop = 43;

    double lingerSeconds = 1.5;
    double fadeTimeSeconds = 0.125;

    std::vector<std::shared_ptr<CarouselItemSlotManager>> carouselSlots = {};

    std::chrono::high_resolution_clock::time_point lastSlotSwap = std::chrono::high_resolution_clock::now();

    bool active = false;
    std::chrono::high_resolution_clock::time_point activeStarted = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point inactiveStarted = std::chrono::high_resolution_clock::now();

    CarouselItemSlotLister(std::string name, uint16_t equipButtonIntent, uint16_t swapLeftIntent, uint16_t swapRightIntent);
    void resetSlotCount(uint8_t count);
    virtual ArbitraryItemEquipSet getEquipSlots(PlayState *play, Input* input);
    virtual void initItemEquips(ItemEquips* equips);
};

#endif