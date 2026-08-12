/**
 * Cane summon system — see somaria_cubes.h.
 *
 * Every summon is a REAL MM actor, never a hijack. The old En_Lightbox hijack that
 * lived here could never work in MM: ACTOR_EN_LIGHTBOX is an OoT-only id that lands
 * out of bounds (0x7F16) in MM's actor table, so Actor_Spawn silently produced
 * nothing and the Cane of Somaria summoned exactly zero blocks. Skijer's NEI
 */

#include "somaria_cubes.h"
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "nei_oot_compat.h" // Matrix_NewMtx -> Matrix_Finalize
#include "transformation_masks/transformation_masks.h"
#include "objects/object_d_lift/object_d_lift.h" // gDampeGraveBrownElevatorDL — the summoned platform

// En_Torch2 (the Elegy of Emptiness shell) selects which form's shell it is from
// its spawn params. Mirrored here rather than including the overlay header, which
// would drag global.h into the z_player translation unit this file lives in.
// Source: src/overlays/actors/ovl_En_Torch2/z_en_torch2.h (EnTorch2Param).
#define TORCH2_PARAM_HUMAN 0
#define TORCH2_PARAM_GORON 1
#define TORCH2_PARAM_ZORA 2
#define TORCH2_PARAM_DEKU 3
#define TORCH2_PARAM_FIERCE_DEITY 4

// ============================================================================
// POOL
// ============================================================================

typedef struct {
    Actor* actor;
    u8 kind;
    u16 seq; // spawn order, so "the oldest of this kind" is answerable
} CaneSummonSlot;

static CaneSummonSlot sSummons[SOMARIA_MAX_CUBES] = { { 0 } };
static u16 sSummonSeq = 0;

static u8 CaneSummon_CapFor(CaneSummonKind kind) {
    switch (kind) {
        case CANE_SUMMON_BLOCK:
            return CANE_MAX_BLOCKS;
        case CANE_SUMMON_PLATFORM:
            return CANE_MAX_PLATFORMS;
        case CANE_SUMMON_STATUE:
        default:
            return CANE_MAX_STATUES;
    }
}

// ============================================================================
// HELPERS
// ============================================================================

void SomariaCube_PlaySound(Actor* actor, u16 sfxId) {
    Audio_PlaySoundGeneral(sfxId, &actor->projectedPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultReverb);
}

u8 SomariaCube_IsSomariaCube(Actor* actor) {
    if (actor == NULL || actor->update == NULL) {
        return 0;
    }
    for (u8 i = 0; i < SOMARIA_MAX_CUBES; i++) {
        if (sSummons[i].actor == actor) {
            return 1;
        }
    }
    return 0;
}

u8 SomariaCube_IsSwitchable(Actor* actor) {
    return SomariaCube_IsSomariaCube(actor);
}

// The Elegy shell form for the player's CURRENT transformation. Fierce Deity puts
// down a human shell, exactly like the vanilla song does.
static u8 CaneSummon_CurrentForm(void) {
    if (!TransformMasks_IsTransformed()) {
        return TORCH2_PARAM_HUMAN;
    }
    // MM form enum (FD=0, Goron=1, Zora=2, Deku=3, Human=4) -> EnTorch2Param.
    switch (MmPlayer_GetForm()) {
        case 1:
            return TORCH2_PARAM_GORON;
        case 2:
            return TORCH2_PARAM_ZORA;
        case 3:
            return TORCH2_PARAM_DEKU;
        case 0:
            return TORCH2_PARAM_FIERCE_DEITY;
        default:
            return TORCH2_PARAM_HUMAN;
    }
}

u8 SomariaCube_GetForm(Actor* actor) {
    if (actor == NULL) {
        return ELEGY_FORM_HUMAN;
    }
    return (u8)actor->params;
}

void CaneSummon_CleanupPool(void) {
    for (u8 i = 0; i < SOMARIA_MAX_CUBES; i++) {
        if (sSummons[i].actor != NULL && sSummons[i].actor->update == NULL) {
            sSummons[i].actor = NULL;
        }
    }
}

void CaneSummon_KillAll(PlayState* play) {
    for (u8 i = 0; i < SOMARIA_MAX_CUBES; i++) {
        if (sSummons[i].actor != NULL && sSummons[i].actor->update != NULL) {
            Actor_Kill(sSummons[i].actor);
        }
        sSummons[i].actor = NULL;
    }
    sSummonSeq = 0;
}

// Claim a slot for `kind`. Only that kind's own budget is consulted: going over it
// evicts the oldest summon OF THAT KIND, so stacking statues never costs you the
// block you carefully placed on a switch.
static s8 CaneSummon_TakeSlot(PlayState* play, CaneSummonKind kind) {
    u8 live = 0;
    s8 oldest = -1;
    u16 oldestSeq = 0xFFFF;
    s8 free = -1;

    CaneSummon_CleanupPool();

    for (u8 i = 0; i < SOMARIA_MAX_CUBES; i++) {
        if (sSummons[i].actor == NULL) {
            if (free < 0) {
                free = (s8)i;
            }
            continue;
        }
        if (sSummons[i].kind != (u8)kind) {
            continue;
        }
        live++;
        if (sSummons[i].seq < oldestSeq) {
            oldestSeq = sSummons[i].seq;
            oldest = (s8)i;
        }
    }

    if ((live >= CaneSummon_CapFor(kind)) && (oldest >= 0)) {
        if (sSummons[oldest].actor->update != NULL) {
            SomariaCube_PlaySound(sSummons[oldest].actor, NA_SE_EV_BLOCK_BOUND);
            Actor_Kill(sSummons[oldest].actor);
        }
        sSummons[oldest].actor = NULL;
        return oldest;
    }

    return free; // -1 only if the pool is somehow entirely full of other kinds
}

// ============================================================================
// PLACEMENT VALIDITY
// ============================================================================

// Half-extents of each summon kind, for the clearance tests.
static f32 CaneSummon_Radius(CaneSummonKind kind) {
    switch (kind) {
        case CANE_SUMMON_BLOCK:
            return CANE_BLOCK_HALF_WIDTH;
        case CANE_SUMMON_PLATFORM:
            return CANE_PLATFORM_RADIUS;
        case CANE_SUMMON_STATUE:
        default:
            return 25.0f;
    }
}

static f32 CaneSummon_Height(CaneSummonKind kind) {
    switch (kind) {
        case CANE_SUMMON_BLOCK:
            return CANE_BLOCK_HEIGHT;
        case CANE_SUMMON_PLATFORM:
            return CANE_PLATFORM_HEIGHT;
        case CANE_SUMMON_STATUE:
        default:
            return 60.0f;
    }
}

u8 CaneSummon_PlacementValid(PlayState* play, CaneSummonKind kind, Vec3f* pos) {
    Player* player = GET_PLAYER(play);
    f32 radius = CaneSummon_Radius(kind);
    f32 height = CaneSummon_Height(kind);

    if (player == NULL) {
        return 0;
    }

    // The platform goes ANYWHERE (user-locked): no ground needed, no clearance
    // needed, and geometry in the way is not a reason to refuse. Its whole purpose
    // is reaching places the level does not offer a floor for, and half-embedding
    // it in a wall to make a ledge is a legitimate use rather than a mistake.
    if (kind == CANE_SUMMON_PLATFORM) {
        return 1;
    }

    // Never place inside Link himself — he would be pushed through the floor by the
    // block's dynapoly the instant it appears.
    f32 dx = pos->x - player->actor.world.pos.x;
    f32 dz = pos->z - player->actor.world.pos.z;
    f32 dy = pos->y - player->actor.world.pos.y;
    if (((dx * dx) + (dz * dz)) < ((radius + 22.0f) * (radius + 22.0f)) && (dy > -height) && (dy < height)) {
        return 0;
    }

    // Not overlapping an existing summon.
    for (u8 i = 0; i < SOMARIA_MAX_CUBES; i++) {
        Actor* other = sSummons[i].actor;
        if (other == NULL || other->update == NULL) {
            continue;
        }
        f32 odx = pos->x - other->world.pos.x;
        f32 odz = pos->z - other->world.pos.z;
        f32 ody = pos->y - other->world.pos.y;
        f32 minDist = radius + CaneSummon_Radius((CaneSummonKind)sSummons[i].kind);
        if (((odx * odx) + (odz * odz)) < (minDist * minDist) && (ody > -height) && (ody < height)) {
            return 0;
        }
    }

    // The spot must not be embedded in a wall: a short line test from Link's chest
    // to the target catches placement through geometry (across a fence, inside a
    // pillar). Placement into open air IS legal for the platform — that is the
    // whole point of a floating platform — so only the block/statue demand ground,
    // which the caller has already resolved by raycasting the floor.
    Vec3f from = player->actor.world.pos;
    Vec3f to = *pos;
    Vec3f hit;
    CollisionPoly* poly = NULL;
    s32 bgId = BGCHECK_SCENE;

    from.y += 40.0f;
    to.y += (height * 0.5f);
    if (BgCheck_EntityLineTest1(&play->colCtx, &from, &to, &hit, &poly, true, false, false, true, &bgId)) {
        return 0;
    }

    return 1;
}

// ============================================================================
// PLACEMENT PREVIEW
// ============================================================================

// A self-contained unit cube (+-1 on every axis), so the preview needs no asset
// from any object bank. Scaled per summon kind at draw time.
static Vtx sPreviewCubeVtx[] = {
    VTX(-1, -1, -1, 0, 0, 0, 0, 0, 255), VTX(1, -1, -1, 0, 0, 0, 0, 0, 255),
    VTX(1, -1, 1, 0, 0, 0, 0, 0, 255),   VTX(-1, -1, 1, 0, 0, 0, 0, 0, 255),
    VTX(-1, 1, -1, 0, 0, 0, 0, 0, 255),  VTX(1, 1, -1, 0, 0, 0, 0, 0, 255),
    VTX(1, 1, 1, 0, 0, 0, 0, 0, 255),    VTX(-1, 1, 1, 0, 0, 0, 0, 0, 255),
};

static Gfx sPreviewCubeDL[] = {
    gsSPVertex(sPreviewCubeVtx, 8, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0), // bottom
    gsSP2Triangles(4, 6, 5, 0, 4, 7, 6, 0), // top
    gsSP2Triangles(0, 5, 1, 0, 0, 4, 5, 0), // -Z
    gsSP2Triangles(1, 6, 2, 0, 1, 5, 6, 0), // +X
    gsSP2Triangles(2, 7, 3, 0, 2, 6, 7, 0), // +Z
    gsSP2Triangles(3, 4, 0, 0, 3, 7, 4, 0), // -X
    gsSPEndDisplayList(),
};

void CaneSummon_DrawPreview(PlayState* play, CaneSummonKind kind, Vec3f* pos, s16 yaw, u8 valid) {
    f32 radius = CaneSummon_Radius(kind);
    f32 height = CaneSummon_Height(kind);
    // Gentle breathing pulse so the ghost never reads as a real placed object.
    f32 pulse = 0.94f + (0.06f * Math_SinS((s16)(play->gameplayFrames * 1500)));

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);

    Matrix_Translate(pos->x, pos->y + (height * 0.5f), pos->z, MTXMODE_NEW);
    Matrix_RotateYS(yaw, MTXMODE_APPLY);
    Matrix_Scale(radius * pulse, (height * 0.5f) * pulse, radius * pulse, MTXMODE_APPLY);

    if (valid) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 90, 170, 255, 110);
        gDPSetEnvColor(POLY_XLU_DISP++, 20, 60, 180, 110);
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 70, 70, 110);
        gDPSetEnvColor(POLY_XLU_DISP++, 150, 0, 0, 110);
    }
    gDPSetCombineLERP(POLY_XLU_DISP++, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE);
    gSPClearGeometryMode(POLY_XLU_DISP++, G_LIGHTING | G_CULL_BACK);

    gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx, __FILE__, __LINE__),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, sPreviewCubeDL);

    CLOSE_DISPS(play->state.gfxCtx);
}

// ============================================================================
// SPAWNERS
// ============================================================================

// The Elegy shell. GAMEPLAY_KEEP is always resident, so this spawns anywhere.
static Actor* CaneSummon_SpawnStatue(PlayState* play, Vec3f* pos, s16 yaw) {
    Actor* statue =
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_TORCH2, pos->x, pos->y, pos->z, 0, yaw, 0, CaneSummon_CurrentForm());

    if (statue == NULL) {
        return NULL;
    }
    // Vanilla shells belong to the room that summoned them; a cane summon should
    // survive the player walking through a door.
    statue->room = -1;
    return statue;
}

// The real pushable block. Its object (GAMEPLAY_DANGEON_KEEP) is only resident in
// dungeons, so outside one we request it and let the caller retry next press —
// the FleetWarpDoor.cpp idiom.
static Actor* CaneSummon_SpawnBlock(PlayState* play, Vec3f* pos, s16 yaw) {
    if (Object_GetSlot(&play->objectCtx, GAMEPLAY_DANGEON_KEEP) < 0) {
        Object_SpawnPersistent(&play->objectCtx, GAMEPLAY_DANGEON_KEEP);
        return NULL; // not resident yet this frame
    }

    Actor* block = Actor_Spawn(&play->actorCtx, play, ACTOR_OBJ_OSHIHIKI, pos->x, pos->y, pos->z, 0, yaw, 0,
                               CANE_BLOCK_PARAMS);
    if (block == NULL) {
        return NULL;
    }
    block->room = -1;
    return block;
}

// The floating platform. Bg_Icefloe was the first pick, but it is a lumpy ice
// floe rather than a platform; Obj_Lift IS one — a square slab with a real texture
// (gDampeGraveBrownElevatorDL) over gDampeGraveBrownElevatorCol dynapoly. SoH uses
// the same actor for the same job (its OoT variant is the collapsing platform).
//
// Two things have to be neutralised for it to serve as a SUMMON:
//
//   1. It is an elevator — it moves. Replacing `update` with a no-op parks it
//      permanently; the dynapoly stays registered for as long as the actor lives
//      and the bg system reads its transform straight off the actor.
//
//   2. ObjLift_Init kills itself when the switch flag in its params is already
//      set. Unlike SoH's variant, MM's kills BEFORE DynaPolyActor_LoadMesh, so a
//      killed platform would have no collision at all and could not be rescued
//      after the fact. The guard is `unk_178 <= 0`, and unk_178 comes from
//      home.rot.z — so spawning with rotZ = 1 skips the flag check outright and
//      the whole of Init runs, scale included.
#define CANE_PLATFORM_SPAWN_ROT_Z 1 // ObjLift_Init: unk_178 > 0 skips the switch-flag kill

static void CaneSummon_PlatformUpdate(Actor* thisx, PlayState* play) {
    // Deliberately empty: a summoned platform neither moves nor collapses.
}

static void CaneSummon_PlatformDraw(Actor* thisx, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);
    // Keep the slab's own texture, push it red so it still reads as a Somaria
    // construct. Components spelled out — a multi-value #define does not survive
    // MSVC's function-like macro expansion.
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetEnvColor(POLY_OPA_DISP++, 210, 70, 70, 255);
    CLOSE_DISPS(play->state.gfxCtx);

    Gfx_DrawDListOpa(play, gDampeGraveBrownElevatorDL);
}

static Actor* CaneSummon_SpawnPlatform(PlayState* play, Vec3f* pos, s16 yaw) {
    if (Object_GetSlot(&play->objectCtx, OBJECT_D_LIFT) < 0) {
        Object_SpawnPersistent(&play->objectCtx, OBJECT_D_LIFT);
        return NULL; // not resident yet this frame
    }

    Actor* plat = Actor_Spawn(&play->actorCtx, play, ACTOR_OBJ_LIFT, pos->x, pos->y, pos->z, 0, yaw,
                              CANE_PLATFORM_SPAWN_ROT_Z, 0);
    if (plat == NULL) {
        return NULL;
    }

    plat->update = CaneSummon_PlatformUpdate;
    plat->draw = CaneSummon_PlatformDraw;
    plat->room = -1;
    return plat;
}

// ============================================================================
// SPAWN
// ============================================================================

Actor* CaneSummon_Spawn(PlayState* play, CaneSummonKind kind, Vec3f* pos, s16 yaw) {
    Actor* summon = NULL;

    switch (kind) {
        case CANE_SUMMON_STATUE:
            summon = CaneSummon_SpawnStatue(play, pos, yaw);
            break;
        case CANE_SUMMON_BLOCK:
            summon = CaneSummon_SpawnBlock(play, pos, yaw);
            break;
        case CANE_SUMMON_PLATFORM:
            summon = CaneSummon_SpawnPlatform(play, pos, yaw);
            break;
        default:
            return NULL;
    }

    if (summon == NULL) {
        return NULL;
    }

    // Only take a pool slot once the actor really exists — the object-not-resident
    // path above returns NULL and must not evict a live summon for nothing.
    s8 slot = CaneSummon_TakeSlot(play, kind);
    if (slot < 0) {
        Actor_Kill(summon);
        return NULL;
    }
    sSummons[slot].actor = summon;
    sSummons[slot].kind = (u8)kind;
    sSummons[slot].seq = sSummonSeq++;

    // NA_SE_PL_MAGIC_SOUL_NORMAL was the sustained soul-magic LOOP, so it started
    // and never stopped. _BALL is the one-shot burst of the same magic.
    SomariaCube_PlaySound(summon, NA_SE_PL_MAGIC_SOUL_BALL);
    return summon;
}
