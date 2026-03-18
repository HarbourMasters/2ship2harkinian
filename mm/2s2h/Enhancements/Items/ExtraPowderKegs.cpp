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

    // Update Goron dialogue to reflect the higher max carry count
    COND_ID_HOOK(OnOpenText, 0x0C87, CVAR, [](u16*, bool* loadFromMessageTable) {
        CustomMessage::Entry entry;
        entry.autoFormat = false;
        entry.msg = "\x1e:\xbb\x01Powder Kegs";
        entry.msg += '\x00';
        entry.msg += " are very\x11volatile, so you can carry only \x01";
        entry.msg += std::to_string(MAX_POWDER_KEGS);
        entry.msg += '\x11';
        entry.msg += '\x00';
        entry.msg += "at a time.\x11\x12If you shoot them with an \x01"
                     "arrow";
        entry.msg += '\x00';
        entry.msg += ",\x11they'll \x01"
                     "explode ";
        entry.msg += '\x00';
        entry.msg += "as soon as they're\x11hit, so be careful.\xbf";
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
    COND_ID_HOOK(OnOpenText, 0x0C8B, CVAR, [](u16*, bool* loadFromMessageTable) {
        CustomMessage::Entry entry;
        entry.autoFormat = false;
        entry.msg = "You can carry only \x01";
        entry.msg += std::to_string(MAX_POWDER_KEGS);
        entry.msg += " Powder\x11Kegs";
        entry.msg += '\x00';
        entry.msg += " at a time. Once you've used\x11them, come back.\xbf";
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
    COND_ID_HOOK(OnOpenText, 0x0673, CVAR, [](u16*, bool* loadFromMessageTable) {
        CustomMessage::Entry entry;
        entry.autoFormat = false;
        entry.msg = "\x1e\x38\xfc\x17Oh, but you already have \x01";
        entry.msg += std::to_string(MAX_POWDER_KEGS);
        entry.msg += '\x00';
        entry.msg += ".\x18\x11\x13\x13\x12\x01Powder Kegs";
        entry.msg += '\x00';
        entry.msg += " are dangerous\x11"
                     "explosives, so you can carry only\x11\x01";
        entry.msg += std::to_string(MAX_POWDER_KEGS);
        entry.msg += '\x00';
        entry.msg += " at a time!\xbf";
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
}

static RegisterShipInitFunc initFunc(RegisterExtraPowderKegs, { CVAR_NAME });
