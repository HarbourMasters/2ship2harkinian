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
#include "MultipleConfigs.h"
#include <variables.h>
extern "C" {
#include <z64.h>
#include <macros.h>
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
}

uint16_t width = 27, height = 27;

ArbitraryItemSlotManager::ArbitraryItemSlotManager(std::string id, uint16_t specialButtonId,
                                                   ArbitraryItemSlotLister* lister) {
    this->arbId = id;
    this->specialButtonId = specialButtonId;
    this->assignedItemSlot = SLOT_NONE;
    this->drawParams = { 0, 0, 27, 27, 620, 620, 255, 0, 0, 255, true };
    this->lister = lister;
    this->loadCVars();
}
ArbitraryItemSlotManager::ArbitraryItemSlotManager(std::string id, ArbitraryItemSlotLister* lister) {
    this->arbId = id;
    this->specialButtonId = 0;
    this->assignedItemSlot = SLOT_NONE;
    this->drawParams = { 0, 0, 27, 27, 620, 620, 255, 0, 0, 255, true };
    this->lister = lister;
    this->loadCVars();
}

ArbitraryItemEquipButton ArbitraryItemSlotManager::makeEquipButton() {
    return {
        this, // userData;
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
        +[](ArbitraryItemEquipButton* self, PlayState* play) {
            return ((ArbitraryItemSlotManager*)self->userData)->getDrawParams(play);
        }, // getDrawParams
        +[](ArbitraryItemEquipButton* self) {
            return ((ArbitraryItemSlotManager*)self->userData)->getAssignedItemSlot();
        }, // getAssignedItemSlot
        +[](ArbitraryItemEquipButton* self, InventorySlot itemSlot) {
            return ((ArbitraryItemSlotManager*)self->userData)->assignItemSlot(itemSlot);
        }, // assignItemSlot

        +[](struct ArbitraryItemEquipButton* self, uint8_t disabled) {
            return ((ArbitraryItemSlotManager*)self->userData)->setDisabled(disabled);
        }, // setDisabled
        +[](struct ArbitraryItemEquipButton* self) {
            return ((ArbitraryItemSlotManager*)self->userData)->isDisabled();
        }, // isDisabled
        +[](struct ArbitraryItemEquipButton* self, PlayState* play, HudVisibility hudMode, s16 dimmingAlpha) {
            ((ArbitraryItemSlotManager*)self->userData)->updateHudAlpha(play, hudMode, dimmingAlpha);
        }, // updateHudAlpha
        +[](struct ArbitraryItemEquipButton* self) {
            return ((ArbitraryItemSlotManager*)self->userData)->arbId.c_str();
        }, // getId
        +[](ArbitraryItemEquipButton* self) {
            return ((ArbitraryItemSlotManager*)self->userData)->getAssignedItemID();
        }, // getAssignedItemID
    };
}

uint8_t ArbitraryItemSlotManager::canTakeAssignment(ItemId item) {
    return true;
}
uint8_t ArbitraryItemSlotManager::assignmentTriggered(Input* input) {
    return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, BUTTON_STATE_PRESS, 0);
}
uint8_t ArbitraryItemSlotManager::activateItem(Input* input, uint8_t buttonState) {
    if (this->disabled) {
        return 0;
    }
    return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, buttonState, 0);
}
uint8_t ArbitraryItemSlotManager::tradeItem(Input* input) {
    if (this->disabled) {
        return 0;
    }
    return CHECK_INTENT(input->cur.intentControls, this->specialButtonId, BUTTON_STATE_PRESS, 0);
}
ArbitraryItemDrawParams ArbitraryItemSlotManager::getDrawParams(PlayState* play) {
    ArbitraryItemDrawParams result = this->drawParams;

    SlotState state = this->lister->parentState.parent(this->state);
    if (this->disabled) {
        state = state.parent(this->disabledState);
    }

    return state.toDrawParams(this->hudAlpha);
}
InventorySlot ArbitraryItemSlotManager::getAssignedItemSlot() {
    // return gSaveContext.save.saveInfo.inventory.items[this->assignedItemSlot];
    return this->assignedItemSlot;
}
InventorySlot ArbitraryItemSlotManager::assignItemSlot(InventorySlot itemSlot) {
    this->assignedItemSlot = itemSlot;
    this->disabled = false;
    return itemSlot;
}

ItemId ArbitraryItemSlotManager::getAssignedItemID() {
    if (this->assignedItemSlot == SLOT_NONE) {
        return ITEM_NONE;
    }
    return (ItemId)gSaveContext.save.saveInfo.inventory.items[this->assignedItemSlot];
}

uint8_t ArbitraryItemSlotManager::setDisabled(uint8_t disabled) {
    if (disabled != this->disabled) {
        this->disabledStarted = std::chrono::high_resolution_clock::now();
    }
    return (this->disabled = disabled);
}

uint8_t ArbitraryItemSlotManager::isDisabled() {
    return this->disabled;
}

void ArbitraryItemSlotManager::updateHudAlpha(PlayState* play, HudVisibility hudMode, s16 dimmingAlpha) {
    int32_t risingAlpha = 255 - dimmingAlpha;

#define disabledCheck()                                              \
    if (this->disabled && play->pauseCtx.state == PAUSE_STATE_OFF) { \
        if (this->hudAlpha != 70) {                                  \
            this->hudAlpha = 70;                                     \
        }                                                            \
    } else {                                                         \
        if (this->hudAlpha != 255) {                                 \
            this->hudAlpha = risingAlpha;                            \
        }                                                            \
    }

    switch (hudMode) {
        case HUD_VISIBILITY_NONE:
        case HUD_VISIBILITY_NONE_ALT:
        case HUD_VISIBILITY_B:
        case HUD_VISIBILITY_A:
        case HUD_VISIBILITY_HEARTS_MAGIC:
        case HUD_VISIBILITY_B_ALT:
        case HUD_VISIBILITY_HEARTS:
        case HUD_VISIBILITY_A_B_MINIMAP:
        case HUD_VISIBILITY_B_MINIMAP:
        case HUD_VISIBILITY_HEARTS_MAGIC_MINIMAP:
        case HUD_VISIBILITY_A_HEARTS_MAGIC_MINIMAP:
        case HUD_VISIBILITY_B_MAGIC:
        case HUD_VISIBILITY_A_B:
        case HUD_VISIBILITY_A_B_HEARTS_MAGIC_MINIMAP:
            this->hudAlpha = dimmingAlpha;
            break;
        case HUD_VISIBILITY_HEARTS_MAGIC_C:
        case HUD_VISIBILITY_ALL_NO_MINIMAP:
        case HUD_VISIBILITY_A_B_C:
            this->hudAlpha = risingAlpha;
            break;
        case HUD_VISIBILITY_HEARTS_WITH_OVERWRITE:
        case HUD_VISIBILITY_A_HEARTS_MAGIC_WITH_OVERWRITE:
        case HUD_VISIBILITY_A_HEARTS_MAGIC_MINIMAP_WITH_OVERWRITE:
        case HUD_VISIBILITY_HEARTS_MAGIC_WITH_OVERWRITE:
            if (gSaveContext.hudVisibilityForceButtonAlphasByStatus) {
                disabledCheck();
            } else {
                this->hudAlpha = dimmingAlpha;
            }
            break;
        case HUD_VISIBILITY_ALL_NO_MINIMAP_W_DISABLED:
        case HUD_VISIBILITY_ALL:
            disabledCheck();
            break;
    }
}

std::string ArbitraryItemSlotManager::getCVarListerString() {
    return this->lister->getCVarListerString() + "." + this->arbId;
}

std::string ArbitraryItemSlotLister::getCVarListerString() {
    return std::string("gEnhancements.equipment.arbitraryEquipmentSlots.") + this->name;
}

void ArbitraryItemSlotManager::saveCVars() {
    this->state.saveCVars(this->getCVarListerString() + ".defaultState");
    this->disabledState.saveCVars(this->getCVarListerString() + ".disabledState");
}
void ArbitraryItemSlotManager::loadCVars() {
    this->state.loadCVars(this->getCVarListerString() + ".defaultState");
    this->disabledState.loadCVars(this->getCVarListerString() + ".disabledState");
}

void ArbitraryItemSlotLister::saveCVars() {
    this->parentState.saveCVars(this->getCVarListerString() + ".defaultState");
    this->disabledState.saveCVars(this->getCVarListerString() + ".disabledState");
}
void ArbitraryItemSlotLister::loadCVars() {
    this->parentState.loadCVars(this->getCVarListerString() + ".defaultState");
    this->disabledState.loadCVars(this->getCVarListerString() + ".disabledState");
}

ArbitraryItemEquipSet ArbitraryItemSlotLister::getEquipSlots(PlayState* play, Input* input) {
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

void ArbitraryItemSlotLister::addEquipSetCallbacks(ArbitraryItemEquipSet* set) {
    set->findSlotWithItem = +[](ArbitraryItemEquipSet* self, uint16_t item) {
        FOREACH_SLOT(*self, slot, {
            if (gSaveContext.save.saveInfo.inventory.items[slot->getAssignedItemSlot(slot)] == item) {
                return 1;
            }
        });
        return -1;
    };
}

void ArbitraryItemSlotLister::initItemEquips(ItemEquips* equips) {
    equips->equipsSlotGetter.userData = this;
    equips->equipsSlotGetter.getEquipSlots = +[](const ArbitraryEquipsSlotGetter* self, PlayState* play, Input* input) {
        return ((ArbitraryItemSlotLister*)self->userData)->getEquipSlots(play, input);
    };

    auto second = this->slots.at(1);
    second->drawParams.rectLeft = width;
    second->drawParams.r = 0;
    second->drawParams.g = 0;
    second->drawParams.b = 255;
}

ArbitraryItemSlotLister::ArbitraryItemSlotLister() {
    this->loadCVars();
}

std::shared_ptr<ArbitraryItemSlotLister> currentLister = NULL;

std::shared_ptr<ArbitraryItemSlotLister> ArbitraryItemSlotLister::getLister() {
    if (currentLister == NULL) {
        currentLister = std::shared_ptr<ArbitraryItemSlotLister>{ new MulitpleItemSlotLister(
            "Slots",
            { std::shared_ptr<ArbitraryItemSlotLister>{ new ArbitraryItemSlotLister() },
              std::shared_ptr<ArbitraryItemSlotLister>{ new MulitpleItemSlotLister(
                  "Carousel Slots",
                  { 
                    // std::shared_ptr<ArbitraryItemSlotLister>{ new CarouselItemSlotLister(
                    //   "1", INTENT_USE_ITEM, INTENT_HOTSWAP_ITEM_LEFT, INTENT_HOTSWAP_ITEM_RIGHT) } 
                      },
                  CarouselItemSlotLister::makeCarousel) } },
            nullptr) };
    }
    return currentLister;
}

extern "C" void initItemEquips(ItemEquips* equips) {
    return ArbitraryItemSlotLister::getLister()->initItemEquips(equips);
}
