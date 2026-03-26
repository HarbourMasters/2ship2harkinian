#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "z64player.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
void Player_Anim_PlayOnceAdjusted(PlayState* play, Player* player, PlayerAnimationHeader* anim);
void Player_AnimSfx_PlayVoice(Player* player, u16 sfxId);
}

#define CVAR_NAME "gEnhancements.Player.AerialSpinLift"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define LIFT_DURATION 24 // 6 frames of lift + 18 frames animation

static bool sLiftUsedThisAirtime = false;
static bool sLiftActive = false;
static s32 sLiftFramesLeft = 0;
static f32 sSpinFrame = 0.0f;

static f32 GetLiftVelocity(Player* player) {
    switch (player->heldItemAction) {
        case PLAYER_IA_SWORD_TWO_HANDED:
            return 4.4f;
        case PLAYER_IA_SWORD_GILDED:
            return 4.0f;
        case PLAYER_IA_SWORD_RAZOR:
            return 3.5f;
        case PLAYER_IA_SWORD_KOKIRI:
            return 2.8f;
        default:
            return 2.8f;
    }
}

void RegisterAerialSpinLift() {
    // Eat B press while airborne (no Z) and trigger our custom spin lift
    COND_HOOK(OnPassPlayerInputs, CVAR, [](Input* input) {
        if (gPlayState == NULL) {
            return;
        }
        Player* player = GET_PLAYER(gPlayState);
        bool airborne = !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND);

        if (airborne && !sLiftUsedThisAirtime && !sLiftActive && !CHECK_BTN_ALL(input->cur.button, BTN_Z) &&
            CHECK_BTN_ALL(input->press.button, BTN_B) &&
            (player->transformation == PLAYER_FORM_HUMAN || player->transformation == PLAYER_FORM_FIERCE_DEITY) &&
            Player_GetMeleeWeaponHeld(player) != PLAYER_MELEEWEAPON_NONE) {
            // Eat the B press
            input->press.button &= ~BTN_B;
            input->cur.button &= ~BTN_B;

            // Start the spin lift
            sLiftActive = true;
            sLiftFramesLeft = LIFT_DURATION;

            // Play spin animation
            PlayerAnimationHeader* spinAnim = Player_IsHoldingTwoHandedWeapon(player)
                                                  ? (PlayerAnimationHeader*)&gPlayerAnim_link_fighter_Lrolling_kiru
                                                  : (PlayerAnimationHeader*)&gPlayerAnim_link_fighter_rolling_kiru;
            Player_Anim_PlayOnceAdjusted(gPlayState, player, spinAnim);

            // Play spin attack voice (same as ground spin release)
            Player_AnimSfx_PlayVoice(player, NA_SE_VO_LI_SWORD_L);

            // Set spin attack state flag so EN_M_THUNDER renders properly
            player->stateFlags1 |= PLAYER_STATE1_CHARGING_SPIN_ATTACK;

            // Spawn magic spin effect (visual only, no magic consumed)
            Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_M_THUNDER,
                        player->bodyPartsPos[PLAYER_BODYPART_WAIST].x, player->bodyPartsPos[PLAYER_BODYPART_WAIST].y,
                        player->bodyPartsPos[PLAYER_BODYPART_WAIST].z, 0, 0, 0,
                        (player->heldItemAction - PLAYER_IA_SWORD_KOKIRI) | (2 << 8));

            // Activate sword hitbox
            player->meleeWeaponState = PLAYER_MELEE_WEAPON_STATE_1;
            player->meleeWeaponInfo[0].active = true;
            player->meleeWeaponInfo[1].active = true;
            player->meleeWeaponInfo[2].active = true;
        }

        // Block further B presses after lift is used
        if (airborne && sLiftUsedThisAirtime && !CHECK_BTN_ALL(input->cur.button, BTN_Z)) {
            input->press.button &= ~BTN_B;
            input->cur.button &= ~BTN_B;
        }
    });

    COND_HOOK(OnActorUpdate, CVAR, [](Actor* actor) {
        if (actor->id != ACTOR_PLAYER || gPlayState == NULL) {
            return;
        }

        Player* player = (Player*)actor;
        bool onGround = player->actor.bgCheckFlags & BGCHECKFLAG_GROUND;

        if (onGround) {
            if (sLiftActive || sLiftUsedThisAirtime) {
                // Clean up hitbox and spin state on landing
                player->meleeWeaponState = PLAYER_MELEE_WEAPON_STATE_0;
                player->meleeWeaponInfo[0].active = false;
                player->meleeWeaponInfo[1].active = false;
                player->meleeWeaponInfo[2].active = false;
                player->stateFlags1 &= ~PLAYER_STATE1_CHARGING_SPIN_ATTACK;
            }
            sLiftUsedThisAirtime = false;
            sLiftActive = false;
            sLiftFramesLeft = 0;
            sSpinFrame = 0.0f;
            return;
        }

        if (sLiftActive && sLiftFramesLeft > 0) {
            // Apply lift velocity for first 3 frames
            if (sLiftFramesLeft > 21) {
                player->actor.velocity.y = GetLiftVelocity(player);
            }

            // Force the spin animation at the current frame each tick
            PlayerAnimationHeader* spinAnim = Player_IsHoldingTwoHandedWeapon(player)
                                                  ? (PlayerAnimationHeader*)&gPlayerAnim_link_fighter_Lrolling_kiru
                                                  : (PlayerAnimationHeader*)&gPlayerAnim_link_fighter_rolling_kiru;
            f32 lastFrame = Animation_GetLastFrame(spinAnim);

            if (sSpinFrame < lastFrame) {
                // Still playing the spin
                PlayerAnimation_Change(gPlayState, &player->skelAnime, spinAnim, 1.0f, sSpinFrame, lastFrame,
                                       ANIMMODE_ONCE, 0.0f);
                sSpinFrame += 1.0f;
            } else {
                // Spin animation finished — go to falling pose
                PlayerAnimation_PlayOnce(gPlayState, &player->skelAnime,
                                         (PlayerAnimationHeader*)&gPlayerAnim_link_normal_jump);
                sLiftActive = false;
                sLiftUsedThisAirtime = true;
                sSpinFrame = 0.0f;
                player->meleeWeaponState = PLAYER_MELEE_WEAPON_STATE_0;
                player->meleeWeaponInfo[0].active = false;
                player->meleeWeaponInfo[1].active = false;
                player->meleeWeaponInfo[2].active = false;
                player->stateFlags1 &= ~PLAYER_STATE1_CHARGING_SPIN_ATTACK;
            }

            sLiftFramesLeft--;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterAerialSpinLift, { CVAR_NAME });
