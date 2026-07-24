/*
 * time_gate_message.cpp - Time Gate "Travel through time?" Yes/No textbox (Skijer's NEI).
 *
 * Modeled on oot_spells_messages.cpp (Farore's Wind menu). MM's message table has no such text, so
 * it is built through 2ship's CustomMessage system with MM's native TWO-choice control code and
 * opened from item_time_gate.c (TimeGate_OpenPromptTextbox). The C side reads play->msgCtx.choiceIndex:
 * 0 = Yes, 1 = No (see item_time_gate.c TimeGate_StateHovering).
 *
 * CustomMessage::StartTextbox renders under the built-in CUSTOM_MESSAGE_ID (0x4B) active-message
 * path (registered at boot by CustomMessage::RegisterHooks) — the same well-tested path Farore's
 * uses — so no per-id OnOpenText hook is needed.
 *
 * Control codes as OCTAL escapes: a \x?? hex escape greedily eats following hex-digit letters
 * (e.g. "\x11No" -> \x11N), so octal (fixed width) is required:
 *   \002 = 0x02 (adjustable-color, as in OoT's choice boxes),
 *   \021 = 0x11 (newline),
 *   \302 = 0xC2 (CTRL_TWO_CHOICE; the decoder's 0x202 case sets TEXTBOX_ENDTYPE_TWO_CHOICE).
 *
 * Compiled standalone via the mods/*.cpp glob; registered in build/x64/mm/2ship.vcxproj.
 */

#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"

void TimeGate_OpenPromptTextbox(void) {
    CustomMessage::Entry entry;
    entry.textboxType = 0; // standard box
    // "Travel through time?" newline TWO_CHOICE "Yes" newline "No"  (choiceIndex 0 = Yes, 1 = No)
    CustomMessage::StartTextbox("Travel through time?\002\021\302Yes\021No", entry);
}

} // extern "C"
