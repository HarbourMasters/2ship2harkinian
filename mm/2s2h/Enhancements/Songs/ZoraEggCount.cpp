#include "ZoraEggCount.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "global.h"

extern "C" {
extern PlayState* gPlayState;
#include "z64.h"
#include "src/overlays/actors/ovl_En_Zoraegg/z_en_zoraegg.h"
#include "src/overlays/actors/ovl_En_Mk/z_en_mk.h"
}

const uint32_t MAX_EGGS = 7;

// every egg is active in marine lab regardless of progress, this tracks only the visible ones in tank
static uint32_t visibleEggCount = 0;

void ResetStatics() {
    visibleEggCount = 0;
}

void RegisterZoraEggCount() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeEndOfCycleSave>([]() { ResetStatics(); });
    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeMoonCrashSaveReset>([]() { ResetStatics(); });

    // zora egg object, figures out how many eggs are being drawn
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(
        ACTOR_EN_ZORAEGG, [](Actor* outerActor) {
            static uint32_t enEggUpdateHook = 0;
            static uint32_t enEggKillHook = 0;
            GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::OnActorUpdate>(enEggUpdateHook);
            GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enEggKillHook);
            enEggUpdateHook = 0;
            enEggKillHook = 0;

            EnZoraegg* enZoraEgg = (EnZoraegg*)outerActor;
            if (enZoraEgg->actor.draw != nullptr && gPlayState->sceneId == SCENE_LABO) {
                visibleEggCount++;
            }

            enEggKillHook =
                GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s8 sceneId, s8 spawnNum) {
                    if (sceneId != SCENE_LABO) {
                        visibleEggCount = 0;
                    }
                    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorUpdate>(enEggUpdateHook);
                    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enEggKillHook);
                    enEggUpdateHook = 0;
                    enEggKillHook = 0;
                });
        });

    // marine researcher, his actor update call is more consistent than the eggs
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(ACTOR_EN_MK, [](Actor* outerActor) {
        static uint32_t enMkUpdateHook = 0;
        static uint32_t enMkKillHook = 0;
        GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::OnActorUpdate>(enMkUpdateHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enMkKillHook);
        enMkUpdateHook = 0;
        enMkKillHook = 0;

        enMkUpdateHook = GameInteractor::Instance->RegisterGameHookForPtr<GameInteractor::OnActorUpdate>(
            (uintptr_t)outerActor, [](Actor* actor) {
                // complete quest if you have enough eggs
                if (gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_LABO].unk_14 != MAX_EGGS &&
                    CVarGetInteger("gEnhancements.Songs.ZoraEggCount", MAX_EGGS) <=
                        gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_LABO].unk_14) {
                    gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_LABO].unk_14 = MAX_EGGS;
                }
            });
        enMkKillHook =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s8 sceneId, s8 spawnNum) {
                GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorUpdate>(enMkUpdateHook);
                GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enMkKillHook);
                enMkUpdateHook = 0;
                enMkKillHook = 0;
            });
    });
}