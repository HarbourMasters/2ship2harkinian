#include "ActorBehavior.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/Rando/Logic/Logic.h"

extern "C" {
#include "variables.h"

#include "overlays/actors/ovl_En_Zob/z_en_zob.h"
}

void Rando::ActorBehavior::InitEnZobBehavior() {
    COND_VB_SHOULD(VB_JAPAS_START_JAM_SESSION, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_OCARINA_BUTTONS], {
        EnZob* japas = va_arg(args, EnZob*);
        if (!(Rando::Logic::canPlaySong(OCARINA_SONG_EVAN_PART1) ||
              Rando::Logic::canPlaySong(OCARINA_SONG_EVAN_PART2))) {
            Message_StartTextbox(gPlayState, 0x1214, NULL);
            *should = false;
        }
    });
}