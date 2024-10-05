#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "overlays/actors/ovl_En_Kendo_Js/z_en_kendo_js.h"

void func_80B274BC(EnKendoJs* thisx, PlayState* play);
}

void RegisterSwordsmanSchool() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldActorUpdate>(
        ACTOR_EN_KENDO_JS, [](Actor* actor, bool* should) {
            EnKendoJs* kendo = (EnKendoJs*)actor;

            if (kendo->actionFunc != func_80B274BC) {
                return;
            }

            if (CVarGetInteger("gEnhancements.Minigames.SwordsmanSchool.FinishEarly", 0) &&
                gSaveContext.minigameScore >=
                    CVarGetInteger("gEnhancements.Minigames.SwordsmanSchool.WinningScore", 30)) {
                kendo->unk_290 = 140;
                kendo->unk_284 = 5;
            }

            if (kendo->unk_290 >= 140 && kendo->unk_284 == 5 &&
                gSaveContext.minigameScore >=
                    CVarGetInteger("gEnhancements.Minigames.SwordsmanSchool.WinningScore", 30)) {
                gSaveContext.minigameScore = 30;
            }
        });
}
