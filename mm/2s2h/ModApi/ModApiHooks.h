#ifndef S2H_MOD_API_HOOKS_H
#define S2H_MOD_API_HOOKS_H

#include <stdarg.h>

#include "2s2h/GameInteractor/GameInteractor.h"

#define DEFINE_HOOK(name, args) typedef void(*S2HCb_##name) args;
#include "2s2h/GameInteractor/GameInteractor_HookTable.h"
#undef DEFINE_HOOK

// The host va_copy's before calling, so reading `args` cannot disturb the VB's other subscribers.
typedef void (*S2HVbCallback)(bool* should, va_list args);

#endif // S2H_MOD_API_HOOKS_H
