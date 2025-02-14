#ifdef ENABLE_NETWORKING
#ifndef NETWORK_ANCHOR_H
#define NETWORK_ANCHOR_H
#ifdef __cplusplus

#include "2s2h/Network/Network.h"
#include <libultraship/libultraship.h>
#include "build.h"
#include "2s2h/Rando/Rando.h"

extern "C" {
#include "variables.h"
#include "z64.h"
}

void DummyPlayer_Init(Actor* actor, PlayState* play);
void DummyPlayer_Update(Actor* actor, PlayState* play);
void DummyPlayer_Draw(Actor* actor, PlayState* play);
void DummyPlayer_Destroy(Actor* actor, PlayState* play);

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
    s32 entrance;

    // Only available in PLAYER_UPDATE packets
    uint8_t transformation;
    PosRot posRot;
    u8 jointTable[159];
    u8 upperJointTable[159];
    uint8_t currentMask;
    uint8_t rightHandType;
    uint8_t leftHandType;
    int8_t currentShield;
    uint8_t sheathType;
    int8_t heldItemAction;
    uint8_t heldItemId;
    int8_t itemAction;
    uint32_t stateFlags1;
    uint32_t stateFlags2;
    uint32_t stateFlags3;
    float unk_B0C;
    int16_t unk_B28;
    int16_t unk_ACC;

    // Ptr to the dummy player
    Player* player;
} AnchorClient;

typedef struct {
    uint32_t ownerClientId;
    u8 pvpMode; // 0 = off, 1 = on, 2 = on with friendly fire
    std::vector<std::string> teams;
} RoomState;

class Anchor : public Network {
  private:
    bool refreshingActors = false;
    bool justLoadedSave = false;
    bool isHandlingUpdateTeamState = false;
    uint32_t ownClientId;

    nlohmann::json PrepClientState();
    nlohmann::json PrepRoomState();
    bool IsSaveLoaded();
    void RegisterHooks();
    void RefreshClientActors();
    void InitializeMultiWorld();
    void HandlePacket_AllClientState(nlohmann::json payload);
    void HandlePacket_DamagePlayer(nlohmann::json payload);
    void HandlePacket_DisableAnchor(nlohmann::json payload);
    void HandlePacket_GameComplete(nlohmann::json payload);
    void HandlePacket_GiveItem(nlohmann::json payload);
    void HandlePacket_PlayerSfx(nlohmann::json payload);
    void HandlePacket_PlayerUpdate(nlohmann::json payload);
    void HandlePacket_RequestTeamState(nlohmann::json payload);
    void HandlePacket_RequestTeleport(nlohmann::json payload);
    void HandlePacket_ServerMessage(nlohmann::json payload);
    void HandlePacket_SetCheckStatus(nlohmann::json payload);
    void HandlePacket_SetFlag(nlohmann::json payload);
    void HandlePacket_TeleportTo(nlohmann::json payload);
    void HandlePacket_UnsetFlag(nlohmann::json payload);
    void HandlePacket_UpdateClientState(nlohmann::json payload);
    void HandlePacket_UpdateDungeonItems(nlohmann::json payload);
    void HandlePacket_UpdateRoomState(nlohmann::json payload);
    void HandlePacket_UpdateTeamState(nlohmann::json payload);

  public:
    inline static const std::string clientVersion = (char*)gBuildVersion;

    // Packet types //
    inline static const std::string ALL_CLIENT_STATE = "ALL_CLIENT_STATE";
    inline static const std::string DAMAGE_PLAYER = "DAMAGE_PLAYER";
    inline static const std::string DISABLE_ANCHOR = "DISABLE_ANCHOR";
    inline static const std::string GAME_COMPLETE = "GAME_COMPLETE";
    inline static const std::string GIVE_ITEM = "GIVE_ITEM";
    inline static const std::string HANDSHAKE = "HANDSHAKE";
    inline static const std::string PLAYER_SFX = "PLAYER_SFX";
    inline static const std::string PLAYER_UPDATE = "PLAYER_UPDATE";
    inline static const std::string REQUEST_TEAM_STATE = "REQUEST_TEAM_STATE";
    inline static const std::string REQUEST_TELEPORT = "REQUEST_TELEPORT";
    inline static const std::string SERVER_MESSAGE = "SERVER_MESSAGE";
    inline static const std::string SET_CHECK_STATUS = "SET_CHECK_STATUS";
    inline static const std::string SET_FLAG = "SET_FLAG";
    inline static const std::string TELEPORT_TO = "TELEPORT_TO";
    inline static const std::string UNSET_FLAG = "UNSET_FLAG";
    inline static const std::string UPDATE_CLIENT_STATE = "UPDATE_CLIENT_STATE";
    inline static const std::string UPDATE_DUNGEON_ITEMS = "UPDATE_DUNGEON_ITEMS";
    inline static const std::string UPDATE_ROOM_STATE = "UPDATE_ROOM_STATE";
    inline static const std::string UPDATE_TEAM_STATE = "UPDATE_TEAM_STATE";

    static Anchor* Instance;
    std::map<uint32_t, AnchorClient> clients;
    std::vector<uint32_t> actorIndexToClientId;
    RoomState roomState;

    void Enable();
    void Disable();
    void OnIncomingJson(nlohmann::json payload);
    void OnConnected();
    void OnDisconnected();
    void DrawMenu();
    void SendJsonToRemote(nlohmann::json packet);

    void SendPacket_DamagePlayer(u32 clientId, u8 damageEffect, u8 damage);
    void SendPacket_GameComplete();
    void SendPacket_GiveItem(u16 modId, s16 getItemId, std::string targetTeamId = "");
    void SendPacket_Handshake();
    void SendPacket_PlayerSfx(u16 sfxId);
    void SendPacket_PlayerUpdate();
    void SendPacket_RequestTeamState();
    void SendPacket_RequestTeleport(u32 clientId);
    void SendPacket_SetCheckStatus(RandoCheckId randoCheckId);
    void SendPacket_SetFlag(s16 sceneId, s16 flagType, s16 flag);
    void SendPacket_TeleportTo(u32 clientId);
    void SendPacket_UnsetFlag(s16 sceneId, s16 flagType, s16 flag);
    void SendPacket_UpdateClientState();
    void SendPacket_UpdateDungeonItems();
    void SendPacket_UpdateRoomState();
    void SendPacket_UpdateTeamState(std::string targetTeamId);
};

typedef enum {
    // Starting at 5 to continue from the last value in the PlayerDamageResponseType enum
    DUMMY_PLAYER_HIT_RESPONSE_STUN = 5,
    DUMMY_PLAYER_HIT_RESPONSE_FIRE,
    DUMMY_PLAYER_HIT_RESPONSE_NORMAL,
} DummyPlayerDamageResponseType;

#endif // __cplusplus
#endif // NETWORK_ANCHOR_H
#endif // ENABLE_NETWORKING
