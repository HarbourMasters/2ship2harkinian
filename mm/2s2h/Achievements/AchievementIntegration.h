#pragma once

#include "2s2h/GameInteractor/GameInteractor.h"
#include <stdarg.h>
#include <map>
#include <vector>
#include <functional>

namespace Achievements {

namespace Integration {
// Entry point for the module
void Init();
void OnFlagSet(FlagType flagType, u32 flag);

// Hook handlers
void OnSceneFlagSet(s16 sceneId, FlagType flagType, u32 flag);
void OnVanillaBehavior(GIVanillaBehavior flag, bool* should, va_list originalArgs);

// External reference for hook registration in Achievements.cpp
extern std::map<GIVanillaBehavior, std::vector<std::pair<std::function<bool(va_list)>, AchievementEvent>>>
    vanillaBehaviorMap;

} // namespace Integration

} // namespace Achievements