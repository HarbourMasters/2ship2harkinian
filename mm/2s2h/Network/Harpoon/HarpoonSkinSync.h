#ifndef HARPOON_SKIN_SYNC_H
#define HARPOON_SKIN_SYNC_H

#include <string>
#include <vector>

namespace HarpoonSkinSync {

// Scans the user's harpoon/skins/ directory for .o2r files and returns the
// display names (filename without extension). Phase G.
std::vector<std::string> ScanLocalCatalog();

// Scans harpoon/gamemodes/ for subfolders containing a gamemode.yaml and
// returns their folder names (gamemode ids). Sent in the handshake's
// installedGamemodes so the server filters the room browser to packs we have.
std::vector<std::string> GetInstalledGamemodes();

// Announce our local catalog + currently-selected slot to the server.
void AnnounceCatalogAndSlots();

// Update a slot name and re-announce.
void SetSlot(const std::string& slotKey, const std::string& skinName);

// Resolve a remote's announced slot name into a local Pak/Resource handle.
// Returns empty string if the user doesn't have that .o2r locally (graceful
// fallback to vanilla rendering).
std::string ResolveSlot(const std::string& announcedName);

} // namespace HarpoonSkinSync

#endif
