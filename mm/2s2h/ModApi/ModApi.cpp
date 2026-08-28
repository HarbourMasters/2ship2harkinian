/**
 * Builds the table mods receive. This is the C++ side of the boundary: it instantiates the
 * GameInteractor's templates, copies each VB's va_list, and reaches the executable's globals,
 * so that a mod never has to.
 */

#include "ModApiHost.h"

#include <spdlog/spdlog.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/BenGui/Notification.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "build.h"
}

// The GI's registration tables are template statics, so every std::function must be built here,
// inside the executable; the variadic lambda forwards each hook's arguments without naming them.
#define DEFINE_HOOK(name, args)                                                     \
    static S2HHookHandle Register##name(S2HCb_##name callback) {                    \
        if (callback == nullptr) {                                                  \
            return 0;                                                               \
        }                                                                           \
        return GameInteractor::Instance->RegisterGameHook<GameInteractor::name>(    \
            [callback](auto&&... hookArgs) { callback(hookArgs...); });             \
    }                                                                               \
    static void Unregister##name(S2HHookHandle handle) {                            \
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::name>(handle); \
    }
#include "2s2h/GameInteractor/GameInteractor_HookTable.h"
#undef DEFINE_HOOK

// Every subscriber of a VB reads the same va_list, so each must work on a copy or it advances the
// list for the others. Copying here means a mod cannot break the game's own enhancements.
static S2HHookHandle RegisterVB(GIVanillaBehavior flag, S2HVbCallback callback) {
    if (callback == nullptr) {
        return 0;
    }

    return GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(
        flag, [callback](GIVanillaBehavior, bool* should, va_list originalArgs) {
            va_list args;
            va_copy(args, originalArgs);
            callback(should, args);
            va_end(args);
        });
}

static void UnregisterVB(S2HHookHandle handle) {
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(handle);
}

static void ApplyCVar(const char* name) {
    if (name == nullptr) {
        return;
    }

    ShipInit::Init(name);
}

static PlayState* GetPlayState() {
    return gPlayState;
}

static SaveContext* GetSaveContext() {
    return &gSaveContext;
}

static RegEditor* GetRegEditor() {
    return gRegEditor;
}

static void LogFromMod(const char* message) {
    if (message == nullptr) {
        return;
    }

    SPDLOG_INFO("[Mod] {}", message);
}

static void NotifyFromMod(const char* message) {
    if (message == nullptr) {
        return;
    }

    Notification::Emit({ .prefix = "[Mod]", .message = message });
}

// Assigned field by field rather than with an initializer list: some entries in the hook table
// carry a trailing semicolon, which is harmless in a declaration but not inside a braced list.
static S2HModApi BuildTable() {
    S2HModApi api = {};

    api.apiVersion = S2H_MOD_API_VERSION;
    api.tableSize = sizeof(S2HModApi);
    api.gameVersion = gBuildVersion;

#define DEFINE_HOOK(name, args)          \
    api.Register##name = Register##name; \
    api.Unregister##name = Unregister##name;
#include "2s2h/GameInteractor/GameInteractor_HookTable.h"
#undef DEFINE_HOOK

    api.RegisterVB = RegisterVB;
    api.UnregisterVB = UnregisterVB;

    api.CVarGetInteger = CVarGetInteger;
    api.CVarSetInteger = CVarSetInteger;
    api.CVarGetFloat = CVarGetFloat;
    api.CVarSetFloat = CVarSetFloat;
    api.CVarApply = ApplyCVar;

    api.GetPlayState = GetPlayState;
    api.GetSaveContext = GetSaveContext;
    api.GetRegEditor = GetRegEditor;

    api.Log = LogFromMod;
    api.Notify = NotifyFromMod;

    return api;
}

const S2HModApi* ModApi_Get() {
    static const S2HModApi sModApi = BuildTable();
    return &sModApi;
}
