#ifndef NETWORK_ANCHOR_H
#define NETWORK_ANCHOR_H
#ifdef __cplusplus

#include "2s2h/Network/Network.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <map>
#include <string>

extern "C" {
#include "z64.h"
}

void DummyPlayer_Init(Actor* actor, PlayState* play);
void DummyPlayer_Update(Actor* actor, PlayState* play);
void DummyPlayer_Draw(Actor* actor, PlayState* play);
void DummyPlayer_Destroy(Actor* actor, PlayState* play);

// Per-client state shared across the room. This is the Majora's Mask flavored equivalent of
// Shipwright's AnchorClient; field names track MM structures (e.g. sceneId rather than sceneNum).
typedef struct {
    uint32_t clientId;
    std::string name;
    Color_RGB8 color;
    std::string clientVersion;
    std::string teamId;
    bool online;
    bool self;
    uint32_t seed;
    bool isSaveLoaded;
    bool isGameComplete;
    s16 sceneId;
    s8 curRoomNum;
    s32 entranceIndex;

    // Only populated by PLAYER_UPDATE packets (real-time, sent every frame).
    s32 form; // PlayerTransformation
    PosRot posRot;
    Vec3s jointTable[PLAYER_LIMB_MAX];
    Vec3s prevTransl;
    Vec3s upperLimbRot;
    u8 movementFlags;
    s8 currentBoots;
    u8 currentMask;
    u32 stateFlags1;
    u32 stateFlags2;
    s8 itemAction;
    s8 heldItemAction;
    s8 invincibilityTimer;
    u8 face;

    // Pointer to the spawned dummy player actor (if any).
    Player* player;
} AnchorClient;

typedef struct {
    uint32_t ownerClientId;
    u8 pvpMode;           // 0 = off, 1 = on, 2 = on with friendly fire
    u8 showLocationsMode; // 0 = none, 1 = team, 2 = all
    u8 teleportMode;      // 0 = off, 1 = team, 2 = all
    u8 syncItemsAndFlags; // 0 = off, 1 = on
} RoomState;

class Anchor : public Network {
  private:
    HOOK_ID processPacketsHookId = 0;
    HOOK_ID sceneInitHookId = 0;
    HOOK_ID actorInitHookId = 0;
    HOOK_ID actorUpdateHookId = 0;
    HOOK_ID flagSetHookId = 0;
    HOOK_ID flagUnsetHookId = 0;
    HOOK_ID sceneFlagSetHookId = 0;
    HOOK_ID sceneFlagUnsetHookId = 0;

    uint32_t spawningDummyPlayerForClientId = 0;
    bool shouldRefreshActors = false;
    // Set while applying an incoming packet so our own send-hooks don't echo it back out.
    bool isApplyingRemotePacket = false;

    void RegisterHooks();
    void UnregisterHooks();

    nlohmann::json PrepClientState();
    nlohmann::json PrepRoomState();

    // Injects our clientId and queues the packet on the network thread.
    void SendPacket(nlohmann::json payload);

    // Dispatches a single incoming packet (called on the game thread).
    void OnIncomingJson(nlohmann::json payload);

    void HandlePacket_AllClientState(nlohmann::json payload);
    void HandlePacket_UpdateClientState(nlohmann::json payload);
    void HandlePacket_PlayerUpdate(nlohmann::json payload);
    void HandlePacket_SetFlag(nlohmann::json payload);
    void HandlePacket_UnsetFlag(nlohmann::json payload);
    void HandlePacket_ServerMessage(nlohmann::json payload);
    void HandlePacket_DisableAnchor(nlohmann::json payload);

  public:
    static Anchor* Instance;

    uint32_t ownClientId = 0;
    static const std::string clientVersion;
    std::map<uint32_t, AnchorClient> clients;
    RoomState roomState = {};

    // Packet types
    inline static const std::string HANDSHAKE = "HANDSHAKE";
    inline static const std::string ALL_CLIENT_STATE = "ALL_CLIENT_STATE";
    inline static const std::string UPDATE_CLIENT_STATE = "UPDATE_CLIENT_STATE";
    inline static const std::string PLAYER_UPDATE = "PLAYER_UPDATE";
    inline static const std::string SET_FLAG = "SET_FLAG";
    inline static const std::string UNSET_FLAG = "UNSET_FLAG";
    inline static const std::string SERVER_MESSAGE = "SERVER_MESSAGE";
    inline static const std::string DISABLE_ANCHOR = "DISABLE_ANCHOR";

    void Enable();
    void Disable();
    void OnConnected() override;
    void OnDisconnected() override;

    // Drains the base class' incoming queue and dispatches each packet. Game thread only.
    void ProcessIncomingPacketQueue();

    bool IsSaveLoaded();

    // Remote player (dummy actor) management.
    void RefreshClientActors();
    uint32_t GetDummyPlayerClientId(const Actor* actor);
    void SetDummyPlayerClientId(const Actor* actor, uint32_t clientId);

    void SendPacket_Handshake();
    void SendPacket_UpdateClientState();
    void SendPacket_PlayerUpdate();
    void SendPacket_SetFlag(s16 sceneId, s16 flagType, s32 flag);
    void SendPacket_UnsetFlag(s16 sceneId, s16 flagType, s32 flag);
};

#endif // __cplusplus
#endif // NETWORK_ANCHOR_H
