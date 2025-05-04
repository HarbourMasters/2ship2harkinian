#include "Handlers.h"
#include "../../AchievementData.h"      // For GetAchievementFromItemGive, AllAchievementData
#include "../../Achievements.h"         // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"       // For IS_RANDO
#include "libultraship/libultraship.h" // For CVarGetInteger
#include <spdlog/spdlog.h>

namespace Handlers {

// Handler function for the OnItemGive hook
void HandleItemGiveTrigger(u8 item) {
    // OnItemGive can trigger multiple achievements (e.g., specific item + max health check)
    // So we iterate through all achievements.
    for (auto const& [key, achData] : AllAchievementData) {
        if (achData.triggerType == AchievementTriggerType::OnItemGive) {
            // Determine if the specific item trigger matches this achievement's requirements
            bool triggerMatches = false;
            // --- Specific Achievement Item Logic ---
            if (achData.id == AID_RANDO_PLAYAS && (item == ITEM_MASK_DEKU || item == ITEM_MASK_GORON || item == ITEM_MASK_ZORA)) {
                triggerMatches = true;
            } else if (achData.id == AID_COLLECT_ALL_MASKS && (item >= ITEM_MASK_DEKU && item <= ITEM_MASK_FIERCE_DEITY)) {
                triggerMatches = true;
            } else if (achData.id == AID_MAX_HEALTH && (item == ITEM_HEART_CONTAINER || item == ITEM_HEART_PIECE)) {
                triggerMatches = true;
            // --- Default Item Logic ---
            } else if (achData.checkItemId == item) { // Check if the achievement is specifically for this item ID
                triggerMatches = true;
            }

            // If the trigger matches, check common conditions (unlocked, relevant, loading, additional lambda)
            if (triggerMatches && CheckCommonAchievementConditions(achData)) {
                SPDLOG_DEBUG("[Achievements] ItemGive Triggered: Item={}, Queuing Achievement: {}", item, achData.name);
                AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
                // Note: Don't break here, another achievement might trigger on the same item give
            }
        }
    }
}

// Registers the OnItemGive hook
void InitItemGiveHandlers() {
    COND_HOOK(OnItemGive, CVAR_ACHIEVEMENTS, HandleItemGiveTrigger);
}

} // namespace Handlers
