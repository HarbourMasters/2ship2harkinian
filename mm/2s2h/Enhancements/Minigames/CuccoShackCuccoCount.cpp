#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "overlays/actors/ovl_En_Hs/z_en_hs.h"
#include "overlays/actors/ovl_En_Nwc/z_en_nwc.h"

void func_8095345C(EnHs* enHs, PlayState* play);
}

void RegisterCuccoShackCuccoCount() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldActorUpdate>(
        ACTOR_EN_HS, [](Actor* actor, bool* should) {
            EnHs* enHs = (EnHs*)actor;

            if (enHs->actionFunc != func_8095345C ||
                CVarGetInteger("gEnhancements.Minigames.CuccoShackCuccoCount", 10) == 10) {
                return;
            }

            // As soon as enough have grown up, set the count to the max
            if (enHs->actor.home.rot.x >= (CVarGetInteger("gEnhancements.Minigames.CuccoShackCuccoCount", 10) * 2) &&
                enHs->actor.home.rot.x != 20) {
                enHs->actor.home.rot.x = 20;
            }

            // As soon as enough are following the player, set the count to the max
            if (enHs->actor.home.rot.z >= (CVarGetInteger("gEnhancements.Minigames.CuccoShackCuccoCount", 10) * 2) &&
                enHs->actor.home.rot.z != 20) {
                enHs->actor.home.rot.z = 20;
            }
        });
}
