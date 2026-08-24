#include "Harpoon.h"
#include "HarpoonSkinSync.h"

#include <spdlog/spdlog.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include <unordered_set>
#include <cstring>

// =============================================================================
// Harpoon — Phase A: connect/handshake/room lifecycle only.
// =============================================================================
//
// CVars used (defaults applied here on first read):
//   gNetwork.Harpoon.Host                : string  (default "54.209.53.9")
//   gNetwork.Harpoon.Port                : int     (default 8765)
//   gNetwork.Harpoon.Name                : string  (default "Player")
//   gNetwork.Harpoon.Color.Value         : color   (default white)
//   gNetwork.Harpoon.ShowOtherPlayersOnMinimap : int (default 0 — OFF)
// =============================================================================

using json = nlohmann::json;

namespace {
constexpr const char* CVAR_HOST = "gNetwork.Harpoon.Host";
constexpr const char* CVAR_PORT = "gNetwork.Harpoon.Port";
constexpr const char* CVAR_NAME = "gNetwork.Harpoon.Name";
constexpr const char* CVAR_COLOR = "gNetwork.Harpoon.Color.Value";

constexpr const char* CLIENT_VERSION = "2S2H-Harpoon/0.1";
} // namespace

Harpoon* Harpoon::Instance() {
    static Harpoon inst;
    static bool hooksInited = false;
    if (!hooksInited) {
        hooksInited = true;
        inst.InitHooks();
    }
    return &inst;
}

HarpoonClient* Harpoon::GetOrCreateClient(uint32_t clientId) {
    // Caller must hold stateMutex_.
    auto& c = clients_[clientId];
    if (c.clientId == 0)
        c.clientId = clientId;
    return &c;
}

Harpoon::~Harpoon() {
    Disable();
}

// -----------------------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------------------

void Harpoon::Enable() {
    if (enabled_.exchange(true)) {
        return;
    }
    {
        std::lock_guard<std::mutex> lk(stateMutex_);
        lastError_.clear();
        currentRoomId_.clear();
        clients_.clear();
        sessionToken_.clear();
    }
    ownClientId_.store(0);
    state_.store(HarpoonConnState::Connecting);

    std::string host = CVarGetString(CVAR_HOST, "54.209.53.9");
    int port = CVarGetInteger(CVAR_PORT, 8765);

    ws_ = std::make_unique<HarpoonWebSocket>();
    ws_->SetOnConnected([this]() { OnWsConnected(); });
    ws_->SetOnDisconnected([this]() { OnWsDisconnected(); });
    ws_->SetOnText([this](const std::string& s) { OnWsText(s); });
    ws_->Connect(host, (uint16_t)port);
    SPDLOG_INFO("[Harpoon] Enable() → connecting to {}:{}", host, port);
}

void Harpoon::Disable() {
    if (!enabled_.exchange(false)) {
        return;
    }
    if (ws_) {
        ws_->Disconnect();
        ws_.reset();
    }
    state_.store(HarpoonConnState::Disconnected);
    ownClientId_.store(0);
    {
        std::lock_guard<std::mutex> lk(stateMutex_);
        currentRoomId_.clear();
        clients_.clear();
        sessionToken_.clear();
    }
    {
        std::lock_guard<std::mutex> lk(inboundMutex_);
        inboundQueue_.clear();
    }
    SPDLOG_INFO("[Harpoon] Disable()");
}

// -----------------------------------------------------------------------------
// Send helpers
// -----------------------------------------------------------------------------

void Harpoon::SendJson(const json& payload) {
    if (!ws_ || !ws_->IsConnected()) {
        return;
    }
    // Harpoon v2 envelope: { type, seq, payload }. Call sites already pass
    // { type, payload:{...} }; we just add the seq and inject clientId into the
    // inner payload (the server keeps it there as a convenience field).
    json envelope = payload;
    if (!envelope.contains("seq")) {
        envelope["seq"] = seqCounter_.fetch_add(1);
    }
    if (envelope.contains("payload") && envelope["payload"].is_object()) {
        envelope["payload"]["clientId"] = ownClientId_.load();
    }
    ws_->SendText(envelope.dump());
}

void Harpoon::CreateRoom(const std::string& name, const std::string& password) {
    // Server rejects ROOM.* before HANDSHAKE_ACK assigns our clientId.
    if (ownClientId_.load() == 0)
        return;
    json inner = { { "name", name }, { "gameMode", gameMode_ }, { "gameModeId", gameMode_ } };
    if (!password.empty())
        inner["password"] = password;
    json msg = { { "type", HarpoonPT::ROOM_CREATE }, { "payload", inner } };
    SendJson(msg);
}

void Harpoon::JoinRoom(const std::string& roomId, const std::string& password) {
    if (ownClientId_.load() == 0)
        return;
    json inner = { { "roomId", roomId } };
    if (!password.empty())
        inner["password"] = password;
    json msg = { { "type", HarpoonPT::ROOM_JOIN }, { "payload", inner } };
    SendJson(msg);
}

void Harpoon::LeaveRoom() {
    if (state_.load() < HarpoonConnState::InRoom)
        return;
    json msg = { { "type", HarpoonPT::ROOM_LEAVE }, { "payload", json::object() } };
    SendJson(msg);
    state_.store(HarpoonConnState::Connected);
    std::lock_guard<std::mutex> lk(stateMutex_);
    currentRoomId_.clear();
    clients_.clear();
}

void Harpoon::RequestRoomList() {
    if (state_.load() < HarpoonConnState::Connected)
        return;
    json msg = { { "type", HarpoonPT::ROOM_LIST }, { "payload", json::object() } };
    SendJson(msg);
}

void Harpoon::SendFlag(bool set, int flagType, int sceneId, uint32_t flag) {
    if (state_.load() < HarpoonConnState::InRoom)
        return;
    json msg = { { "type", set ? HarpoonPT::SAVE_SET_FLAG : HarpoonPT::SAVE_UNSET_FLAG },
                 { "payload",
                   {
                       { "clientId", ownClientId_.load() },
                       { "flagType", flagType },
                       { "sceneId", sceneId },
                       { "flag", flag },
                   } } };
    SendJson(msg);
}

// -----------------------------------------------------------------------------
// WS callbacks (worker thread). All they do is queue work for the game thread.
// -----------------------------------------------------------------------------

void Harpoon::OnWsConnected() {
    state_.store(HarpoonConnState::Connected);

    // Harpoon v2 HARPOON.HANDSHAKE — MUST carry protocol:"harpoon" or the
    // official server rejects us as a generic WS client (this was the cause of
    // "no me deja conectarme"). color is an {r,g,b} object; installedGamemodes
    // is the list of folders under harpoon/gamemodes/ that have a gamemode.yaml.
    Color_RGBA8 color = CVarGetColor(CVAR_COLOR, Color_RGBA8{ 100, 255, 100, 255 });
    json payload = {
        { "protocol", "harpoon" },
        { "name", CVarGetString(CVAR_NAME, "Player") },
        { "color", { { "r", color.r }, { "g", color.g }, { "b", color.b } } },
        { "clientVersion", CLIENT_VERSION },
        { "installedGamemodes", HarpoonSkinSync::GetInstalledGamemodes() },
    };
    json hs = { { "type", HarpoonPT::HANDSHAKE }, { "payload", payload } };
    SendJson(hs);
    // NOTE: skin sync announce is room-scoped — sending it here (pre-room) makes
    // the server reply "must be in a room". We announce after ROOM.JOINED.
}

void Harpoon::OnWsDisconnected() {
    state_.store(HarpoonConnState::Disconnected);
    ownClientId_.store(0);
    std::lock_guard<std::mutex> lk(stateMutex_);
    currentRoomId_.clear();
    clients_.clear();
}

void Harpoon::OnWsText(const std::string& text) {
    json env;
    try {
        env = json::parse(text);
    } catch (const std::exception& e) {
        SPDLOG_WARN("[Harpoon] dropped malformed packet: {}", e.what());
        return;
    }
    std::lock_guard<std::mutex> lk(inboundMutex_);
    inboundQueue_.push_back(std::move(env));
}

// -----------------------------------------------------------------------------
// Game-thread dispatch
// -----------------------------------------------------------------------------

void Harpoon::DrainIncomingQueue() {
    std::deque<json> drained;
    {
        std::lock_guard<std::mutex> lk(inboundMutex_);
        drained.swap(inboundQueue_);
    }
    for (auto& env : drained) {
        HandlePacket(env);
    }
}

void Harpoon::HandlePacket(const json& env) {
    if (!env.contains("type") || !env["type"].is_string())
        return;
    const std::string type = env["type"].get<std::string>();
    const json payload = env.value("payload", json::object());

    if (type == HarpoonPT::HANDSHAKE_ACK) {
        HandleHandshakeAck(payload);
    } else if (type == HarpoonPT::SERVER_INFO) {
        HandleServerInfo(payload);
    } else if (type == HarpoonPT::ROOM_JOINED) {
        HandleRoomJoined(payload);
    } else if (type == HarpoonPT::ROOM_MEMBERS_UPDATED) {
        HandleRoomMembersUpdated(payload);
    } else if (type == HarpoonPT::ROOM_LIST_RESPONSE) {
        HandleRoomListResponse(payload);
    } else if (type == HarpoonPT::ERROR_MSG) {
        HandleError(payload);
    } else if (type == HarpoonPT::PLAYER_UPDATE_TRANSFORM) {
        HandlePlayerUpdateTransform(payload);
    } else if (type == HarpoonPT::PLAYER_UPDATE_SKELETON) {
        HandlePlayerUpdateSkeleton(payload);
    } else if (type == HarpoonPT::PLAYER_UPDATE_VISUAL_STATE) {
        HandlePlayerUpdateVisualState(payload);
    } else if (type == HarpoonPT::PLAYER_UPDATE_EQUIP_VISIBLE) {
        HandlePlayerUpdateEquipVisible(payload);
    } else if (type == HarpoonPT::PLAYER_UPDATE_CUSTOM_ITEMS) {
        HandlePlayerUpdateCustomItems(payload);
    } else if (type == HarpoonPT::PLAYER_SET_TRANSFORMATION) {
        HandlePlayerSetTransformation(payload);
    } else if (type == HarpoonPT::COMBAT_DEAL_DAMAGE) {
        HandleCombatDealDamage(payload);
    } else if (type == HarpoonPT::COMBAT_APPLY_STATUS) {
        HandleCombatApplyStatus(payload);
    } else if (type == HarpoonPT::APPEARANCE_SPAWN_VFX_ACTOR) {
        HandleAppearanceSpawnVfxActor(payload);
    } else if (type == HarpoonPT::APPEARANCE_SKIN_UPDATE) {
        HandleAppearanceSkinUpdate(payload);
    } else if (type == HarpoonPT::SAVE_SET_FLAG) {
        HandleSaveFlag(payload, true);
    } else if (type == HarpoonPT::SAVE_UNSET_FLAG) {
        HandleSaveFlag(payload, false);
    }
}

// -----------------------------------------------------------------------------
// Player state handlers (Phase C). Just patch the client struct; the dummy
// actor reads it during its Update/Draw on the game thread.
// -----------------------------------------------------------------------------

void Harpoon::HandlePlayerUpdateTransform(const json& payload) {
    uint32_t cid = payload.value("clientId", 0u);
    if (!cid || cid == ownClientId_.load())
        return;
    if (!payload.contains("posRot"))
        return;
    const json& pr = payload["posRot"];
    std::lock_guard<std::mutex> lk(stateMutex_);
    HarpoonClient* c = GetOrCreateClient(cid);
    if (pr.contains("pos")) {
        c->posX = pr["pos"].value("x", 0.0f);
        c->posY = pr["pos"].value("y", 0.0f);
        c->posZ = pr["pos"].value("z", 0.0f);
    }
    if (pr.contains("rot")) {
        c->rotX = (int16_t)pr["rot"].value("x", 0);
        c->rotY = (int16_t)pr["rot"].value("y", 0);
        c->rotZ = (int16_t)pr["rot"].value("z", 0);
    }
}

void Harpoon::HandlePlayerUpdateSkeleton(const json& payload) {
    uint32_t cid = payload.value("clientId", 0u);
    if (!cid || cid == ownClientId_.load())
        return;
    if (!payload.contains("jointTable") || !payload["jointTable"].is_array())
        return;
    const auto& arr = payload["jointTable"];
    std::lock_guard<std::mutex> lk(stateMutex_);
    HarpoonClient* c = GetOrCreateClient(cid);
    size_t n = std::min<size_t>(arr.size(), sizeof(c->jointTable) / sizeof(int16_t));
    for (size_t i = 0; i < n; ++i)
        c->jointTable[i] = (int16_t)arr[i].get<int>();
}

void Harpoon::HandlePlayerUpdateVisualState(const json& payload) {
    uint32_t cid = payload.value("clientId", 0u);
    if (!cid || cid == ownClientId_.load())
        return;
    std::lock_guard<std::mutex> lk(stateMutex_);
    HarpoonClient* c = GetOrCreateClient(cid);
    // Server schema key is "sceneNum" (carries MM's sceneId value).
    c->sceneId = (int16_t)payload.value("sceneNum", (int)c->sceneId);
}

void Harpoon::HandlePlayerUpdateEquipVisible(const json& payload) {
    uint32_t cid = payload.value("clientId", 0u);
    if (!cid || cid == ownClientId_.load())
        return;
    std::lock_guard<std::mutex> lk(stateMutex_);
    HarpoonClient* c = GetOrCreateClient(cid);
    c->currentBoots = (int8_t)payload.value("currentBoots", 0);
    c->currentShield = (int8_t)payload.value("currentShield", 0);
    c->itemAction = (int8_t)payload.value("itemAction", 0);
    c->heldItemAction = (int8_t)payload.value("heldItemAction", 0);
    c->currentMask = (uint8_t)payload.value("currentMask", 0);
}

// NEI custom items: the sender base64s a raw CustomItemVisualSync. Both ends run
// the same binary, so the layout matches; "ciVer" (the struct's size) is checked
// on the draw side before anything is reinterpreted, so a mismatched build just
// stops drawing peers' custom items instead of reading garbage.
namespace {
std::vector<uint8_t> Base64Decode(const std::string& in) {
    static constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int8_t rev[256];
    memset(rev, -1, sizeof(rev));
    for (int i = 0; i < 64; ++i)
        rev[(uint8_t)kAlphabet[i]] = (int8_t)i;

    std::vector<uint8_t> out;
    out.reserve((in.size() / 4) * 3);
    uint32_t acc = 0;
    int bits = 0;
    for (char ch : in) {
        int8_t v = rev[(uint8_t)ch];
        if (v < 0)
            continue; // '=' padding and any stray whitespace
        acc = (acc << 6) | (uint32_t)v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back((uint8_t)((acc >> bits) & 0xFF));
        }
    }
    return out;
}
} // namespace

void Harpoon::HandlePlayerUpdateCustomItems(const json& payload) {
    uint32_t cid = payload.value("clientId", 0u);
    if (!cid || cid == ownClientId_.load())
        return;
    std::string blob = payload.value("ciBlob", std::string());
    if (blob.empty())
        return;
    std::vector<uint8_t> bytes = Base64Decode(blob);
    if (bytes.empty())
        return;
    std::lock_guard<std::mutex> lk(stateMutex_);
    HarpoonClient* c = GetOrCreateClient(cid);
    c->customItemBlob = std::move(bytes);
}

void Harpoon::HandlePlayerSetTransformation(const json& payload) {
    uint32_t cid = payload.value("clientId", 0u);
    if (!cid || cid == ownClientId_.load())
        return;
    std::lock_guard<std::mutex> lk(stateMutex_);
    HarpoonClient* c = GetOrCreateClient(cid);
    c->transformation = (uint8_t)payload.value("transformation", 4);
}

// -----------------------------------------------------------------------------
// Combat handlers (Phase E). The actual damage application is implemented in
// HarpoonHookHandlers.cpp where it has access to PlayState.
// -----------------------------------------------------------------------------

extern "C" void Harpoon_ApplyIncomingDamage(uint8_t damageEffect, uint8_t damage, uint32_t attackerClientId);
extern "C" void Harpoon_ApplyIncomingStatus(uint8_t effect, uint16_t durationFrames);

void Harpoon::HandleCombatDealDamage(const json& payload) {
    // Field names match SoH's SendPacket_Damage; "targetCid"/"weaponSource" are
    // the legacy 2ship spelling, kept as a fallback for older peers.
    uint32_t targetCid = payload.value("targetClientId", payload.value("targetCid", 0u));
    if (targetCid != ownClientId_.load())
        return;
    if (!IsPvpActive())
        return; // gamemode gates PvP, not a manual toggle
    uint8_t damage = (uint8_t)payload.value("damage", 0);
    uint8_t effect = (uint8_t)payload.value("damageEffect", payload.value("weaponSource", 0));
    uint32_t attacker = payload.value("clientId", 0u);
    Harpoon_ApplyIncomingDamage(effect, damage, attacker);
}

void Harpoon::HandleCombatApplyStatus(const json& payload) {
    uint32_t targetCid = payload.value("targetCid", 0u);
    if (targetCid != ownClientId_.load())
        return;
    if (!IsPvpActive())
        return;
    uint8_t effect = (uint8_t)payload.value("effect", 0);
    uint16_t duration = (uint16_t)payload.value("durationFrames", 0);
    Harpoon_ApplyIncomingStatus(effect, duration);
}

// -----------------------------------------------------------------------------
// Projectile mirror (Phase F) — declared in ProjectileMirror.cpp.
// -----------------------------------------------------------------------------

extern "C" void Harpoon_SpawnRemoteVfxActor(uint32_t srcCid, int16_t actorId, float px, float py, float pz, int16_t rx,
                                            int16_t ry, int16_t rz, int16_t params);

void Harpoon::HandleAppearanceSpawnVfxActor(const json& payload) {
    uint32_t cid = payload.value("clientId", 0u);
    if (!cid || cid == ownClientId_.load())
        return;
    int16_t actorId = (int16_t)payload.value("actorId", 0);
    float px = payload.value("posX", 0.0f), py = payload.value("posY", 0.0f), pz = payload.value("posZ", 0.0f);
    int16_t rx = (int16_t)payload.value("rotX", 0);
    int16_t ry = (int16_t)payload.value("rotY", 0);
    int16_t rz = (int16_t)payload.value("rotZ", 0);
    int16_t params = (int16_t)payload.value("params", 0);
    Harpoon_SpawnRemoteVfxActor(cid, actorId, px, py, pz, rx, ry, rz, params);
}

// -----------------------------------------------------------------------------
// Skin sync (Phase G).
// -----------------------------------------------------------------------------

void Harpoon::HandleAppearanceSkinUpdate(const json& payload) {
    uint32_t cid = payload.value("clientId", 0u);
    if (!cid || cid == ownClientId_.load())
        return;
    std::lock_guard<std::mutex> lk(stateMutex_);
    HarpoonClient* c = GetOrCreateClient(cid);
    c->adultSkinName = payload.value("adultSkinName", "");
    c->equipSkinName = payload.value("equipSkinName", "");
}

// -----------------------------------------------------------------------------
// Flag sync (Phase I). Applied on the game thread; the apply function writes to
// gSaveContext directly so it works cross-scene. Implemented in
// HarpoonHookHandlers.cpp where MM save headers are in scope.
// -----------------------------------------------------------------------------

extern "C" void Harpoon_ApplyRemoteFlag(bool set, int flagType, int sceneId, uint32_t flag);

void Harpoon::HandleSaveFlag(const json& payload, bool set) {
    uint32_t cid = payload.value("clientId", 0u);
    if (cid == ownClientId_.load())
        return; // ignore echoes of our own flags
    int flagType = payload.value("flagType", 0);
    int sceneId = payload.value("sceneId", -1);
    uint32_t flag = payload.value("flag", 0u);
    Harpoon_ApplyRemoteFlag(set, flagType, sceneId, flag);
}

void Harpoon::HandleHandshakeAck(const json& payload) {
    // v2 server sends client_id / session_token; legacy sends clientId / sessionToken.
    uint32_t cid = payload.value("client_id", payload.value("clientId", 0u));
    std::string token = payload.value("session_token", payload.value("sessionToken", std::string("")));
    if (cid)
        ownClientId_.store(cid);
    {
        std::lock_guard<std::mutex> lk(stateMutex_);
        sessionToken_ = std::move(token);
    }
    SPDLOG_INFO("[Harpoon] HANDSHAKE_ACK clientId={}", cid);
}

void Harpoon::HandleServerInfo(const json& payload) {
    uint32_t cid = payload.value("client_id", payload.value("ownClientId", 0u));
    if (cid)
        ownClientId_.store(cid);
    std::string token = payload.value("session_token", payload.value("sessionToken", std::string("")));
    if (!token.empty()) {
        std::lock_guard<std::mutex> lk(stateMutex_);
        sessionToken_ = std::move(token);
    }
    SPDLOG_INFO("[Harpoon] SERVER_INFO ownClientId={}", cid);
}

void Harpoon::HandleRoomJoined(const json& payload) {
    // v2 server uses snake_case (room_id / gamemode_id); accept camelCase too.
    std::string id = payload.value("room_id", payload.value("roomId", std::string("")));
    std::string gm = payload.value("gamemode_id", payload.value("gameMode", std::string("")));
    {
        std::lock_guard<std::mutex> lk(stateMutex_);
        currentRoomId_ = id;
        if (!gm.empty())
            gameMode_ = gm; // joiner adopts the room's gamemode
    }
    state_.store(HarpoonConnState::InRoom);
    SPDLOG_INFO("[Harpoon] ROOM_JOINED id={} gamemode={}", id, gm);

    // Now that we're in a room, room-scoped skin sync is allowed.
    HarpoonSkinSync::AnnounceCatalogAndSlots();
}

void Harpoon::HandleRoomMembersUpdated(const json& payload) {
    // Server roster key is "clients" (NOT "members"); each entry has clientId,
    // name, color:{r,g,b}, sceneNum. Receiving this means we ARE in the room —
    // use it as the authoritative "joined" signal too.
    if (!payload.contains("clients") || !payload["clients"].is_array())
        return;
    uint32_t own = ownClientId_.load();
    std::lock_guard<std::mutex> lk(stateMutex_);
    std::unordered_set<uint32_t> seen;
    for (const auto& m : payload["clients"]) {
        uint32_t cid = m.value("clientId", 0u);
        if (cid == 0 || cid == own)
            continue;
        seen.insert(cid);
        HarpoonClient& c = clients_[cid]; // get-or-create preserves pose/dummy
        c.clientId = cid;
        c.name = m.value("name", std::string("Player"));
        if (m.contains("color") && m["color"].is_object()) {
            uint8_t r = (uint8_t)m["color"].value("r", 100);
            uint8_t g = (uint8_t)m["color"].value("g", 255);
            uint8_t b = (uint8_t)m["color"].value("b", 100);
            c.colorRgba = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | 0xFF;
        }
        // The server reflects each client's VISUAL_STATE sceneNum into the roster
        // (it does NOT relay PLAYER.UPDATE_VISUAL_STATE directly) — this is how
        // we learn which scene a peer is in. Without it sceneId stays -1.
        if (m.contains("sceneNum")) {
            c.sceneId = (int16_t)m.value("sceneNum", (int)c.sceneId);
        } else if (m.contains("scene_num")) {
            c.sceneId = (int16_t)m.value("scene_num", (int)c.sceneId);
        }
    }
    // Drop clients no longer in the roster.
    for (auto it = clients_.begin(); it != clients_.end();) {
        if (seen.find(it->first) == seen.end())
            it = clients_.erase(it);
        else
            ++it;
    }
    if (state_.load() < HarpoonConnState::InRoom) {
        state_.store(HarpoonConnState::InRoom);
    }
}

void Harpoon::HandleRoomListResponse(const json& payload) {
    if (!payload.contains("rooms") || !payload["rooms"].is_array())
        return;
    std::vector<HarpoonRoomInfo> next;
    for (const auto& r : payload["rooms"]) {
        HarpoonRoomInfo info;
        // v2 server emits camelCase aliases but accept snake_case too.
        info.roomId = r.value("roomId", r.value("room_id", std::string("")));
        info.name = r.value("name", std::string(""));
        info.gameMode = r.value("gameMode", r.value("gamemode_id", std::string("")));
        info.state = r.value("phase", r.value("state", std::string("lobby")));
        info.playerCount = r.value("playerCount", r.value("player_count", 0));
        info.maxPlayers = r.value("maxPlayers", r.value("max_players", 16));
        info.hasPassword = r.value("hasPassword", r.value("has_password", false));
        if (!info.roomId.empty())
            next.push_back(std::move(info));
    }
    std::lock_guard<std::mutex> lk(stateMutex_);
    roomList_ = std::move(next);
}

void Harpoon::HandleError(const json& payload) {
    std::string code = payload.value("code", std::string(""));
    std::string msg = payload.value("message", std::string(""));
    std::string full = code.empty() ? msg : (msg.empty() ? code : code + ": " + msg);
    if (full.empty())
        full = "unknown";
    {
        std::lock_guard<std::mutex> lk(stateMutex_);
        lastError_ = full;
    }
    SPDLOG_WARN("[Harpoon] server ERROR: {}", full);
}

// -----------------------------------------------------------------------------
// Read-only views
// -----------------------------------------------------------------------------

std::string Harpoon::LastError() const {
    std::lock_guard<std::mutex> lk(stateMutex_);
    return lastError_;
}

std::string Harpoon::CurrentRoomId() const {
    std::lock_guard<std::mutex> lk(stateMutex_);
    return currentRoomId_;
}

std::vector<HarpoonClient> Harpoon::GetClientsSnapshot() const {
    std::lock_guard<std::mutex> lk(stateMutex_);
    std::vector<HarpoonClient> out;
    out.reserve(clients_.size());
    for (const auto& [_, c] : clients_)
        out.push_back(c);
    return out;
}

std::vector<HarpoonRoomInfo> Harpoon::GetRoomListSnapshot() const {
    std::lock_guard<std::mutex> lk(stateMutex_);
    return roomList_;
}

// -----------------------------------------------------------------------------
// Gamemode-driven behavior. These replace the old manual CVar checkboxes — the
// active room's gamemode decides what's on, exactly like the gamemode.yaml says.
// -----------------------------------------------------------------------------

bool Harpoon::IsGeoguessr() const {
    std::lock_guard<std::mutex> lk(stateMutex_);
    return state_.load() == HarpoonConnState::InRoom && gameMode_ == "geoguessr";
}

bool Harpoon::IsPvpActive() const {
    std::lock_guard<std::mutex> lk(stateMutex_);
    // geoguessr is the PvP gamemode; coop modes (story/randomizer/default) are not.
    return state_.load() == HarpoonConnState::InRoom && gameMode_ == "geoguessr";
}

bool Harpoon::NametagsVisible() const {
    std::lock_guard<std::mutex> lk(stateMutex_);
    if (state_.load() != HarpoonConnState::InRoom)
        return false;
    return gameMode_ != "geoguessr"; // hidden in geoguessr by design
}

bool Harpoon::MinimapVisible() const {
    std::lock_guard<std::mutex> lk(stateMutex_);
    if (state_.load() != HarpoonConnState::InRoom)
        return false;
    return gameMode_ != "geoguessr";
}

// -----------------------------------------------------------------------------
// Hook registration. The hook bodies live in HarpoonHookHandlers.cpp where they
// have access to GameInteractor types and MM headers (PlayState, Actor, Player).
// -----------------------------------------------------------------------------

extern "C" int Harpoon_RegisterFrameHook();
extern "C" int Harpoon_RegisterActorUpdateHook();
extern "C" int Harpoon_RegisterDrawHook();
extern "C" int Harpoon_RegisterSceneFlagSetHook();
extern "C" int Harpoon_RegisterSceneFlagUnsetHook();
extern "C" int Harpoon_RegisterFlagSetHook();
extern "C" int Harpoon_RegisterFlagUnsetHook();
extern "C" int Harpoon_RegisterPlayDestroyHook();
extern "C" int Harpoon_RegisterActorDestroyHook();
extern "C" int Harpoon_RegisterInputHook();
extern "C" int Harpoon_RegisterBlindnessHook();
extern "C" void Harpoon_UnregisterHook(int kind, int hookId);

void Harpoon::InitHooks() {
    if (hookFrameId_ != -1)
        return;
    hookFrameId_ = Harpoon_RegisterFrameHook();
    hookActorUpdateId_ = Harpoon_RegisterActorUpdateHook();
    hookSceneInitId_ = Harpoon_RegisterDrawHook(); // reused field: draw hook id
    hookSceneFlagSetId_ = Harpoon_RegisterSceneFlagSetHook();
    hookSceneFlagUnsetId_ = Harpoon_RegisterSceneFlagUnsetHook();
    hookFlagSetId_ = Harpoon_RegisterFlagSetHook();
    hookFlagUnsetId_ = Harpoon_RegisterFlagUnsetHook();
    // Peer-actor lifecycle + incoming status effects. These have no id fields of
    // their own; ShutdownHooks is a no-op anyway (2ship's GameInteractor has no
    // typed unregister), so the ids are dropped deliberately.
    hookActorDestroyId_ = Harpoon_RegisterActorDestroyHook();
    Harpoon_RegisterPlayDestroyHook();
    Harpoon_RegisterInputHook();
    Harpoon_RegisterBlindnessHook();
    SPDLOG_INFO("[Harpoon] hooks registered");
}

void Harpoon::ShutdownHooks() {
    if (hookFrameId_ == -1)
        return;
    Harpoon_UnregisterHook(0, hookFrameId_);
    hookFrameId_ = -1;
    Harpoon_UnregisterHook(1, hookActorUpdateId_);
    hookActorUpdateId_ = -1;
    Harpoon_UnregisterHook(2, hookSceneInitId_);
    hookSceneInitId_ = -1;
    Harpoon_UnregisterHook(4, hookSceneFlagSetId_);
    hookSceneFlagSetId_ = -1;
    Harpoon_UnregisterHook(5, hookSceneFlagUnsetId_);
    hookSceneFlagUnsetId_ = -1;
    Harpoon_UnregisterHook(6, hookFlagSetId_);
    hookFlagSetId_ = -1;
    Harpoon_UnregisterHook(7, hookFlagUnsetId_);
    hookFlagUnsetId_ = -1;
}
