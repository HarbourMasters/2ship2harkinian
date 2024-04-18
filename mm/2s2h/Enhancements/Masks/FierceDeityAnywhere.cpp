#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterFierceDeityAnywhere() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(GI_VB_DISABLE_FD_MASK, [](GIVanillaBehavior _, bool* should, void* __) {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            *should = false;
        }
    });
}
