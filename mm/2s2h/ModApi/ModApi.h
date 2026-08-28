#ifndef S2H_MOD_API_H
#define S2H_MOD_API_H

#include <stdint.h>

#include "ModApiHooks.h"

// A mod is refused unless its manifest declares this same code_version.
#define S2H_MOD_API_VERSION 1

#ifdef _WIN32
#define S2H_MOD_EXPORT __declspec(dllexport)
#else
#define S2H_MOD_EXPORT __attribute__((visibility("default")))
#endif

typedef uint32_t S2HHookHandle;

typedef struct {
    // Keep these first: a mod reads them to decide whether the rest of the layout is its own.
    uint32_t apiVersion;
    uint32_t tableSize;
    const char* gameVersion;

#define DEFINE_HOOK(name, args)                             \
    S2HHookHandle (*Register##name)(S2HCb_##name callback); \
    void (*Unregister##name)(S2HHookHandle handle);
#include "2s2h/GameInteractor/GameInteractor_HookTable.h"
#undef DEFINE_HOOK

    // Write false to `should` to replace the vanilla behaviour; read the extra arguments with
    // va_arg, in the order the call site in mm/src passes them.
    S2HHookHandle (*RegisterVB)(GIVanillaBehavior flag, S2HVbCallback callback);
    void (*UnregisterVB)(S2HHookHandle handle);

    int32_t (*CVarGetInteger)(const char* name, int32_t defaultValue);
    void (*CVarSetInteger)(const char* name, int32_t value);
    float (*CVarGetFloat)(const char* name, float defaultValue);
    void (*CVarSetFloat)(const char* name, float value);
    // Enhancements re-register through ShipInit; a changed CVar does nothing until this runs.
    void (*CVarApply)(const char* name);

    PlayState* (*GetPlayState)(void);
    SaveContext* (*GetSaveContext)(void);
    // Assign to a mod-local `gRegEditor` and the game's REG macros work inside the mod.
    RegEditor* (*GetRegEditor)(void);

    void (*Log)(const char* message);
    void (*Notify)(const char* message);
} S2HModApi;

#define S2H_MOD_API_MATCHES(api) \
    ((api) != NULL && (api)->apiVersion == S2H_MOD_API_VERSION && (api)->tableSize == sizeof(S2HModApi))

typedef void (*S2HModSetApiFunc)(const S2HModApi* api);
typedef void (*S2HModInitFunc)(void);
typedef void (*S2HModExitFunc)(void);

#endif // S2H_MOD_API_H
