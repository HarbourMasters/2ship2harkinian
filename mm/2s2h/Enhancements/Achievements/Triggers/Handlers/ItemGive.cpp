#include "Handlers.h"
#include "../../AchievementData.h"      // For GetAchievementsFromItemGive
#include "../../Achievements.h"         // For AchievementSystem singleton
#include "2s2h/GameInteractor/GameInteractor.h" // For GameInteractor, COND_HOOK
#include "2s2h/Rando/Rando.h"       // For IS_RANDO
#include "libultraship/libultraship.h" // For CVarGetInteger
#include <spdlog/spdlog.h>
#include <z64item.h> // For ITEM_NONE, ITEM_MASK_DEKU etc.

namespace Handlers {

// Handler function for the OnItemGive hook
void HandleItemGiveTrigger(u8 item) {
    // Get all achievements that have triggerType == OnItemGive.
    std::vector<AchievementStaticData> allOnItemGiveAchievements = GetAchievementsFromItemGive();

    for (const auto& achData : allOnItemGiveAchievements) {
        bool eventParameterMatches = false;

        // Case 1: Achievement is for a specific item ID
        if (achData.checkItemId == item) {
            eventParameterMatches = true;
        }
        // Case 2: Achievement has no specific item ID (ITEM_NONE), so check its itemCategoryFilter (if any)
        else if (achData.checkItemId == ITEM_NONE) {
            if (achData.itemCategoryFilter != nullptr) {
                // If a category filter exists, use it to determine if the item matches.
                if (achData.itemCategoryFilter(item)) {
                    eventParameterMatches = true;
                }
            } else {
                // If checkItemId is ITEM_NONE and there's NO itemCategoryFilter,
                // it means this achievement triggers on *any* item being given.
                // Its additionalCondition is solely responsible for deciding if it unlocks.
                eventParameterMatches = true;
            }
        }
        // If achData.checkItemId is something else (not 'item' and not ITEM_NONE), then this specific 'item' event
        // is not relevant for this achData, so eventParameterMatches remains false.

        if (eventParameterMatches) {
            // Basic pre-checks from CheckCommonAchievementConditions (excluding additionalCondition)
            if (AchievementSystem::Instance().IsLoadingOrInitializing() || 
                AchievementSystem::Instance().IsAchievementUnlocked(achData.id) ||
                !AchievementSystem::Instance().IsAchievementRelevantForGameMode(achData.id, IS_RANDO)) {
                // Skip if loading, already unlocked, or not relevant
            } else {
                if (achData.hasProgressTracking) {
                    AchievementSystem::Instance().UpdateAchievementProgress(achData.id);
                } else {
                    // Not progress-tracked, so check additionalCondition directly for unlock
                    if (achData.additionalCondition == nullptr || achData.additionalCondition()) {
                        SPDLOG_DEBUG("[Achievements] ItemGive (Direct): Item={}, Queuing Achievement: {}", item, achData.name);
                        AchievementSystem::Instance().QueueAchievementUnlock(achData.id);
                    }
                }
            }
        }
    }
}

// Registers the OnItemGive hook
void InitItemGiveHandlers() {
    COND_HOOK(OnItemGive, CVAR_ACHIEVEMENTS, HandleItemGiveTrigger);
}

} // namespace Handlers
