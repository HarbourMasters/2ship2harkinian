#ifndef _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS
#define _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS
#include <stdint.h>
#include <chrono>
extern "C" {
#include <z64save.h>
}
#include "ArbitraryItemSlotsGUI.h"

#define ARB_EQUIP_ITEM_1 101
#define ARB_EQUIP_ITEM_2 102

struct ArbitraryItemSlotsManagerOptions;

struct ArbitraryItemSlotManager {
    uint16_t arbId;
    uint16_t specialButtonId;
    uint16_t assignedItem;
    ArbitraryItemDrawParams drawParams;
    std::chrono::high_resolution_clock::time_point createPoint = std::chrono::high_resolution_clock::now();

    ArbitraryItemSlotManager(uint16_t id);
    ArbitraryItemSlotManager(uint16_t id, uint16_t specialButtonId);

    virtual ArbitraryItemEquipButton makeEquipButton();
    virtual uint8_t canTakeAssignment(ItemId item);
    virtual uint8_t assignmentTriggered(Input* input);
    virtual uint8_t activateItem(Input* input, uint8_t buttonState);
    virtual uint8_t tradeItem(Input* input);
    virtual uint16_t getAssignedItem();
    virtual uint16_t assignItem(uint16_t item);
    virtual ArbitraryItemDrawParams getDrawParams(PlayState *play);
};

struct ArbitraryItemSlotLister {
    std::shared_ptr<ArbitraryItemSlotsListerOptions> options{new ArbitraryItemSlotsListerOptions()};
    std::vector<ArbitraryItemEquipButton> baseSlots;
    std::vector<std::shared_ptr<ArbitraryItemSlotManager>> slots = {
        std::shared_ptr<ArbitraryItemSlotManager>{ new ArbitraryItemSlotManager(1, ARB_EQUIP_ITEM_1) },
        std::shared_ptr<ArbitraryItemSlotManager>{ new ArbitraryItemSlotManager(2, ARB_EQUIP_ITEM_2) }
    };

    virtual ArbitraryItemEquipSet getEquipSlots(Input* input);

    virtual void initItemEquips(ItemEquips* equips);

    static std::shared_ptr<ArbitraryItemSlotLister> getLister();
};

#endif