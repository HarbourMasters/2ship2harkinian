
#include "CustomMessage.h"
#include <libultraship/bridge.h>
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"
#include <map>

extern "C" {
#include "z64.h"
extern PlayState* gPlayState;
}

static s32 activeMessageModId = -1;
static s32 activeMessageTextId = -1;

#define DEFINE_MESSAGE(textId, typePos, msg) { SHIP_TEXT_##textId, typePos, msg "\xBF" },

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

void CustomMessage_Replace(std::string* msg, const std::string& placeholder, const std::string& value) {
    size_t pos = 0;
    while ((pos = msg->find(placeholder, pos)) != std::string::npos) {
        msg->replace(pos, placeholder.length(), value);
        pos += value.length();
    }
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
    char buff[1280] = { 0 };

    // Copy message metadata
    memcpy(buff, msgEntry->segment, 11);

    // Convert the rest of the message to a std::string
    std::string msg = msgEntry->segment + 11;
    // Allow the message to be modified by the game interactor
    GameInteractor_ExecuteOnHandleCustomMessage(activeMessageModId, activeMessageTextId, &msg);

    // If messaage is too long, truncate it and add the message end character
    if (msg.length() > 1269) {
        msg = msg.substr(0, 1268);
        msg += '\xBF';
    }

    // Copy modified message to the buffer
    memcpy(buff + 11, msg.c_str(), msg.length());

    // Set the message length and copy the buffer to the real message buffer
    msgCtx->msgLength = msg.length() + 11;
    memcpy(&font->msgBuf, buff, msgCtx->msgLength);

    activeMessageModId = -1;
    activeMessageTextId = -1;
}
