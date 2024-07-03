#include "intent-control-types.h"
#include <stdint.h>
#include <vector>
#include <memory>
#include <cmath>
#include <chrono>
#include "ArbitraryItemSlots.h"

extern "C" {
#include <z64save.h>
}

#include "CarouselItemSlots.h"

uint16_t width = 27, height = 27;

ArbitraryItemSlotManager::ArbitraryItemSlotManager(uint16_t id, uint16_t specialButtonId) {
    // this->arbId = id;
    this->specialButtonId = specialButtonId;
    this->assignedItem = ITEM_NONE;
    this->drawParams = {
        0, 0, 27, 27, 620, 620, 255, 0, 0, 255, true
    };
}
ArbitraryItemSlotManager::ArbitraryItemSlotManager(uint16_t id) {
    this->arbId = id;
    this->specialButtonId = 0;
    this->assignedItem = ITEM_NONE;
    this->drawParams = {
        0, 0, 27, 27, 620, 620, 255, 0, 0, 255, true
    };
}

ArbitraryItemEquipButton ArbitraryItemSlotManager::makeEquipButton() {
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
        +[](ArbitraryItemEquipButton* self, PlayState *play) {
            return ((ArbitraryItemSlotManager*)self->userData)->getDrawParams(play);
        }, // getDrawParams
        +[](ArbitraryItemEquipButton* self) {
            return ((ArbitraryItemSlotManager*)self->userData)->getAssignedItem();
        }, // getAssignedItem
        +[](ArbitraryItemEquipButton* self, uint16_t item) {
            return ((ArbitraryItemSlotManager*)self->userData)->assignItem(item);
        }, // assignItem

        +[](struct ArbitraryItemEquipButton* self, uint8_t disabled) {
            return ((ArbitraryItemSlotManager*)self->userData)->setDisabled(disabled);
        }, // setDisabled
        +[](struct ArbitraryItemEquipButton* self){
            return ((ArbitraryItemSlotManager*)self->userData)->isDisabled();
        }, // isDisabled
    };
}

uint8_t ArbitraryItemSlotManager::canTakeAssignment(ItemId item) {
    return true;
}
uint8_t ArbitraryItemSlotManager::assignmentTriggered(Input* input) {
    return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, BUTTON_STATE_PRESS, 0);
}
uint8_t ArbitraryItemSlotManager::activateItem(Input* input, uint8_t buttonState) {
    return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, buttonState, 0);
}
uint8_t ArbitraryItemSlotManager::tradeItem(Input* input) {
    return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, BUTTON_STATE_PRESS, 0);
}
ArbitraryItemDrawParams ArbitraryItemSlotManager::getDrawParams(PlayState *play) {
    std::chrono::duration<double, std::ratio<1LL, 1LL>> diff =
        (std::chrono::high_resolution_clock::now() - this->createPoint);

    ArbitraryItemDrawParams result = this->drawParams;
    double angle = diff.count() * (M_PI / 4);
    result.rectLeft += (5 * width) * sin(angle);

    return result;
}
uint16_t ArbitraryItemSlotManager::getAssignedItem() {
    return this->assignedItem;
}
uint16_t ArbitraryItemSlotManager::assignItem(uint16_t item) {
    this->assignedItem = item;
    this->disabled = false;
    return item;
}

uint8_t ArbitraryItemSlotManager::setDisabled(uint8_t disabled){
    return (this->disabled = disabled);
}

uint8_t ArbitraryItemSlotManager::isDisabled(){
    return this->disabled;
}

ArbitraryItemEquipSet ArbitraryItemSlotLister::getEquipSlots(PlayState *play, Input* input){
    this->baseSlots = {};

    for (auto& slot : this->slots) {
        this->baseSlots.push_back(slot->makeEquipButton());
    }

    ArbitraryItemEquipSet set;
    set.equips = this->baseSlots.data();
    set.count = this->baseSlots.size();
    this->addEquipSetCallbacks(&set);
    
    return set;
}

void ArbitraryItemSlotLister::addEquipSetCallbacks(ArbitraryItemEquipSet* set){
    set->findSlotWithItem = +[](ArbitraryItemEquipSet* self, uint16_t item){
        FOREACH_SLOT(*self, slot, {
            if(slot->getAssignedItem(slot) == item){
                return 1;
            }
        });
        return -1;
    };
}

void ArbitraryItemSlotLister::initItemEquips(ItemEquips* equips) {
    equips->equipsSlotGetter.userData = this;
    equips->equipsSlotGetter.getEquipSlots = +[](ArbitraryEquipsSlotGetter* self, PlayState *play, Input* input) {
        return ((ArbitraryItemSlotLister*)self->userData)->getEquipSlots(play, input);
    };

    auto second = this->slots.at(1);
    second->drawParams.rectLeft = width;
    second->drawParams.r = 0;
    second->drawParams.g = 0;
    second->drawParams.b = 255;
}

std::shared_ptr<ArbitraryItemSlotLister> currentLister = NULL;

std::shared_ptr<ArbitraryItemSlotLister> ArbitraryItemSlotLister::getLister(){
    if (currentLister == NULL) {
        // currentLister = new ArbitraryItemSlotLister();
        currentLister = std::shared_ptr<ArbitraryItemSlotLister>{new CarouselItemSlotLister(INTENT_USE_ITEM)};
    }
    return currentLister;
}

extern "C" void initItemEquips(ItemEquips* equips) {
    return ArbitraryItemSlotLister::getLister()->initItemEquips(equips);
}
