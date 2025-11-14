#include "ActorBehavior.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/Rando/Logic/Logic.h"

extern "C" {
#include "variables.h"
#include "functions.h"

#include "include/z64player.h"
extern s32 Player_SetAction(PlayState* play, Player* player, PlayerActionFunc actionFunc, s32 arg3);
extern void Player_Action_1(Player* player, PlayState* play);
}

std::map<OcarinaButtonIndex, RandoInf> ocarinaButtonToFlagMap = {
    { OCARINA_BTN_A, RANDO_INF_OBTAINED_OCARINA_BUTTON_A },
    { OCARINA_BTN_C_DOWN, RANDO_INF_OBTAINED_OCARINA_BUTTON_C_DOWN },
    { OCARINA_BTN_C_RIGHT, RANDO_INF_OBTAINED_OCARINA_BUTTON_C_RIGHT },
    { OCARINA_BTN_C_LEFT, RANDO_INF_OBTAINED_OCARINA_BUTTON_C_LEFT },
    { OCARINA_BTN_C_UP, RANDO_INF_OBTAINED_OCARINA_BUTTON_C_UP },
};

void RespawnOnWaterTouch(Player* player) {
    // This is Honey & Darlings Shop, touching the water ends the minigame as its vanilla behavior.
    // No reason to handle it a second time here.
    if (gPlayState->sceneId == SCENE_BOWLING) {
        return;
    }

    if (player->stateFlags1 & PLAYER_STATE1_8000000) {
        // Mimic Deku Hop failure behavior
        Player_SetAction(gPlayState, player, Player_Action_1, 0);
        player->stateFlags1 |= PLAYER_STATE1_20000000;
    }
}

void Rando::ActorBehavior::InitPlayerBehavior() {
    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_SWIM], [](Actor* actor) {
        if (!Flags_GetRandoInf(RANDO_INF_OBTAINED_SWIM)) {
            RespawnOnWaterTouch(GET_PLAYER(gPlayState));
        }
    });
    COND_VB_SHOULD(VB_PLAY_OCARINA_NOTE, IS_RANDO, {
        u8* sCurOcarinaButtonIndex = va_arg(args, u8*);
        u8* sCurOcarinaPitch = va_arg(args, u8*);
        bool canPlayNote = true;

        auto findNote = ocarinaButtonToFlagMap.find(static_cast<OcarinaButtonIndex>(*sCurOcarinaButtonIndex));
        if (findNote != ocarinaButtonToFlagMap.end()) {
            canPlayNote = Flags_GetRandoInf(findNote->second);
        }

        if (!canPlayNote) {
            *sCurOcarinaButtonIndex = OCARINA_BTN_INVALID;
            *sCurOcarinaPitch = OCARINA_PITCH_NONE;
            Audio_PlaySfx(NA_SE_SY_OCARINA_ERROR);
        }
    });
}
