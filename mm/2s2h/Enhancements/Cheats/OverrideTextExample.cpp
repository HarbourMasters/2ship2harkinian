#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "2s2h/CustomMessage/CustomMessage.h"

void RegisterOverrideTextExample() {
    // Overrides the sign in south clock town message
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnOpenText>(0x1C18, [](u16* textId) {
        CustomMessage_SetActiveMessage(MOD_ID_SHIP, SHIP_TEXT_HELLO_WORLD);
        *textId = CUSTOM_MESSAGE_ID;
    });
}
