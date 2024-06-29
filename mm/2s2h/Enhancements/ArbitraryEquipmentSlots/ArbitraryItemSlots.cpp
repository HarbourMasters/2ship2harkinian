#include "intent-control-types.h"
#include <stdint.h>
#include <vector>
#include <memory>
#include <cmath>
#include <chrono>

extern "C" {
#include <z64save.h>
}

#define ARB_EQUIP_ITEM_1 101
#define ARB_EQUIP_ITEM_2 102

uint16_t width = 27, height = 27;

struct ArbitraryItemSlotManager {
    uint16_t arbId;
    uint16_t specialButtonId;
    uint16_t assignedItem;
    ArbitraryItemDrawParams drawParams;
    std::chrono::high_resolution_clock::time_point createPoint = std::chrono::high_resolution_clock::now();

    ArbitraryItemSlotManager(uint16_t id, uint16_t specialButtonId) {
        this->arbId = id;
        this->specialButtonId = specialButtonId; // ARB_EQUIP_ITEM_1;
        this->assignedItem = ITEM_NONE;
        this->drawParams = {
            0, 0, 27, 27, 620, 620, 255, 0, 0, 255,
        };
    }

    ArbitraryItemEquipButton makeEquipButton() {
        return {
            this->arbId, // id;
            this,        // userData;
            +[](ArbitraryItemEquipButton* self, ItemId item) {
                return ((ArbitraryItemSlotManager*)self->userData)->canTakeAssignment(item);
            }, // canTakeAssignment
            +[](ArbitraryItemEquipButton* self, Input* input) {
                return ((ArbitraryItemSlotManager*)self->userData)->assignmentTriggered(input);
            }, // assignmentTriggered
            +[](ArbitraryItemEquipButton* self, Input* input, uint8_t buttonState) {
                return ((ArbitraryItemSlotManager*)self->userData)->activateItem(input, buttonState);
            }, // activateItem
            +[](ArbitraryItemEquipButton* self, Input* input) {
                return ((ArbitraryItemSlotManager*)self->userData)->tradeItem(input);
            }, // tradeItem
            +[](ArbitraryItemEquipButton* self) {
                return ((ArbitraryItemSlotManager*)self->userData)->getDrawParams();
            }, // getDrawParams
            +[](ArbitraryItemEquipButton* self) {
                return ((ArbitraryItemSlotManager*)self->userData)->getAssignedItem();
            }, // getAssignedItem
            +[](ArbitraryItemEquipButton* self, uint16_t item) {
                return ((ArbitraryItemSlotManager*)self->userData)->assignItem(item);
            }, // assignItem
        };
    }

    uint8_t canTakeAssignment(ItemId item) {
        return true;
    }
    uint8_t assignmentTriggered(Input* input) {
        return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, BUTTON_STATE_PRESS, 0);
    }
    uint8_t activateItem(Input* input, uint8_t buttonState) {
        return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, buttonState, 0);
    }
    uint8_t tradeItem(Input* input) {
        return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, BUTTON_STATE_PRESS, 0);
    }
    ArbitraryItemDrawParams getDrawParams() {
        std::chrono::duration<double, std::ratio<1LL, 1LL>> diff =
            (std::chrono::high_resolution_clock::now() - this->createPoint);

        ArbitraryItemDrawParams result = this->drawParams;
        double angle = diff.count() * (M_PI / 4);
        result.rectLeft += (5 * width) * sin(angle);

        return result;
    }
    uint16_t getAssignedItem() {
        return this->assignedItem;
    }
    uint16_t assignItem(uint16_t item) {
        this->assignedItem = item;
        return item;
    }
};

struct ArbitraryItemSlotLister {
    std::vector<ArbitraryItemEquipButton> baseSlots;
    std::vector<std::shared_ptr<ArbitraryItemSlotManager>> slots = {
        std::shared_ptr<ArbitraryItemSlotManager>{ new ArbitraryItemSlotManager(1, ARB_EQUIP_ITEM_1) },
        std::shared_ptr<ArbitraryItemSlotManager>{ new ArbitraryItemSlotManager(2, ARB_EQUIP_ITEM_2) }
    };

    virtual void initItemEquips(ItemEquips* equips) {
        equips->equipsSlotGetter.userData = this;
        equips->equipsSlotGetter.getEquipSlots = +[](ArbitraryEquipsSlotGetter* self) {
            ((ArbitraryItemSlotLister*)self->userData)->baseSlots = {};

            for (auto& slot : ((ArbitraryItemSlotLister*)self->userData)->slots) {
                ((ArbitraryItemSlotLister*)self->userData)->baseSlots.push_back(slot->makeEquipButton());
            }

            ArbitraryItemEquipSet set;
            set.equips = ((ArbitraryItemSlotLister*)self->userData)->baseSlots.data();
            set.count = ((ArbitraryItemSlotLister*)self->userData)->baseSlots.size();
            return set;
        };

        auto second = this->slots.at(1);
        second->drawParams.rectLeft = width;
        second->drawParams.r = 0;
        second->drawParams.g = 0;
        second->drawParams.b = 255;
    }
};

ArbitraryItemSlotLister* currentLister = NULL;

extern "C" void initItemEquips(ItemEquips* equips) {
    if (currentLister == NULL) {
        currentLister = new ArbitraryItemSlotLister();
    }
    return currentLister->initItemEquips(equips);
}
