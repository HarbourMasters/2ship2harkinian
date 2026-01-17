#ifndef RANDO_ACTOR_BEHAVIOR_CURIOSITY_SHOP_REFILLS_H
#define RANDO_ACTOR_BEHAVIOR_CURIOSITY_SHOP_REFILLS_H

#include <array>
#include <cstddef>

#include "Rando/Types.h"

namespace Rando::ActorBehavior {

constexpr s16 sCuriosityShopRefillBaseId = 0x4000;

struct CuriosityShopRefillEntry {
    RandoItemId itemId;
    RandoCheckId prerequisiteCheck;
    RandoItemId prerequisiteItemId;
    u16 defaultPrice;
};

inline constexpr std::array<CuriosityShopRefillEntry, 3> sCuriosityShopRefills = {
    CuriosityShopRefillEntry{ RI_SEAHORSE, RC_PINNACLE_ROCK_REUNITE_SEAHORSE, RI_UNKNOWN, 100 },
    CuriosityShopRefillEntry{ RI_GOLD_DUST_REFILL, RC_UNKNOWN, RI_BOTTLE_GOLD_DUST, 200 },
    CuriosityShopRefillEntry{ RI_CHATEAU_ROMANI_REFILL, RC_UNKNOWN, RI_BOTTLE_CHATEAU_ROMANI, 200 },
};

inline bool IsCuriosityShopRefillShopId(s16 shopId) {
    return shopId >= sCuriosityShopRefillBaseId &&
           shopId < sCuriosityShopRefillBaseId + static_cast<s16>(sCuriosityShopRefills.size());
}

inline size_t GetCuriosityShopRefillIndex(s16 shopId) {
    return static_cast<size_t>(shopId - sCuriosityShopRefillBaseId);
}

} // namespace Rando::ActorBehavior

#endif
