#ifndef _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_CAROUSEL_GUI
#define _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_CAROUSEL_GUI
#include "../ArbitraryItemSlotsGUI.h"

struct CarouselListerOptions : public ArbitraryItemSlotsListerOptions {
    virtual void drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotLister* manager) override;
};

#endif