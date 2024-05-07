#include "GameInteractor.h"

extern "C" {
#include "z64actor.h"
}

#include <libultraship/bridge.h>

void GameInteractor_ExecuteOnGameStateMainFinish() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnGameStateMainFinish>();
}

void GameInteractor_ExecuteOnGameStateDrawFinish() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnGameStateDrawFinish>();
}

void GameInteractor_ExecuteOnGameStateUpdate() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnGameStateUpdate>();
}

void GameInteractor_ExecuteBeforeEndOfCycleSave() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::BeforeEndOfCycleSave>();
}

void GameInteractor_ExecuteAfterEndOfCycleSave() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::AfterEndOfCycleSave>();
}

void GameInteractor_ExecuteOnSceneInit(s16 sceneId, s8 spawnNum) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSceneInit>(sceneId, spawnNum);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnSceneInit>(sceneId, sceneId, spawnNum);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnSceneInit>(sceneId, spawnNum);
}

void GameInteractor_ExecuteOnRoomInit(s16 sceneId, s8 roomNum) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnRoomInit>(sceneId, roomNum);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnRoomInit>(sceneId, sceneId, roomNum);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnRoomInit>(sceneId, roomNum);
}

bool GameInteractor_ShouldActorInit(Actor* actor) {
    bool result = true;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldActorInit>(actor, &result);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldActorInit>(actor->id, actor, &result);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::ShouldActorInit>((uintptr_t)actor, actor, &result);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldActorInit>(actor, &result);
    return result;
}

void GameInteractor_ExecuteOnActorInit(Actor* actor) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnActorInit>(actor);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnActorInit>(actor->id, actor);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnActorInit>((uintptr_t)actor, actor);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnActorInit>(actor);
}

bool GameInteractor_ShouldActorUpdate(Actor* actor) {
    bool result = true;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldActorUpdate>(actor, &result);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldActorUpdate>(actor->id, actor, &result);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::ShouldActorUpdate>((uintptr_t)actor, actor, &result);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldActorUpdate>(actor, &result);
    return result;
}

void GameInteractor_ExecuteOnActorUpdate(Actor* actor) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnActorUpdate>(actor);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnActorUpdate>(actor->id, actor);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnActorUpdate>((uintptr_t)actor, actor);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnActorUpdate>(actor);
}

bool GameInteractor_ShouldActorDraw(Actor* actor) {
    bool result = true;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldActorDraw>(actor, &result);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldActorDraw>(actor->id, actor, &result);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::ShouldActorDraw>((uintptr_t)actor, actor, &result);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldActorDraw>(actor, &result);
    return result;
}

void GameInteractor_ExecuteOnActorDraw(Actor* actor) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnActorDraw>(actor);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnActorDraw>(actor->id, actor);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnActorDraw>((uintptr_t)actor, actor);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnActorDraw>(actor);
}

void GameInteractor_ExecuteOnActorKill(Actor* actor) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnActorKill>(actor);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnActorKill>(actor->id, actor);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnActorKill>((uintptr_t)actor, actor);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnActorKill>(actor);
}

void GameInteractor_ExecuteOnSceneFlagSet(s16 sceneId, FlagType flagType, u32 flag) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSceneFlagSet>(sceneId, flagType, flag);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnSceneFlagSet>(sceneId, flagType, flag);
}

void GameInteractor_ExecuteOnSceneFlagUnset(s16 sceneId, FlagType flagType, u32 flag) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSceneFlagUnset>(sceneId, flagType, flag);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnSceneFlagUnset>(sceneId, flagType, flag);
}

void GameInteractor_ExecuteOnFlagSet(FlagType flagType, u32 flag) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnFlagSet>(flagType, flag);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnFlagSet>(flagType, flag);
}

void GameInteractor_ExecuteOnFlagUnset(FlagType flagType, u32 flag) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnFlagUnset>(flagType, flag);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnFlagUnset>(flagType, flag);
}

void GameInteractor_ExecuteOnCameraChangeModeFlags(Camera* camera) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnCameraChangeModeFlags>(camera);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnCameraChangeModeFlags>(camera->uid, camera);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnCameraChangeModeFlags>((uintptr_t)camera, camera);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnCameraChangeModeFlags>(camera);
}

void GameInteractor_ExecuteOnOpenText(u16 textId) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnOpenText>(textId);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnOpenText>(textId, textId);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnOpenText>(textId);
}

bool GameInteractor_ShouldItemGive(u8 item) {
    bool result = true;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldItemGive>(item, &result);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldItemGive>(item, item, &result);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldItemGive>(item, &result);
    return result;
}

void GameInteractor_ExecuteOnItemGive(u8 item) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnItemGive>(item);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnItemGive>(item, item);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnItemGive>(item);
}

bool GameInteractor_Should(GIVanillaBehavior flag, bool result, void* opt) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldVanillaBehavior>(flag, &result, opt);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldVanillaBehavior>(flag, flag, &result, opt);
    if (opt != nullptr) {
        GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::ShouldVanillaBehavior>((uintptr_t)opt, flag, &result, opt);
    }
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldVanillaBehavior>(flag, &result, opt);
    return result;
}
