#include "2s2h/Network/Anchor/Anchor.h"
#include "2s2h/BenGui/Notification.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "variables.h"
#include "functions.h"
extern PlayState* gPlayState;
}

/**
 * GIVE_ITEM
 *
 * Sent when the local player is granted an item (via Item_Give). Applied on teammates so found
 * items are shared. The isApplyingRemotePacket guard prevents the applied Item_Give from echoing
 * back out through the OnItemGive hook.
 *
 * Like SoH/Anchor, ALL items are shared, but only "notable" items raise a notification. Junk drops
 * and refills (rupees, recovery hearts, magic jars, ammo) are still given silently.
 */

// Mirrors SoH's ITEM_CATEGORY_JUNK: the contiguous block of drops/refills. MM has no item-category
// enum, so we test the id ranges directly.
static bool Anchor_IsJunkItem(u8 item) {
    return item == ITEM_MAGIC_JAR_SMALL || item == ITEM_MAGIC_JAR_BIG ||
           (item >= ITEM_RECOVERY_HEART && item <= ITEM_BOMBCHUS_5);
}

void Anchor::SendPacket_GiveItem(u8 item) {
    if (!IsSaveLoaded() || isApplyingRemotePacket || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = GIVE_ITEM;
    payload["targetTeamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");
    payload["addToQueue"] = true;
    payload["item"] = item;

    SendPacket(payload);
}

void Anchor::HandlePacket_GiveItem(nlohmann::json payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    uint32_t clientId = payload.at("clientId").get<uint32_t>();
    u8 item = payload.at("item").get<u8>();

    isApplyingRemotePacket = true;
    Item_Give(gPlayState, item);
    isApplyingRemotePacket = false;

    // Share everything, but don't spam notifications for junk drops/refills (rupees, hearts, ammo).
    if (!Anchor_IsJunkItem(item) && clients.contains(clientId)) {
        Notification::Emit({
            .prefix = clients[clientId].name,
            .message = "shared an item",
        });
    }
}
