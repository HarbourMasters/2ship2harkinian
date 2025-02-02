#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/actors/ovl_En_Tk/z_en_tk.h"
#include "overlays/actors/ovl_En_Bigpo/z_en_bigpo.h"

void func_80AEDBEC(EnTk* thisx, PlayState* play);
void EnBigpo_SetupSpawnCutscene(EnBigpo* thisx);
void EnBigpo_RevealedFireIdle(EnBigpo* thisx, PlayState* play);
}

#define CVAR_NAME "gEnhancements.Timesavers.DampeDiggingSkip"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterDampeDiggingSkip() {

    COND_ID_HOOK(ShouldActorDraw, ACTOR_EN_BIGPO, CVAR, [](Actor* actor, bool* should) {
        EnBigpo* firePo = (EnBigpo*)actor;
        bool dampeCutsceneTriggered = false;

        if (firePo->actionFunc == EnBigpo_RevealedFireIdle) {
            dampeCutsceneTriggered = false;

            if (!dampeCutsceneTriggered) {
                EnBigpo_SetupSpawnCutscene((EnBigpo*)firePo->actor.parent);

                Actor* dampeActor = SubS_FindActor(gPlayState, NULL, ACTORCAT_NPC, ACTOR_EN_TK);
                if (dampeActor != NULL) {
                    EnTk* dampe = (EnTk*)dampeActor;
                    if (dampe->unk_2CA) {
                        func_80AEDBEC(dampe, gPlayState);
                        dampeCutsceneTriggered = true;
                    }
                }
            }
        }
    });
}
static RegisterShipInitFunc initFunc(RegisterDampeDiggingSkip, { CVAR_NAME });
