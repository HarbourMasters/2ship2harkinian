#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include <math.h>

extern "C" {
#include "variables.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
void func_80834D50(PlayState* play, Player* player, PlayerAnimationHeader* anim, f32 speed, u16 sfxId);
s32 func_808373F8(PlayState* play, Player* player, u16 sfxId);
}

#define CVAR_NAME "gEnhancements.Player.ChainJump"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define LANDING_VELOCITY_THRESHOLD -10.0f
#define CHAIN_JUMP_WINDOW 4
#define JUMP2_MULTIPLIER 1.25f
#define JUMP3_MULTIPLIER 1.75f

// Check that Link is in a state where A would normally do a roll (no interactions available)
static bool CanJump(Player* player, PlayState* play, bool skipSpeedCheck = false) {
    // Has something to interact with — don't jump
    if (player->talkActor != NULL)
        return false;
    if (player->interactRangeActor != NULL)
        return false;

    // Z-targeting or Z held — don't interfere with jump slash / backflip / sidehop / roll
    if (player->stateFlags1 & PLAYER_STATE1_100000)
        return false;
    if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->cur.button, BTN_Z))
        return false;

    // Cutscene, action lock, ledge grab/climb, carrying, charging, putaway, swimming, talking
    if (player->stateFlags1 &
        (PLAYER_STATE1_1 | PLAYER_STATE1_4 | PLAYER_STATE1_4000 | PLAYER_STATE1_40000 | PLAYER_STATE1_20000000 |
         PLAYER_STATE1_CARRYING_ACTOR | PLAYER_STATE1_CHARGING_SPIN_ATTACK | PLAYER_STATE1_FRIENDLY_ACTOR_FOCUS |
         PLAYER_STATE1_8000000 | PLAYER_STATE1_200 | PLAYER_STATE1_400 | PLAYER_STATE1_8))
        return false;

    // Holding actor (grab)
    if (player->heldActor != NULL)
        return false;

    // Indoor rooms don't allow rolls
    if (play->roomCtx.curRoom.type == ROOM_TYPE_INDOORS)
        return false;

    // Must be on the ground
    if (!(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND))
        return false;

    // Must have some speed — below this, A does a normal roll instead
    if (!skipSpeedCheck && player->speedXZ < 1.0f)
        return false;

    return true;
}

static f32 sLaunchVelocityY = 0.0f; // velocity.y on the frame Link left the ground from jump 1
static f32 sPrevVelocityY = 0.0f;
static f32 sLandingSpeedXZ = 0.0f;
static f32 sPrevSpeedXZ = 0.0f;
static s32 sChainJumpTimer = 0;
static s32 sChainJumpCount = 0;
static bool sWasOnGround = true;

void RegisterChainJump() {
    COND_HOOK(OnActorUpdate, CVAR, [](Actor* actor) {
        if (actor->id != ACTOR_PLAYER || gPlayState == NULL) {
            return;
        }

        Player* player = (Player*)actor;
        Input* input = CONTROLLER1(&gPlayState->state);

        bool onGround = player->actor.bgCheckFlags & BGCHECKFLAG_GROUND;

        // Capture launch velocity on the frame Link leaves the ground after jump 1
        if (sWasOnGround && !onGround && sChainJumpCount == 1) {
            sLaunchVelocityY = player->actor.velocity.y;
        }

        // Just landed this frame
        if (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
            sLandingSpeedXZ = sPrevSpeedXZ;

            if (sPrevVelocityY > LANDING_VELOCITY_THRESHOLD) {
                sChainJumpTimer = CHAIN_JUMP_WINDOW;
                if (sChainJumpCount >= 3) {
                    sChainJumpCount = 0;
                }
            } else {
                sChainJumpCount = 0;
                sChainJumpTimer = 0;
            }
        }

        sWasOnGround = onGround;
        sPrevVelocityY = player->actor.velocity.y;
        sPrevSpeedXZ = player->speedXZ;

        // Manual jump from ground (no chain state needed)
        if (sChainJumpTimer <= 0 && CanJump(player, gPlayState) && CHECK_BTN_ALL(input->press.button, BTN_A)) {

            func_808373F8(gPlayState, player, NA_SE_VO_LI_AUTO_JUMP);
            sChainJumpCount = 1;
            return;
        }

        // Chain jumps on landing (skip speed check — we restore speed ourselves)
        if (sChainJumpTimer > 0) {
            if (CHECK_BTN_ALL(input->press.button, BTN_A) && CanJump(player, gPlayState, true)) {

                sChainJumpCount++;

                f32 multiplier;
                PlayerAnimationHeader* anim;

                if (sChainJumpCount >= 3) {
                    multiplier = JUMP3_MULTIPLIER;
                    anim = (PlayerAnimationHeader*)&gPlayerAnim_link_normal_newroll_jump_20f;
                } else {
                    multiplier = JUMP2_MULTIPLIER;
                    anim = (PlayerAnimationHeader*)&gPlayerAnim_link_normal_jump;
                }

                f32 bounceSpeed = fabsf(sLaunchVelocityY) * multiplier;

                player->speedXZ = sLandingSpeedXZ;
                func_80834D50(gPlayState, player, anim, bounceSpeed, NA_SE_VO_LI_AUTO_JUMP);

                sChainJumpTimer = 0;
            } else {
                sChainJumpTimer--;
                if (sChainJumpTimer <= 0) {
                    sChainJumpCount = 0;
                }
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterChainJump, { CVAR_NAME });
