#include "PauseOwlWarp.h"
#include <libultraship/libultraship.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

extern f32 sPauseMenuVerticalOffset;
extern u16 sCursorPointsToOcarinaModes[];
extern u16 sOwlWarpPauseItems[];
extern u16 D_80AF343C[];
extern s16 sInDungeonScene;

bool IsOwlWarpEnabled() {
    return CVarGetInteger("gEnhancements.Songs.PauseOwlWarp", 0) && CHECK_QUEST_ITEM(QUEST_SONG_SOARING) &&
           (gSaveContext.save.saveInfo.playerData.owlActivationFlags != 0 ||
            gSaveContext.save.saveInfo.playerData.owlActivationFlags == (1 << 15));
}
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
            if (pauseCtx->worldMapPoints[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]]) {
                pauseCtx->cursorItem[PAUSE_MAP] =
                    sOwlWarpPauseItems[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]] - ITEM_MAP_POINT_GREAT_BAY;
                pauseCtx->cursorSlot[PAUSE_MAP] = 31 + pauseCtx->cursorPoint[PAUSE_WORLD_MAP];
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }
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
            if (pauseCtx->worldMapPoints[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]]) {
                pauseCtx->cursorItem[PAUSE_MAP] =
                    sOwlWarpPauseItems[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]] - ITEM_MAP_POINT_GREAT_BAY;
                pauseCtx->cursorSlot[PAUSE_MAP] = 31 + pauseCtx->cursorPoint[PAUSE_WORLD_MAP];
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }
        } else {
            sStickAdjTimer++;
        }
    } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT && pauseCtx->stickAdjX > 30) {
        KaleidoScope_MoveCursorFromSpecialPos(gPlayState);
        for (int i = OWL_WARP_GREAT_BAY_COAST; i <= OWL_WARP_STONE_TOWER; i++) {
            if (pauseCtx->worldMapPoints[i]) {
                pauseCtx->cursorPoint[PAUSE_WORLD_MAP] = i;
                pauseCtx->cursorItem[PAUSE_MAP] =
                    sOwlWarpPauseItems[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]] - ITEM_MAP_POINT_GREAT_BAY;
                pauseCtx->cursorSlot[PAUSE_MAP] = 31 + pauseCtx->cursorPoint[PAUSE_WORLD_MAP];
                KaleidoScope_UpdateWorldMapCursor(gPlayState);
                KaleidoScope_UpdateNamePanel(gPlayState);
                return;
            }
        }
        KaleidoScope_MoveCursorToSpecialPos(gPlayState, PAUSE_CURSOR_PAGE_RIGHT);
    } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT && pauseCtx->stickAdjX < -30) {
        KaleidoScope_MoveCursorFromSpecialPos(gPlayState);
        for (int i = OWL_WARP_STONE_TOWER; i >= OWL_WARP_GREAT_BAY_COAST; i--) {
            if (pauseCtx->worldMapPoints[i]) {
                pauseCtx->cursorPoint[PAUSE_WORLD_MAP] = i;
                pauseCtx->cursorItem[PAUSE_MAP] =
                    sOwlWarpPauseItems[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]] - ITEM_MAP_POINT_GREAT_BAY;
                pauseCtx->cursorSlot[PAUSE_MAP] = 31 + pauseCtx->cursorPoint[PAUSE_WORLD_MAP];
                KaleidoScope_UpdateWorldMapCursor(gPlayState);
                KaleidoScope_UpdateNamePanel(gPlayState);
                return;
            }
        }
        KaleidoScope_MoveCursorToSpecialPos(gPlayState, PAUSE_CURSOR_PAGE_LEFT);
    }
}

void RegisterPauseOwlWarp() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnKaleidoUpdate>([](PauseContext* pauseCtx) {
        if (!sInDungeonScene && IsOwlWarpEnabled() && CHECK_QUEST_ITEM(QUEST_SONG_SOARING)) {
            // Initialize worldMapPoints based on owl activation flags
            for (int i = OWL_WARP_STONE_TOWER; i >= OWL_WARP_GREAT_BAY_COAST; i--) {
                pauseCtx->worldMapPoints[i] = (gSaveContext.save.saveInfo.playerData.owlActivationFlags >> i) & 1;
            }

            // Special condition for when only the 15th owl statue is activated
            if (gSaveContext.save.saveInfo.playerData.owlActivationFlags == (1 << 15)) {
                for (int i = REGION_GREAT_BAY; i < REGION_MAX; i++) {
                    if ((gSaveContext.save.saveInfo.regionsVisited >> i) & 1) {
                        switch (i) {
                            case REGION_GREAT_BAY:
                                pauseCtx->worldMapPoints[OWL_WARP_GREAT_BAY_COAST] = true;
                                break;
                            case REGION_ZORA_HALL:
                                pauseCtx->worldMapPoints[OWL_WARP_ZORA_CAPE] = true;
                                break;
                            case REGION_ROMANI_RANCH:
                                pauseCtx->worldMapPoints[OWL_WARP_SNOWHEAD] = true;
                                break;
                            case REGION_DEKU_PALACE:
                                pauseCtx->worldMapPoints[OWL_WARP_MOUNTAIN_VILLAGE] = true;
                                break;
                            case REGION_WOODFALL:
                                pauseCtx->worldMapPoints[OWL_WARP_CLOCK_TOWN] = true;
                                break;
                            case REGION_CLOCK_TOWN:
                                pauseCtx->worldMapPoints[OWL_WARP_MILK_ROAD] = true;
                                break;
                            case REGION_SNOWHEAD:
                                pauseCtx->worldMapPoints[OWL_WARP_WOODFALL] = true;
                                break;
                            case REGION_IKANA_GRAVEYARD:
                                pauseCtx->worldMapPoints[OWL_WARP_SOUTHERN_SWAMP] = true;
                                break;
                            case REGION_IKANA_CANYON:
                                pauseCtx->worldMapPoints[OWL_WARP_IKANA_CANYON] = true;
                                break;
                            case REGION_GORON_VILLAGE:
                                pauseCtx->worldMapPoints[OWL_WARP_STONE_TOWER] = true;
                                break;
                        }
                    }
                }
            }

            // Ensure cursor starts at an activated point
            if (!pauseCtx->worldMapPoints[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]]) {
                for (int i = OWL_WARP_GREAT_BAY_COAST; i <= OWL_WARP_STONE_TOWER; i++) {
                    if (pauseCtx->worldMapPoints[i]) {
                        pauseCtx->cursorPoint[PAUSE_WORLD_MAP] = i;
                        break;
                    }
                }
            }

            Player* player = GET_PLAYER(gPlayState);
            Input* input = &gPlayState->state.input[0];

            if ((pauseCtx->state == PAUSE_STATE_MAIN && pauseCtx->pageIndex == PAUSE_MAP &&
                 pauseCtx->mapPageRoll == 0) ||
                isConfirming) {
                if (CHECK_BTN_ALL(input->press.button, BTN_START) || CHECK_BTN_ALL(input->press.button, BTN_B)) {
                    ClosePauseMenu(pauseCtx);
                } else if (CHECK_BTN_ALL(input->press.button, BTN_A) && !isConfirming) {
                    if (pauseCtx->cursorSpecialPos == 0 &&
                        pauseCtx->worldMapPoints[pauseCtx->cursorPoint[PAUSE_WORLD_MAP]]) {
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
