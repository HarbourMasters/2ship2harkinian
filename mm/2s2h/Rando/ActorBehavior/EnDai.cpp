#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "overlays/actors/ovl_En_Dai/z_en_dai.h"
}

// Handles opening Snowhead Temple based on form (Goron) or song (Goron Lullaby), or no requirements
void Rando::ActorBehavior::InitEnDaiBehavior() {
    bool shouldRegisterVB = IS_RANDO && (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] ||
                                         !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG]);
    bool shouldRegisterOnSceneInit = IS_RANDO && !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] &&
                                     !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG];

    COND_VB_SHOULD(VB_OPEN_SNOWHEAD_FROM_SONG, shouldRegisterVB, {
        EnDai* enDai = va_arg(args, EnDai*);
        Player* player = GET_PLAYER(gPlayState);

        *should =
            (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG] ||
             (gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_EVENT &&
              gPlayState->msgCtx.lastPlayedSong == OCARINA_SONG_GORON_LULLABY)) &&
            (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] ||
             (player->transformation == PLAYER_FORM_GORON && (gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_EVENT ||
                                                              gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_ACTIVE)));

        if (*should) {
            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
            gPlayState->msgCtx.ocarinaMode = OCARINA_MODE_END;
            Message_CloseTextbox(gPlayState);
        }
    });

    COND_ID_HOOK(OnSceneInit, SCENE_12HAKUGINMAE, shouldRegisterOnSceneInit,
                 [](s16 sceneId, s8 spawnNum) { SET_WEEKEVENTREG(WEEKEVENTREG_30_01); });
}
