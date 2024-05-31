#include "ZoraEggCount.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

void RegisterZoraEggCount() {
    REGISTER_VB_SHOULD(GI_VB_SET_ZORA_EGG_COUNT, {
        if (CVarGetInteger("gEnhancements.Songs.ZoraEggCount", 7) > 7 ||
            CVarGetInteger("gEnhancements.Songs.ZoraEggCount", 7) < 1) {
            CVarSetInteger("gEnhancements.Songs.ZoraEggCount", 7);
        }
        s32* count = static_cast<int*>(opt);
        *count = CVarGetInteger("gEnhancements.Songs.ZoraEggCount", 7);
    });
}