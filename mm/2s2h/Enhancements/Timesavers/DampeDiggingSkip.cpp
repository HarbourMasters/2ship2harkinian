#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/actors/ovl_En_Tk/z_en_tk.h"
#include "overlays/actors/ovl_En_Bigpo/z_en_bigpo.h"

void func_80AEE6B8(EnTk* thisx, PlayState* play);
void func_80AEDBEC(EnTk* thisx, PlayState* play);
void EnBigpo_SetupSpawnCutscene(EnBigpo* thisx);
void EnBigpo_RevealedFireIdle(EnBigpo* thisx, PlayState* play);
}

#define CVAR_NAME "gEnhancements.Timesavers.DampeDiggingSkip"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterDampeDiggingSkip() {

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EN_TK, CVAR, [](Actor* actor, bool* should) {
        EnTk* dampe = (EnTk*)actor;

        if (dampe->actionFunc != func_80AEE6B8) {
            return;
        }

        if (dampe->unk_2CA & 0x20) {
            CutsceneManager_Stop(dampe->csIdList[0]);
            Message_CloseTextbox(gPlayState);
            func_80AEDBEC(dampe, gPlayState);
        }
    });

    COND_ID_HOOK(ShouldActorDraw, ACTOR_EN_BIGPO, CVAR, [](Actor* actor, bool* should) {
        EnBigpo* firePo = (EnBigpo*)actor;

        if (firePo->actionFunc == EnBigpo_RevealedFireIdle) {
            EnBigpo_SetupSpawnCutscene((EnBigpo*)firePo->actor.parent);
        }
    });
}
static RegisterShipInitFunc initFunc(RegisterDampeDiggingSkip, { CVAR_NAME });