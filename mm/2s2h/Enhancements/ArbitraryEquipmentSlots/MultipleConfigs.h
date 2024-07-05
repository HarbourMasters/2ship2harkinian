#ifndef _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_MULTIPLE
#define _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_MULTIPLE
#include "./ArbitraryItemSlots.h"


struct MultipleItemSlotsListerOptions : public ArbitraryItemSlotsListerOptions {
    virtual void drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotLister* lister) override;
};

struct MulitpleItemSlotLister : public ArbitraryItemSlotLister {
    std::vector<std::shared_ptr<ArbitraryItemSlotLister>> subListers;

    MulitpleItemSlotLister(std::string name, std::vector<std::shared_ptr<ArbitraryItemSlotLister>> subListers);

    virtual ArbitraryItemEquipSet getEquipSlots(PlayState *play, Input* input) override;

    virtual void initItemEquips(ItemEquips* equips) override;
};

#endif