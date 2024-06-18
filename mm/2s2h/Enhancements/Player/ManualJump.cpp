#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include <z64.h>
#include <macros.h>
#include <global.h>
#include <functions.h>
#include <z64player.h>
#include "objects/gameplay_keep/gameplay_keep.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
extern PlayerAnimationHeader* anim;
void func_80834D50(PlayState* play, Player* player, PlayerAnimationHeader* anim, f32 speed, u16 sfxId);
void func_80834DB8(Player* player, PlayerAnimationHeader* anim, f32 speed, PlayState* play);
}

void RegisterManualJump() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPassPlayerInputs>([](Input* input) {
        if (gPlayState == NULL) {
            return;
        }
        if (gSaveContext.save.playerForm != PLAYER_FORM_HUMAN) {
            return;
        }
        if (CVarGetInteger("gEnhancements.Player.ManualJump", 0)) {
            Player* player = GET_PLAYER(gPlayState);
            s8 temp = player->unk_AE3[player->unk_ADE];
            if (player->stateFlags1 & PLAYER_STATE1_8000 + PLAYER_STATE1_20000) {
                if (!(player->stateFlags1 & PLAYER_STATE1_8000000) &&
                    (Player_GetMeleeWeaponHeld(player) != PLAYER_MELEEWEAPON_NONE)) {
                    if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
                        if (temp == 0) {
                            func_80834D50(gPlayState, player,
                                          (PlayerAnimationHeader*)&gPlayerAnim_link_fighter_front_jump, 5.0f,
                                          NA_SE_VO_LI_SWORD_N);
                        } else if (temp == -1) {
                            func_80834DB8(player, (PlayerAnimationHeader*)&gPlayerAnim_link_normal_jump,
                                          REG(69) / 100.0f, gPlayState);
                        }
                    }
                }
            }
        }
    });
}