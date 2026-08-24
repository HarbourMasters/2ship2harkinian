// =============================================================================
// HarpoonSkinSync — catalog sync of .o2r skin mods (Phase G, stub).
//
// This implements the file-system catalog scan + announce protocol. Actual
// rendering of the resolved skin on a dummy player requires the Pak Loader
// integration that's still being plumbed in 2ship; until then, ResolveSlot()
// just returns the name as-is and the dummy renderer ignores it.
// =============================================================================

#include "HarpoonSkinSync.h"
#include "Harpoon.h"

#include <ship/Context.h>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace HarpoonSkinSync {

namespace {
std::filesystem::path HarpoonRoot() {
    auto base = Ship::Context::GetRawInstance()->GetAppDirectoryPath();
    return std::filesystem::path(base) / "harpoon";
}
std::filesystem::path SkinsDir() {
    return HarpoonRoot() / "skins";
}
std::filesystem::path GamemodesDir() {
    return HarpoonRoot() / "gamemodes";
}
} // namespace

std::vector<std::string> GetInstalledGamemodes() {
    std::vector<std::string> out;
    std::error_code ec;
    auto dir = GamemodesDir();
    if (!std::filesystem::exists(dir, ec))
        return out;
    for (auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_directory())
            continue;
        if (std::filesystem::exists(entry.path() / "gamemode.yaml", ec)) {
            out.push_back(entry.path().filename().string());
        }
    }
    return out;
}

std::vector<std::string> ScanLocalCatalog() {
    std::vector<std::string> out;
    std::error_code ec;
    auto dir = SkinsDir();
    if (!std::filesystem::exists(dir, ec))
        return out;
    for (auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file())
            continue;
        auto p = entry.path();
        if (p.extension() == ".o2r") {
            out.push_back(p.stem().string());
        }
    }
    return out;
}

void AnnounceCatalogAndSlots() {
    auto* h = Harpoon::Instance();
    if (h->State() < HarpoonConnState::Connected)
        return;

    auto catalog = ScanLocalCatalog();
    nlohmann::json announce = {
        { "type", HarpoonPT::APPEARANCE_SKIN_ANNOUNCE },
        { "payload", { { "catalog", catalog } } },
    };
    h->SendJson(announce);

    nlohmann::json slots = {
        { "type", HarpoonPT::APPEARANCE_SKIN_UPDATE },
        { "payload",
          {
              { "clientId", h->OwnClientId() },
              { "adultSkinName", CVarGetString("gNetwork.Harpoon.Skin.Adult", "") },
              { "equipSkinName", CVarGetString("gNetwork.Harpoon.Skin.Equip", "") },
          } },
    };
    h->SendJson(slots);
}

void SetSlot(const std::string& slotKey, const std::string& skinName) {
    if (slotKey == "adult")
        CVarSetString("gNetwork.Harpoon.Skin.Adult", skinName.c_str());
    else if (slotKey == "equip")
        CVarSetString("gNetwork.Harpoon.Skin.Equip", skinName.c_str());
    AnnounceCatalogAndSlots();
}

std::string ResolveSlot(const std::string& announcedName) {
    auto catalog = ScanLocalCatalog();
    for (const auto& s : catalog)
        if (s == announcedName)
            return s;
    return ""; // fallback to vanilla rendering
}

} // namespace HarpoonSkinSync
