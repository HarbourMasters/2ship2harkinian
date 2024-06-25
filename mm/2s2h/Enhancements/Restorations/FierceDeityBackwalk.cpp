#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterFierceDeityBackwalk() {
    REGISTER_VB_SHOULD(GI_VB_ANCHORED_BACKWALK, {
        Player* player = (Player*)opt;
        // If the player is Fierce Deity, the anchored backwalk animation has a longer duration.
        if (CVarGetInteger("gEnhancements.Restorations.FierceDeityBackwalk", 0) &&
            player->transformation == PLAYER_FORM_FIERCE_DEITY) {
            f32 currentFrame = player->skelAnime.curFrame;
            *should = player->skelAnime.curFrame > 9.0f;
        }
    });
}
