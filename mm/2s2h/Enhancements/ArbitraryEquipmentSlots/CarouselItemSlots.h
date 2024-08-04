#ifndef _2SHIP_ENHANCEMENT_CAROUSEL_ITEM_SLOTS
#define _2SHIP_ENHANCEMENT_CAROUSEL_ITEM_SLOTS

#include "ArbitraryItemSlots.h"
#include "intent-control-types.h"
#include <algorithm>
#include "MultipleConfigs.h"

struct CarouselItemSlotLister;

struct CarouselItemSlotManager : public ArbitraryItemSlotManager {
    CarouselItemSlotManager(std::string id, CarouselItemSlotLister* lister);
    int32_t getLeftOffset(int16_t index);
    bool isSelectedSlot();

    virtual void saveCVars();
    virtual void loadCVars();

    virtual uint8_t canTakeAssignment(ItemId item);
    virtual uint8_t assignmentTriggered(Input* input);
    virtual uint8_t activateItem(Input* input, uint8_t buttonState);
    virtual uint8_t tradeItem(Input* input);

    virtual ArbitraryItemDrawParams getDrawParams(PlayState* play);

    virtual SlotState getIndexSlotState(int16_t index, double width);
    virtual double getScrollTimeRatio();
    virtual double getActivationRatio();
};

struct CarouselItemSlotLister : public ArbitraryItemSlotLister {
    static std::shared_ptr<CarouselItemSlotLister> makeCarousel(MulitpleItemSlotLister* parent);
    static std::vector<std::weak_ptr<CarouselItemSlotLister>> existingCarousels;
    CAROUSEL_BUTTONS_TYPE buttons;
    uint32_t processedInputOnFrame = 0;

    int carouselIndexRadius = 1;

    SlotState scrollingState;
    SlotState scrollingSelectedState;

    bool prevWasPaused = false;
    /**
     * Direction of the carousel in radians
     */
    float carouselDirectionAngle = 0;

    int slotCount = 3;
    int16_t selectedIndex = 0;
    int16_t previousSelectedIndex = 0;

    bool activePositionDifferent = true;

    double lingerSeconds = 1.5;
    double fadeTimeSeconds = 0.125;

    std::vector<std::shared_ptr<CarouselItemSlotManager>> carouselSlots = {};

    std::chrono::high_resolution_clock::time_point lastSlotSwap = std::chrono::high_resolution_clock::now();

    bool active = false;
    std::chrono::high_resolution_clock::time_point activeStarted = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point inactiveStarted = std::chrono::high_resolution_clock::now();

    void resetSlotCount(uint8_t count);

    virtual void saveCVars() override;
    virtual void loadCVars() override;

    virtual ArbitraryItemEquipSet getEquipSlots(PlayState* play, Input* input);
    virtual void initItemEquips(ItemEquips* equips);
private: 
    CarouselItemSlotLister(std::string name, CAROUSEL_BUTTONS_TYPE buttons);
};

#endif