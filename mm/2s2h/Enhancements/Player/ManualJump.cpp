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
extern FloorEffect sPlayerFloorEffect;
extern FloorType sPlayerFloorType;
void func_80834D50(PlayState* play, Player* player, PlayerAnimationHeader* anim, f32 speed, u16 sfxId);
void func_80834DB8(Player* player, PlayerAnimationHeader* anim, f32 speed, PlayState* play);
s32 func_8082FBE8(Player* player);
s32 Player_CanUpdateItems(Player* player);
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
            if (CHECK_BTN_ALL(input->press.button, BTN_A) &&
                (gPlayState->roomCtx.curRoom.behaviorType1 != ROOM_BEHAVIOR_TYPE1_2) &&
                ((FloorType)sPlayerFloorType != FLOOR_TYPE_7) && ((FloorEffect)sPlayerFloorEffect != FLOOR_EFFECT_1)) {
                if (!(player->stateFlags1 & PLAYER_STATE1_8000000) &&
                    (Player_GetMeleeWeaponHeld(player) != PLAYER_MELEEWEAPON_NONE) && func_8082FBE8(player) &&
                    Player_CanUpdateItems(player) && !(player->stateFlags3 & PLAYER_STATE3_2) &&
                    (player->actor.bgCheckFlags &= ~BGCHECKFLAG_GROUND)) {
                    if (temp == 0) {
                        func_80834D50(gPlayState, player, (PlayerAnimationHeader*)&gPlayerAnim_link_fighter_front_jump,
                                      5.8f, NA_SE_VO_LI_SWORD_N);
                    } else if (temp == -1) {
                        func_80834DB8(player, (PlayerAnimationHeader*)&gPlayerAnim_link_normal_jump, REG(69) / 100.0f,
                                      gPlayState);
                    }
                    player->stateFlags3 |= PLAYER_STATE3_2;
                }
            }
        }
    });
}