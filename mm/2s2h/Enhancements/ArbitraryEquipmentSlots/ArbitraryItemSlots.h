#ifndef _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS
#define _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS
#include <stdint.h>
#include <chrono>
extern "C" {
#include <z64save.h>
}
#include "ArbitraryItemSlotsGUI.h"
#include "./SlotState.h"

#define ARB_EQUIP_ITEM_1 101
#define ARB_EQUIP_ITEM_2 102

struct ArbitraryItemSlotsManagerOptions;
struct ArbitraryItemSlotLister;

struct ArbitraryItemSlotManager {
    std::string arbId;
    uint16_t specialButtonId;
    uint16_t assignedItem;
    int32_t hudAlpha = 255;
    ArbitraryItemDrawParams drawParams;
    ArbitraryItemSlotLister* lister;
    std::chrono::high_resolution_clock::time_point createPoint = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point disabledStarted = std::chrono::high_resolution_clock::now();
    bool disabled = false;
    SlotState state;
    SlotState disabledState{};

    ArbitraryItemSlotManager(std::string id, ArbitraryItemSlotLister* lister);
    ArbitraryItemSlotManager(std::string id, uint16_t specialButtonId, ArbitraryItemSlotLister* lister);

    virtual std::string getCVarListerString();

    virtual void saveCVars();
    virtual void loadCVars();

    virtual ArbitraryItemEquipButton makeEquipButton();
    virtual uint8_t canTakeAssignment(ItemId item);
    virtual uint8_t assignmentTriggered(Input* input);
    virtual uint8_t activateItem(Input* input, uint8_t buttonState);
    virtual uint8_t tradeItem(Input* input);
    virtual uint16_t getAssignedItem();
    virtual uint16_t assignItem(uint16_t item);
    virtual ArbitraryItemDrawParams getDrawParams(PlayState *play);

    virtual uint8_t setDisabled(uint8_t disabled);
    virtual uint8_t isDisabled();
    virtual void updateHudAlpha(PlayState *play, HudVisibility hudMode, s16 dimmingAlpha);
};

struct ArbitraryItemSlotLister {
    std::string name = "Dedicated Slots";
    SlotState parentState;
    SlotState disabledState;
    std::shared_ptr<ArbitraryItemSlotsListerOptions> options{new ArbitraryItemSlotsListerOptions()};
    std::vector<ArbitraryItemEquipButton> baseSlots;
    std::vector<std::shared_ptr<ArbitraryItemSlotManager>> slots = {
        std::shared_ptr<ArbitraryItemSlotManager>{ new ArbitraryItemSlotManager("1", ARB_EQUIP_ITEM_1, this) },
        std::shared_ptr<ArbitraryItemSlotManager>{ new ArbitraryItemSlotManager("2", ARB_EQUIP_ITEM_2, this) }
    };

    virtual std::string getCVarListerString();
    
    virtual void saveCVars();
    virtual void loadCVars();

    virtual ArbitraryItemEquipSet getEquipSlots(PlayState *play, Input* input);

    virtual void initItemEquips(ItemEquips* equips);
    virtual void addEquipSetCallbacks(ArbitraryItemEquipSet* set);

    ArbitraryItemSlotLister();

    static std::shared_ptr<ArbitraryItemSlotLister> getLister();
};

#endif