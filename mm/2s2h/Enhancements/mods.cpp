#include "mods.h"
#include <libultraship/bridge.h>
#include "GameInteractor/GameInteractor.h"

extern "C" {
#include <z64.h>
#include "macros.h"
// #include "functions.h"
// #include "variables.h"
extern PlayState* gPlayState;
}

void RegisterMoonJumpOnL() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
        if (!gPlayState) return;
        
        if (CVarGetInteger("gDeveloperTools.MoonJumpOnL", 0)) {
            Player* player = GET_PLAYER(gPlayState);

            if (CHECK_BTN_ANY(gPlayState->state.input[0].cur.button, BTN_L)) {
                player->actor.velocity.y = 6.34375f;
            }
        }
    });
}

void InitMods() {
    RegisterMoonJumpOnL();
}