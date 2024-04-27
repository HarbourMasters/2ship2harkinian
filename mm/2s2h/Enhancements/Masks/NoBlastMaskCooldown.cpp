#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterNoBlastMaskCooldown() {
    REGISTER_VB_SHOULD(GI_VB_BLAST_MASK_COOLDOWN, {
        if (CVarGetInteger("gEnhancements.Masks.NoBlastMaskCooldown", 0)) {
            *should = false;
        }
    });
}
