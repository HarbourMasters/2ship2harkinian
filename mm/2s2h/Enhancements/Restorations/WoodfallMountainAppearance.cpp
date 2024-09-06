
#include <libultraship/bridge.h>
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64save.h"
#include "overlays/actors/ovl_Bg_Breakwall/z_bg_breakwall.h"
}

void RegisterWoodfallMountainAppearance() {
    static HOOK_ID breakwallInitID = 0;
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorInit>(breakwallInitID);
    breakwallInitID = 0;

    if (!CVarGetInteger("gEnhancements.Restorations.WoodfallMountainAppearance", 0)) {
        return;
    }

    breakwallInitID = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(
        ACTOR_BG_BREAKWALL, [](Actor* actor) {
            if (BGBREAKWALL_GET_F(actor) == 2 && CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)) {
                actor->params = (actor->params & 0xFFF0) | 3;
            }
        });
}
