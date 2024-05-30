#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "z64.h"
#include "global.h"

#include "overlays/actors/ovl_En_Ginko_Man/z_en_ginko_man.h"

void RegisterBankerDialogue() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(
        ACTOR_EN_GINKO_MAN, [](Actor* outerActor) {
            static uint32_t enGinkoUpdateHook = 0;
            static uint32_t enGinkoKillHook = 0;
            GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::OnActorUpdate>(enGinkoUpdateHook);
            GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enGinkoKillHook);
            enGinkoUpdateHook = 0;
            enGinkoKillHook = 0;

            enGinkoUpdateHook = GameInteractor::Instance->RegisterGameHookForPtr<GameInteractor::OnActorUpdate>(
                (uintptr_t)outerActor, [](Actor* actor) {
                    EnGinkoMan* enGinko = (EnGinkoMan*)actor;
                    if (CVarGetInteger("gEnhancements.Actor.BankerDepositRupees", 0)) {
                        if (gPlayState->msgCtx.currentTextId == 1104 ) {
                            if (CHECK_BTN_ALL(gPlayState->state.input[0].cur.button, BTN_Z) &&
                                gPlayState->msgCtx.bankRupeesSelected != gSaveContext.save.saveInfo.playerData.rupees) {
                                char firstChar = (gSaveContext.save.saveInfo.playerData.rupees / 100) + '0';
                                char secondChar = ((gSaveContext.save.saveInfo.playerData.rupees / 10) % 10) + '0';
                                char thirdChar = (gSaveContext.save.saveInfo.playerData.rupees % 10) + '0';
                                const char rupeeChar[3] = { firstChar, secondChar, thirdChar };

                                for (int i = 0; i <= 2; i++) {
                                    gPlayState->msgCtx.bankRupeesSelected =
                                        gPlayState->msgCtx.decodedBuffer.schar[gPlayState->msgCtx.unk120C0 + i] =
                                            rupeeChar[i];
                                    Font_LoadCharNES(
                                        gPlayState,
                                        gPlayState->msgCtx.decodedBuffer.schar[gPlayState->msgCtx.unk120C0 + i],
                                        gPlayState->msgCtx.unk120C4 + (i << 7));
                                }
                                Audio_PlaySfx(NA_SE_SY_RUPY_COUNT);
                            }
                            if (CHECK_BTN_ALL(gPlayState->state.input[0].cur.button, BTN_R) &&
                                gPlayState->msgCtx.bankRupeesSelected != 0) {
                                for (int i = 0; i <= 2; i++) {
                                    gPlayState->msgCtx.bankRupeesSelected =
                                        gPlayState->msgCtx.decodedBuffer.schar[gPlayState->msgCtx.unk120C0 + i] = '0';
                                    Font_LoadCharNES(
                                        gPlayState,
                                        gPlayState->msgCtx.decodedBuffer.schar[gPlayState->msgCtx.unk120C0 + i],
                                        gPlayState->msgCtx.unk120C4 + (i << 7));
                                }
                                Audio_PlaySfx(NA_SE_SY_RUPY_COUNT);
                            }
                        }
                    }
                });
            enGinkoKillHook =
                GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s8 sceneId, s8 spawnNum) {
                    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorUpdate>(enGinkoUpdateHook);
                    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enGinkoKillHook);
                    enGinkoUpdateHook = 0;
                    enGinkoKillHook = 0;
                });
        });
}