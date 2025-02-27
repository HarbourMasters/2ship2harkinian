#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "overlays/actors/ovl_Dm_Char08/z_dm_char08.h"
}

// Handles opening Great Bay Temple based on form (Zora) or song (New Wave), or no requirements
void Rando::ActorBehavior::InitDmChar08Behavior() {

    bool shouldRegisterOnSceneInit = IS_RANDO && !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] &&
                                     !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG];
    bool shouldRegisterVB = IS_RANDO && !shouldRegisterOnSceneInit &&
                            (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] ||
                             !RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG]);

    COND_VB_SHOULD(VB_OPEN_GREAT_BAY_FROM_SONG, shouldRegisterVB, {
        DmChar08* dmChar08 = va_arg(args, DmChar08*);
        Player* player = GET_PLAYER(gPlayState);

        *should =
            !CHECK_WEEKEVENTREG(WEEKEVENTREG_53_20) &&
            ((player->actor.world.pos.x > -5780.0f) && (player->actor.world.pos.x < -5385.0f) &&
             (player->actor.world.pos.z > 1120.0f) && (player->actor.world.pos.z < 2100.0f)) &&
            (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_SONG] ||
             (gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_EVENT &&
              gPlayState->msgCtx.lastPlayedSong == OCARINA_SONG_NEW_WAVE)) &&
            (!RANDO_SAVE_OPTIONS[RO_ACCESS_DUNGEONS_REQUIRES_FORM] ||
             (player->transformation == PLAYER_FORM_ZORA && (gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_EVENT ||
                                                             gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_ACTIVE)));

        if (*should) {
            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
            gPlayState->msgCtx.ocarinaMode = OCARINA_MODE_END;
            Message_CloseTextbox(gPlayState);
        }
    });

    COND_ID_HOOK(OnSceneInit, SCENE_31MISAKI, shouldRegisterOnSceneInit,
                 [](s16 sceneId, s8 spawnNum) { SET_WEEKEVENTREG(WEEKEVENTREG_53_20); });
}
