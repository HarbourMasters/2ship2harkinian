#ifndef ACHIEVEMENT_INTEGRATION_H
#define ACHIEVEMENT_INTEGRATION_H

// Standard library
#include <cstdarg>
#include <functional>
#include <map>
#include <vector>

// 2s2h
#include "2s2h/Achievements/StaticData/Types.h"
#include "2s2h/GameInteractor/GameInteractor.h"

namespace Achievements {

namespace Integration {

void Init();

void OnFlagSet(FlagType flagType, u32 flag);

void OnSceneFlagSet(s16 sceneId, FlagType flagType, u32 flag);

void OnVanillaBehavior(GIVanillaBehavior flag, bool* should, va_list originalArgs);

extern std::map<GIVanillaBehavior, std::vector<std::pair<std::function<bool(va_list)>, AchievementEvent>>>
    vanillaBehaviorMap;

} // namespace Integration

} // namespace Achievements

#endif // ACHIEVEMENT_INTEGRATION_H
