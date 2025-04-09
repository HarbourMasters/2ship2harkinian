#include "Enhancements.h"
#include "Achievements/Achievements.h"

void InitEnhancements() {
    // Cycle & Saving
    RegisterSavingEnhancements();
    RegisterAutosave();
    // Achievements
    InitializeAchievementSystem();

    // Uncomment to enable the demo behavior, this shows of different modding capabilities
    // void RegisterDemoBehavior();
    // RegisterDemoBehavior();
}
