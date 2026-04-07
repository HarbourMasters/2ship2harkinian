#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/CustomMessage/CustomMessage.h"

#define CVAR_NAME "gEnhancements.Saving.PersistentOwlSaves"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

// "You can save your progress and quit here."
static constexpr u16 TEXT_ID_OWL_SAVE = 0xC01;

// "Warning: If you reopen this Owl File, then reset without saving..."
static constexpr size_t TEXT_WARNING_BEGIN = 258;
static constexpr size_t TEXT_WARNING_LENGTH = 261;

static void ModifySaveExplanation(u16* textId, bool* loadFromMessageTable) {
    // Get original message
    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
    entry.msg.erase(TEXT_WARNING_BEGIN, TEXT_WARNING_LENGTH);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

static void RegisterSkipOwlSaveWarning() {
    COND_ID_HOOK(OnOpenText, TEXT_ID_OWL_SAVE, CVAR, ModifySaveExplanation);
}

static RegisterShipInitFunc initFunc(RegisterSkipOwlSaveWarning, { CVAR_NAME });
