#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "overlays/actors/ovl_Obj_Bean/z_obj_bean.h"
void func_809388A8(ObjBean* objBean, PlayState* play);
}

void Rando::ActorBehavior::InitObjBeanBehavior() {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_OBJ_BEAN, IS_RANDO, [](Actor* actor, bool* should) {
        ObjBean* objBean = (ObjBean*)actor;
        // Bean is in moving state, and player is in Get Item state
        if (objBean->actionFunc == func_809388A8 && GET_PLAYER(gPlayState)->stateFlags1 & PLAYER_STATE1_400) {
            *should = false;
        }
    });
}
