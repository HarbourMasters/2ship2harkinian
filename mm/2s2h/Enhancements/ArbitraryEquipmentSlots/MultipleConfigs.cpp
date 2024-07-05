#include "./MultipleConfigs.h"

void MultipleItemSlotsListerOptions::drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotLister* lister) {
    auto mulLister = (MulitpleItemSlotLister*) lister;
    
    ImGui::BeginTabBar("Multiple Lister Tabs");
    for(auto sub : mulLister->subListers){
        std::string label = sub->name;
        if(ImGui::BeginTabItem(label.c_str())){
            sub->options->drawOptions(win, sub.get());
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
}

MulitpleItemSlotLister::MulitpleItemSlotLister(std::string name, std::vector<std::shared_ptr<ArbitraryItemSlotLister>> subListers){
    this->name = name;
    this->subListers = subListers;
    this->options = std::shared_ptr<ArbitraryItemSlotsListerOptions>(
        new MultipleItemSlotsListerOptions()
    );
}

ArbitraryItemEquipSet MulitpleItemSlotLister::getEquipSlots(PlayState *play, Input* input) {
    this->baseSlots = {};

    for(auto sub : this->subListers){
        auto set = sub->getEquipSlots(play, input);
        for(size_t i = 0; i < set.count; i++){
            this->baseSlots.push_back(set.equips[i]);
        }
    }

    ArbitraryItemEquipSet set;
    set.equips = this->baseSlots.data();
    set.count = this->baseSlots.size();
    this->addEquipSetCallbacks(&set);

    return set;
}

void MulitpleItemSlotLister::initItemEquips(ItemEquips* equips) {
    equips->equipsSlotGetter.userData = this;
    equips->equipsSlotGetter.getEquipSlots = +[](ArbitraryEquipsSlotGetter* self, PlayState* play, Input* input) {
        return ((MulitpleItemSlotLister*)self->userData)->getEquipSlots(play, input);
    };
}
