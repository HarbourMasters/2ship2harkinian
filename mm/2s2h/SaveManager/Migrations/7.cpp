#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"

void SaveManager_Migration_7(nlohmann::json& j) {
    // If achievements array doesn't exist, create it and initialize to all zeros
    if (!j["save"]["shipSaveInfo"].contains("achievements")) {
        // Create an array of 8 zeros for the achievements
        u32 achievements[8] = { 0 };
        j["save"]["shipSaveInfo"]["achievements"] = achievements;
    }
}