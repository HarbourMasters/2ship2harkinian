#ifndef SOULS_H
#define SOULS_H

#include "Rando/Rando.h"

extern std::unordered_map<RandoItemId, std::tuple<std::function<void()>, std::vector<ActorId>, RandoInf>> soulMap;

#endif // SOULS_
