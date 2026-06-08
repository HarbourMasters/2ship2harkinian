#include "2s2h/Network/Anchor/Anchor.h"
#include "2s2h/BenGui/Notification.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include <vector>

extern "C" {
#include "macros.h"
#include "variables.h"
extern PlayState* gPlayState;
}

using json = nlohmann::json;

/**
 * REQUEST_TEAM_STATE / UPDATE_TEAM_STATE
 *
 * On loading a save (or connecting while already in game) a client requests the team's current
 * save state. A teammate responds with UPDATE_TEAM_STATE, a curated subset of gSaveContext
 * (progression-relevant fields), which the requester adopts. Also pushed on saving the game so the
 * server's last-known team state stays fresh for late joiners.
 *
 * This is what reconciles save *values* (magic, hearts, upgrades, inventory) that aren't covered
 * by per-event flag or item-give packets.
 */

void Anchor::SendPacket_RequestTeamState() {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    json payload;
    payload["type"] = REQUEST_TEAM_STATE;
    payload["targetTeamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");
    SendPacket(payload);
}

void Anchor::HandlePacket_RequestTeamState(json payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }
    SendPacket_UpdateTeamState();
}

void Anchor::SendPacket_UpdateTeamState() {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    SavePlayerData& pd = gSaveContext.save.saveInfo.playerData;
    Inventory& inv = gSaveContext.save.saveInfo.inventory;
    SaveInfo& si = gSaveContext.save.saveInfo;

    json state;
    state["healthCapacity"] = pd.healthCapacity;
    state["magicLevel"] = pd.magicLevel;
    state["isMagicAcquired"] = pd.isMagicAcquired;
    state["isDoubleMagicAcquired"] = pd.isDoubleMagicAcquired;
    state["doubleDefense"] = pd.doubleDefense;
    state["owlActivationFlags"] = pd.owlActivationFlags;

    state["items"] = std::vector<u8>(inv.items, inv.items + ARRAY_COUNT(inv.items));
    state["ammo"] = std::vector<s8>(inv.ammo, inv.ammo + ARRAY_COUNT(inv.ammo));
    state["upgrades"] = inv.upgrades;
    state["questItems"] = inv.questItems;
    state["dungeonItems"] = std::vector<u8>(inv.dungeonItems, inv.dungeonItems + ARRAY_COUNT(inv.dungeonItems));
    state["dungeonKeys"] = std::vector<s8>(inv.dungeonKeys, inv.dungeonKeys + ARRAY_COUNT(inv.dungeonKeys));
    state["defenseHearts"] = inv.defenseHearts;
    state["strayFairies"] = std::vector<s8>(inv.strayFairies, inv.strayFairies + ARRAY_COUNT(inv.strayFairies));

    state["weekEventReg"] = std::vector<u8>(si.weekEventReg, si.weekEventReg + ARRAY_COUNT(si.weekEventReg));
    state["eventInf"] =
        std::vector<u8>(gSaveContext.eventInf, gSaveContext.eventInf + ARRAY_COUNT(gSaveContext.eventInf));
    state["scenesVisible"] = std::vector<u32>(si.scenesVisible, si.scenesVisible + ARRAY_COUNT(si.scenesVisible));
    state["skullTokenCount"] = si.skullTokenCount;
    state["regionsVisited"] = si.regionsVisited;

    json payload;
    payload["type"] = UPDATE_TEAM_STATE;
    payload["targetTeamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");
    payload["state"] = state;
    SendPacket(payload);
}

template <typename T, size_t N> static void CopyJsonArray(const json& src, const char* key, T (&dest)[N]) {
    if (!src.contains(key)) {
        return;
    }
    std::vector<T> v = src[key].get<std::vector<T>>();
    for (size_t i = 0; i < v.size() && i < N; i++) {
        dest[i] = v[i];
    }
}

void Anchor::HandlePacket_UpdateTeamState(json payload) {
    if (!roomState.syncItemsAndFlags || !payload.contains("state") || !IsSaveLoaded()) {
        return;
    }

    const json& s = payload["state"];
    SavePlayerData& pd = gSaveContext.save.saveInfo.playerData;
    Inventory& inv = gSaveContext.save.saveInfo.inventory;
    SaveInfo& si = gSaveContext.save.saveInfo;

    // Capture pre-apply values so we can report what changed.
    u8 oldMagicAcquired = pd.isMagicAcquired;
    u8 oldDoubleMagic = pd.isDoubleMagicAcquired;
    u8 oldDoubleDefense = pd.doubleDefense;
    s16 oldHealthCapacity = pd.healthCapacity;
    s8 oldMagicLevel = pd.magicLevel;

    std::string senderName = "A teammate";
    if (payload.contains("clientId")) {
        uint32_t senderId = payload["clientId"].get<uint32_t>();
        if (clients.contains(senderId)) {
            senderName = clients[senderId].name;
        }
    }

    isApplyingRemotePacket = true;

    pd.healthCapacity = s.value("healthCapacity", pd.healthCapacity);
    pd.magicLevel = s.value("magicLevel", pd.magicLevel);
    pd.isMagicAcquired = s.value("isMagicAcquired", pd.isMagicAcquired);
    pd.isDoubleMagicAcquired = s.value("isDoubleMagicAcquired", pd.isDoubleMagicAcquired);
    pd.doubleDefense = s.value("doubleDefense", pd.doubleDefense);
    pd.owlActivationFlags = s.value("owlActivationFlags", pd.owlActivationFlags);

    CopyJsonArray(s, "items", inv.items);
    CopyJsonArray(s, "ammo", inv.ammo);
    inv.upgrades = s.value("upgrades", inv.upgrades);
    inv.questItems = s.value("questItems", inv.questItems);
    CopyJsonArray(s, "dungeonItems", inv.dungeonItems);
    CopyJsonArray(s, "dungeonKeys", inv.dungeonKeys);
    inv.defenseHearts = s.value("defenseHearts", inv.defenseHearts);
    CopyJsonArray(s, "strayFairies", inv.strayFairies);

    CopyJsonArray(s, "weekEventReg", si.weekEventReg);
    CopyJsonArray(s, "eventInf", gSaveContext.eventInf);
    CopyJsonArray(s, "scenesVisible", si.scenesVisible);
    si.skullTokenCount = s.value("skullTokenCount", si.skullTokenCount);
    si.regionsVisited = s.value("regionsVisited", si.regionsVisited);

    isApplyingRemotePacket = false;

    bool notified = ReconcileUpgrades(senderName, oldMagicAcquired, oldDoubleMagic, oldDoubleDefense, oldHealthCapacity,
                                      oldMagicLevel);
    if (!notified) {
        Notification::Emit({ .message = "Save synced from team" });
    }
}

// Kicks the magic meter and emits contextual notifications after upgrade values change. Shared by
// the team-state (join) path and the live GIVE_UPGRADE path.
bool Anchor::ReconcileUpgrades(const std::string& senderName, u8 oldMagicAcquired, u8 oldDoubleMagic,
                               u8 oldDoubleDefense, s16 oldHealthCapacity, s8 oldMagicLevel) {
    (void)oldMagicLevel;
    SavePlayerData& pd = gSaveContext.save.saveInfo.playerData;

    // When magic is newly acquired (or upgraded to double), replicate the game's native grant
    // (z_parameter.c Interface magic update): set the level, ramp/fill the meter, and enable the
    // Deku form's magic attack on B. Setting only magicLevel shows the meter but leaves magic
    // unusable (e.g. the Deku bubble), because the B-button equip is never set.
    bool newMagic = !oldMagicAcquired && pd.isMagicAcquired;
    bool newDoubleMagic = !oldDoubleMagic && pd.isDoubleMagicAcquired;
    if (newMagic || newDoubleMagic) {
        pd.magicLevel = pd.isDoubleMagicAcquired ? 2 : 1;
        gSaveContext.magicFillTarget = pd.magicLevel * MAGIC_NORMAL_METER;
        pd.magic = 0;
        gSaveContext.magicState = MAGIC_STATE_STEP_CAPACITY;
        BUTTON_ITEM_EQUIP(PLAYER_FORM_DEKU, EQUIP_SLOT_B) = ITEM_DEKU_NUT;
    }

    // Adopted values are now our baseline; don't let the dirty-check re-broadcast them.
    SnapshotSaveUpgrades();

    bool notified = false;
    auto emit = [&](const char* what) {
        Notification::Emit({ .prefix = senderName, .message = "found", .suffix = what });
        notified = true;
    };
    if (!oldMagicAcquired && pd.isMagicAcquired) {
        emit("Magic Power");
    }
    if (!oldDoubleMagic && pd.isDoubleMagicAcquired) {
        emit("Double Magic");
    }
    if (!oldDoubleDefense && pd.doubleDefense) {
        emit("Double Defense");
    }
    if (pd.healthCapacity > oldHealthCapacity) {
        emit("a Heart Container");
    }
    return notified;
}

/**
 * GIVE_UPGRADE
 *
 * Lightweight, live-broadcast packet for save-value upgrades that aren't items or flags (magic,
 * double magic, double defense, heart capacity). Unlike UPDATE_TEAM_STATE (which the server only
 * delivers to clients awaiting state), this uses the generic team broadcast so it reaches online
 * teammates immediately. Values are applied monotonically so a stale packet never downgrades.
 */

void Anchor::SendPacket_GiveUpgrade() {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    SavePlayerData& pd = gSaveContext.save.saveInfo.playerData;

    json payload;
    payload["type"] = GIVE_UPGRADE;
    payload["targetTeamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");
    payload["addToQueue"] = true;
    payload["magicLevel"] = pd.magicLevel;
    payload["isMagicAcquired"] = pd.isMagicAcquired;
    payload["isDoubleMagicAcquired"] = pd.isDoubleMagicAcquired;
    payload["doubleDefense"] = pd.doubleDefense;
    payload["healthCapacity"] = pd.healthCapacity;
    SendPacket(payload);
}

void Anchor::HandlePacket_GiveUpgrade(json payload) {
    if (!roomState.syncItemsAndFlags || !IsSaveLoaded()) {
        return;
    }

    SavePlayerData& pd = gSaveContext.save.saveInfo.playerData;
    u8 oldMagicAcquired = pd.isMagicAcquired;
    u8 oldDoubleMagic = pd.isDoubleMagicAcquired;
    u8 oldDoubleDefense = pd.doubleDefense;
    s16 oldHealthCapacity = pd.healthCapacity;
    s8 oldMagicLevel = pd.magicLevel;

    std::string senderName = "A teammate";
    if (payload.contains("clientId")) {
        uint32_t senderId = payload["clientId"].get<uint32_t>();
        if (clients.contains(senderId)) {
            senderName = clients[senderId].name;
        }
    }

    isApplyingRemotePacket = true;
    s8 newMagicLevel = payload.value("magicLevel", pd.magicLevel);
    if (newMagicLevel > pd.magicLevel) {
        pd.magicLevel = newMagicLevel;
    }
    if (payload.value("isMagicAcquired", (u8)0)) {
        pd.isMagicAcquired = true;
    }
    if (payload.value("isDoubleMagicAcquired", (u8)0)) {
        pd.isDoubleMagicAcquired = true;
    }
    if (payload.value("doubleDefense", (u8)0)) {
        pd.doubleDefense = true;
    }
    s16 newHealthCapacity = payload.value("healthCapacity", pd.healthCapacity);
    if (newHealthCapacity > pd.healthCapacity) {
        pd.healthCapacity = newHealthCapacity;
    }
    isApplyingRemotePacket = false;

    ReconcileUpgrades(senderName, oldMagicAcquired, oldDoubleMagic, oldDoubleDefense, oldHealthCapacity, oldMagicLevel);
}

void Anchor::SnapshotSaveUpgrades() {
    SavePlayerData& pd = gSaveContext.save.saveInfo.playerData;
    prevMagicLevel = pd.magicLevel;
    prevIsMagicAcquired = pd.isMagicAcquired;
    prevIsDoubleMagicAcquired = pd.isDoubleMagicAcquired;
    prevDoubleDefense = pd.doubleDefense;
    prevHealthCapacity = pd.healthCapacity;
    upgradeSnapshotValid = true;
}

// Detects mid-game upgrade acquisitions (magic, double magic, double defense, heart capacity)
// that aren't covered by flag/item packets, and pushes team state so they propagate live.
void Anchor::CheckAndPushSaveUpgrades() {
    if (!isConnected || !roomState.syncItemsAndFlags || !IsSaveLoaded() || isApplyingRemotePacket) {
        return;
    }

    if (!upgradeSnapshotValid) {
        SnapshotSaveUpgrades();
        return;
    }

    SavePlayerData& pd = gSaveContext.save.saveInfo.playerData;
    if (pd.magicLevel != prevMagicLevel || pd.isMagicAcquired != prevIsMagicAcquired ||
        pd.isDoubleMagicAcquired != prevIsDoubleMagicAcquired || pd.doubleDefense != prevDoubleDefense ||
        pd.healthCapacity != prevHealthCapacity) {
        SnapshotSaveUpgrades();
        // Live-broadcast the upgrade (UPDATE_TEAM_STATE is not forwarded to online teammates).
        SendPacket_GiveUpgrade();
    }
}
