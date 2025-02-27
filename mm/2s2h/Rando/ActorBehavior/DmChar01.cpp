#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "overlays/actors/ovl_Dm_Char01/z_dm_char01.h"
}

// Handles opening Woodfall Temple based on form (Deku) or song (Sonata of Awakening), or no requirements
void Rando::ActorBehavior::InitDmChar01Behavior() {
    bool shouldRegisterVB = IS_RANDO && (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] ||
                                         !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG]);
    bool shouldRegisterOnSceneInit = IS_RANDO && !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] &&
                                     !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG];

    COND_VB_SHOULD(VB_OPEN_WOODFALL_FROM_SONG, shouldRegisterVB, {
        DmChar01* dmChar01 = va_arg(args, DmChar01*);
        Player* player = GET_PLAYER(gPlayState);

        *should =
            (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG] ||
             (gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_EVENT &&
              gPlayState->msgCtx.lastPlayedSong == OCARINA_SONG_SONATA)) &&
            (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] ||
             (player->transformation == PLAYER_FORM_DEKU && (gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_EVENT ||
                                                             gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_ACTIVE)));

        if (*should) {
            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
            gPlayState->msgCtx.ocarinaMode = OCARINA_MODE_END;
            Message_CloseTextbox(gPlayState);
        }
    });

    COND_ID_HOOK(OnSceneInit, SCENE_21MITURINMAE, shouldRegisterOnSceneInit,
                 [](s16 sceneId, s8 spawnNum) { SET_WEEKEVENTREG(WEEKEVENTREG_20_01); });
}
