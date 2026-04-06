#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include <math.h>

extern "C" {
#include "variables.h"
#include "z64player.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
void func_80834D50(PlayState* play, Player* player, PlayerAnimationHeader* anim, f32 speed, u16 sfxId);
s32 func_808373F8(PlayState* play, Player* player, u16 sfxId);
bool Player_IsHoldingHookshot(Player* player);
void func_808395F0(PlayState* play, Player* player, PlayerMeleeWeaponAnimation meleeWeaponAnim, f32 linearVelocity,
                   f32 yVelocity);
s32 Player_GetMovementSpeedAndYaw(Player* player, f32* outSpeedTarget, s16* outYawTarget, f32 speedMode,
                                  PlayState* play);
s32 func_8083CBC4(Player* player, f32 arg1, s16 arg2, f32 arg3, f32 arg4, f32 arg5, s16 arg6);
void Player_Action_29(Player* player, PlayState* play);
}

#define CVAR_NAME "gEnhancements.Player.PlatformerMode"
#define CVAR CVarGetInteger(CVAR_NAME, 0)
#define CVAR_DEKU_JUMP_NAME "gEnhancements.Player.PlatformerMode.DekuJump"
#define CVAR_DEKU_JUMP CVarGetInteger(CVAR_DEKU_JUMP_NAME, 0)

// ===== Helpers =====

static PlayerAnimationHeader* GetSpinAnim(Player* player) {
    return Player_IsHoldingTwoHandedWeapon(player) ? (PlayerAnimationHeader*)&gPlayerAnim_link_fighter_Lrolling_kiru
                                                   : (PlayerAnimationHeader*)&gPlayerAnim_link_fighter_rolling_kiru;
}

static void ClearLiftState(Player* player) {
    player->meleeWeaponState = PLAYER_MELEE_WEAPON_STATE_0;
    player->meleeWeaponInfo[0].active = false;
    player->meleeWeaponInfo[1].active = false;
    player->meleeWeaponInfo[2].active = false;
}

static bool IsSpinLiftEligible(Player* player) {
    if (player->transformation != PLAYER_FORM_HUMAN && player->transformation != PLAYER_FORM_FIERCE_DEITY) {
        return false;
    }
    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_NONE) {
        return false;
    }
    // Only allow spin when Link's B button is an actual sword (not deku stick, bow, etc.)
    if (player->heldItemAction < PLAYER_IA_SWORD_KOKIRI || player->heldItemAction > PLAYER_IA_SWORD_TWO_HANDED) {
        return false;
    }
    return true;
}

// ===== Chain Jump =====

// 3 frame window to jump again (does allow a cool 1 frame turn for jumps 2 & 3)
#define CHAIN_JUMP_WINDOW 3
#define JUMP2_MULTIPLIER 1.25f
#define JUMP3_MULTIPLIER 1.75f

static bool CanJump(Player* player, PlayState* play, bool skipSpeedCheck = false) {
    // Deku can only chain jump if the sub-option is enabled
    if (player->transformation == PLAYER_FORM_DEKU && !CVAR_DEKU_JUMP) {
        return false;
    }
    // No jumping while talking to people or things, otherwise you get free WWT
    if (player->talkActor != NULL || player->interactRangeActor != NULL) {
        return false;
    }
    if (player->stateFlags1 & (PLAYER_STATE1_1 |                      // Scene/door transition
                               PLAYER_STATE1_4 |                      // Climbing ledge
                               PLAYER_STATE1_200 |                    // Blocking cutscene (pictograph/scene transition)
                               PLAYER_STATE1_400 |                    // Getting item (pickup animation)
                               PLAYER_STATE1_4000 |                   // Climbing (hold transition)
                               PLAYER_STATE1_40000 |                  // Jumping/airborne
                               PLAYER_STATE1_100000 |                 // Unknown blocking state
                               PLAYER_STATE1_8000000 |                // Swimming
                               PLAYER_STATE1_20000000 |               // Time stopped, animations continue
                               PLAYER_STATE1_CHARGING_SPIN_ATTACK |   // Charging spin attack (holding B)
                               PLAYER_STATE1_FRIENDLY_ACTOR_FOCUS)) { // Focusing on a friendly actor
        return false;
    }
    // Can't jump while carrying things, but hookshot in hand is fine
    if (player->heldActor != NULL && !Player_IsHoldingHookshot(player)) {
        return false;
    }
    // Only allow jumping when A shows Attack/Curl/None — block any contextual
    // action (Down, Grab, Climb, etc.) as an extra layer against glitch setups.
    u16 doAction = play->interfaceCtx.aButtonDoAction;
    if (doAction != DO_ACTION_ATTACK && doAction != DO_ACTION_NONE && doAction != DO_ACTION_CURL) {
        return false;
    }
    // Link must be on the ground to jump (Crazy I know)
    if (!(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        return false;
    }
    // Block jumping while slashing — prevents ISG from jump-canceling a landing jump slash
    if (player->meleeWeaponState != PLAYER_MELEE_WEAPON_STATE_0) {
        return false;
    }
    // Block jumping at low speed or backwalking (but not HESS or Superslide-to-ESS)
    if (!skipSpeedCheck && player->speedXZ < 1.0f && player->speedXZ > -6.0f) {
        return false;
    }
    return true;
}

// ===== Shared state =====

// Chain jump
static f32 sLaunchVelocityY = 0.0f;
static f32 sPrevVelocityY = 0.0f;
static f32 sLandingSpeedXZ = 0.0f;
static f32 sPrevSpeedXZ = 0.0f;
static s32 sChainJumpTimer = 0;
static s32 sChainJumpCount = 0;
static bool sWasOnGround = true;

// Wall kick
//  3 frame window to kick off the wall
//  3+ XZ speed required
//  must be within 55 deg of the wall (bonus 5 each way because exact 45 is easy to hit)
//  5 frames before another wallkick is allowed
#define WALL_KICK_WINDOW 3
#define WALL_KICK_MIN_SPEED 3.0f
#define WALL_KICK_ANGLE_THRESHOLD 0x2555
#define WALL_KICK_AIRBORNE_DELAY 5

// Speed multiplier by reaction frame: 1st = 1.25x, 2nd = 1.1x, 3rd = 1.0x
static const f32 sWallKickSpeedMultipliers[WALL_KICK_WINDOW] = { 1.25f, 1.1f, 1.0f };
static s32 sWallKickTimer = 0;
static s32 sWallKickFrame = 0;
static f32 sWallKickSpeed = 0.0f;
static s16 sWallKickBounceYaw = 0;
static s32 sAirborneTimer = 0;

// Spin lift
// Counter for timing the initial velocity boost (first 3 frames)
#define LIFT_DURATION 24

static bool sLiftUsedThisAirtime = false;
static bool sLiftActive = false;
static s32 sLiftFramesLeft = 0;
static f32 sSpinFrame = 0.0f;
static bool sPlatformerLaunch = false;
static bool sBPressedWhileAirborne = false; // set by input hook, consumed by update hook

// Better sword = more lift. GFS/Helix gives the most, Kokiri the least.
static f32 GetLiftVelocity(Player* player) {
    switch (player->heldItemAction) {
        case PLAYER_IA_SWORD_TWO_HANDED:
            return 4.4f;
        case PLAYER_IA_SWORD_GILDED:
            return 4.0f;
        case PLAYER_IA_SWORD_RAZOR:
            return 3.5f;
        default:
            return 2.8f;
    }
}

// ===== Registration =====

void RegisterPlatformerMode() {

    // Input hook: captures B press and suppresses it so the vanilla jump slash
    // doesn't also fire. The actual spin lift logic runs in the actor update below.
    COND_HOOK(OnPassPlayerInputs, CVAR, [](Input* input) {
        if (gPlayState == NULL) {
            return;
        }

        Player* player = GET_PLAYER(gPlayState);
        bool airborne = !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND);

        if (!airborne || !IsSpinLiftEligible(player)) {
            return;
        }

        // While the spin lift is active or already used this jump, suppress B
        // (prevents vanilla jump slash) and item buttons (prevents pulling items
        // mid-spin — Player_UpdateUpperBody runs outside the action function so
        // Player_Action_29 alone can't block it).
        if (sLiftActive || sLiftUsedThisAirtime) {
            u16 itemButtons = BTN_B | BTN_CLEFT | BTN_CDOWN | BTN_CRIGHT;
            input->press.button &= ~itemButtons;
            input->cur.button &= ~itemButtons;
            return;
        }

        // Detect a fresh B press and pass it to the actor update via a flag.
        // We eat the input here so the game's own jump-slash code never sees it.
        if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
            bool holdingZ = CHECK_BTN_ALL(input->cur.button, BTN_Z);
            if (!holdingZ || sPlatformerLaunch) {
                sBPressedWhileAirborne = true;
                input->press.button &= ~BTN_B;
                input->cur.button &= ~BTN_B;
            }
        }
    });

    // Main update: chain jump, wall kick, and spin lift all live here so state
    // changes and animation happen at the same point in the frame.
    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, CVAR, [](Actor* actor) {
        Player* player = (Player*)actor;
        Input* input = CONTROLLER1(&gPlayState->state);
        bool onGround = player->actor.bgCheckFlags & BGCHECKFLAG_GROUND;

        // ---- Chain Jump tracking ----

        if (sWasOnGround && !onGround && sChainJumpCount == 1) {
            sLaunchVelocityY = player->actor.velocity.y;
        }

        if (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
            sLandingSpeedXZ = sPrevSpeedXZ;
            if (sPrevVelocityY > -10.0f) {
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

        // ---- Chain Jump execution ----

        // Jump 1:  holding Z preserves vanilla A actions like roll, backflip, sidehop, etc
        if (sChainJumpTimer <= 0 && CanJump(player, gPlayState) && !CHECK_BTN_ALL(input->cur.button, BTN_Z) &&
            CHECK_BTN_ALL(input->press.button, BTN_A)) {
            func_808373F8(gPlayState, player, NA_SE_VO_LI_AUTO_JUMP);
            sChainJumpCount = 1;
            sPlatformerLaunch = true;
            return;
        }

        // Jumps 2 & 3: Z is allowed since we're already in a chain
        if (sChainJumpTimer > 0) {
            if (CHECK_BTN_ALL(input->press.button, BTN_A) && CanJump(player, gPlayState, true)) {
                sChainJumpCount++;
                f32 mult = (sChainJumpCount >= 3) ? JUMP3_MULTIPLIER : JUMP2_MULTIPLIER;
                PlayerAnimationHeader* anim = (sChainJumpCount >= 3)
                                                  ? (PlayerAnimationHeader*)&gPlayerAnim_link_normal_newroll_jump_20f
                                                  : (PlayerAnimationHeader*)&gPlayerAnim_link_normal_jump;
                player->speedXZ = sLandingSpeedXZ;
                func_80834D50(gPlayState, player, anim, fabsf(sLaunchVelocityY) * mult, NA_SE_VO_LI_AUTO_JUMP);
                sChainJumpTimer = 0;
                sPlatformerLaunch = true;
            } else {
                sChainJumpTimer--;
                if (sChainJumpTimer <= 0) {
                    sChainJumpCount = 0;
                }
            }
        }

        // ---- Wall Kick ----

        if (onGround) {
            sWallKickTimer = 0;
            sAirborneTimer = 0;
        } else {
            sAirborneTimer++;

            if (sAirborneTimer >= WALL_KICK_AIRBORNE_DELAY && !CHECK_BTN_ALL(input->cur.button, BTN_Z)) {
                bool touchingWall = player->actor.bgCheckFlags & BGCHECKFLAG_PLAYER_WALL_INTERACT;

                if (touchingWall && sWallKickTimer <= 0 && player->actor.velocity.y >= -10.0f &&
                    player->speedXZ >= WALL_KICK_MIN_SPEED) {
                    s16 angleDiff = player->actor.shape.rot.y - player->actor.wallYaw;
                    if (ABS_ALT(ABS_ALT(angleDiff) - 0x8000) <= WALL_KICK_ANGLE_THRESHOLD) {
                        sWallKickTimer = WALL_KICK_WINDOW;
                        sWallKickFrame = 0;
                        sWallKickSpeed = player->speedXZ;
                        sWallKickBounceYaw = (2 * player->actor.wallYaw) + 0x8000 - player->actor.shape.rot.y;
                    }
                }

                if (sWallKickTimer > 0) {
                    if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
                        f32 kickSpeed = sWallKickSpeed * sWallKickSpeedMultipliers[sWallKickFrame];
                        player->actor.shape.rot.y = sWallKickBounceYaw;
                        player->yaw = sWallKickBounceYaw;
                        player->speedXZ = kickSpeed;
                        func_80834D50(gPlayState, player, (PlayerAnimationHeader*)&gPlayerAnim_link_normal_jump,
                                      kickSpeed, NA_SE_VO_LI_AUTO_JUMP);
                        sWallKickTimer = 0;
                        sChainJumpCount = 0;
                        sPlatformerLaunch = true;
                    } else {
                        sWallKickFrame++;
                        sWallKickTimer--;
                    }
                }
            }
        }

        // ---- Aerial Spin Lift ----

        // Reset everything on landing
        if (onGround) {
            if (sLiftActive) {
                ClearLiftState(player);
            }
            sLiftUsedThisAirtime = false;
            sLiftActive = false;
            sLiftFramesLeft = 0;
            sSpinFrame = 0.0f;
            sPlatformerLaunch = false;
            sBPressedWhileAirborne = false;
            return;
        }

        // Activate on B press (flag was set by the input hook this frame).
        // Safety: re-check eligibility in case form/sword changed between hooks.
        if (sBPressedWhileAirborne && IsSpinLiftEligible(player)) {
            sLiftActive = true;
            sLiftFramesLeft = LIFT_DURATION;
            // Save momentum — func_808395F0 stomps yaw and speedXZ
            s16 savedYaw = player->yaw;
            f32 savedSpeed = player->speedXZ;
            // Use the engine's jump slash setup — puts Link into Player_Action_29
            // which locks out items, ledge grabs, and all state transitions.
            func_808395F0(gPlayState, player, PLAYER_MWA_JUMPSLASH_START, savedSpeed, GetLiftVelocity(player));
            // Restore momentum so Link continues in his original direction
            player->yaw = savedYaw;
            player->speedXZ = savedSpeed;
        }
        sBPressedWhileAirborne = false;

        // If something interrupted the spin (damage, action change), bail out
        if (sLiftActive && player->actionFunc != Player_Action_29) {
            sLiftActive = false;
            sLiftUsedThisAirtime = true;
            ClearLiftState(player);
        }

        // Run spin animation while active
        if (sLiftActive) {
            // Air drift — Player_Action_29 doesn't process stick input,
            // so we apply the same air control that Player_Action_25 uses.
            f32 speedTarget;
            s16 yawTarget;
            Player_GetMovementSpeedAndYaw(player, &speedTarget, &yawTarget, 0.0f, gPlayState);
            func_8083CBC4(player, speedTarget, yawTarget, 1.0f, 0.05f, 0.1f, 0xC8);

            // Upward boost for the first 3 frames
            if (sLiftFramesLeft > LIFT_DURATION - 3) {
                player->actor.velocity.y = GetLiftVelocity(player);
            }

            // Advance or hold the spin animation
            PlayerAnimationHeader* spinAnim = GetSpinAnim(player);
            f32 lastFrame = Animation_GetLastFrame(spinAnim);

            if (sSpinFrame < lastFrame) {
                PlayerAnimation_Change(gPlayState, &player->skelAnime, spinAnim, 1.0f, sSpinFrame, lastFrame,
                                       ANIMMODE_ONCE, 0.0f);
                sSpinFrame += 1.0f;

                // Keep the weapon hitbox alive during the spin — our custom animation
                // doesn't trigger the engine's per-frame weapon activation events.
                player->meleeWeaponState = PLAYER_MELEE_WEAPON_STATE_1;
                player->meleeWeaponInfo[0].active = true;
                player->meleeWeaponInfo[1].active = true;
                player->meleeWeaponInfo[2].active = true;
            } else {
                // Spin finished — hold the last frame, deactivate hitbox
                PlayerAnimation_Change(gPlayState, &player->skelAnime, spinAnim, 0.0f, lastFrame, lastFrame,
                                       ANIMMODE_ONCE, 0.0f);
                ClearLiftState(player);
                sLiftUsedThisAirtime = true;
            }

            if (sLiftFramesLeft > 0) {
                sLiftFramesLeft--;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterPlatformerMode, { CVAR_NAME, CVAR_DEKU_JUMP_NAME });

// Known "bugs" that are staying because they are cool

// You can wall kick out of side hops, normal sidehops don't do much
// untargeted sidehops (release Z on the frame you press A)
// does more or less a mega sidehop which can be extended with a spin lift

// you can spin lift out of megaflips, mega sidehops and recoil flips by releasing Z
// this makes them even longer and might open a few new skips

// You can do a 1 frame turn around on jump 3 of chain and then recoil flip the direction
// you had been moving to get an absurdly large recoil distance from nothing

// Frame perfect jumps out of a recoil or mega let you carry that speed into a GIGANTIC 2nd jump
// that has so much negative y velocity when you hit the ground you can't triple out of it

// Spin lifting out of backflips and sidehops can be done by releasing Z after jumping
