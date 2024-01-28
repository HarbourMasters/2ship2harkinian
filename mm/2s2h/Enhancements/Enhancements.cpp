#include "Enhancements.h"
#include <libultraship/bridge.h>
#include "GameInteractor/GameInteractor.h"

extern "C" {
#include <z64.h>
#include "macros.h"
// #include "functions.h"
// #include "variables.h"
extern PlayState* gPlayState;
}

static uint32_t moonJumpOnLGameStateUpdateHookId = 0;
void RegisterMoonJumpOnL() {
    if (moonJumpOnLGameStateUpdateHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateUpdate>(moonJumpOnLGameStateUpdateHookId);
        moonJumpOnLGameStateUpdateHookId = 0;
    }

    if (CVarGetInteger("gDeveloperTools.MoonJumpOnL", 0)) {
        moonJumpOnLGameStateUpdateHookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
            if (!gPlayState) return;

            Player* player = GET_PLAYER(gPlayState);

            if (CHECK_BTN_ANY(gPlayState->state.input[0].cur.button, BTN_L)) {
                player->actor.velocity.y = 6.34375f;
            }
        });
    }
}

void InitEnhancements() {
    RegisterMoonJumpOnL();
}