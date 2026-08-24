/**
 * remains_ally_common.c - Shared behavior helpers for the boss-remains summon allies.
 *
 * See remains_ally_common.h. This file is unity-#included by boss_remains.cpp inside
 * its extern "C" block (so the symbols get C linkage), NOT compiled standalone and NOT
 * added to CMake/vcxproj. It therefore compiles as C++; the code below stays in the
 * common C/C++ subset (no designated initializers, explicit casts where a conversion
 * would be implicit in C).
 *
 * Only verified MM engine symbols are used (see the header comment for the list). The
 * two motion helpers drive the same ground-movement chain a normal walking enemy uses:
 *   steer world.rot.y (Math_SmoothStepToS) -> set speed -> Actor_MoveWithGravity
 *   -> Actor_UpdateBgCheckInfo (wall + floor/water).
 * world.rot.y is deliberately the yaw that is steered because Actor_MoveWithGravity /
 * Actor_UpdateVelocityWithGravity derive velocity from world.rot.y (z_actor.c:1330).
 */

#include "remains_ally_common.h"

// ---- Tuning shared by both motion helpers ----------------------------------
// Turn rate for Math_SmoothStepToS(&world.rot.y, ...): scale 4 = ease toward the
// target, capped at ~0x1000 (~22.5 deg) per frame, minimum 0x100 so it never stalls.
#define REMAINS_ALLY_YAW_SCALE 4
#define REMAINS_ALLY_YAW_MAXSTEP 0x1000
#define REMAINS_ALLY_YAW_MINSTEP 0x100

// Ground bg-check box for a small ally. Wall + floor/water only (no ceiling check,
// so ceilingCheckHeight is 0). These are modest, ally-sized values.
#define REMAINS_ALLY_WALL_HEIGHT 26.0f
#define REMAINS_ALLY_WALL_RADIUS 10.0f
#define REMAINS_ALLY_BGCHECK_FLAGS (UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4)

// Enemy-bearing actor categories an ally treats as attack targets. Allies are
// ACTORCAT_MISC, so they never appear in these lists and can't target each other.
static const u8 sRemainsAllyEnemyCats[2] = { ACTORCAT_ENEMY, ACTORCAT_BOSS };

Actor* RemainsAlly_FindNearestEnemy(PlayState* play, Vec3f* from, f32 maxDist) {
    Actor* nearest = NULL;
    f32 nearestDist;
    s32 i;

    if ((play == NULL) || (from == NULL)) {
        return NULL;
    }

    nearestDist = maxDist;

    for (i = 0; i < (s32)(sizeof(sRemainsAllyEnemyCats) / sizeof(sRemainsAllyEnemyCats[0])); i++) {
        Actor* actor = play->actorCtx.actorLists[sRemainsAllyEnemyCats[i]].first;

        while (actor != NULL) {
            // Skip anything mid-kill (Actor_Kill nulls update, z_actor.c:1225) or
            // already dead (health is u8, so == 0 is the health<=0 case).
            if ((actor->update != NULL) && (actor->colChkInfo.health != 0)) {
                f32 dist = Actor_WorldDistXZToPoint(actor, from);

                if (dist <= nearestDist) {
                    nearestDist = dist;
                    nearest = actor;
                }
            }
            actor = actor->next;
        }
    }

    return nearest;
}

s16 RemainsAlly_HomeTowardPos(PlayState* play, Actor* actor, Vec3f* targetPos, f32 speed) {
    s16 targetYaw;

    if ((play == NULL) || (actor == NULL) || (targetPos == NULL)) {
        return 0;
    }

    targetYaw = Actor_WorldYawTowardPoint(actor, targetPos);

    // Steer the movement yaw (world.rot.y drives Actor_MoveWithGravity) toward the
    // target, then face the model the way it is moving.
    Math_SmoothStepToS(&actor->world.rot.y, targetYaw, REMAINS_ALLY_YAW_SCALE, REMAINS_ALLY_YAW_MAXSTEP,
                       REMAINS_ALLY_YAW_MINSTEP);
    actor->shape.rot.y = actor->world.rot.y;

    actor->speed = speed;
    Actor_MoveWithGravity(actor);
    Actor_UpdateBgCheckInfo(play, actor, REMAINS_ALLY_WALL_HEIGHT, REMAINS_ALLY_WALL_RADIUS, 0.0f,
                            REMAINS_ALLY_BGCHECK_FLAGS);

    return (s16)(targetYaw - actor->world.rot.y);
}

void RemainsAlly_FollowPlayer(PlayState* play, Actor* actor, f32 followDist, f32 speed) {
    Player* player;
    f32 dist;

    if ((play == NULL) || (actor == NULL)) {
        return;
    }

    player = GET_PLAYER(play);
    if (player == NULL) {
        return;
    }

    dist = Actor_WorldDistXZToActor(actor, &player->actor);

    if (dist > followDist) {
        // Too far away: home in on the player's world position at the given speed.
        RemainsAlly_HomeTowardPos(play, actor, &player->actor.world.pos, speed);
    } else {
        // Close enough: face the player, stop, and let gravity settle it to the floor.
        Math_SmoothStepToS(&actor->world.rot.y, Actor_WorldYawTowardActor(actor, &player->actor),
                           REMAINS_ALLY_YAW_SCALE, REMAINS_ALLY_YAW_MAXSTEP, REMAINS_ALLY_YAW_MINSTEP);
        actor->shape.rot.y = actor->world.rot.y;

        actor->speed = 0.0f;
        Actor_MoveWithGravity(actor);
        Actor_UpdateBgCheckInfo(play, actor, REMAINS_ALLY_WALL_HEIGHT, REMAINS_ALLY_WALL_RADIUS, 0.0f,
                                REMAINS_ALLY_BGCHECK_FLAGS);
    }
}
