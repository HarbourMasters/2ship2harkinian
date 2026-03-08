#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipInit.hpp"

#define CVAR_NAME "gEnhancements.Items.ExtraPowderKegs"
#define CVAR CVarGetInteger(CVAR_NAME, 0)
#define MAX_POWDER_KEGS 3

void RegisterExtraPowderKegs() {
    // When giving a powder keg, increment ammo instead of setting to 1
    COND_VB_SHOULD(VB_POWDER_KEG_SET_AMMO_ON_GIVE, CVAR, {
        *should = false;
        if (AMMO(ITEM_POWDER_KEG) < MAX_POWDER_KEGS) {
            AMMO(ITEM_POWDER_KEG)++;
        }
    });

    // When clamping ammo, use our max instead of 1
    COND_VB_SHOULD(VB_POWDER_KEG_CAP_AMMO, CVAR, {
        *should = false;
        if (AMMO(ITEM_POWDER_KEG) > MAX_POWDER_KEGS) {
            AMMO(ITEM_POWDER_KEG) = MAX_POWDER_KEGS;
        } else if (AMMO(ITEM_POWDER_KEG) < 0) {
            AMMO(ITEM_POWDER_KEG) = 0;
        }
    });

    // Allow buying more kegs if below max
    COND_VB_SHOULD(VB_POWDER_KEG_CHECK_HAS, CVAR, {
        *should = (AMMO(ITEM_POWDER_KEG) >= MAX_POWDER_KEGS) || (gPlayState->actorCtx.flags & ACTORCTX_FLAG_0);
    });

    // Show green ammo text only when at max capacity (3), not at 1
    COND_VB_SHOULD(VB_POWDER_KEG_AMMO_AT_CAPACITY, CVAR, { *should = (AMMO(ITEM_POWDER_KEG) >= MAX_POWDER_KEGS); });

    // Update Goron dialogue to say "three" instead of "one"
    COND_ID_HOOK(OnOpenText, 0x0C87, CVAR, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        CustomMessage::Replace(&entry.msg, "one", "three");
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
    COND_ID_HOOK(OnOpenText, 0x0C8B, CVAR, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        CustomMessage::Replace(&entry.msg, "one", "three");
        CustomMessage::Replace(&entry.msg, "Keg", "Kegs");
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
    COND_ID_HOOK(OnOpenText, 0x0673, CVAR, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        CustomMessage::Replace(&entry.msg, "one", "three");
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
}

static RegisterShipInitFunc initFunc(RegisterExtraPowderKegs, { CVAR_NAME });
