#include "ZoraEggCount.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "global.h"

void RegisterZoraEggCount() {
    REGISTER_VB_SHOULD(GI_VB_SET_ZORA_EGG_COUNT, {
        if (CVarGetInteger("gEnhancements.Songs.ZoraEggCount", 7) > 7 ||
            CVarGetInteger("gEnhancements.Songs.ZoraEggCount", 7) < 1) {
            CVarSetInteger("gEnhancements.Songs.ZoraEggCount", 7);
        }
        s32* count = static_cast<int*>(opt);
        // Ignore variable count if you have no eggs, fixes dialogue with marine researcher.
        if ((gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_LABO].unk_14 & 7) <= 0) {
            *count = 7;
        } else {
            *count = CVarGetInteger("gEnhancements.Songs.ZoraEggCount", 7);
        }
    });
}