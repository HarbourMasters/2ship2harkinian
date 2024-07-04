#include "CarouselItemSlots.h"
#include <libultraship/libultraship.h> // TODO: Without this import, below imports break a bunch of internal `extern` imports deeper in the 2ship include tree for some reason
extern "C" {
#include <z64.h>
#include <macros.h>
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
}
#include "./guis/CarouselGUI.h"

CarouselItemSlotManager::CarouselItemSlotManager(uint16_t id, CarouselItemSlotLister* lister): ArbitraryItemSlotManager(id) {
    this->lister = lister;
    this->scrollPosition = 0;
}

int32_t CarouselItemSlotManager::getLeftOffset(int16_t index){
    int8_t negative = index < 0 ? -1 : 1;
    int abs = index * negative;
    abs %= this->lister->carouselSlots.size();
    
    if(negative < 0){
        abs *= -1;
        abs += this->lister->carouselSlots.size();
        abs %= this->lister->carouselSlots.size();
    }

    auto it = std::find_if(
        this->lister->carouselSlots.begin(),
        this->lister->carouselSlots.end(),
        [this](
            std::shared_ptr<CarouselItemSlotManager> slot
        ){
            return slot.get() == this;
        }
    );
    long thisIndex = it - this->lister->carouselSlots.begin();

    long dist = thisIndex - abs;

    if(std::abs(dist) > ((double)this->lister->carouselSlots.size() / 2.0 )){
        if(dist < 0){
            dist += this->lister->carouselSlots.size();
        }
        else {
            dist -= this->lister->carouselSlots.size();
        }
    }

    return dist;
}

ArbitraryItemDrawParams CarouselItemSlotManager::getDrawParams(PlayState *play){
    std::chrono::duration<double, std::ratio<1LL, 1LL>> elapsedTime =
        (std::chrono::high_resolution_clock::now() - this->lister->lastSlotSwap);

    auto dist = this->getLeftOffset(this->lister->selectedIndex);
    auto prevDist = this->getLeftOffset(this->lister->previousSelectedIndex);

    double targetLeft = 27 * dist;
    double prevLeft = 27 * prevDist;

    double stepSize = ((targetLeft - prevLeft) / 10.0);

    if(stepSize >= 0 && this->scrollPosition > targetLeft){
        this->scrollPosition = targetLeft;
    }
    else if(stepSize < 0 && this->scrollPosition < targetLeft) {
        this->scrollPosition = targetLeft;
    }

    if(std::abs(targetLeft - this->scrollPosition) < std::abs(stepSize)){
        this->scrollPosition = targetLeft;
    }
    else {
        this->scrollPosition += stepSize;
    }
    
    this->drawParams.r = this->lister->rgb[0] * 255;
    this->drawParams.g = this->lister->rgb[1] * 255;
    this->drawParams.b = this->lister->rgb[2] * 255;

    double alpha = std::pow(0.5, std::abs(dist)) * 255.0;

    bool isGameUnpaused = play->pauseCtx.state == PAUSE_STATE_OFF;
    this->drawParams.a = alpha;

    auto result = this->drawParams;
    float angleRads = this->lister->carouselDirectionAngle;
    result.rectLeft = std::cos(angleRads) * this->scrollPosition;
    result.rectTop = std::sin(angleRads) * this->scrollPosition;

    int32_t centerPositionLeft = this->lister->restPositionLeft;
    int32_t centerPositionTop = this->lister->restPositionTop;

    int32_t diffLeft = this->lister->restPositionLeft - this->lister->activePositionLeft;
    int32_t diffTop = this->lister->restPositionTop - this->lister->activePositionTop;

    if(this->disabled){
        result.r = 128;
        result.g = 0;
        result.b = 0;
    }

    if(this->lister->active){
            std::chrono::duration<double, std::ratio<1LL, 1LL>> time =
                (std::chrono::high_resolution_clock::now() - this->lister->activeStarted);

            double timeSince = time.count();
            double ratio = timeSince / this->lister->fadeTimeSeconds;
            double angle = std::clamp(ratio * (M_PI / 2), 0.0, M_PI / 2.0);

        if(this->lister->activePositionDifferent){
            centerPositionLeft = this->lister->restPositionLeft - std::sin(angle) * (diffLeft);
            centerPositionTop = this->lister->restPositionTop - std::sin(angle) * (diffTop);
        }

        if(isGameUnpaused && dist != 0){
            result.a = alpha * std::sin(angle);
        }

        if(dist == 0){
            result.r += (255 - result.r) * std::sin(angle);
            result.g += (255 - result.g) * std::sin(angle);
            result.b += (255 - result.b) * std::sin(angle);
        }
    }
    else {
        std::chrono::duration<double, std::ratio<1LL, 1LL>> time =
            (std::chrono::high_resolution_clock::now() - this->lister->inactiveStarted);

        double timeSince = time.count();
        double ratio = timeSince / this->lister->fadeTimeSeconds;
        double angle = std::clamp(ratio * (M_PI / 2), 0.0, M_PI / 2.0);

        if(this->lister->activePositionDifferent){
            centerPositionLeft = this->lister->restPositionLeft - std::cos(angle) * (diffLeft);
            centerPositionTop = this->lister->restPositionTop - std::cos(angle) * (diffTop);
        }
        
        if(isGameUnpaused && dist != 0){
            result.a = alpha * std::cos(angle);
        }

        if(dist == 0){
            result.r += (255 - result.r) * std::cos(angle);
            result.g += (255 - result.g) * std::cos(angle);
            result.b += (255 - result.b) * std::cos(angle);
        }
    }
    
    result.rectLeft += centerPositionLeft;
    result.rectTop += centerPositionTop;

    return result;
}

bool CarouselItemSlotManager::isSelectedSlot(){
    int8_t negative = this->lister->selectedIndex < 0 ? -1 : 1;
    int abs = this->lister->selectedIndex * negative;
    abs %= this->lister->carouselSlots.size();
    
    if(negative < 0){
        abs *= -1;
        abs += this->lister->carouselSlots.size();
        abs %= this->lister->carouselSlots.size();
    }

    auto it = std::find_if(
        this->lister->carouselSlots.begin(),
        this->lister->carouselSlots.end(),
        [this](
            std::shared_ptr<CarouselItemSlotManager> slot
        ){
            return slot.get() == this;
        }
    );
    long thisIndex = it - this->lister->carouselSlots.begin();

    return thisIndex == abs;
}

uint8_t CarouselItemSlotManager::canTakeAssignment(ItemId item){
    if(!this->isSelectedSlot()){
        return 0;
    }

    return true;
}
uint8_t CarouselItemSlotManager::assignmentTriggered(Input* input){
    if(!this->isSelectedSlot()){
        return 0;
    }

    return CHECK_INTENT(input->press.intentControls, this->lister->equipButtonIntent, BUTTON_STATE_PRESS, 0);
}
uint8_t CarouselItemSlotManager::activateItem(Input* input, uint8_t buttonState){
    if(!this->isSelectedSlot()){
        return 0;
    }
    else if(this->disabled){
        return 0;
    }

    return CHECK_INTENT(input->press.intentControls, this->lister->equipButtonIntent, buttonState, 0);
}
uint8_t CarouselItemSlotManager::tradeItem(Input* input){
    if(!this->isSelectedSlot()){
        return 0;
    }

    return CHECK_INTENT(input->cur.intentControls, this->lister->equipButtonIntent, BUTTON_STATE_PRESS, 0);
}

void CarouselItemSlotLister::resetSlotCount(uint8_t count){
    std::vector<std::shared_ptr<CarouselItemSlotManager>> newSlots;

    for(size_t i = 0; i < count; i++){
        if(i < this->carouselSlots.size()){
            auto existing = this->carouselSlots.at(i);
            existing->arbId = i + 1;
            newSlots.push_back(existing);
        }
        else {
            auto newSlot = std::shared_ptr<CarouselItemSlotManager>{ new CarouselItemSlotManager(i + 1, this) };
            newSlots.push_back(newSlot);
        }
    }

    this->slotCount = newSlots.size();
    this->carouselSlots = newSlots;
}

CarouselItemSlotLister::CarouselItemSlotLister(uint16_t equipButtonIntent, uint16_t swapLeftIntent, uint16_t swapRightIntent){
    this->resetSlotCount(this->slotCount);
    this->equipButtonIntent = equipButtonIntent;
    this->swapLeftIntent = swapLeftIntent;
    this->swapRightIntent = swapRightIntent;
    this->options = std::shared_ptr<CarouselListerOptions>(new CarouselListerOptions());
}

ArbitraryItemEquipSet CarouselItemSlotLister::getEquipSlots(PlayState *play, Input* input){
    play->state.frames;
    // Only process carousel index changes when the frame has changed to prevent the same button press from changing the index twice.
    if (input != NULL && play != NULL && this->processedInputOnFrame != play->state.frames){
        this->processedInputOnFrame = play->state.frames;
        bool isPaused = play->pauseCtx.state != PAUSE_STATE_OFF;
        bool pauseMenuJustClosed = this->prevWasPaused && !isPaused;
        this->prevWasPaused = isPaused;

        int16_t prev = selectedIndex;
        int8_t direction = 0;

        if(CHECK_INTENT(input->press.intentControls, this->swapLeftIntent, BUTTON_STATE_PRESS, 0)){
            direction--;
        }
        if(CHECK_INTENT(input->press.intentControls, this->swapRightIntent, BUTTON_STATE_PRESS, 0)){
            direction++;
        }

        selectedIndex += direction;

        // Normalize the index into a valid point withing the index range
        if(prev != selectedIndex){
            while(selectedIndex < 0){
                selectedIndex += this->carouselSlots.size();
            }
            while(selectedIndex >= this->carouselSlots.size()){
                selectedIndex -= this->carouselSlots.size();
            }
        }

        // When outside the pause menu, swap to the next non-disabled item slot.
        // Disable this when paused so that items can be assigned to disabled slots.
        // Don't swap immediately after closing the pause menu so that the game has a moment to update disabled status.
        if(play != NULL && !isPaused && !pauseMenuJustClosed){
            // If the direction is zero, set to one so that the while-loop below doesn't loop forever.
            if(direction == 0 ){
                direction = 1;
            }
            // End the loop if no non-disabled item slot was found.
            size_t iterationCount = 0;
            while(this->carouselSlots.at(selectedIndex)->disabled && iterationCount++ < this->carouselSlots.size()){
                selectedIndex += direction;
                while(selectedIndex < 0){
                    selectedIndex += this->carouselSlots.size();
                }
                while(selectedIndex >= this->carouselSlots.size()){
                    selectedIndex -= this->carouselSlots.size();
                }
            }
            // If no non-disabled item slot was found, then reset the index.
            if(iterationCount >= this->carouselSlots.size()){
                selectedIndex = prev;
            }
        }

        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::ratio<1LL, 1LL>> elapsedActiveTime =
            (now - this->activeStarted);
        std::chrono::duration<double, std::ratio<1LL, 1LL>> elapsedSwapTime =
            (now - this->lastSlotSwap);

        // Set tracking info for animation purposes
        if(prev != selectedIndex){
            this->previousSelectedIndex = prev;
            this->lastSlotSwap = now;
            if(!this->active){
                this->active = true;
                this->activeStarted = this->lastSlotSwap;
            }
        }
        else if(this->active && elapsedSwapTime.count() >= this->lingerSeconds) {
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

void CarouselItemSlotLister::initItemEquips(ItemEquips* equips){
    equips->equipsSlotGetter.userData = this;
    equips->equipsSlotGetter.getEquipSlots = +[](ArbitraryEquipsSlotGetter* self, PlayState* play, Input* input) {
        return ((CarouselItemSlotLister*)self->userData)->getEquipSlots(play, input);
    };
}
