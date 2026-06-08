#include "Anchor.h"
#include "2s2h/NameTag/NameTag.h"
#include <cstring>

extern "C" {
#include "macros.h"
#include "variables.h"
#include "functions.h"
extern PlayState* gPlayState;

// Per-form physical properties (shadow scale, etc). Not declared in a header, but has external
// linkage in z_player.c. The dummy needs ageProperties set before Player_InitCommon runs.
extern PlayerAgeProperties sPlayerAgeProperties[];

void Player_Draw(Actor* thisx, PlayState* play);
}

static void DummyPlayer_CopyVec3s(Vec3s* dest, Vec3s* src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

// Dummy players are spawned as ACTOR_PLAYER (so the full player skeleton/overlay is loaded), then
// repurposed in Anchor's ShouldActorInit hook: moved to ACTORCAT_NPC, relabeled ACTOR_EN_TEST, and
// given these init/update/draw/destroy functions.
void DummyPlayer_Init(Actor* actor, PlayState* play) {
    Player* player = (Player*)actor;

    uint32_t clientId = Anchor::Instance->GetDummyPlayerClientId(actor);
    if (!Anchor::Instance->clients.contains(clientId)) {
        Actor_Kill(actor);
        return;
    }

    AnchorClient& client = Anchor::Instance->clients[clientId];

    // Player_InitCommon picks behavior from these; set them before it runs to avoid using the
    // local player's state (and to avoid dereferencing a null ageProperties).
    player->transformation = client.form;
    player->ageProperties = &sPlayerAgeProperties[client.form];
    actor->room = -1;

    play->playerInit(player, play, gPlayerSkeletons[client.form]);

    // Populate the per-model display list pointers (waist/sheath/hands/etc). Without this the
    // gameplay limb-draw callback dereferences null DList arrays and crashes.
    player->itemAction = client.itemAction;
    player->heldItemAction = client.heldItemAction;
    Player_SetModelGroup(player, Player_ActionToModelGroup(player, (PlayerItemAction)player->heldItemAction));

    NameTag_RegisterForActor(actor, client.name.c_str());
}

void DummyPlayer_Update(Actor* actor, PlayState* play) {
    Player* player = (Player*)actor;

    uint32_t clientId = Anchor::Instance->GetDummyPlayerClientId(actor);
    if (!Anchor::Instance->clients.contains(clientId)) {
        Actor_Kill(actor);
        return;
    }

    AnchorClient& client = Anchor::Instance->clients[clientId];

    // Hide the dummy when the client is in another scene / not in game.
    if (client.sceneId != gPlayState->sceneId || !client.online || !client.isSaveLoaded) {
        actor->world.pos.x = -9999.0f;
        actor->world.pos.y = -9999.0f;
        actor->world.pos.z = -9999.0f;
        actor->shape.shadowAlpha = 0;
        return;
    }

    actor->shape.shadowAlpha = 255;

    DummyPlayer_CopyVec3s(&player->upperLimbRot, &client.upperLimbRot);
    DummyPlayer_CopyVec3s(&actor->shape.rot, &client.posRot.rot);
    Math_Vec3f_Copy(&actor->world.pos, &client.posRot.pos);

    memcpy(player->skelAnime.jointTable, client.jointTable, sizeof(Vec3s) * PLAYER_LIMB_MAX);
    player->skelAnime.movementFlags = client.movementFlags;
    DummyPlayer_CopyVec3s(&player->skelAnime.prevTransl, &client.prevTransl);
    player->currentBoots = client.currentBoots;
    player->currentMask = client.currentMask;
    player->stateFlags1 = client.stateFlags1;
    player->stateFlags2 = client.stateFlags2;
    player->itemAction = client.itemAction;
    // Re-select the model group when the held item changes so the correct DLists are bound.
    if (player->heldItemAction != client.heldItemAction) {
        player->heldItemAction = client.heldItemAction;
        Player_SetModelGroup(player, Player_ActionToModelGroup(player, (PlayerItemAction)player->heldItemAction));
    }
    player->invincibilityTimer = client.invincibilityTimer;

    // Apply in-place animation root motion (mirrors Player's own translation handling).
    Vec3f diff;
    SkelAnime_UpdateTranslation(&player->skelAnime, &diff, actor->shape.rot.y);
    if (player->skelAnime.movementFlags & 1) {
        actor->world.pos.x += diff.x * actor->scale.x;
        actor->world.pos.z += diff.z * actor->scale.z;
    }
    if (player->skelAnime.movementFlags & 2) {
        actor->world.pos.y += diff.y * actor->scale.y;
    }
}

void DummyPlayer_Draw(Actor* actor, PlayState* play) {
    Player* player = (Player*)actor;

    uint32_t clientId = Anchor::Instance->GetDummyPlayerClientId(actor);
    if (!Anchor::Instance->clients.contains(clientId)) {
        Actor_Kill(actor);
        return;
    }

    AnchorClient& client = Anchor::Instance->clients[clientId];
    if (client.sceneId != gPlayState->sceneId || !client.online || !client.isSaveLoaded) {
        return;
    }

    Player_Draw((Actor*)player, play);
}

void DummyPlayer_Destroy(Actor* actor, PlayState* play) {
    // The actor was spawned as ACTOR_PLAYER but relabeled ACTOR_EN_TEST. Restore the id so the
    // ActorDB's numLoaded for ACTOR_PLAYER is decremented correctly on destroy.
    actor->id = ACTOR_PLAYER;
}
