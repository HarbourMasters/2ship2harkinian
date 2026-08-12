#include "Spoiler.h"
#include <fstream>
#include "BenPort.h"
#include "Rando/Rando.h" // gSaveContext / RANDO_SAVE_* (finalSeed for the area sidecar)

namespace Rando {

namespace Spoiler {

void SaveToFile(const std::string& fileName, nlohmann::json spoiler) {
    std::string filePath = Ship::Context::GetPathRelativeToAppDirectory("randomizer/" + fileName, appShortName);
    std::ofstream fileStream(filePath);
    if (!fileStream.is_open()) {
        throw std::runtime_error("Failed to open spoiler file");
    }

    fileStream << spoiler.dump(4);
}

nlohmann::json LoadFromFile(const std::string& fileName) {
    std::string spoilerFilePath = Ship::Context::GetPathRelativeToAppDirectory("randomizer/" + fileName, appShortName);
    std::ifstream fileStream(spoilerFilePath);
    if (!fileStream.is_open()) {
        throw std::runtime_error("Failed to open spoiler file");
    }

    nlohmann::json spoiler;
    try {
        fileStream >> spoiler;
    } catch (nlohmann::json::exception& e) { throw std::runtime_error("Failed to parse spoiler file"); }

    if (!spoiler.contains("type") || spoiler["type"] != "2S2H_RANDO_SPOILER") {
        throw std::runtime_error("Spoiler file is not a valid spoiler file");
    }

    return spoiler;
}

// ---- Cross-game hints: sidecar holding the OoT areas ----
// Stored apart from the spoiler because the spoiler is only read when the file is created, while
// hints are queried at any point during play (and after restarting the game). Keyed by the save's
// finalSeed, so a save from a different seed can never read the wrong areas. Skijer's NEI

static std::string OotAreasFileName(uint32_t finalSeed) {
    return "combo_ootareas_" + std::to_string(finalSeed) + ".json";
}

void SaveOotItemAreas(uint32_t finalSeed, const nlohmann::json& ootItemAreas) {
    if (ootItemAreas.is_null() || !ootItemAreas.is_object() || ootItemAreas.empty()) {
        return;
    }
    try {
        std::string filePath =
            Ship::Context::GetPathRelativeToAppDirectory("randomizer/" + OotAreasFileName(finalSeed), appShortName);
        std::ofstream fileStream(filePath);
        if (fileStream.is_open()) {
            fileStream << ootItemAreas.dump(4);
        }
    } catch (...) {
        // Without the sidecar the cross-game hints say "Unknown Location"; not a reason to abort.
    }
}

std::string GetOotAreaForItem(const std::string& riSpoilerName) {
    static nlohmann::json sCache;
    static uint32_t sCachedSeed = 0;
    static bool sTried = false;

    uint32_t finalSeed = gSaveContext.save.shipSaveInfo.rando.finalSeed;
    if (finalSeed == 0 || riSpoilerName.empty()) {
        return "";
    }
    if (!sTried || sCachedSeed != finalSeed) {
        sTried = true;
        sCachedSeed = finalSeed;
        sCache = nlohmann::json::object();
        try {
            std::string filePath =
                Ship::Context::GetPathRelativeToAppDirectory("randomizer/" + OotAreasFileName(finalSeed), appShortName);
            std::ifstream fileStream(filePath);
            if (fileStream.is_open()) {
                fileStream >> sCache;
            }
        } catch (...) { sCache = nlohmann::json::object(); }
        if (!sCache.is_object()) {
            sCache = nlohmann::json::object();
        }
    }
    auto it = sCache.find(riSpoilerName);
    if (it != sCache.end() && it->is_string()) {
        return it->get<std::string>();
    }
    return "";
}

} // namespace Spoiler

} // namespace Rando
