#include "PauseOwlWarp.h"
#include <libultraship/libultraship.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
    #include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

    extern f32 sPauseMenuVerticalOffset;
    extern u16 sCursorPointsToOcarinaModes[];
    extern u16 sOwlWarpPauseItems[];
    extern u16 D_80AF343C[];
}

static bool isConfirming = false;
static u16 sStickAdjTimer = 0;

void ResetMessageContext() {
    gPlayState->msgCtx.msgLength = 0;
    gPlayState->msgCtx.msgMode = MSGMODE_NONE;
}

void ClosePauseMenu(PauseContext* pauseCtx) {
    Interface_SetAButtonDoAction(gPlayState, DO_ACTION_NONE);
    Audio_PlaySfx_PauseMenuOpenOrClose(SFX_PAUSE_MENU_CLOSE);
    gPlayState->msgCtx.ocarinaMode = OCARINA_MODE_END;
    gSaveContext.prevHudVisibility = HUD_VISIBILITY_ALL;
    isConfirming = false;
    Message_CloseTextbox(gPlayState);
    pauseCtx->state = PAUSE_STATE_UNPAUSE_SETUP;
}

void HandleConfirmingState(PauseContext* pauseCtx, Input* input) {
    if (Message_ShouldAdvance(gPlayState)) {
        ResetMessageContext();
        isConfirming = false;
        if (gPlayState->msgCtx.choiceIndex == 0) { // Yes
            Interface_SetAButtonDoAction(gPlayState, DO_ACTION_NONE);
            Audio_PlaySfx_PauseMenuOpenOrClose(SFX_PAUSE_MENU_CLOSE);
            gPlayState->msgCtx.ocarinaMode = sCursorPointsToOcarinaModes[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]];
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            Message_CloseTextbox(gPlayState);
            sPauseMenuVerticalOffset = -6240.0f;
            pauseCtx->state = PAUSE_STATE_UNPAUSE_SETUP;
            gPlayState->nextEntrance = D_80AF343C[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]];
            gPlayState->transitionTrigger = TRANS_TRIGGER_START;
            gPlayState->transitionType = TRANS_TYPE_FADE_WHITE;
        } else { // No
            Interface_SetAButtonDoAction(gPlayState, DO_ACTION_WARP);
            Audio_PlaySfx(NA_SE_SY_MESSAGE_PASS);
            Message_CloseTextbox(gPlayState);
        }
    } else if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
        ResetMessageContext();
        isConfirming = false;
        Audio_PlaySfx(NA_SE_SY_MESSAGE_PASS);
        Message_CloseTextbox(gPlayState);
    } else if (CHECK_BTN_ALL(input->press.button, BTN_START)) {
        ClosePauseMenu(pauseCtx);
    }
}

void UpdateCursorForOwlWarpPoints(PauseContext* pauseCtx) {
    if (pauseCtx->cursorSpecialPos == 0) {
        if (pauseCtx->stickAdjX > 30) {
            pauseCtx->cursorShrinkRate = 4.0f;
            sStickAdjTimer = 0;
            do {
                pauseCtx->cursorPoint[PAUSE_WORLD_MAP]++;
                if (pauseCtx->cursorPoint[PAUSE_WORLD_MAP] > OWL_WARP_STONE_TOWER) {
                    KaleidoScope_MoveCursorToSpecialPos(gPlayState, PAUSE_CURSOR_PAGE_RIGHT);
                    return;
                }
            } while (!pauseCtx->worldMapPoints[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]]);
            pauseCtx->cursorItem[PAUSE_MAP] = sOwlWarpPauseItems[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]] - ITEM_MAP_POINT_GREAT_BAY;
            pauseCtx->cursorSlot[PAUSE_MAP] = 31 + pauseCtx->cursorPoint[PAUSE_WORLD_MAP];
            Audio_PlaySfx(NA_SE_SY_CURSOR);
        } else if (pauseCtx->stickAdjX < -30) {
            pauseCtx->cursorShrinkRate = 4.0f;
            sStickAdjTimer = 0;
            do {
                pauseCtx->cursorPoint[PAUSE_WORLD_MAP]--;
                if (pauseCtx->cursorPoint[PAUSE_WORLD_MAP] < OWL_WARP_GREAT_BAY_COAST) {
                    KaleidoScope_MoveCursorToSpecialPos(gPlayState, PAUSE_CURSOR_PAGE_LEFT);
                    return;
                }
            } while (!pauseCtx->worldMapPoints[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]]);
            pauseCtx->cursorItem[PAUSE_MAP] = sOwlWarpPauseItems[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]] - ITEM_MAP_POINT_GREAT_BAY;
            pauseCtx->cursorSlot[PAUSE_MAP] = 31 + pauseCtx->cursorPoint[PAUSE_WORLD_MAP];
            Audio_PlaySfx(NA_SE_SY_CURSOR);
        } else {
            sStickAdjTimer++;
        }
    } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT && pauseCtx->stickAdjX > 30) {
        KaleidoScope_MoveCursorFromSpecialPos(gPlayState);
        pauseCtx->cursorPoint[PAUSE_WORLD_MAP] = OWL_WARP_GREAT_BAY_COAST;
        pauseCtx->cursorItem[PAUSE_MAP] = sOwlWarpPauseItems[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]] - ITEM_MAP_POINT_GREAT_BAY;
        pauseCtx->cursorSlot[PAUSE_MAP] = 31 + pauseCtx->cursorPoint[PAUSE_WORLD_MAP];
        KaleidoScope_UpdateWorldMapCursor(gPlayState);
        KaleidoScope_UpdateNamePanel(gPlayState);
    } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT && pauseCtx->stickAdjX < -30) {
        KaleidoScope_MoveCursorFromSpecialPos(gPlayState);
        pauseCtx->cursorPoint[PAUSE_WORLD_MAP] = OWL_WARP_STONE_TOWER;
        pauseCtx->cursorItem[PAUSE_MAP] = sOwlWarpPauseItems[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]] - ITEM_MAP_POINT_GREAT_BAY;
        pauseCtx->cursorSlot[PAUSE_MAP] = 31 + pauseCtx->cursorPoint[PAUSE_WORLD_MAP];
        KaleidoScope_UpdateWorldMapCursor(gPlayState);
        KaleidoScope_UpdateNamePanel(gPlayState);
    }
}

void RegisterPauseOwlWarp() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnKaleidoUpdate>([](PauseContext* pauseCtx) {
        if (CVarGetInteger("gEnhancements.Songs.PauseOwlWarp", 0)) {
            Player* player = GET_PLAYER(gPlayState);
            Input* input = &gPlayState->state.input[0];

            if ((pauseCtx->state == PAUSE_STATE_MAIN && pauseCtx->pageIndex == PAUSE_MAP && pauseCtx->mapPageRoll == 0) || isConfirming) {
                if (CHECK_BTN_ALL(input->press.button, BTN_START) || CHECK_BTN_ALL(input->press.button, BTN_B)) {
                    ClosePauseMenu(pauseCtx);
                } else if (CHECK_BTN_ALL(input->press.button, BTN_A) && !isConfirming) {
                    if (pauseCtx->cursorSpecialPos == 0 && pauseCtx->worldMapPoints[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]]) {
                        Audio_PlaySfx(NA_SE_SY_DECIDE);
                        Message_StartTextbox(gPlayState, 0x1B93, NULL);
                        isConfirming = true;
                    }
                } else if (isConfirming) {
                    HandleConfirmingState(pauseCtx, input);
                } else {
                    KaleidoScope_UpdateOwlWarpNamePanel(gPlayState);
                }

                UpdateCursorForOwlWarpPoints(pauseCtx);
            }
        }
    });
}
