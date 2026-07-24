// FleetWarpDoor.cpp — MM South Clock Town FLEET HOLE (Door_Ana grotto) spawner.
//
// Spawns a real open Door_Ana at the cross-game warp spot, marked with FLEET_HOLE_PARAM so
// z_door_ana.c treats it as a PURE VISUAL (never grabs/warps). The FleetWarpArrival.cpp proximity
// trigger runs the cross-game fade+flip. Door_Ana draws gameplay_field_keep's grotto DL, which is
// NOT resident in town -> Object_SpawnPersistent it, then retry Actor_Spawn until it sticks.

#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "FleetShipCombo.h"
#include "FleetSync.h"

#include <spdlog/spdlog.h>

extern "C" {
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
extern PlayState* gPlayState;
}

namespace {

// Must match kMmHole* in FleetWarpArrival.cpp (the arrival pops Link out of this spot).
constexpr float kHoleX = -527.0f;
constexpr float kHoleY = 100.0f;
constexpr float kHoleZ = -1173.719f;

void FleetHoleSpawnTick() {
    static Actor* sHole = nullptr;
    static s16 sSceneWithHole = -1;
    static u32 sLastFrameCount = 0;

    if (FleetShipCombo_GetActiveGame() < 0 || gPlayState == NULL) {
        sHole = nullptr;
        sSceneWithHole = -1;
        return;
    }
    // Scene-load detection (soh mailbox_actor.c pattern): a fresh PlayState re-inits state.frames
    // to 0, so a frame-counter REWIND means a reload even when sceneId is unchanged AND even when
    // the new PlayState reuses the old one's arena address (a pointer compare misses that).
    s32 sceneLoaded = (gPlayState->sceneId != sSceneWithHole) || (gPlayState->state.frames < sLastFrameCount);
    sLastFrameCount = gPlayState->state.frames;
    if (sceneLoaded) {
        sHole = nullptr;
        sSceneWithHole = -1;
    }
    if (gPlayState->sceneId != SCENE_CLOCKTOWER) { // 0x6F South Clock Town
        return;
    }
    if (sHole != nullptr) {
        return; // already placed this scene
    }
    if (Object_GetSlot(&gPlayState->objectCtx, GAMEPLAY_FIELD_KEEP) < 0) {
        Object_SpawnPersistent(&gPlayState->objectCtx, GAMEPLAY_FIELD_KEEP);
        return; // object not resident this frame yet -> retry next frame
    }
    sHole = Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_DOOR_ANA, kHoleX, kHoleY, kHoleZ, 0, 0, 0,
                        FLEET_HOLE_PARAM /* pure visual: z_door_ana.c never grabs/warps it */);
    if (sHole != nullptr) {
        sHole->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED;
        sSceneWithHole = gPlayState->sceneId;
        SPDLOG_INFO("[FleetSync] Clock Town fleet hole spawned");
    } else {
        SPDLOG_WARN("[FleetSync] Clock Town fleet hole Actor_Spawn FAILED (retrying)");
    }
}

void RegisterFleetWarpDoor() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayDrawWorldEnd>(FleetHoleSpawnTick);
}

} // namespace

static RegisterShipInitFunc initFunc(RegisterFleetWarpDoor, {});
