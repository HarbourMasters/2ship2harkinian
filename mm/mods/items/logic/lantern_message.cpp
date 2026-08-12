/*
 * lantern_message.cpp - Poe Lantern fire-catch textbox (Skijer's NEI).
 *
 * MM's message table has no text for this, so it is built through 2ship's CustomMessage system,
 * exactly like time_gate_message.cpp / oot_spells_messages.cpp. Opened from
 * Player_Action_SwingLantern (item_lantern.c) the frame the catch animation ends; the fire type is
 * handed over in gLanternCatchPending, mirroring SoH's BuildLanternCatchMessage in ItemMessages.cpp
 * so both games say the same thing.
 *
 * CustomMessage::StartTextbox renders under the built-in CUSTOM_MESSAGE_ID (0x4B) active-message
 * path (registered at boot by CustomMessage::RegisterHooks), so no per-id OnOpenText hook is needed.
 *
 * %r/%g/%b/%p/%w are CustomMessage's colour placeholders (see ReplaceColorChars).
 *
 * Compiled standalone via the mods/*.cpp glob; registered in build/x64/mm/2ship.vcxproj.
 */

#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"

extern u8 gLanternCatchPending; // LanternFireType of the fire just caught (item_lantern.c)

void Lantern_OpenCatchTextbox(void) {
    CustomMessage::Entry entry;
    const char* msg;

    entry.textboxType = 0; // standard box

    switch (gLanternCatchPending) {
        case 1: // REGULAR
            msg = "You caught %rRegular Fire%w!\nSwing to %rlight torches%w, %rburn grass%w\nand throw flames.";
            break;
        case 2: // BLUE
            msg = "You caught %bBlue Fire%w!\nIts flame %bfreezes%w what it touches.";
            break;
        case 3: // POE / shadow
            msg = "You caught %pShadow Fire%w!\nHold the lantern to %preveal the\ninvisible%w and %pdispel illusions%w.";
            break;
        case 4: // GREEN
            msg = "You caught %gGreen Fire%w!\nIt burns far longer and %grestores\nhealth and magic%w while you rest.";
            break;
        default:
            msg = "The lantern is empty.";
            break;
    }

    CustomMessage::StartTextbox(msg, entry);
}

} // extern "C"
