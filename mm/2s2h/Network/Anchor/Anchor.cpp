#include "Anchor.h"
#include "JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include <spdlog/spdlog.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/BenGui/Notification.h"
#include "2s2h/ObjectExtension/ObjectExtension.h"
#include "2s2h/NameTag/NameTag.h"

extern "C" {
#include "macros.h"
#include "variables.h"
#include "functions.h"
extern PlayState* gPlayState;
extern char gGitCommitHash[];
}

const std::string Anchor::clientVersion = (const char*)gGitCommitHash;

// MARK: - Lifecycle

void Anchor::Enable() {
    ownClientId = CVarGetInteger("gNetwork.Anchor.LastClientId", 0);
    roomState.ownerClientId = 0;

    // Register the game-thread hooks once for the lifetime of the connection. The handlers
    // bail out early when not connected, so it is safe to leave them registered.
    RegisterHooks();

    Network::Enable(CVarGetString("gNetwork.Anchor.Host", "anchor.hm64.org"),
                    CVarGetInteger("gNetwork.Anchor.Port", 43383));
}

void Anchor::Disable() {
    Network::Disable();
    UnregisterHooks();
    clients.clear();
}

void Anchor::OnConnected() {
    // Runs on the network thread; isConnected is already true here.
    SendPacket_Handshake();
}

void Anchor::OnDisconnected() {
}

void Anchor::RegisterHooks() {
    if (processPacketsHookId == 0) {
        processPacketsHookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateMainStart>([]() {
            if (Anchor::Instance->isConnected) {
                Anchor::Instance->ProcessIncomingPacketQueue();
            }
        });
    }
    if (sceneInitHookId == 0) {
        sceneInitHookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s8 sceneId,
                                                                                                     s8 spawnNum) {
            if (Anchor::Instance->isConnected) {
                Anchor::Instance->SendPacket_UpdateClientState();
                // Respawn dummy players for this scene on the next player update tick.
                Anchor::Instance->shouldRefreshActors = true;
            }
        });
    }
    if (actorInitHookId == 0) {
        // Intercept the player actors we spawn for remote clients and repurpose them as dummies.
        actorInitHookId = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldActorInit>(
            ACTOR_PLAYER, [](Actor* actor, bool* should) {
                if (Anchor::Instance->spawningDummyPlayerForClientId != 0) {
                    Anchor::Instance->SetDummyPlayerClientId(actor, Anchor::Instance->spawningDummyPlayerForClientId);
                    Actor_ChangeCategory(gPlayState, &gPlayState->actorCtx, actor, ACTORCAT_NPC);
                    actor->id = ACTOR_EN_TEST;
                    actor->init = DummyPlayer_Init;
                    actor->update = DummyPlayer_Update;
                    actor->draw = DummyPlayer_Draw;
                    actor->destroy = DummyPlayer_Destroy;
                }
            });
    }
    if (actorUpdateHookId == 0) {
        // Fires every frame for the local player only (dummies are relabeled ACTOR_EN_TEST).
        actorUpdateHookId = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(
            ACTOR_PLAYER, [](Actor* actor) {
                if (!Anchor::Instance->isConnected) {
                    return;
                }
                if (Anchor::Instance->shouldRefreshActors) {
                    Anchor::Instance->shouldRefreshActors = false;
                    Anchor::Instance->RefreshClientActors();
                }
                Anchor::Instance->SendPacket_PlayerUpdate();
            });
    }
}

void Anchor::UnregisterHooks() {
    if (processPacketsHookId != 0) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateMainStart>(processPacketsHookId);
        processPacketsHookId = 0;
    }
    if (sceneInitHookId != 0) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(sceneInitHookId);
        sceneInitHookId = 0;
    }
    if (actorInitHookId != 0) {
        GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldActorInit>(actorInitHookId);
        actorInitHookId = 0;
    }
    if (actorUpdateHookId != 0) {
        GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorUpdate>(actorUpdateHookId);
        actorUpdateHookId = 0;
    }
}

// MARK: - Send

void Anchor::SendPacket(nlohmann::json payload) {
    if (!isConnected) {
        return;
    }

    payload["clientId"] = ownClientId;
    QueueOutgoingPacket(payload);
}

// MARK: - Receive

void Anchor::ProcessIncomingPacketQueue() {
    std::queue<nlohmann::json> packetsToProcess;
    SwapIncomingPacketQueue(packetsToProcess);

    while (!packetsToProcess.empty()) {
        nlohmann::json payload = packetsToProcess.front();
        packetsToProcess.pop();

        try {
            OnIncomingJson(payload);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("[Anchor] Exception while processing incoming packet: {}", e.what());
            SPDLOG_ERROR("[Anchor] Packet: {}", payload.dump());
        } catch (...) { SPDLOG_ERROR("[Anchor] Unknown exception while processing incoming packet"); }
    }
}

void Anchor::OnIncomingJson(nlohmann::json payload) {
    if (!payload.contains("type")) {
        return;
    }

    std::string packetType = payload["type"].get<std::string>();

    if (packetType == ALL_CLIENT_STATE) {
        HandlePacket_AllClientState(payload);
    } else if (packetType == UPDATE_CLIENT_STATE) {
        HandlePacket_UpdateClientState(payload);
    } else if (packetType == PLAYER_UPDATE) {
        HandlePacket_PlayerUpdate(payload);
    } else if (packetType == SERVER_MESSAGE) {
        HandlePacket_ServerMessage(payload);
    } else if (packetType == DISABLE_ANCHOR) {
        HandlePacket_DisableAnchor(payload);
    }
}

// MARK: - Helpers

bool Anchor::IsSaveLoaded() {
    if (gPlayState == nullptr) {
        return false;
    }
    if (GET_PLAYER(gPlayState) == nullptr) {
        return false;
    }
    if (gSaveContext.gameMode != GAMEMODE_NORMAL) {
        return false;
    }
    return true;
}

// MARK: - Packets

nlohmann::json Anchor::PrepRoomState() {
    nlohmann::json payload;
    payload["pvpMode"] = roomState.pvpMode;
    payload["showLocationsMode"] = roomState.showLocationsMode;
    payload["teleportMode"] = roomState.teleportMode;
    payload["syncItemsAndFlags"] = roomState.syncItemsAndFlags;
    return payload;
}

nlohmann::json Anchor::PrepClientState() {
    nlohmann::json payload;
    payload["name"] = CVarGetString("gNetwork.Anchor.Name", "");
    payload["color"] = CVarGetColor24("gNetwork.Anchor.Color.Value", { 100, 255, 100 });
    payload["clientVersion"] = clientVersion;
    payload["teamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");
    payload["online"] = true;

    if (IsSaveLoaded()) {
        payload["seed"] = 0;
        payload["isSaveLoaded"] = true;
        payload["isGameComplete"] = false;
        payload["sceneId"] = gPlayState->sceneId;
        payload["curRoomNum"] = gPlayState->roomCtx.curRoom.num;
        payload["entranceIndex"] = gSaveContext.save.entrance;
    } else {
        payload["seed"] = 0;
        payload["isSaveLoaded"] = false;
        payload["isGameComplete"] = false;
        payload["sceneId"] = SCENE_MAX;
        payload["curRoomNum"] = -1;
        payload["entranceIndex"] = 0;
    }

    return payload;
}

// HANDSHAKE: sent on first connection, includes our room settings (in case the room needs
// creating) and our current client state.
void Anchor::SendPacket_Handshake() {
    nlohmann::json payload;
    payload["type"] = HANDSHAKE;
    payload["roomId"] = CVarGetString("gNetwork.Anchor.RoomId", "");
    payload["roomState"] = PrepRoomState();
    payload["clientState"] = PrepClientState();
    SendPacket(payload);
}

// UPDATE_CLIENT_STATE: small subset of cached state, sent on scene change etc.
void Anchor::SendPacket_UpdateClientState() {
    nlohmann::json payload;
    payload["type"] = UPDATE_CLIENT_STATE;
    payload["state"] = PrepClientState();
    SendPacket(payload);
}

// ALL_CLIENT_STATE: full roster, broadcast by the server when a client connects/disconnects.
void Anchor::HandlePacket_AllClientState(nlohmann::json payload) {
    std::vector<AnchorClient> newClients = payload["state"].get<std::vector<AnchorClient>>();

    for (auto& client : newClients) {
        if (client.self) {
            ownClientId = client.clientId;
            CVarSetInteger("gNetwork.Anchor.LastClientId", ownClientId);
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
            clients[client.clientId].self = true;
        } else {
            bool wasKnown = clients.contains(client.clientId);
            bool wasOnline = wasKnown && clients[client.clientId].online;
            clients[client.clientId].self = false;
            if ((!wasKnown && client.online) || (wasKnown && wasOnline != client.online)) {
                Notification::Emit({
                    .prefix = client.name,
                    .message = client.online ? "Connected" : "Disconnected",
                });
            }
        }

        clients[client.clientId].clientId = client.clientId;
        clients[client.clientId].name = client.name;
        clients[client.clientId].color = client.color;
        clients[client.clientId].clientVersion = client.clientVersion;
        clients[client.clientId].teamId = client.teamId;
        clients[client.clientId].online = client.online;
        clients[client.clientId].seed = client.seed;
        clients[client.clientId].isSaveLoaded = client.isSaveLoaded;
        clients[client.clientId].isGameComplete = client.isGameComplete;
        clients[client.clientId].sceneId = client.sceneId;
        clients[client.clientId].curRoomNum = client.curRoomNum;
        clients[client.clientId].entranceIndex = client.entranceIndex;
    }

    // Remove clients no longer present in the roster.
    std::vector<uint32_t> clientsToRemove;
    for (auto& [clientId, client] : clients) {
        if (std::find_if(newClients.begin(), newClients.end(),
                         [clientId](AnchorClient& c) { return c.clientId == clientId; }) == newClients.end()) {
            clientsToRemove.push_back(clientId);
        }
    }
    for (auto& clientId : clientsToRemove) {
        clients.erase(clientId);
    }

    // Roster changed; respawn dummy players on the next player update tick.
    shouldRefreshActors = true;
}

void Anchor::HandlePacket_UpdateClientState(nlohmann::json payload) {
    uint32_t clientId = payload.at("clientId").get<uint32_t>();
    if (!clients.contains(clientId)) {
        return;
    }

    s16 oldSceneId = clients[clientId].sceneId;
    AnchorClient client = payload["state"].get<AnchorClient>();
    clients[clientId].name = client.name;
    clients[clientId].color = client.color;
    clients[clientId].clientVersion = client.clientVersion;
    clients[clientId].teamId = client.teamId;
    clients[clientId].online = client.online;
    clients[clientId].seed = client.seed;
    clients[clientId].isSaveLoaded = client.isSaveLoaded;
    clients[clientId].isGameComplete = client.isGameComplete;
    clients[clientId].sceneId = client.sceneId;
    clients[clientId].curRoomNum = client.curRoomNum;
    clients[clientId].entranceIndex = client.entranceIndex;

    // A remote player entering or leaving our scene must (de)spawn their dummy.
    if (oldSceneId != client.sceneId) {
        shouldRefreshActors = true;
    }
}

void Anchor::HandlePacket_ServerMessage(nlohmann::json payload) {
    if (payload.contains("message")) {
        Notification::Emit({
            .prefix = "Server: ",
            .message = payload["message"].get<std::string>(),
        });
    }
}

void Anchor::HandlePacket_DisableAnchor(nlohmann::json payload) {
    CVarClear("gNetwork.Anchor.Enabled");
    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    Disable();
}

// MARK: - Dummy players

// Attaches the owning clientId to a dummy player actor via the ObjectExtension system.
struct DummyPlayerClientId {
    uint32_t clientId = 0;
};
static ObjectExtension::Register<DummyPlayerClientId> DummyPlayerClientIdRegister;

uint32_t Anchor::GetDummyPlayerClientId(const Actor* actor) {
    const DummyPlayerClientId* data = ObjectExtension::GetInstance().Get<DummyPlayerClientId>(actor);
    return data != nullptr ? data->clientId : 0;
}

void Anchor::SetDummyPlayerClientId(const Actor* actor, uint32_t clientId) {
    ObjectExtension::GetInstance().Set<DummyPlayerClientId>(actor, DummyPlayerClientId{ clientId });
}

// Kills all existing dummy players and respawns one for each online client in our scene.
void Anchor::RefreshClientActors() {
    if (!IsSaveLoaded()) {
        return;
    }

    Actor* actor = gPlayState->actorCtx.actorLists[ACTORCAT_NPC].first;
    while (actor != NULL) {
        Actor* next = actor->next;
        if (actor->id == ACTOR_EN_TEST && actor->update == DummyPlayer_Update) {
            NameTag_RemoveAllForActor(actor);
            Actor_Kill(actor);
        }
        actor = next;
    }

    for (auto& [clientId, client] : clients) {
        if (!client.online || client.self || !client.isSaveLoaded) {
            continue;
        }
        if (client.sceneId != gPlayState->sceneId) {
            continue;
        }

        spawningDummyPlayerForClientId = clientId;
        // ShouldActorInit (registered above) intercepts this spawn and converts it to a dummy.
        Actor* dummy =
            Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_PLAYER, client.posRot.pos.x, client.posRot.pos.y,
                        client.posRot.pos.z, client.posRot.rot.x, client.posRot.rot.y, client.posRot.rot.z, 0);
        client.player = (Player*)dummy;
    }
    spawningDummyPlayerForClientId = 0;
}
