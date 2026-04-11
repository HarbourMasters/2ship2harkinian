#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/CustomMessage/CustomMessage.h"

#define CVAR_NAME "gEnhancements.Saving.PersistentOwlSaves"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// "You can save your progress and quit here."
static constexpr u16 TEXT_ID_OWL_SAVE = 0xC01;

static void ModifySaveExplanation(u16* textId, bool* loadFromMessageTable) {
    size_t warningStart;
    size_t warningLength;

    // TODO: Add different cases when other versions are supported
    switch (ResourceMgr_GetGameVersion(0)) {
        case MM_NTSC_US_10:
        case MM_NTSC_US_GC:
            // "Warning: If you reopen this Owl File, then reset without saving..."
            warningStart = 258;
            warningLength = 261;
            break;
        default:
            // Unknown region, don't modify the message
            return;
    }

    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
    entry.msg.erase(warningStart, warningLength);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

static void RegisterSkipOwlSaveWarning() {
    COND_ID_HOOK(OnOpenText, TEXT_ID_OWL_SAVE, CVAR, ModifySaveExplanation);
}

static RegisterShipInitFunc initFunc(RegisterSkipOwlSaveWarning, { CVAR_NAME });
