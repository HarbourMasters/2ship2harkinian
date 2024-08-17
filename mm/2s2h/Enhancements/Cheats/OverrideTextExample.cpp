#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
extern SaveContext gSaveContext;
}

// This is not actually used, but it's a good example of how to override text
// until we have good examples of usage. This will be deleted eventually.
void RegisterOverrideTextExample() {
    // Overrides the sign in south clock town message
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnOpenText>(0x1C18, [](u16* textId) {
        CustomMessage_SetActiveMessage(MOD_ID_SHIP, SHIP_TEXT_HELLO_WORLD);
        *textId = CUSTOM_MESSAGE_ID;
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnHandleCustomMessage>(
        [](s32 modId, s32 textId, std::string* msg) {
            if (modId != MOD_ID_SHIP || textId != SHIP_TEXT_HELLO_WORLD) {
                return;
            }

            CustomMessage_Replace(msg, "{{rupees}}", std::to_string(gSaveContext.save.saveInfo.playerData.rupees));
        });
}
