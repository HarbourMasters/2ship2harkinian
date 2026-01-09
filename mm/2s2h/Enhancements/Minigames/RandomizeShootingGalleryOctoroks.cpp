#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/actors/ovl_En_Syateki_Man/z_en_syateki_man.h"
#include "overlays/actors/ovl_En_Syateki_Okuta/z_en_syateki_okuta.h"
void EnSyatekiMan_Town_RunGame(EnSyatekiMan* enSyatekiMan, PlayState* play);
}

#define CVAR_NAME "gEnhancements.Minigames.RandomizeShootingGalleryOctoroks"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterRandomizeShootingGalleryOctoroks() {
    COND_ID_HOOK(OnActorUpdate, ACTOR_EN_SYATEKI_MAN, CVAR, [](Actor* actor) {
        EnSyatekiMan* enSyatekiMan = (EnSyatekiMan*)actor;

        if (enSyatekiMan->actionFunc != EnSyatekiMan_Town_RunGame) {
            return;
        }

        if (enSyatekiMan->shootingGameState != SG_GAME_STATE_RUNNING) {
            return;
        }

        if (enSyatekiMan->octorokState != SG_OCTO_STATE_APPEARING) {
            return;
        }

        // Extract octorok types from vanilla flags
        s16 octorokTypes[9];
        s32 octorokCount = 0;
        for (s32 i = 0; i < 9; i++) {
            s16 type = SG_OCTO_GET_TYPE(enSyatekiMan->octorokFlags, i);
            if (type != SG_OCTO_TYPE_NONE) {
                octorokTypes[octorokCount++] = type;
            }
        }

        if (octorokCount == 0) {
            return;
        }

        // Shuffle the positions 0-8
        s32 positions[9];
        for (s32 i = 0; i < 9; i++) {
            positions[i] = i;
        }
        for (s32 i = 8; i > 0; i--) {
            s32 j = (s32)(Rand_ZeroOne() * (i + 1));
            if (j > i)
                j = i;
            s32 temp = positions[i];
            positions[i] = positions[j];
            positions[j] = temp;
        }

        // Rebuild octorokFlags with new positions
        s32 newFlags = 0;
        for (s32 i = 0; i < octorokCount; i++) {
            newFlags |= SG_OCTO_SET_FLAG(octorokTypes[i], positions[i]);
        }

        enSyatekiMan->octorokFlags = newFlags;
    });
}

static RegisterShipInitFunc initFunc(RegisterRandomizeShootingGalleryOctoroks, { CVAR_NAME });
