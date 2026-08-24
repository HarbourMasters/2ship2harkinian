// =============================================================================
// ProjectileMirror — render remote players' arrows/bombs/etc. (Phase F, stub).
//
// Today this only implements the receive side: when a remote client sends
// APPEARANCE.SPAWN_VFX_ACTOR with actorId=EN_ARROW or EN_BOM, we spawn the
// corresponding actor locally. The actor uses its vanilla owner-less behavior
// so it flies / explodes without belonging to anyone in particular.
//
// Send side (announcing OUR projectile spawns to peers) is a follow-up — it
// needs a hook on `Actor_Spawn` or per-actor init filtering, which the current
// 2ship GameInteractor doesn't expose neatly. Until that lands, peers won't
// see your arrows visually; they'll only feel the damage from CombatSync.
// =============================================================================

#include "../Harpoon.h"

extern "C" {
#include "z64.h"
#include "z64actor.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
extern PlayState* gPlayState;
}

extern "C" void Harpoon_SpawnRemoteVfxActor(uint32_t /*srcCid*/, int16_t actorId, float px, float py, float pz,
                                            int16_t rx, int16_t ry, int16_t rz, int16_t params) {
    if (!gPlayState)
        return;
    Actor_Spawn(&gPlayState->actorCtx, gPlayState, actorId, px, py, pz, rx, ry, rz, params);
}
