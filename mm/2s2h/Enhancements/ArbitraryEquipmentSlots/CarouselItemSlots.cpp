#include "CarouselItemSlots.h"
#include <libultraship/libultraship.h> // TODO: Without this import, below imports break a bunch of internal `extern` imports deeper in the 2ship include tree for some reason
extern "C" {
#include <z64.h>
#include <macros.h>
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
}
#include "./guis/CarouselGUI.h"

CarouselItemSlotManager::CarouselItemSlotManager(std::string id, CarouselItemSlotLister* lister)
    : ArbitraryItemSlotManager(id, lister) {
    // this->scrollPosition = 0;
}

int32_t CarouselItemSlotManager::getLeftOffset(int16_t index) {
    int8_t negative = index < 0 ? -1 : 1;
    int abs = index * negative;
    auto l = (CarouselItemSlotLister*)lister;
    abs %= l->carouselSlots.size();

    if (negative < 0) {
        abs *= -1;
        abs += l->carouselSlots.size();
        abs %= l->carouselSlots.size();
    }

    auto it = std::find_if(l->carouselSlots.begin(), l->carouselSlots.end(),
                           [this](std::shared_ptr<CarouselItemSlotManager> slot) { return slot.get() == this; });
    long thisIndex = it - l->carouselSlots.begin();

    long dist = thisIndex - abs;

    if (std::abs(dist) > ((double)l->carouselSlots.size() / 2.0)) {
        if (dist < 0) {
            dist += l->carouselSlots.size();
        } else {
            dist -= l->carouselSlots.size();
        }
    }

    return dist;
}

SlotState CarouselItemSlotManager::getIndexSlotState(int16_t relativeIndex, double width) {
    auto l = (CarouselItemSlotLister*)lister;
    SlotState resultState;
    double distance = width * relativeIndex;

    float angleRads = l->carouselDirectionAngle;

    resultState.posLeft += std::cos(angleRads) * distance;
    resultState.posTop += std::sin(angleRads) * distance;

    if (std::abs(relativeIndex) > l->carouselIndexRadius) {
        resultState.transparency = 0;
    }

    return resultState;
}

double CarouselItemSlotManager::getScrollTimeRatio() {
    auto l = (CarouselItemSlotLister*)lister;
    std::chrono::duration<double, std::ratio<1LL, 1LL>> elapsedTime =
        (std::chrono::high_resolution_clock::now() - l->lastSlotSwap);

    return std::clamp(elapsedTime.count() / l->fadeTimeSeconds, 0.0, 1.0);
}
double CarouselItemSlotManager::getActivationRatio() {
    auto l = (CarouselItemSlotLister*)lister;
    std::chrono::duration<double, std::ratio<1LL, 1LL>> elapsedTime =
        (std::chrono::high_resolution_clock::now() - l->activeStarted);

    if (!l->active) {
        elapsedTime = (std::chrono::high_resolution_clock::now() - l->inactiveStarted);
    }

    double clampedRatio = std::clamp(elapsedTime.count() / l->fadeTimeSeconds, 0.0, 1.0);

    if (!l->active) {
        clampedRatio = 1.0 - clampedRatio;
    }

    return clampedRatio;
}

ArbitraryItemDrawParams CarouselItemSlotManager::getDrawParams(PlayState* play) {
    auto l = (CarouselItemSlotLister*)lister;

    SlotState defaultState = l->parentState;
    SlotState scrollingState = l->scrollingState;
    SlotState scrollingSelectedState = l->scrollingSelectedState;

    auto dist = this->getLeftOffset(l->selectedIndex);
    auto prevDist = this->getLeftOffset(l->previousSelectedIndex);

    SlotState result = defaultState.parent(scrollingState);

    SlotState scrollingCurPositionState = this->getIndexSlotState(dist, scrollingSelectedState.getWidth());
    SlotState scrollingPrevPositionState = this->getIndexSlotState(prevDist, scrollingSelectedState.getWidth());
    SlotState scrollingPositionState =
        scrollingCurPositionState.blend(scrollingPrevPositionState, this->getScrollTimeRatio());

    result = result.parent(scrollingPositionState);

    if (dist == 0) {
        result = result.parent(scrollingSelectedState);
    } else {
        defaultState.transparency = 0;
    }

    if (this->disabled) {
        result = result.parent(l->disabledState);
    }

    result = result.blend(defaultState, this->getActivationRatio());

    return result.toDrawParams(this->hudAlpha);
}

bool CarouselItemSlotManager::isSelectedSlot() {
    auto l = (CarouselItemSlotLister*)lister;
    int8_t negative = l->selectedIndex < 0 ? -1 : 1;
    int abs = l->selectedIndex * negative;
    abs %= l->carouselSlots.size();

    if (negative < 0) {
        abs *= -1;
        abs += l->carouselSlots.size();
        abs %= l->carouselSlots.size();
    }

    auto it = std::find_if(l->carouselSlots.begin(), l->carouselSlots.end(),
                           [this](std::shared_ptr<CarouselItemSlotManager> slot) { return slot.get() == this; });
    long thisIndex = it - l->carouselSlots.begin();

    return thisIndex == abs;
}

uint8_t CarouselItemSlotManager::canTakeAssignment(ItemId item) {
    if (!this->isSelectedSlot()) {
        return 0;
    }

    return true;
}
uint8_t CarouselItemSlotManager::assignmentTriggered(Input* input) {
    if (!this->isSelectedSlot()) {
        return 0;
    }

    auto l = (CarouselItemSlotLister*)lister;
    return CHECK_INTENT(input->press.intentControls, l->buttons.useItem, BUTTON_STATE_PRESS, 0);
}
uint8_t CarouselItemSlotManager::activateItem(Input* input, uint8_t buttonState) {
    if (!this->isSelectedSlot()) {
        return 0;
    } else if (this->disabled) {
        return 0;
    }

    auto l = (CarouselItemSlotLister*)lister;
    return CHECK_INTENT(input->press.intentControls, l->buttons.useItem, buttonState, 0);
}
uint8_t CarouselItemSlotManager::tradeItem(Input* input) {
    if (!this->isSelectedSlot()) {
        return 0;
    }

    auto l = (CarouselItemSlotLister*)lister;
    return CHECK_INTENT(input->cur.intentControls, l->buttons.useItem, BUTTON_STATE_PRESS, 0);
}

#define LOAD_CVAR(name, cvarFun) this->name = cvarFun((this->getCVarListerString() + "." #name).c_str(), this->name)
#define SET_CVAR(name, cvarFun) cvarFun((this->getCVarListerString() + "." #name).c_str(), this->name)

void CarouselItemSlotManager::loadCVars() {
    ArbitraryItemSlotManager::loadCVars();
}
void CarouselItemSlotManager::saveCVars() {
    ArbitraryItemSlotManager::saveCVars();
}

void CarouselItemSlotLister::loadCVars() {
    ArbitraryItemSlotLister::loadCVars();
    LOAD_CVAR(slotCount, CVarGetInteger);
    LOAD_CVAR(carouselIndexRadius, CVarGetInteger);
    LOAD_CVAR(carouselDirectionAngle, CVarGetFloat);
    this->parentState.loadCVars(this->getCVarListerString() + ".defaultState");
}
void CarouselItemSlotLister::saveCVars() {
    ArbitraryItemSlotLister::saveCVars();
    SET_CVAR(slotCount, CVarSetInteger);
    SET_CVAR(carouselIndexRadius, CVarSetInteger);
    SET_CVAR(carouselDirectionAngle, CVarSetFloat);

    this->parentState.saveCVars(this->getCVarListerString() + ".defaultState");
}

#undef LOAD_CVAR
#undef SET_CVAR

void CarouselItemSlotLister::resetSlotCount(uint8_t count) {
    std::vector<std::shared_ptr<CarouselItemSlotManager>> newSlots;

    for (size_t i = 0; i < count; i++) {
        if (i < this->carouselSlots.size()) {
            auto existing = this->carouselSlots.at(i);
            existing->arbId = i + 1;
            newSlots.push_back(existing);
        } else {
            auto newSlot = std::shared_ptr<CarouselItemSlotManager>{ new CarouselItemSlotManager(
                "carouselSlotNum" + std::to_string(i + 1), this) };
            newSlots.push_back(newSlot);
        }
    }

    this->slotCount = newSlots.size();
    this->carouselSlots = newSlots;
}

CarouselItemSlotLister::CarouselItemSlotLister(std::string name, CAROUSEL_BUTTONS_TYPE buttons) {
    this->name = name;
    this->buttons = buttons;
    this->options = std::shared_ptr<CarouselListerOptions>(new CarouselListerOptions());

    this->parentState.posLeft = 345;
    this->parentState.posTop = 32;

    this->scrollingState.posTop = 17;

    this->scrollingSelectedState.scale = 1.25;
    this->scrollingSelectedState.rgb[0] = 1;
    this->scrollingSelectedState.rgb[1] = 1;
    this->scrollingSelectedState.rgb[2] = 1;

    this->loadCVars();
    this->resetSlotCount(this->slotCount);
}

ArbitraryItemEquipSet CarouselItemSlotLister::getEquipSlots(PlayState* play, Input* input) {
    play->state.frames;
    // Only process carousel index changes when the frame has changed to prevent the same button press from changing the
    // index twice.
    if (input != NULL && play != NULL && this->processedInputOnFrame != play->state.frames) {
        this->processedInputOnFrame = play->state.frames;
        bool isPaused = play->pauseCtx.state != PAUSE_STATE_OFF;
        bool pauseMenuJustClosed = this->prevWasPaused && !isPaused;
        this->prevWasPaused = isPaused;

        int16_t prev = selectedIndex;
        int8_t direction = 0;

        if (CHECK_INTENT(input->press.intentControls, this->buttons.swapLeft, BUTTON_STATE_PRESS, 0)) {
            direction--;
        }
        if (CHECK_INTENT(input->press.intentControls, this->buttons.swapRight, BUTTON_STATE_PRESS, 0)) {
            direction++;
        }

        selectedIndex += direction;

        // Normalize the index into a valid point withing the index range
        if (prev != selectedIndex) {
            while (selectedIndex < 0) {
                selectedIndex += this->carouselSlots.size();
            }
            while (selectedIndex >= this->carouselSlots.size()) {
                selectedIndex -= this->carouselSlots.size();
            }
        }

        // When outside the pause menu, swap to the next non-disabled item slot.
        // Disable this when paused so that items can be assigned to disabled slots.
        // Don't swap immediately after closing the pause menu so that the game has a moment to update disabled status.
        if (play != NULL && !isPaused && !pauseMenuJustClosed) {
            // If the direction is zero, set to one so that the while-loop below doesn't loop forever.
            if (direction == 0) {
                direction = 1;
            }
            // End the loop if no non-disabled item slot was found.
            size_t iterationCount = 0;
            while (this->carouselSlots.at(selectedIndex)->disabled && iterationCount++ < this->carouselSlots.size()) {
                selectedIndex += direction;
                while (selectedIndex < 0) {
                    selectedIndex += this->carouselSlots.size();
                }
                while (selectedIndex >= this->carouselSlots.size()) {
                    selectedIndex -= this->carouselSlots.size();
                }
            }
            // If no non-disabled item slot was found, then reset the index.
            if (iterationCount >= this->carouselSlots.size()) {
                selectedIndex = prev;
            }
        }

        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::ratio<1LL, 1LL>> elapsedActiveTime = (now - this->activeStarted);
        std::chrono::duration<double, std::ratio<1LL, 1LL>> elapsedSwapTime = (now - this->lastSlotSwap);

        // Set tracking info for animation purposes
        if (prev != selectedIndex) {
            this->previousSelectedIndex = prev;
            this->lastSlotSwap = now;
            if (!this->active) {
                this->active = true;
                this->activeStarted = this->lastSlotSwap;
            }
        } else if (this->active && elapsedSwapTime.count() >= this->lingerSeconds) {
            this->active = false;
            this->inactiveStarted = now;
        }
    }

    this->baseSlots = {};

    for (auto& slot : this->carouselSlots) {
        this->baseSlots.push_back(slot->makeEquipButton());
    }

    ArbitraryItemEquipSet set;
    set.equips = this->baseSlots.data();
    set.count = this->baseSlots.size();

    this->addEquipSetCallbacks(&set);
    return set;
}

void CarouselItemSlotLister::initItemEquips(ItemEquips* equips) {
    equips->equipsSlotGetter.userData = this;
    equips->equipsSlotGetter.getEquipSlots = +[](const ArbitraryEquipsSlotGetter* self, PlayState* play, Input* input) {
        return ((CarouselItemSlotLister*)self->userData)->getEquipSlots(play, input);
    };
}

std::vector<std::weak_ptr<CarouselItemSlotLister>> CarouselItemSlotLister::existingCarousels;

std::shared_ptr<CarouselItemSlotLister> CarouselItemSlotLister::makeCarousel(MulitpleItemSlotLister* parent) {
    uint8_t carouselNumber = 1;
    while(carouselNumber <= 5){
        bool numberWasInUse = false;
        for(auto listerRef : CarouselItemSlotLister::existingCarousels){
            if(!listerRef.expired()){
                auto lister = listerRef.lock();
                if(lister->buttons.carouselNumber == carouselNumber){
                    carouselNumber++;
                    numberWasInUse = true;
                    break;
                }
            }
        }
        if(!numberWasInUse){
            break;
        }
    }

    std::printf("Making carousel #%d\n", carouselNumber);
    auto config = INDEXED_CAROUSELS[carouselNumber - 1];

    auto newLister = std::shared_ptr<CarouselItemSlotLister>{ 
        new CarouselItemSlotLister(
            std::string("Carousel #") + std::to_string(config.carouselNumber), 
            config
        ) 
    };

    CarouselItemSlotLister::existingCarousels.push_back(newLister);

    return newLister;
}