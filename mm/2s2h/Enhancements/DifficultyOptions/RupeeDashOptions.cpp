#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "global.h"

uint32_t bleedInterval = 0;

void RupeeReductionEffect(s8 amount) {
    if (CVarGetInteger("gEnhancements.Difficulty.RupeeSound", 0)) {
        gSaveContext.save.saveInfo.playerData.rupees -= amount;
    } else {
        Rupees_ChangeBy(-amount);
    }
}

void RegisterRupeeDash() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(ACTOR_PLAYER, [](Actor* outerActor) {
        static uint32_t playerUpdateHook = 0;
        static uint32_t playerKillHook = 0;
        GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::OnActorUpdate>(playerUpdateHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(playerKillHook);
        playerUpdateHook = 0;
        playerKillHook = 0;

        playerUpdateHook = GameInteractor::Instance->RegisterGameHookForPtr<GameInteractor::OnActorUpdate>(
                (uintptr_t)outerActor, [](Actor* actor) {
            if (CVarGetInteger("gEnhancements.Difficulty.RupeeBleed", 0)) {
                    if (bleedInterval <= 0) {
                        if (gSaveContext.save.saveInfo.playerData.rupees >= 1) {
                            s8 walletSize = CUR_UPG_VALUE(UPG_WALLET) + 1;
                            RupeeReductionEffect(walletSize);
                        } else {
                            gSaveContext.save.saveInfo.playerData.health -= 0x10;
                        }
                        bleedInterval = (CVarGetInteger("gEnhancements.Difficulty.BleedInterval", 0) * 20);
                    }
                    bleedInterval--;
            } else {
                playerKillHook = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s8 sceneId, s8 spawnNum) {
                        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorUpdate>(playerUpdateHook);
                        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(playerKillHook);
                });
            }
        });
    });
}