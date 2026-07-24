#ifndef HARPOON_H
#define HARPOON_H
#ifdef __cplusplus

// =============================================================================
// Harpoon — Multiplayer client for 2ship2harkinian (Phase A stub).
// =============================================================================
//
// Mirror-port of Ship of Harkinian's Harpoon class. This first iteration only
// covers the lifecycle skeleton: connect to the official relay, handshake,
// create/join/leave a room, and parse member updates. Player sync, dummy
// rendering, PvP, projectile mirror and skin sync land in later phases.
//
// Server endpoint: ws://54.209.53.9:8765 (configurable via CVars).
// Protocol envelope: { "type": "<NAMESPACE.NAME>", "seq": N, "payload": {...} }
// =============================================================================

#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "HarpoonWebSocket.h"

// -----------------------------------------------------------------------------
// Packet type constants. Mirror SoH naming so the server relay treats us the
// same. Only the subset used in Phase A is defined; more land in later phases.
// -----------------------------------------------------------------------------
namespace HarpoonPT {
    // Lifecycle.
    constexpr const char* HANDSHAKE      = "HARPOON.HANDSHAKE";
    constexpr const char* HANDSHAKE_ACK  = "HARPOON.HANDSHAKE_ACK";
    constexpr const char* SERVER_INFO    = "HARPOON.SERVER_INFO";
    constexpr const char* RESUME         = "HARPOON.RESUME";
    constexpr const char* ERROR_MSG      = "HARPOON.ERROR";

    // Room lifecycle.
    constexpr const char* ROOM_CREATE          = "ROOM.CREATE";
    constexpr const char* ROOM_JOIN            = "ROOM.JOIN";
    constexpr const char* ROOM_LEAVE           = "ROOM.LEAVE";
    constexpr const char* ROOM_LIST            = "ROOM.LIST";
    constexpr const char* ROOM_LIST_RESPONSE   = "ROOM.LIST_RESPONSE";
    constexpr const char* ROOM_JOINED          = "ROOM.JOINED";
    constexpr const char* ROOM_MEMBERS_UPDATED = "ROOM.MEMBERS_UPDATED";
    constexpr const char* ROOM_PHASE_CHANGED   = "ROOM.PHASE_CHANGED";

    // Player state.
    constexpr const char* PLAYER_UPDATE_TRANSFORM   = "PLAYER.UPDATE_TRANSFORM";
    constexpr const char* PLAYER_UPDATE_SKELETON    = "PLAYER.UPDATE_SKELETON";
    constexpr const char* PLAYER_UPDATE_VISUAL_STATE = "PLAYER.UPDATE_VISUAL_STATE";
    constexpr const char* PLAYER_UPDATE_EQUIP_VISIBLE = "PLAYER.UPDATE_EQUIP_VISIBLE";
    constexpr const char* PLAYER_SET_TRANSFORMATION = "PLAYER.SET_TRANSFORMATION";
    constexpr const char* PLAYER_KILL               = "PLAYER.KILL";

    // Combat (Phase E).
    constexpr const char* COMBAT_DEAL_DAMAGE   = "COMBAT.DEAL_DAMAGE";
    constexpr const char* COMBAT_APPLY_STATUS  = "COMBAT.APPLY_STATUS";

    // Projectile mirror (Phase F).
    constexpr const char* APPEARANCE_SPAWN_VFX_ACTOR = "APPEARANCE.SPAWN_VFX_ACTOR";

    // Skin sync (Phase G).
    constexpr const char* APPEARANCE_SKIN_ANNOUNCE = "APPEARANCE.SKIN_SYNC.ANNOUNCE_CATALOG";
    constexpr const char* APPEARANCE_SKIN_UPDATE   = "APPEARANCE.SKIN_SYNC.UPDATE_SLOTS";

    // Save / flag sync (Phase I).
    constexpr const char* SAVE_SET_FLAG   = "SAVE.SET_FLAG";
    constexpr const char* SAVE_UNSET_FLAG = "SAVE.UNSET_FLAG";
}

enum class HarpoonConnState {
    Disconnected = 0,
    Connecting,
    Connected,
    InRoom,
};

// -----------------------------------------------------------------------------
// Per-remote-client state. Phase A only fills identity fields; Phase C+ fill
// pose/skeleton/equipment.
// -----------------------------------------------------------------------------
struct HarpoonClient {
    uint32_t clientId = 0;
    std::string name;
    uint32_t colorRgba = 0xFFFFFFFF;

    // Pose / skeleton (Phase C).
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    int16_t rotX = 0, rotY = 0, rotZ = 0;
    int16_t jointTable[24 * 3] = { 0 };  // 24 limbs * (x,y,z) Vec3s

    // Visual state.
    uint8_t transformation = 4;  // PLAYER_FORM_HUMAN
    uint8_t currentMask = 0;
    uint8_t heldItemId = 0;
    int16_t sceneId = -1;
    int8_t roomNum = -1;

    // Equipment (Phase J) — drives held weapon / shield / mask / boots on the dummy.
    int8_t currentBoots = 0;
    int8_t currentShield = 0;
    int8_t itemAction = 0;
    int8_t heldItemAction = 0;

    // Skin slots (Phase G).
    std::string adultSkinName;
    std::string equipSkinName;

    // Dummy actor handle (Phase D). Opaque from this header's POV.
    void* dummyActor = nullptr;
};

// One entry in the room browser (populated from ROOM.LIST_RESPONSE).
struct HarpoonRoomInfo {
    std::string roomId;
    std::string name;
    std::string gameMode;
    std::string state;
    int playerCount = 0;
    int maxPlayers = 0;
    bool hasPassword = false;
};

// Status effect type enum mirrored from Harpoon SoH (Phase E).
enum HarpoonStatus : uint8_t {
    HARPOON_STATUS_NONE      = 0,
    HARPOON_STATUS_BURN_DOT  = 1,
    HARPOON_STATUS_FREEZE    = 2,
    HARPOON_STATUS_BLINDNESS = 3,
    HARPOON_STATUS_STUN      = 4,
};

// -----------------------------------------------------------------------------
// Harpoon singleton. Wraps HarpoonWebSocket + JSON dispatch + state.
// -----------------------------------------------------------------------------
class Harpoon {
  public:
    static Harpoon* Instance();

    // Lifecycle.
    void Enable();
    void Disable();
    bool IsEnabled() const { return enabled_.load(); }
    HarpoonConnState State() const { return state_.load(); }

    // Room operations (called from game thread / UI). Safe no-op if not connected.
    void CreateRoom(const std::string& name, const std::string& password = "");
    void JoinRoom(const std::string& roomId, const std::string& password = "");
    void LeaveRoom();
    void RequestRoomList();

    // Broadcast a local flag change to peers (Phase I). sceneId is ignored for
    // global flag types. Safe no-op if not in a room.
    void SendFlag(bool set, int flagType, int sceneId, uint32_t flag);

    // Low-level send. Adds envelope (seq) if not present, then queues onto the
    // WebSocket worker thread. Thread-safe.
    void SendJson(const nlohmann::json& payload);

    // Called from the game thread once per frame to drain pending packets from
    // the WS receive queue and dispatch them.
    void DrainIncomingQueue();

    // Mutable access for hook handlers to fill remote client state from packets
    // (Phase C+). Caller MUST acquire StateMutex().
    HarpoonClient* GetOrCreateClient(uint32_t clientId);
    std::mutex& StateMutex() const { return stateMutex_; }
    std::unordered_map<uint32_t, HarpoonClient>& ClientsRaw() { return clients_; }

    // Init / shutdown. Init registers GameInteractor hooks (Drain per frame,
    // OnActorUpdate for player sync, OnSceneInit for dummy spawn, etc.). Must
    // be called once at boot (currently from BenGui startup).
    void InitHooks();
    void ShutdownHooks();

    // Read-only views (caller must hold no lock; mutex internal).
    uint32_t OwnClientId() const { return ownClientId_.load(); }
    std::string LastError() const;
    std::string CurrentRoomId() const;
    std::vector<HarpoonClient> GetClientsSnapshot() const;
    std::vector<HarpoonRoomInfo> GetRoomListSnapshot() const;

    // Selected gamemode for the next ROOM.CREATE (menu picks it). Defaults to
    // "default" — the restrictive baseline pack.
    void SetGameMode(const std::string& gm) { std::lock_guard<std::mutex> lk(stateMutex_); gameMode_ = gm; }
    std::string GameMode() const { std::lock_guard<std::mutex> lk(stateMutex_); return gameMode_; }

    // Convenience used by the menu's status line / buttons.
    bool IsConnected() const { return state_.load() >= HarpoonConnState::Connected; }
    bool IsConnecting() const { return enabled_.load() && state_.load() == HarpoonConnState::Connecting; }
    bool IsReady() const { return IsConnected() && ownClientId_.load() != 0; }

    // Behavior driven by the active room's gamemode (NOT manual toggles). The
    // gamemode id maps to its gamemode.yaml: "geoguessr" => PvP on, nametags
    // and minimap markers hidden. Coop modes => nametags/minimap shown, PvP off.
    bool IsPvpActive() const;       // gamemode permits PvP (e.g. geoguessr)
    bool NametagsVisible() const;   // false in geoguessr (hidden by design)
    bool MinimapVisible() const;    // false in geoguessr
    bool IsGeoguessr() const;       // current room's gamemode == "geoguessr"

  private:
    Harpoon() = default;
    ~Harpoon();

    void OnWsConnected();
    void OnWsDisconnected();
    void OnWsText(const std::string& text);

    void HandlePacket(const nlohmann::json& env);
    void HandleHandshakeAck(const nlohmann::json& payload);
    void HandleServerInfo(const nlohmann::json& payload);
    void HandleRoomJoined(const nlohmann::json& payload);
    void HandleRoomMembersUpdated(const nlohmann::json& payload);
    void HandleRoomListResponse(const nlohmann::json& payload);
    void HandleError(const nlohmann::json& payload);
    void HandlePlayerUpdateTransform(const nlohmann::json& payload);
    void HandlePlayerUpdateSkeleton(const nlohmann::json& payload);
    void HandlePlayerUpdateVisualState(const nlohmann::json& payload);
    void HandlePlayerUpdateEquipVisible(const nlohmann::json& payload);
    void HandlePlayerSetTransformation(const nlohmann::json& payload);
    void HandleCombatDealDamage(const nlohmann::json& payload);
    void HandleCombatApplyStatus(const nlohmann::json& payload);
    void HandleAppearanceSpawnVfxActor(const nlohmann::json& payload);
    void HandleAppearanceSkinUpdate(const nlohmann::json& payload);
    void HandleSaveFlag(const nlohmann::json& payload, bool set);

    int hookSceneFlagSetId_ = -1;
    int hookSceneFlagUnsetId_ = -1;
    int hookFlagSetId_ = -1;
    int hookFlagUnsetId_ = -1;

    int hookFrameId_ = -1;
    int hookActorUpdateId_ = -1;
    int hookSceneInitId_ = -1;
    int hookActorDestroyId_ = -1;

    std::unique_ptr<HarpoonWebSocket> ws_;
    std::atomic<bool> enabled_{ false };
    std::atomic<HarpoonConnState> state_{ HarpoonConnState::Disconnected };
    std::atomic<uint32_t> ownClientId_{ 0 };
    std::atomic<uint64_t> seqCounter_{ 1 };

    mutable std::mutex stateMutex_;
    std::string sessionToken_;
    std::string currentRoomId_;
    std::string lastError_;
    std::string gameMode_ = "default";
    std::unordered_map<uint32_t, HarpoonClient> clients_;
    std::vector<HarpoonRoomInfo> roomList_;

    std::mutex inboundMutex_;
    std::deque<nlohmann::json> inboundQueue_;
};

#endif // __cplusplus
#endif // HARPOON_H
