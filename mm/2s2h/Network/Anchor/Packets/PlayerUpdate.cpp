#include "2s2h/Network/Anchor/Anchor.h"
#include "2s2h/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "macros.h"
#include "variables.h"
#include "functions.h"
extern PlayState* gPlayState;
}

/**
 * PLAYER_UPDATE
 *
 * Real-time data needed to render another player in the same scene. Sent every frame to each
 * other client currently in the same scene. Marked "quiet" so it does not spam the logs.
 *
 * This is sent _a lot_, so keep the payload as small as is practical.
 */

void Anchor::SendPacket_PlayerUpdate() {
    if (!IsSaveLoaded()) {
        return;
    }

    // Only send if at least one other client is in our scene.
    uint32_t recipients = 0;
    for (auto& [clientId, client] : clients) {
        if (!client.self && client.online && client.isSaveLoaded && client.sceneId == gPlayState->sceneId) {
            recipients++;
        }
    }
    if (recipients == 0) {
        return;
    }

    Player* player = GET_PLAYER(gPlayState);

    nlohmann::json payload;
    payload["type"] = PLAYER_UPDATE;
    payload["sceneId"] = gPlayState->sceneId;
    payload["entranceIndex"] = gSaveContext.save.entrance;
    payload["form"] = player->transformation;
    payload["posRot"]["pos"] = player->actor.world.pos;
    payload["posRot"]["rot"] = player->actor.shape.rot;

    std::vector<int> jointArray;
    jointArray.reserve(PLAYER_LIMB_MAX * 3);
    for (size_t i = 0; i < PLAYER_LIMB_MAX; i++) {
        Vec3s joint = player->skelAnime.jointTable[i];
        jointArray.push_back(joint.x);
        jointArray.push_back(joint.y);
        jointArray.push_back(joint.z);
    }
    payload["jointTable"] = jointArray;
    payload["prevTransl"] = player->skelAnime.prevTransl;
    payload["movementFlags"] = player->skelAnime.movementFlags;
    payload["upperLimbRot"] = player->upperLimbRot;
    payload["currentBoots"] = player->currentBoots;
    payload["currentMask"] = player->currentMask;
    payload["stateFlags1"] = player->stateFlags1;
    payload["stateFlags2"] = player->stateFlags2;
    payload["itemAction"] = player->itemAction;
    payload["heldItemAction"] = player->heldItemAction;
    payload["invincibilityTimer"] = player->invincibilityTimer;
    payload["face"] = player->actor.shape.face;
    payload["quiet"] = true;

    for (auto& [clientId, client] : clients) {
        if (!client.self && client.online && client.isSaveLoaded && client.sceneId == gPlayState->sceneId) {
            payload["targetClientId"] = clientId;
            SendPacket(payload);
        }
    }
}

void Anchor::HandlePacket_PlayerUpdate(nlohmann::json payload) {
    uint32_t clientId = payload["clientId"].get<uint32_t>();
    if (!clients.contains(clientId)) {
        return;
    }

    auto& client = clients[clientId];

    s32 newForm = payload.value("form", (s32)PLAYER_FORM_HUMAN);
    s16 newSceneId = payload.value("sceneId", (s16)SCENE_MAX);
    if (client.form != newForm || client.sceneId != newSceneId) {
        // A form change means a different skeleton; a scene change means the dummy must be
        // (de)spawned. Either way, respawn dummy actors on the next tick.
        shouldRefreshActors = true;
    }

    client.sceneId = newSceneId;
    client.entranceIndex = payload.value("entranceIndex", (s32)0);
    client.form = newForm;
    client.posRot = payload.value("posRot", PosRot{ 0 });

    std::vector<int> jointArray = payload.value("jointTable", std::vector<int>{});
    jointArray.resize(PLAYER_LIMB_MAX * 3); // pad in case of missing data
    for (size_t i = 0; i < PLAYER_LIMB_MAX; i++) {
        client.jointTable[i].x = jointArray[i * 3];
        client.jointTable[i].y = jointArray[i * 3 + 1];
        client.jointTable[i].z = jointArray[i * 3 + 2];
    }

    client.prevTransl = payload.value("prevTransl", Vec3s{ 0 });
    client.movementFlags = payload.value("movementFlags", (u8)0);
    client.upperLimbRot = payload.value("upperLimbRot", Vec3s{ 0 });
    client.currentBoots = payload.value("currentBoots", (s8)0);
    client.currentMask = payload.value("currentMask", (u8)0);
    client.stateFlags1 = payload.value("stateFlags1", (u32)0);
    client.stateFlags2 = payload.value("stateFlags2", (u32)0);
    client.itemAction = payload.value("itemAction", (s8)0);
    client.heldItemAction = payload.value("heldItemAction", (s8)0);
    client.invincibilityTimer = payload.value("invincibilityTimer", (s8)0);
    client.face = payload.value("face", (u8)0);
}
