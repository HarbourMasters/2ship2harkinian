#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "z64.h"
#include "global.h"

#include "src\overlays\actors\ovl_En_Ginko_Man\z_en_ginko_man.h"

void RegisterBankerDialogue() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorUpdate>([](Actor* actor) {
        if (actor->id == ACTOR_EN_GINKO_MAN) {
            if (gPlayState->msgCtx.currentTextId == 1104) {
                if (CHECK_BTN_ALL(gPlayState->state.input[0].cur.button, BTN_Z)) {
                    char firstChar = (gSaveContext.save.saveInfo.playerData.rupees / 100) + '0';
                    char secondChar = ((gSaveContext.save.saveInfo.playerData.rupees / 10) % 10) + '0';
                    char thirdChar = (gSaveContext.save.saveInfo.playerData.rupees % 10) + '0';
                    const char rupeeChar[3] = { firstChar, secondChar, thirdChar };

                    for (int i = 22; i <= 24; i++) {
                        gPlayState->msgCtx.bankRupeesSelected = gPlayState->msgCtx.decodedBuffer.schar[i] = rupeeChar[i - 22];
                        Font_LoadCharNES(gPlayState, gPlayState->msgCtx.decodedBuffer.schar[i],
                                         gPlayState->msgCtx.unk120C4 + (0 + (i - 22) << 7));
                    }
                    Audio_PlaySfx(NA_SE_SY_RUPY_COUNT);
                }
                if (CHECK_BTN_ALL(gPlayState->state.input[0].cur.button, BTN_R)) {
                    for (int i = 22; i <= 24; i++) {
                        gPlayState->msgCtx.bankRupeesSelected = gPlayState->msgCtx.decodedBuffer.schar[i] = '0';
                        Font_LoadCharNES(gPlayState, gPlayState->msgCtx.decodedBuffer.schar[i],
                                         gPlayState->msgCtx.unk120C4 + (0 + (i - 22) << 7));
                    }
                    Audio_PlaySfx(NA_SE_SY_RUPY_COUNT);
                }
            }
        }
    });
}