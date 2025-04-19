#include "2s2h/SaveManager/SaveManager.h"
#include "z64.h"
#include "../../Enhancements/Achievements/Achievements.h"

void SaveManager_Migration_7(nlohmann::json& j) {
    if (!j["save"]["shipSaveInfo"].contains("achievementData") ||
        !j["save"]["shipSaveInfo"]["achievementData"].is_object()) {
        j["save"]["shipSaveInfo"]["achievementData"] = nlohmann::json::object();
    }
}