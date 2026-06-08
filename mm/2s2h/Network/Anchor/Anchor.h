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
    inline static const std::string SERVER_MESSAGE = "SERVER_MESSAGE";
    inline static const std::string DISABLE_ANCHOR = "DISABLE_ANCHOR";

    void Enable();
    void Disable();
    void OnConnected() override;
    void OnDisconnected() override;

    // Drains the base class' incoming queue and dispatches each packet. Game thread only.
    void ProcessIncomingPacketQueue();

    bool IsSaveLoaded();

    void SendPacket_Handshake();
    void SendPacket_UpdateClientState();
};

#endif // __cplusplus
#endif // NETWORK_ANCHOR_H
