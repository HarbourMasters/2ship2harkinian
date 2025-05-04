#include "AchievementTriggers.h"
#include "Handlers/Handlers.h" // Include a central header for all handler initializers

// Performs one-time setup for the trigger module.
// Currently minimal, mirroring Rando::ActorBehavior::Init.
void AchievementTriggers::Init() {
    // Potential future setup: Create actor extensions if needed, etc.
}

// Registers/unregisters all achievement trigger hooks based on CVars.
// Called via OnSaveLoad hook. Mirrors Rando::MiscBehavior::OnFileLoad structure.
void AchievementTriggers::OnFileLoad() {
    // Call initializer functions for each trigger type handler.
    // These functions (defined in Handlers/*.cpp) will use COND_HOOK/COND_ID_HOOK.
    Handlers::InitFlagSetHandlers();
    Handlers::InitItemGiveHandlers();
    Handlers::InitSceneInitHandlers();
    Handlers::InitActorInitHandlers();
    Handlers::InitEndOfCycleSaveHandlers();
    Handlers::InitVanillaBehaviorHandlers();
    // Add calls for other handlers if new trigger types are added.
} 