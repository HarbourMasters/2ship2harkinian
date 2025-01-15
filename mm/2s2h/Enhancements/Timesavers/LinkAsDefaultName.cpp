#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/gamestates/ovl_file_choose/z_file_select.h"
extern FileSelectState* gFileSelectState;
}

#define CVAR_NAME "gEnhancements.Timesavers.LinkAsDefaultName"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterLinkAsDefaultName() {
    static u8 sLinkName[] = { 0x15, 0x2C, 0x31, 0x2E, 0x3E, 0x3E, 0x3E, 0x3E };

    COND_VB_SHOULD(VB_PREVENT_FILESELECT_EMPTY_NAME, CVAR, {
        *should = true;
        gFileSelectState->newFileNameCharCount = 4;
        memcpy(&gFileSelectState->fileNames[gFileSelectState->buttonIndex], &sLinkName, ARRAY_COUNT(sLinkName));
    });
}

static RegisterShipInitFunc initFunc(RegisterLinkAsDefaultName, { CVAR_NAME });