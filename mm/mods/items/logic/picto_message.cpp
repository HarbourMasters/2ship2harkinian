/**
 * picto_message.cpp - Pictograph Box "Keep this picture?" textbox (Skijer's NEI).
 *
 * MM shows a 2-choice message right after the shutter (z_parameter.c: Message_StartTextbox 0xF8):
 * Keep / Discard. We register an OnOpenText-for-ID hook for a custom textId and build the prompt with
 * 2ship's CustomMessage system (CustomMessage::LoadCustomMessageIntoFont). picto_box.c opens it via
 * Message_StartTextbox(PICTO_KEEP_TEXTID) and reads msgCtx.choiceIndex (0 = keep, !=0 = discard),
 * exactly like MM's PICTO_BOX_STATE_PHOTO handler.
 *
 * 2ship port notes:
 *   • 2ship's CustomMessage is a namespace of free functions over a CustomMessage::Entry (NOT SoH's
 *     CustomMessage class). We fill an Entry and call LoadCustomMessageIntoFont; setting
 *     *loadFromMessageTable = false makes the engine use the font buffer we just wrote.
 *   • SoH's TWO_WAY_CHOICE() (OoT control char) -> MM's two-choice control byte CTRL_TWO_CHOICE = 0xC2
 *     (z_message.c case 0x202 sets TEXTBOX_ENDTYPE_TWO_CHOICE). The two options follow it, separated by
 *     a newline ('\n' -> CTRL_NEWLINE 0x11 via the Entry auto-format). choiceIndex 0 = first option.
 *   • Colors use 2ship's %-codes (%r red, %g green, %w white) resolved by ReplaceColorChars.
 *
 * New .cpp -> add it in the VS Solution Explorer (the CMake mods glob picks up *.cpp).
 */

#include <spdlog/spdlog.h>
#include <soh/OTRGlobals.h>
#include "soh/ShipInit.hpp"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/custom-message/CustomMessageManager.h" // shim -> 2s2h/CustomMessage/CustomMessage.h

#include <string>

extern "C" {
#include "variables.h"
#include "functions.h"
#include "macros.h"
#include "mods/items/logic/snap.h" // Snap_CheckFlag + PICTO_VALID_* (the photographed subject)
#include "mods/nei_save.h"          // Nei_Save()->pictoHasPhoto (replace-vs-keep wording)
}

#define PICTO_KEEP_TEXTID 0x6F08    // must match mods/items/logic/snap.h
#define PICTO_REPLACE_TEXTID 0x6F09 // must match mods/items/logic/snap.h

// MM two-choice control byte (CTRL_TWO_CHOICE). Placed where the prompt ends and the choices begin.
#define PICTO_TWO_CHOICE "\xC2"

// Build MM's "Keep this picture?" 2-choice prompt. choiceIndex 0 = "Keep it", 1 = "Throw it away".
static void Picto_BuildKeepMessage(uint16_t* textId, bool* loadFromMessageTable) {
    // Name the photographed subject, like 2Ship's BetterPictoMessage: read the validation flags set at
    // the shutter (Snap_RecordPictographedActors) and pick the target. Later checks override earlier;
    // Lulu needs all three body parts. -> "Keep this picture of a Pirate?" etc.
    std::string target;
    if (Snap_CheckFlag(PICTO_VALID_IN_SWAMP))
        target = "the Swamp";
    if (Snap_CheckFlag(PICTO_VALID_MONKEY))
        target = "a Monkey";
    if (Snap_CheckFlag(PICTO_VALID_BIG_OCTO))
        target = "a Big Octo";
    if (Snap_CheckFlag(PICTO_VALID_LULU_HEAD) && Snap_CheckFlag(PICTO_VALID_LULU_RIGHT_ARM) &&
        Snap_CheckFlag(PICTO_VALID_LULU_LEFT_ARM))
        target = "Lulu";
    if (Snap_CheckFlag(PICTO_VALID_SCARECROW))
        target = "a Scarecrow";
    if (Snap_CheckFlag(PICTO_VALID_TINGLE))
        target = "Tingle";
    if (Snap_CheckFlag(PICTO_VALID_PIRATE_GOOD))
        target = "a Pirate";
    if (Snap_CheckFlag(PICTO_VALID_DEKU_KING))
        target = "the Deku King";

    // If a photo is already saved, warn that keeping this one replaces it (the pictograph syncs
    // OoT<->MM). %g = green choices; %r/%w color the prompt.
    std::string question;
    if (Nei_Save()->pictoHasPhoto) {
        question = target.empty() ? std::string("You already have a %rpicture%w. Replace it?")
                                  : ("You already have a %rpicture%w. Replace it with this of " + target + "?");
    } else {
        question = target.empty() ? std::string("Keep this %rpicture%w?")
                                  : ("Keep this %rpicture of " + target + "%w?");
    }

    CustomMessage::Entry entry;
    entry.msg = question + PICTO_TWO_CHOICE + "%gKeep it\nThrow it away";
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

// "You already have a pictograph. Replace it?" — shown before overwriting an existing (synced) photo.
// choiceIndex 0 = "Replace it", 1 = "Cancel".
static void Picto_BuildReplaceMessage(uint16_t* textId, bool* loadFromMessageTable) {
    CustomMessage::Entry entry;
    entry.msg = std::string("You already have a %rpictograph%w. Replace it?") + PICTO_TWO_CHOICE +
                "%gReplace it\nCancel";
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

static void Picto_RegisterMessageHooks() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnOpenText>(PICTO_KEEP_TEXTID,
                                                                                Picto_BuildKeepMessage);
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnOpenText>(PICTO_REPLACE_TEXTID,
                                                                                Picto_BuildReplaceMessage);
}

static RegisterShipInitFunc sPictoMessageInit(Picto_RegisterMessageHooks);
