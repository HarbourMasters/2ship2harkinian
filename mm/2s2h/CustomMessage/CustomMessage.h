#ifndef CUSTOM_MESSAGE_H
#define CUSTOM_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#include "z64.h"
#endif

static const u16 CUSTOM_MESSAGE_ID = 0x4000;

typedef enum ModId {
    MOD_ID_VANILLA = 0,
    MOD_ID_SHIP = 1,
} ModId;

#define DEFINE_MESSAGE(textId, typePos, msg) SHIP_TEXT_##textId,

typedef enum ShipTextId {
#include "2s2h/CustomMessage/ShipMessages.h"
    SHIP_TEXT_MAX,
} ShipTextId;

#undef DEFINE_MESSAGE

void CustomMessage_StartTextbox(PlayState* play, s32 modId, s32 textId, Actor* actor);
void CustomMessage_ContinueTextbox(PlayState* play, s32 modId, s32 textId);
void CustomMessage_HandleCustomMessage();

#ifdef __cplusplus
};
void CustomMessage_SetActiveMessage(s32 modId, s32 textId);
#endif

#endif
