#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/ShipInit.hpp"
#include "2s2h/cvar_prefixes.h"

extern "C" {
#include "src/overlays/gamestates/ovl_file_choose/z_file_select.h"

extern FileSelectState* gFileSelectState;
}

#define CVAR_NAME CVAR_ENHANCEMENT("Saving.FileSlot3")
#define CVAR CVarGetInteger(CVAR_NAME, true)

void RegisterFileSlot3() {
    // Reset the file select state to reload the save metadata
    if (gFileSelectState != NULL) {
        STOP_GAMESTATE(&gFileSelectState->state);
        SET_NEXT_GAMESTATE(&gFileSelectState->state, FileSelect_Init, sizeof(FileSelectState));
    }
}

static RegisterShipInitFunc initFunc(RegisterFileSlot3, { CVAR_NAME });
