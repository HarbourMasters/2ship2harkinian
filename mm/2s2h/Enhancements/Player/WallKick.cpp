#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include <math.h>

extern "C" {
#include "variables.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
void func_80834D50(PlayState* play, Player* player, PlayerAnimationHeader* anim, f32 speed, u16 sfxId);
}

#define CVAR_NAME "gEnhancements.Player.WallKick"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define WALL_KICK_WINDOW 3
#define WALL_KICK_MIN_SPEED 3.0f
#define WALL_KICK_ANGLE_THRESHOLD 0x2555 // ~55 degrees in binary angle (0x4000 = 90 deg)

// Speed multipliers per frame within the window (rewards fast reaction)
static const f32 sWallKickSpeedMultipliers[WALL_KICK_WINDOW] = { 1.25f, 1.1f, 1.0f };

static s32 sWallKickTimer = 0;
static s32 sWallKickFrame = 0;
static f32 sWallKickSpeed = 0.0f;
static s16 sWallKickBounceYaw = 0;
static s32 sAirborneTimer = 0;

#define WALL_KICK_AIRBORNE_DELAY 5

void RegisterWallKick() {
    COND_HOOK(OnActorUpdate, CVAR, [](Actor* actor) {
        if (actor->id != ACTOR_PLAYER || gPlayState == NULL) {
            return;
        }

        Player* player = (Player*)actor;
        Input* input = CONTROLLER1(&gPlayState->state);

        bool onGround = player->actor.bgCheckFlags & BGCHECKFLAG_GROUND;
        bool touchingWall = player->actor.bgCheckFlags & BGCHECKFLAG_PLAYER_WALL_INTERACT;

        if (onGround) {
            sWallKickTimer = 0;
            sAirborneTimer = 0;
            return;
        }

        sAirborneTimer++;

        if (sAirborneTimer < WALL_KICK_AIRBORNE_DELAY) {
            return;
        }

        if (CHECK_BTN_ALL(input->cur.button, BTN_Z)) {
            return;
        }

        // Detect wall contact while airborne
        if (touchingWall && sWallKickTimer <= 0) {
            f32 speed = player->speedXZ;

            if (player->actor.velocity.y < -10.0f) {
                return;
            }

            if (speed <= 0.0f || speed < WALL_KICK_MIN_SPEED) {
                return;
            }

            s16 wallNormal = player->actor.wallYaw;
            s16 playerYaw = player->actor.shape.rot.y;
            s16 angleDiff = playerYaw - wallNormal;
            s16 angleToWall = ABS_ALT(ABS_ALT(angleDiff) - 0x8000);

            if (angleToWall > WALL_KICK_ANGLE_THRESHOLD) {
                return;
            }

            sWallKickTimer = WALL_KICK_WINDOW;
            sWallKickFrame = 0;
            sWallKickSpeed = speed;
            sWallKickBounceYaw = (2 * wallNormal) + 0x8000 - playerYaw;
        }

        // Wall kick window active
        if (sWallKickTimer > 0) {
            if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
                f32 multiplier = sWallKickSpeedMultipliers[sWallKickFrame];
                f32 kickSpeed = sWallKickSpeed * multiplier;

                player->actor.shape.rot.y = sWallKickBounceYaw;
                player->yaw = sWallKickBounceYaw;
                player->speedXZ = kickSpeed;

                func_80834D50(gPlayState, player, (PlayerAnimationHeader*)&gPlayerAnim_link_normal_jump, kickSpeed,
                              NA_SE_VO_LI_AUTO_JUMP);

                sWallKickTimer = 0;
            } else {
                sWallKickFrame++;
                sWallKickTimer--;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterWallKick, { CVAR_NAME });
