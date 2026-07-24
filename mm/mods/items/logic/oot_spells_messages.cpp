/**
 * oot_spells_messages.cpp — C bridge for the OoT spell textboxes (Skijer's NEI).
 *
 * Farore's Wind menu: OoT opens message 0x3B ("You cast Farore's Wind!" — a THREE-choice box:
 * Return to the Warp Point / Dispel the Warp Point / Exit; verbatim from the OoT decomp
 * text/message_data.h:1797). MM's message table has no such text, so it's re-created through
 * 2ship's CustomMessage system with MM's native three-choice control code (0xC3). The C side
 * (item_oot_spells.c Player_Action_OotFwMenu) reads play->msgCtx.choiceIndex:
 * 0 = Return, 1 = Dispel, 2 = Exit (no effect).
 *
 * Compiled standalone via the mods/*.cpp CMake glob.
 */

#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"

void OotSpells_OpenFwTextbox(void) {
    CustomMessage::Entry entry;

    entry.textboxType = 0; // standard box
    // OoT message 0x003B — VERBATIM from the OoT decomp (gc-eu-mq-dbg text/message_data.h:1797):
    //   "You cast Farore's Wind!\n" THREE_CHOICE
    //       COLOR(ADJUSTABLE) "Return to the Warp Point\n" "Dispel the Warp Point\n" "Exit" COLOR(DEFAULT)
    // It IS a three-choice box. The handler Player_Action_8085063C reads choiceIndex 0 (return to the
    // warp point) and 1 (dispel); choiceIndex 2 (Exit) falls through and just closes with no effect —
    // Player_Action_OotFwMenu mirrors this exactly.
    //
    // Control codes as OCTAL escapes — a \x?? hex escape greedily eats following hex-digit letters
    // (\x11Dispel -> \x11D = 0x11D), so octal (fixed 3 digits) is required:
    //   \002 = 0x02, \021 = 0x11 (newline), \303 = 0xC3 (CTRL_THREE_CHOICE).
    CustomMessage::StartTextbox(
        "You cast Farore's Wind!\002\021\303Return to the Warp Point\021Dispel the Warp Point\021Exit", entry);
}

} // extern "C"
