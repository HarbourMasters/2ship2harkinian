#include "InvisibleEnemies.h"
#include "libultraship/libultraship.h"
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "macros.h"
#include "z64.h"
extern PlayState* gPlayState;
}

void RegisterInvisibleEnemies() {
    static HOOK_ID enemyLensReactHookId = 0;
    static HOOK_ID roomLensModeHookId = 0;
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorInit>(enemyLensReactHookId);
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnRoomInit>(roomLensModeHookId);

    if (CVarGetInteger("gModes.InvisibleEnemies", 0)) {
        enemyLensReactHookId =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorInit>([](Actor* actor) {
                if (actor->category == ACTORCAT_ENEMY) {
                    actor->flags |= ACTOR_FLAG_REACT_TO_LENS;
                }
            });

        roomLensModeHookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnRoomInit>(
            [](s8 sceneId, s8 roomNum) { gPlayState->roomCtx.curRoom.lensMode = LENS_MODE_HIDE_ACTORS; });
    }
}
