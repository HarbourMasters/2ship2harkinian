#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "variables.h"

void RegisterFierceDeityPutaway() {
    REGISTER_VB_SHOULD(GI_VB_SHOULD_PUTAWAY, {
        if (CVarGetInteger("gEnhancements.Player.FierceDeityPutaway", 0)) {
            Player* player = GET_PLAYER(gPlayState);
            if (player->transformation == PLAYER_FORM_FIERCE_DEITY)
                *should = true;
        }
    });

    REGISTER_VB_SHOULD(GI_VB_FD_ALWAYS_WIELD_SWORD, {
        if (CVarGetInteger("gEnhancements.Player.FierceDeityPutaway", 0)) {
            Player* player = GET_PLAYER(gPlayState);
            *should = false;
        }
    });
}
