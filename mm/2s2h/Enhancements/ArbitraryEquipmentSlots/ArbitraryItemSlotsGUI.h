#ifndef _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_GUI
#define _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_GUI
#include "Gui.h"
#include "window/gui/GuiWindow.h"
#include <memory>
#include "2s2h/BenGui/UIWidgets.hpp"

struct ArbitraryItemSlotLister;
struct ArbitraryItemSlotManager;

struct ArbitraryItemSlotsWindow : public Ship::GuiWindow {
    ArbitraryItemSlotsWindow();

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;
};

struct ArbitraryItemSlotsListerOptions {
    virtual void drawOptions(ArbitraryItemSlotLister* manager){

    }
};
struct ArbitraryItemSlotsManagerOptions {
    virtual void drawOptions(ArbitraryItemSlotManager* lister){}
};

extern std::shared_ptr<ArbitraryItemSlotsWindow> arbItemSlotsWindow;

#endif