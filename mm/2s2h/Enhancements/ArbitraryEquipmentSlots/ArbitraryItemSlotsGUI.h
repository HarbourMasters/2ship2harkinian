#ifndef _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_GUI
#define _2SHIP_ENHANCEMENT_ARBITRARY_ITEM_SLOTS_GUI
#include "Gui.h"
#include "window/gui/GuiWindow.h"
#include <memory>
#include "2s2h/BenGui/UIWidgets.hpp"
#include "InputEditorWindow.h"
#include "./SlotState.h"

struct ArbitraryItemSlotLister;
struct ArbitraryItemSlotManager;

struct ArbitraryItemSlotsWindow : public Ship::GuiWindow {

    ArbitraryItemSlotsWindow();

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;
};

struct ArbitraryItemSlotsListerOptions {
    virtual void drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotLister* manager);
    virtual void drawSlotState(ArbitraryItemSlotsWindow* win, SlotState* state, std::string appendex);
};
struct ArbitraryItemSlotsManagerOptions {
    virtual void drawOptions(ArbitraryItemSlotsWindow* win, ArbitraryItemSlotManager* lister) {
    }
};

extern std::shared_ptr<ArbitraryItemSlotsWindow> arbItemSlotsWindow;

#endif