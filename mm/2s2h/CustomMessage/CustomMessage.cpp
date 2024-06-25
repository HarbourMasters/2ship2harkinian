
#include "CustomMessage.h"
#include <map>

extern "C" {
#include "z64.h"
extern PlayState* gPlayState;
}

static s32 activeMessageModId = -1;
static s32 activeMessageTextId = -1;

#define DEFINE_MESSAGE(textId, typePos, msg) { SHIP_TEXT_##textId, typePos, msg },

MessageTableEntry shipMessagesTable[] = {
#include "2s2h/CustomMessage/ShipMessages.h"
    { 0xFFFF, 0, NULL },
};

#undef DEFINE_MESSAGE

// Intended to be used in conjunction with GI's OnOpenText hook
void CustomMessage_SetActiveMessage(s32 modId, s32 textId) {
    activeMessageModId = modId;
    activeMessageTextId = textId;
}

// Intended to be called in place of Message_StartTextbox within the source code when overriding messages
extern "C" void CustomMessage_StartTextbox(PlayState* play, s32 modId, s32 textId, Actor* actor) {
    activeMessageModId = modId;
    activeMessageTextId = textId;
    Message_StartTextbox(play, CUSTOM_MESSAGE_ID, actor);
}

// Intended to be called in place of Message_ContinueTextbox within the source code when overriding messages
extern "C" void CustomMessage_ContinueTextbox(PlayState* play, s32 modId, s32 textId) {
    activeMessageModId = modId;
    activeMessageTextId = textId;
    Message_ContinueTextbox(play, CUSTOM_MESSAGE_ID);
}

extern "C" void CustomMessage_HandleCustomMessage() {
    if (activeMessageTextId == -1 || activeMessageModId == -1)
        return;

    // Currently we only support the ship mod
    if (activeMessageModId != MOD_ID_SHIP)
        return;

    MessageContext* msgCtx = &gPlayState->msgCtx;
    Font* font = &msgCtx->font;

    MessageTableEntry* msgEntry = &shipMessagesTable[activeMessageTextId];
    const char* msg = shipMessagesTable[activeMessageTextId].segment + 11;
    msgEntry->msgSize = strlen(msg) + 12;

    msgCtx->msgLength = msgEntry->msgSize;
    memcpy(&font->msgBuf, msgEntry->segment, msgEntry->msgSize);
    // Set the last byte to 0xBF to indicate the end of the message
    font->msgBuf.schar[msgEntry->msgSize - 1] = '\xbf';

    activeMessageModId = -1;
    activeMessageTextId = -1;
}
