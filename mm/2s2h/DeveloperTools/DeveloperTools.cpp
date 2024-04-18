#include "DeveloperTools.h"
#include <libultraship/bridge.h>
#include <spdlog/spdlog.h>
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64actor.h"
}

void RegisterPreventActorUpdateHooks() {
    static HOOK_ID hookId = 0;
    if (hookId != 0) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::ShouldActorUpdate>(hookId);
        hookId = 0;
    }

    if (CVarGetInteger("gDeveloperTools.PreventActorUpdate", 0)) {
        hookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::ShouldActorUpdate>([](Actor* actor, bool* result) {
            *result = false;
        });
    }
}

void RegisterPreventActorDrawHooks() {
    static HOOK_ID hookId = 0;
    if (hookId != 0) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::ShouldActorDraw>(hookId);
        hookId = 0;
    }

    if (CVarGetInteger("gDeveloperTools.PreventActorDraw", 0)) {
        hookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::ShouldActorDraw>([](Actor* actor, bool* result) {
            *result = false;
        });
    }
}

void RegisterPreventActorInitHooks() {
    static HOOK_ID hookId = 0;
    if (hookId != 0) {
        GameInteractor::Instance->UnregisterGameHookForFilter<GameInteractor::ShouldActorInit>(hookId);
        hookId = 0;
    }

    if (CVarGetInteger("gDeveloperTools.PreventActorInit", 0)) {
        hookId = GameInteractor::Instance->RegisterGameHookForFilter<GameInteractor::ShouldActorInit>(GameInteractor::HookFilter::SActorNotPlayer, [](Actor* actor, bool* result) {
            *result = false;
        });
    }
}

void InitDeveloperTools() {
    RegisterPreventActorUpdateHooks();
    RegisterPreventActorDrawHooks();
    RegisterPreventActorInitHooks();
}
