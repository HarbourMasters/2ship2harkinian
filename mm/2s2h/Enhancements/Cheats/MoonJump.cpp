#include "MoonJump.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "variables.h"

static uint32_t moonJumpOnLGameStateUpdateHookId = 0;
void RegisterMoonJumpOnL() {
    if (moonJumpOnLGameStateUpdateHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateUpdate>(
            moonJumpOnLGameStateUpdateHookId);
        moonJumpOnLGameStateUpdateHookId = 0;
    }

    if (CVarGetInteger("gCheats.MoonJumpOnL", 0)) {
        moonJumpOnLGameStateUpdateHookId =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
                if (gPlayState == nullptr)
                    return;

                Player* player = GET_PLAYER(gPlayState);

                if (player != nullptr && CHECK_BTN_ANY(gPlayState->state.input[0].cur.button, BTN_L)) {
                    player->actor.velocity.y = 6.34375f;
                }
            });
    }
}
