/*
 * File: z_file_choose_JP.c
 * Overlay: ovl_file_choose
 * Description: Japanese version of z_file_choose_NES, file select screen
 */

#include "z_file_select.h"
#include "overlays/gamestates/ovl_opening/z_opening.h"
#include "z64rumble.h"
#include "z64save.h"
#include "z64shrink_window.h"
#include "z64view.h"
#include "interface/parameter_static/parameter_static.h"
#include "misc/title_static/title_static.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include <string.h>
#include "BenPort.h"

// FileSelect_SetView
void FileSelect_SetView_JP(FileSelectState* this, f32 eyeX, f32 eyeY, f32 eyeZ) {
    Vec3f eye;
    Vec3f lookAt;
    Vec3f up;

    eye.x = eyeX;
    eye.y = eyeY;
    eye.z = eyeZ;

    lookAt.x = lookAt.y = lookAt.z = 0.0f;

    up.x = up.z = 0.0f;
    up.y = 1.0f;

    View_LookAt(&this->view, &eye, &lookAt, &up);
    View_Apply(&this->view, VIEW_ALL | VIEW_FORCE_VIEWING | VIEW_FORCE_VIEWPORT | VIEW_FORCE_PROJECTION_PERSPECTIVE);
}

// FileSelect_DrawTexQuadIA8
Gfx* FileSelect_DrawTexQuadIA8_JP(Gfx* gfx, TexturePtr texture, s16 width, s16 height, s16 point) {
    gDPLoadTextureBlock(gfx++, texture, G_IM_FMT_IA, G_IM_SIZ_8b, width, height, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSP1Quadrangle(gfx++, point, point + 2, point + 3, point + 1, 0);

    return gfx;
}

// FileSelect_FadeInMenuElements
void FileSelect_FadeInMenuElements_JP(FileSelectState* this) {
    SramContext* sramCtx = &this->sramCtx;
    s16 i;

    this->titleAlpha[FS_TITLE_CUR] += 20;
    this->windowAlpha += 16;

    for (i = 0; i < 3; i++) {
        this->fileButtonAlpha[i] = this->windowAlpha;

        if (!gSaveContext.flashSaveAvailable) {
            if (NO_FLASH_SLOT_OCCUPIED(sramCtx, i)) {
                this->nameBoxAlpha[i] = this->nameAlpha[i] = this->windowAlpha;
                this->connectorAlpha[i] += 20;
                if (this->connectorAlpha[i] >= 255) {
                    this->connectorAlpha[i] = 255;
                }
            }
        } else if (SLOT_OCCUPIED(this, i)) {
            this->nameBoxAlpha[i] = this->nameAlpha[i] = this->windowAlpha;
            this->connectorAlpha[i] += 20;

            if (this->connectorAlpha[i] >= 255) {
                this->connectorAlpha[i] = 255;
            }
        }
    }

    this->actionButtonAlpha[FS_BTN_ACTION_COPY] = this->actionButtonAlpha[FS_BTN_ACTION_ERASE] =
        this->optionButtonAlpha = this->windowAlpha;
}

// FileSelect_SplitNumber
void FileSelect_SplitNumber_JP(u16 value, u16* hundreds, u16* tens, u16* ones) {
    *hundreds = 0;
    *tens = 0;
    *ones = value;

    do {
        if ((*ones - 100) < 0) {
            break;
        }
        (*hundreds)++;
        *ones -= 100;
    } while (true);

    do {
        if ((*ones - 10) < 0) {
            break;
        }
        (*tens)++;
        *ones -= 10;
    } while (true);
}

// FileSelect_StartFadeIn
void FileSelect_StartFadeIn_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    FileSelect_FadeInMenuElements_JP(this);
    this->screenFillAlpha -= 40;
    this->windowPosX -= 20;

    if (this->windowPosX <= -94) {
        this->windowPosX = -94;
        this->configMode = CM_FADE_IN_END;
        this->screenFillAlpha = 0;
    }
}

// FileSelect_FinishFadeIn
void FileSelect_FinishFadeIn_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->controlsAlpha += 20;
    FileSelect_FadeInMenuElements_JP(this);

    if (this->titleAlpha[FS_TITLE_CUR] >= 255) {
        this->titleAlpha[FS_TITLE_CUR] = 255;
        this->controlsAlpha = 255;
        this->windowAlpha = 200;
        this->configMode = CM_MAIN_MENU;
    }
}

// sEmptyName
u8 D_8081568C_cj0[8] = { 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF };

// FileSelect_UpdateMainMenu
void FileSelect_UpdateMainMenu_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;
    Input* input = CONTROLLER1(&this->state);

    if (CHECK_BTN_ALL(input->press.button, BTN_START) || CHECK_BTN_ALL(input->press.button, BTN_A)) {
        if (this->buttonIndex < 3) {
            if (!gSaveContext.flashSaveAvailable) {
                if (!NO_FLASH_SLOT_OCCUPIED(sramCtx, this->buttonIndex)) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                    this->configMode = 0x22;
                    this->kbdButton = 0x63;
                    this->charPage = 0;
                    if (/*gSaveContext.options.language != LANGUAGE_JPN*/ false) {
                        this->charPage = 2;
                    }
                    this->kbdX = 0;
                    this->kbdY = 0;
                    this->charIndex = 0;
                    this->charBgAlpha = 0;
                    this->newFileNameCharCount = 0;
                    this->nameEntryBoxPosX = 0x78;
                    this->nameEntryBoxAlpha = 0;
                    memcpy(this->fileNames[this->buttonIndex], &D_8081568C_cj0, sizeof(D_8081568C_cj0));
                } else {
                    Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                    this->actionTimer = 4;
                    this->selectMode = 0;
                    this->selectedFileIndex = this->buttonIndex;
                    this->menuMode = 1;
                    this->nextTitleLabel = 1;
                }

            } else if (!SLOT_OCCUPIED(this, this->buttonIndex)) {
                Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                this->configMode = 0x22;
                this->kbdButton = 0x63;
                this->charPage = 0;
                if (/*gSaveContext.options.language != LANGUAGE_JPN*/ false) {
                    this->charPage = 2;
                }
                this->kbdX = 0;
                this->kbdY = 0;
                this->charIndex = 0;
                this->charBgAlpha = 0;
                this->newFileNameCharCount = 0;
                this->nameEntryBoxPosX = 0x78;
                this->nameEntryBoxAlpha = 0;
                memcpy(this->fileNames[this->buttonIndex], &D_8081568C_cj0, sizeof(D_8081568C_cj0));
            } else {
                Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                this->actionTimer = 4;
                this->selectMode = 0;
                this->selectedFileIndex = this->buttonIndex;
                this->menuMode = 1;
                this->nextTitleLabel = 1;
            }
        } else if (this->warningLabel == -1) {
            Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
            this->prevConfigMode = this->configMode;
            if (this->buttonIndex == 3) {
                this->configMode = 3;
                this->nextTitleLabel = 2;
            } else if (this->buttonIndex == 4) {
                this->configMode = 0x15;
                this->nextTitleLabel = 6;
            } else {
                this->configMode = 0x27;
                this->kbdButton = 0;
                this->kbdX = 0;
                this->kbdY = 0;
                this->charBgAlpha = 0;
                this->newFileNameCharCount = 0;
                this->nameEntryBoxPosX = 0x78;
            }
            this->actionTimer = 4;
        } else {
            Audio_PlaySfx(NA_SE_SY_FSEL_ERROR);
        }
    } else if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
        gSaveContext.gameMode = GAMEMODE_TITLE_SCREEN;
        STOP_GAMESTATE(&this->state);
        SET_NEXT_GAMESTATE(&this->state, TitleSetup_Init, 0x210);
    } else {

        if (ABS_ALT(this->stickAdjY) > 30) {
            Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
            if (this->stickAdjY > 30) {
                this->buttonIndex--;
                if (this->buttonIndex == 2) {
                    this->buttonIndex = 1;
                }
                if (this->buttonIndex < 0) {
                    this->buttonIndex = 5;
                }
            } else {
                this->buttonIndex++;
                if (this->buttonIndex == 2) {
                    this->buttonIndex = 3;
                }
                if (this->buttonIndex >= 6) {
                    this->buttonIndex = 0;
                }
            }
        }
        if (this->buttonIndex == 3) {
            if (gSaveContext.flashSaveAvailable == 0) {
                if (!NO_FLASH_SLOT_OCCUPIED(sramCtx, 0) && !NO_FLASH_SLOT_OCCUPIED(sramCtx, 1) &&
                    !NO_FLASH_SLOT_OCCUPIED(sramCtx, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_FILE_COPY;
                    this->emptyFileTextAlpha = 255;
                } else if (NO_FLASH_SLOT_OCCUPIED(sramCtx, 0) && NO_FLASH_SLOT_OCCUPIED(sramCtx, 1) &&
                           NO_FLASH_SLOT_OCCUPIED(sramCtx, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_EMPTY_FILES;
                    this->emptyFileTextAlpha = 255;
                } else {
                    this->warningLabel = FS_WARNING_NONE;
                }
            } else {
                if (!SLOT_OCCUPIED(this, 0) && !SLOT_OCCUPIED(this, 1)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_FILE_COPY;
                    this->emptyFileTextAlpha = 255;
                } else if (SLOT_OCCUPIED(this, 0) && SLOT_OCCUPIED(this, 1)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_EMPTY_FILES;
                    this->emptyFileTextAlpha = 255;
                } else {
                    this->warningLabel = FS_WARNING_NONE;
                }
            }
        } else if (this->buttonIndex == 4) {
            if (!gSaveContext.flashSaveAvailable) {
                if (!NO_FLASH_SLOT_OCCUPIED(sramCtx, 0) && !NO_FLASH_SLOT_OCCUPIED(sramCtx, 1) &&
                    !NO_FLASH_SLOT_OCCUPIED(sramCtx, 2)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_FILE_ERASE;
                    this->emptyFileTextAlpha = 255;
                } else {
                    this->warningLabel = FS_WARNING_NONE;
                }
            } else {
                if (!SLOT_OCCUPIED(this, 0) && !SLOT_OCCUPIED(this, 1)) {
                    this->warningButtonIndex = this->buttonIndex;
                    this->warningLabel = FS_WARNING_NO_FILE_ERASE;
                    this->emptyFileTextAlpha = 255;
                } else {
                    this->warningLabel = FS_WARNING_NONE;
                }
            }
        } else {
            this->warningLabel = -1;
        }
    }
}

// FileSelect_UnusedCM31
void FileSelect_UnusedCM31_JP(s32 arg0) {
}

// FileSelect_UnusedCMDelay
void FileSelect_UnusedCMDelay_JP(GameState* thisx) {
    static s16 D_80815694_cj0 = 0; // D_80814564
    FileSelectState* this = (FileSelectState*)thisx;

    D_80815694_cj0 += 2;

    if (D_80815694_cj0 == 254) {
        this->configMode = this->nextConfigMode;
        D_80815694_cj0 = 0;
    }
}

// FileSelect_RotateToNameEntry
void FileSelect_RotateToNameEntry_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->windowRot += 50.0f;

    if (this->windowRot >= 314.0f) {
        this->windowRot = 314.0f;
        this->configMode = CM_START_NAME_ENTRY;
    }
}

// FileSelect_RotateToOptions
void FileSelect_RotateToOptions_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->windowRot += 50.0f;

    if (this->windowRot >= 314.0f) {
        this->windowRot = 314.0f;
        this->configMode = CM_START_OPTIONS;
    }
}

// FileSelect_RotateToMain
void FileSelect_RotateToMain_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->windowRot += 50.0f;

    if (this->windowRot >= 628.0f) {
        this->windowRot = 0.0f;
        this->configMode = CM_MAIN_MENU;
    }
}

// sCursorAlphaTargets
s16 D_8081574C_cj0[] = { 70, 200 };

// FileSelect_PulsateCursor
void FileSelect_PulsateCursor_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    s32 alphaStep = ABS_ALT(this->highlightColor[3] - D_8081574C_cj0[this->highlightPulseDir]) / this->highlightTimer;

    if (this->highlightColor[3] >= D_8081574C_cj0[this->highlightPulseDir]) {
        this->highlightColor[3] -= alphaStep;
    } else {
        this->highlightColor[3] += alphaStep;
    }

    this->highlightTimer--;

    if (this->highlightTimer == 0) {
        this->highlightColor[3] = D_8081574C_cj0[this->highlightPulseDir];
        this->highlightTimer = 20;
        this->highlightPulseDir ^= 1;
    }
}

// sConfigModeUpdateFuncs
void (*D_80815698_cj0[])(GameState*) = {
    FileSelect_StartFadeIn_JP,
    FileSelect_FinishFadeIn_JP,
    FileSelect_UpdateMainMenu_JP,
    FileSelect_SetupCopySource,
    FileSelect_SelectCopySource,
    FileSelect_SetupCopyDest1,
    FileSelect_SetupCopyDest2,
    FileSelect_SelectCopyDest,
    FileSelect_ExitToCopySource1,
    FileSelect_ExitToCopySource2,
    FileSelect_SetupCopyConfirm1,
    FileSelect_SetupCopyConfirm2,
    FileSelect_CopyConfirm,
    FileSelect_CopyWaitForFlashSave,
    FileSelect_ReturnToCopyDest,
    FileSelect_CopyAnim1,
    FileSelect_CopyAnim2,
    FileSelect_CopyAnim3,
    FileSelect_CopyAnim4,
    FileSelect_CopyAnim5,
    FileSelect_ExitCopyToMain,
    FileSelect_SetupEraseSelect,
    FileSelect_EraseSelect,
    FileSelect_SetupEraseConfirm1,
    FileSelect_SetupEraseConfirm2,
    FileSelect_EraseConfirm,
    FileSelect_ExitToEraseSelect1,
    FileSelect_ExitToEraseSelect2,
    FileSelect_EraseAnim1,
    FileSelect_EraseWaitForFlashSave,
    FileSelect_EraseAnim2,
    FileSelect_EraseAnim3,
    FileSelect_ExitEraseToMain,
    FileSelect_UnusedCM31_JP,
    FileSelect_RotateToNameEntry_JP,
    FileSelect_StartNameEntry_JP,
    FileSelect_UpdateKeyboardCursor_JP,
    FileSelect_NameEntryWaitForFlashSave_JP,
    FileSelect_RotateToMain_JP,
    FileSelect_RotateToOptions_JP,
    FileSelect_StartOptions_JP,
    FileSelect_UpdateOptionsMenu_JP,
    FileSelect_OptionsWaitForFlashSave_JP,
    FileSelect_RotateToMain_JP,
    FileSelect_UnusedCMDelay_JP,
};

// FileSelect_ConfigModeUpdate
void FileSelect_ConfigModeUpdate_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    D_80815698_cj0[this->configMode](&this->state);
}

// FileSelect_SetWindowVtx
void FileSelect_SetWindowVtx_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    s16 i;
    s16 j;
    s16 x;
    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    this->windowVtx = GRAPH_ALLOC(this->state.gfxCtx, 80 * sizeof(Vtx));
    tmp = this->windowPosX - 90;

    for (x = 0, i = 0; i < 4; i++) {
        tmp += 0x40;
        tmp2 = (i == 3) ? 0x30 : 0x40;

        for (j = 0, tmp3 = 0x50; j < 5; j++, x += 4, tmp3 -= 0x20) {
            this->windowVtx[x].v.ob[0] = this->windowVtx[x + 2].v.ob[0] = tmp;

            this->windowVtx[x + 1].v.ob[0] = this->windowVtx[x + 3].v.ob[0] = tmp2 + tmp;

            this->windowVtx[x].v.ob[1] = this->windowVtx[x + 1].v.ob[1] = tmp3;

            this->windowVtx[x + 2].v.ob[1] = this->windowVtx[x + 3].v.ob[1] = tmp3 - 0x20;

            this->windowVtx[x].v.ob[2] = this->windowVtx[x + 1].v.ob[2] = this->windowVtx[x + 2].v.ob[2] =
                this->windowVtx[x + 3].v.ob[2] = 0;

            this->windowVtx[x].v.flag = this->windowVtx[x + 1].v.flag = this->windowVtx[x + 2].v.flag =
                this->windowVtx[x + 3].v.flag = 0;

            this->windowVtx[x].v.tc[0] = this->windowVtx[x].v.tc[1] = this->windowVtx[x + 1].v.tc[1] =
                this->windowVtx[x + 2].v.tc[0] = 0;

            this->windowVtx[x + 1].v.tc[0] = this->windowVtx[x + 3].v.tc[0] = tmp2 << 5;

            this->windowVtx[x + 2].v.tc[1] = this->windowVtx[x + 3].v.tc[1] = 32 << 5;

            this->windowVtx[x].v.cn[0] = this->windowVtx[x + 2].v.cn[0] = this->windowVtx[x].v.cn[1] =
                this->windowVtx[x + 2].v.cn[1] = this->windowVtx[x].v.cn[2] = this->windowVtx[x + 2].v.cn[2] =
                    this->windowVtx[x + 1].v.cn[0] = this->windowVtx[x + 3].v.cn[0] = this->windowVtx[x + 1].v.cn[1] =
                        this->windowVtx[x + 3].v.cn[1] = this->windowVtx[x + 1].v.cn[2] =
                            this->windowVtx[x + 3].v.cn[2] = this->windowVtx[x].v.cn[3] =
                                this->windowVtx[x + 2].v.cn[3] = this->windowVtx[x + 1].v.cn[3] =
                                    this->windowVtx[x + 3].v.cn[3] = 255;
        }
    }
}

// sFileInfoBoxPartWidths
s16 D_80815668_cj0[] = { 36, 36, 36, 36, 24, 28, 28 };

// sWalletFirstDigit
s16 D_80815684_cj0[] = {
    1, // tens (Default Wallet)
    0, // hundreds (Adult Wallet)
    0, // hundreds (Giant Wallet)
};

// D_80814620
s16 D_80815750_cj0[] = { 8, 8, 8, 0 };

// D_80814628
s16 D_80815758_cj0[] = { 12, 12, 12, 0 };

// D_80814630
s16 D_80815760_cj0[] = { 12, 12, 12, 0 };

// D_80814638
s16 D_80815768_cj0[] = { 88, 104, 120, 940, 944, 948 };

// D_80814644
s16 D_80815774_cj0[] = { 88, 104, 120, 944 };

// D_8081464C
s16 D_8081577C_cj0[] = { 940, 944 };

// D_80814650
s16 D_80815780_cj0[] = { 940, 944 };

// D_80814280
s16 D_80815784_cj0[] = {
    2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 0, 1, 1, 2, 1, 1, 4, 2, 2, 2, 1, 1, 0, 2, 0, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 2, 2, 2, 2, 2, 3, 2, 2, 4, 3, 2, 4, 1, 2, 2, 1, 1, 2, 2, 3, 2, 2, 0, 2, 2, 2, 0, 3, 1, 0,
};

// FileSelect_SetWindowContentVtx
void FileSelect_SetWindowContentVtx_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    u16 vtxId;
    s16 j;
    s16 i;
    s16 spAC;
    u16 spA4[3];
    s32 index;
    s32 posY;
    s32 relPosY;
    s32 posX;
    s32 tempPosY;

    this->windowContentVtx = GRAPH_ALLOC(this->state.gfxCtx, 960 * sizeof(Vtx));

    for (vtxId = 0; vtxId < 960; vtxId += 4) {
        this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = 0x12C;

        this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 16;

        this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = 0;

        this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
            this->windowContentVtx[vtxId + 0].v.ob[1] - 16;

        this->windowContentVtx[vtxId + 0].v.ob[2] = this->windowContentVtx[vtxId + 1].v.ob[2] =
            this->windowContentVtx[vtxId + 2].v.ob[2] = this->windowContentVtx[vtxId + 3].v.ob[2] = 0;

        this->windowContentVtx[vtxId + 0].v.flag = this->windowContentVtx[vtxId + 1].v.flag =
            this->windowContentVtx[vtxId + 2].v.flag = this->windowContentVtx[vtxId + 3].v.flag = 0;

        this->windowContentVtx[vtxId + 0].v.tc[0] = this->windowContentVtx[vtxId + 0].v.tc[1] =
            this->windowContentVtx[vtxId + 1].v.tc[1] = this->windowContentVtx[vtxId + 2].v.tc[0] = 0;

        this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 2].v.tc[1] =
            this->windowContentVtx[vtxId + 3].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x200;

        this->windowContentVtx[vtxId + 0].v.cn[0] = this->windowContentVtx[vtxId + 1].v.cn[0] =
            this->windowContentVtx[vtxId + 2].v.cn[0] = this->windowContentVtx[vtxId + 3].v.cn[0] =
                this->windowContentVtx[vtxId + 0].v.cn[1] = this->windowContentVtx[vtxId + 1].v.cn[1] =
                    this->windowContentVtx[vtxId + 2].v.cn[1] = this->windowContentVtx[vtxId + 3].v.cn[1] =
                        this->windowContentVtx[vtxId + 0].v.cn[2] = this->windowContentVtx[vtxId + 1].v.cn[2] =
                            this->windowContentVtx[vtxId + 2].v.cn[2] = this->windowContentVtx[vtxId + 3].v.cn[2] =
                                this->windowContentVtx[vtxId + 0].v.cn[3] = this->windowContentVtx[vtxId + 1].v.cn[3] =
                                    this->windowContentVtx[vtxId + 2].v.cn[3] =
                                        this->windowContentVtx[vtxId + 3].v.cn[3] = 255;
    }

    this->windowContentVtx[0].v.ob[0] = this->windowContentVtx[2].v.ob[0] = this->windowPosX;

    this->windowContentVtx[1].v.ob[0] = this->windowContentVtx[3].v.ob[0] = this->windowContentVtx[0].v.ob[0] + 0x80;

    this->windowContentVtx[0].v.ob[1] = this->windowContentVtx[1].v.ob[1] = 0x48;

    this->windowContentVtx[2].v.ob[1] = this->windowContentVtx[3].v.ob[1] = this->windowContentVtx[0].v.ob[1] - 0x10;

    this->windowContentVtx[1].v.tc[0] = this->windowContentVtx[3].v.tc[0] = 0x1000;

    for (vtxId = 4, i = 0; i < 3; i++) {
        posX = this->windowPosX - 6;

        for (j = 0; j < 7; j++, vtxId += 4) {
            this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX;

            this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + D_80815668_cj0[j];

            this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] =
                this->fileNamesY[i] + 0x2C;

            this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
                this->windowContentVtx[vtxId + 0].v.ob[1] - 0x38;

            this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = D_80815668_cj0[j]
                                                                                                    << 5;

            this->windowContentVtx[vtxId + 2].v.tc[1] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x700;

            posX += D_80815668_cj0[j];
        }
    }

    posX = this->windowPosX - 6;
    posY = 44;

    for (j = 0; j < 3; j++, vtxId += 16, posY -= 0x10) {
        this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX;

        this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 64;

        this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] =
            this->buttonYOffsets[j] + posY;

        this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
            this->windowContentVtx[vtxId + 0].v.ob[1] - 16;

        this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x800;

        this->windowContentVtx[vtxId + 4].v.ob[0] = this->windowContentVtx[vtxId + 6].v.ob[0] = posX + 0x40;

        this->windowContentVtx[vtxId + 5].v.ob[0] = this->windowContentVtx[vtxId + 7].v.ob[0] =
            this->windowContentVtx[vtxId + 4].v.ob[0] + 108;

        this->windowContentVtx[vtxId + 4].v.ob[1] = this->windowContentVtx[vtxId + 5].v.ob[1] =
            this->buttonYOffsets[j] + posY;

        this->windowContentVtx[vtxId + 6].v.ob[1] = this->windowContentVtx[vtxId + 7].v.ob[1] =
            this->windowContentVtx[vtxId + 4].v.ob[1] - 16;

        this->windowContentVtx[vtxId + 5].v.tc[0] = this->windowContentVtx[vtxId + 7].v.tc[0] = 0xD80;

        this->windowContentVtx[vtxId + 8].v.ob[0] = this->windowContentVtx[vtxId + 10].v.ob[0] = posX + 52;

        this->windowContentVtx[vtxId + 9].v.ob[0] = this->windowContentVtx[vtxId + 11].v.ob[0] =
            this->windowContentVtx[vtxId + 8].v.ob[0] + 0x18;

        this->windowContentVtx[vtxId + 8].v.ob[1] = this->windowContentVtx[vtxId + 9].v.ob[1] =
            this->buttonYOffsets[j] + posY;

        this->windowContentVtx[vtxId + 10].v.ob[1] = this->windowContentVtx[vtxId + 11].v.ob[1] =
            this->windowContentVtx[vtxId + 8].v.ob[1] - 0x10;

        this->windowContentVtx[vtxId + 9].v.tc[0] = this->windowContentVtx[vtxId + 11].v.tc[0] = 0x300;

        this->windowContentVtx[vtxId + 12].v.ob[0] = this->windowContentVtx[vtxId + 14].v.ob[0] = posX + 0xA9;

        this->windowContentVtx[vtxId + 13].v.ob[0] = this->windowContentVtx[vtxId + 15].v.ob[0] =
            this->windowContentVtx[vtxId + 12].v.ob[0] + 52;

        this->windowContentVtx[vtxId + 12].v.ob[1] = this->windowContentVtx[vtxId + 13].v.ob[1] =
            this->buttonYOffsets[j] + posY;

        this->windowContentVtx[vtxId + 14].v.ob[1] = this->windowContentVtx[vtxId + 15].v.ob[1] =
            this->windowContentVtx[vtxId + 12].v.ob[1] - 16;

        this->windowContentVtx[vtxId + 13].v.tc[0] = this->windowContentVtx[vtxId + 15].v.tc[0] = 0x680;
    }

    posY = 44;

    for (j = 0; j < 3; j++, posY -= 16) {
        if (!gSaveContext.flashSaveAvailable) {
            continue;
        }
        spAC = j;
        if (this->isOwlSave[j + 2]) {
            spAC = j + 2;
        }

        posX = this->windowPosX - 6;
        if (this->configMode == 0x10 && j == this->copyDestFileIndex) {
            relPosY = this->fileNamesY[j] + 44;
        } else if ((this->configMode == 0x11 || this->configMode == 0x12) && j == this->copyDestFileIndex) {
            relPosY = this->buttonYOffsets[j] + posY;
        } else {
            relPosY = posY + this->buttonYOffsets[j] + this->fileNamesY[j];
        }

        tempPosY = relPosY - 2;

        for (i = 0; i < 8; i++, vtxId += 4) {
            this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX + 0x4E;

            this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + 11;

            this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
                this->windowContentVtx[vtxId + 0].v.ob[1] - 12;

            this->windowContentVtx[vtxId + 32].v.ob[0] = this->windowContentVtx[vtxId + 34].v.ob[0] = posX + 0x4F;

            this->windowContentVtx[vtxId + 33].v.ob[0] = this->windowContentVtx[vtxId + 35].v.ob[0] =
                this->windowContentVtx[vtxId + 32].v.ob[0] + 11;

            this->windowContentVtx[vtxId + 32].v.ob[1] = this->windowContentVtx[vtxId + 33].v.ob[1] = tempPosY - 1;

            this->windowContentVtx[vtxId + 34].v.ob[1] = this->windowContentVtx[vtxId + 35].v.ob[1] =
                this->windowContentVtx[vtxId + 32].v.ob[1] - 12;

            posX += 10;
        }

        vtxId += 32;
        posX = this->windowPosX + 14;
        tempPosY = relPosY - 0x18;

        FileSelect_SplitNumber_JP(this->rupees[spAC], &spA4[0], &spA4[1], &spA4[2]);

        index = D_80815684_cj0[this->walletUpgrades[spAC]];

        for (i = 0; i < 3; i++, vtxId += 4, index++) {
            this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] =
                D_80815784_cj0[spA4[index]] + posX;

            this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + D_80815758_cj0[i];

            this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
                this->windowContentVtx[vtxId + 0].v.ob[1] - D_80815760_cj0[i];

            this->windowContentVtx[vtxId + 12].v.ob[0] = this->windowContentVtx[vtxId + 14].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + 1;

            this->windowContentVtx[vtxId + 13].v.ob[0] = this->windowContentVtx[vtxId + 15].v.ob[0] =
                this->windowContentVtx[vtxId + 12].v.ob[0] + D_80815758_cj0[i];

            this->windowContentVtx[vtxId + 12].v.ob[1] = this->windowContentVtx[vtxId + 13].v.ob[1] = tempPosY - 1;

            this->windowContentVtx[vtxId + 14].v.ob[1] = this->windowContentVtx[vtxId + 15].v.ob[1] =
                this->windowContentVtx[vtxId + 12].v.ob[1] - D_80815760_cj0[i];

            posX += D_80815750_cj0[i];
        }
        vtxId += 12;
        posX = this->windowPosX + 0x2A;
        tempPosY = relPosY - 0x2A;
        FileSelect_SplitNumber_JP(this->maskCount[spAC], &spA4[0], &spA4[1], &spA4[2]);

        for (i = 1; i < 3; i++, vtxId += 4) {

            this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] =
                D_80815784_cj0[spA4[i]] + posX;

            this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + D_80815758_cj0[i];

            this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
                this->windowContentVtx[vtxId + 0].v.ob[1] - D_80815760_cj0[i];

            this->windowContentVtx[vtxId + 8].v.ob[0] = this->windowContentVtx[vtxId + 10].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + 1;

            this->windowContentVtx[vtxId + 9].v.ob[0] = this->windowContentVtx[vtxId + 11].v.ob[0] =
                this->windowContentVtx[vtxId + 8].v.ob[0] + D_80815758_cj0[i];

            this->windowContentVtx[vtxId + 8].v.ob[1] = this->windowContentVtx[vtxId + 9].v.ob[1] = tempPosY - 1;

            this->windowContentVtx[vtxId + 10].v.ob[1] = this->windowContentVtx[vtxId + 11].v.ob[1] =
                this->windowContentVtx[vtxId + 8].v.ob[1] - D_80815760_cj0[i];
            posX += D_80815750_cj0[i];
        }
        vtxId += 8;

        tempPosY = relPosY - 0x10;
        posX = this->windowPosX + 0x3F;

        for (i = 0; i < 20; i++, vtxId += 4) {
            this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX;

            this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + 10;
            this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
                this->windowContentVtx[vtxId + 0].v.ob[1] - 10;
            if (i == 9) {
                tempPosY -= 8;
                posX = this->windowPosX + 0x36;
            }
            posX += 9;
        }

        tempPosY = relPosY - 0x20;
        posX = this->windowPosX + 0x40;

        for (i = 0; i < 4; i++, vtxId += 4) {
            this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX;

            this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + 0x14;

            this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

            this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
                this->windowContentVtx[vtxId + 0].v.ob[1] - 0x14;

            this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 2].v.tc[1] =
                this->windowContentVtx[vtxId + 3].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x400;
            posX += 0x18;
        }

        tempPosY = relPosY - 0x15;

        this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = this->windowPosX - 1;

        this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 16;

        this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

        this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
            this->windowContentVtx[vtxId + 0].v.ob[1] - 16;

        this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x200;

        this->windowContentVtx[vtxId + 2].v.tc[1] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x200;

        vtxId += 4;

        this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = this->windowPosX + 0x27;

        this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 24;

        this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

        this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
            this->windowContentVtx[vtxId + 0].v.ob[1] - 16;

        this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x300;

        this->windowContentVtx[vtxId + 2].v.tc[1] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x200;

        tempPosY = relPosY - 0x27;

        vtxId += 4;

        this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = this->windowPosX - 10;

        this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 64;

        this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY;

        this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
            this->windowContentVtx[vtxId + 0].v.ob[1] - 16;

        this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x800;

        this->windowContentVtx[vtxId + 2].v.tc[1] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x200;

        this->windowContentVtx[vtxId + 4].v.ob[0] = this->windowContentVtx[vtxId + 6].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 1;

        this->windowContentVtx[vtxId + 5].v.ob[0] = this->windowContentVtx[vtxId + 7].v.ob[0] =
            this->windowContentVtx[vtxId + 4].v.ob[0] + 64;

        this->windowContentVtx[vtxId + 4].v.ob[1] = this->windowContentVtx[vtxId + 5].v.ob[1] = tempPosY - 1;

        this->windowContentVtx[vtxId + 6].v.ob[1] = this->windowContentVtx[vtxId + 7].v.ob[1] =
            this->windowContentVtx[vtxId + 4].v.ob[1] - 16;

        this->windowContentVtx[vtxId + 5].v.tc[0] = this->windowContentVtx[vtxId + 7].v.tc[0] = 0x800;

        this->windowContentVtx[vtxId + 6].v.tc[1] = this->windowContentVtx[vtxId + 7].v.tc[1] = 0x200;

        vtxId += 8;

        posX = this->windowPosX + 0xA3;

        if ((this->configMode == 0x10) && (j == this->copyDestFileIndex)) {
            tempPosY = this->fileNamesY[j] + 0x2C;
        } else if (((this->configMode == 0x11) || (this->configMode == 0x12)) && (j == this->copyDestFileIndex)) {
            tempPosY = this->buttonYOffsets[j] + posY;
        } else {
            tempPosY = posY + this->buttonYOffsets[j] + this->fileNamesY[j];
        }

        this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX + 14;

        this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 24;

        this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY - 2;

        this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
            this->windowContentVtx[vtxId + 0].v.ob[1] - 12;

        this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x300;

        this->windowContentVtx[vtxId + 2].v.tc[1] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x180;

        vtxId += 4;

        for (i = 0; i < 2; i++, vtxId += 4) {

            this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX + 2 + i;

            this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + 0x30;

            this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] =
                (tempPosY - i) - 0x12;

            this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
                this->windowContentVtx[vtxId + 0].v.ob[1] - 0x18;

            this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x600;

            this->windowContentVtx[vtxId + 2].v.tc[1] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x300;
        }
        posX += 6;
        index = vtxId;

        for (i = 0; i < 5; i++, vtxId += 4) {
            this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX;

            this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
                this->windowContentVtx[vtxId + 0].v.ob[0] + 12;

            this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] = tempPosY - 0x2A;

            this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
                this->windowContentVtx[vtxId + 0].v.ob[1] - 12;

            this->windowContentVtx[vtxId + 20].v.ob[0] = this->windowContentVtx[vtxId + 22].v.ob[0] = posX + 1;

            this->windowContentVtx[vtxId + 21].v.ob[0] = this->windowContentVtx[vtxId + 23].v.ob[0] =
                this->windowContentVtx[vtxId + 20].v.ob[0] + 12;

            this->windowContentVtx[vtxId + 20].v.ob[1] = this->windowContentVtx[vtxId + 21].v.ob[1] = tempPosY - 0x2B;

            this->windowContentVtx[vtxId + 22].v.ob[1] = this->windowContentVtx[vtxId + 23].v.ob[1] =
                this->windowContentVtx[vtxId + 20].v.ob[1] - 12;
            posX += 8;
        }

        this->windowContentVtx[index + 8].v.ob[0] = this->windowContentVtx[index + 10].v.ob[0] =
            this->windowContentVtx[index + 8].v.ob[0] + 3;

        this->windowContentVtx[index + 9].v.ob[0] = this->windowContentVtx[index + 11].v.ob[0] =
            this->windowContentVtx[index + 8].v.ob[0] + 12;

        this->windowContentVtx[index + 28].v.ob[0] = this->windowContentVtx[index + 30].v.ob[0] =
            this->windowContentVtx[index + 8].v.ob[0] + 1;

        this->windowContentVtx[index + 29].v.ob[0] = this->windowContentVtx[index + 31].v.ob[0] =
            this->windowContentVtx[index + 28].v.ob[0] + 12;

        vtxId += 20;
    }
    posY = -12;
    posX = this->windowPosX - 6;

    for (j = 0; j < 2; j++, vtxId += 4) {
        this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX;

        this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 0x40;

        this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] =
            this->buttonYOffsets[j + 3] + posY;

        this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
            this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;

        this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x800;
        posY -= 0x10;
    }
    this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = posX;

    this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
        this->windowContentVtx[vtxId + 0].v.ob[0] + 0x40;

    this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] =
        this->buttonYOffsets[5] - 0x34;

    this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
        this->windowContentVtx[vtxId + 0].v.ob[1] - 0x10;

    this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x800;
    vtxId += 4;

    if (((this->menuMode == 0) && (this->configMode >= 2)) || ((this->menuMode == 1) && (this->selectMode == 3))) {
        if (this->menuMode == 0) {
            if ((this->configMode == 4) || (this->configMode == 7) || (this->configMode == 0x16)) {
                j = D_80815774_cj0[this->buttonIndex];
            } else if ((this->configMode == 0x19) || (this->configMode == 0xC)) {
                j = D_8081577C_cj0[this->buttonIndex];
            } else {
                j = D_80815768_cj0[this->buttonIndex];
            }
        } else {
            j = D_80815780_cj0[this->confirmButtonIndex];
        }
        this->windowContentVtx[vtxId + 0].v.ob[0] = this->windowContentVtx[vtxId + 2].v.ob[0] = this->windowPosX - 10;

        this->windowContentVtx[vtxId + 1].v.ob[0] = this->windowContentVtx[vtxId + 3].v.ob[0] =
            this->windowContentVtx[vtxId + 0].v.ob[0] + 72;

        this->windowContentVtx[vtxId + 0].v.ob[1] = this->windowContentVtx[vtxId + 1].v.ob[1] =
            this->windowContentVtx[j].v.ob[1] + 4;

        this->windowContentVtx[vtxId + 2].v.ob[1] = this->windowContentVtx[vtxId + 3].v.ob[1] =
            this->windowContentVtx[vtxId + 0].v.ob[1] - 24;

        this->windowContentVtx[vtxId + 1].v.tc[0] = this->windowContentVtx[vtxId + 3].v.tc[0] = 0x900;

        this->windowContentVtx[vtxId + 2].v.tc[1] = this->windowContentVtx[vtxId + 3].v.tc[1] = 0x300;
    }

    this->windowContentVtx[vtxId + 4].v.ob[0] = this->windowContentVtx[vtxId + 6].v.ob[0] = this->windowPosX + 58;

    this->windowContentVtx[vtxId + 5].v.ob[0] = this->windowContentVtx[vtxId + 7].v.ob[0] =
        this->windowContentVtx[vtxId + 4].v.ob[0] + 128;

    this->windowContentVtx[vtxId + 4].v.ob[1] = this->windowContentVtx[vtxId + 5].v.ob[1] =
        this->windowContentVtx[D_80815768_cj0[this->warningButtonIndex]].v.ob[1];

    this->windowContentVtx[vtxId + 6].v.ob[1] = this->windowContentVtx[vtxId + 7].v.ob[1] =
        this->windowContentVtx[vtxId + 4].v.ob[1] - 16;

    this->windowContentVtx[vtxId + 5].v.tc[0] = this->windowContentVtx[vtxId + 7].v.tc[0] = 0x1000;
}

// D_80814654
u16 D_80815808_cj0[] = {
    0x88,
    0x194,
    0x2A0,
};

// sFileSelRemainsTextures
TexturePtr D_80815810_cj0[] = {
    gFileSelOdolwasRemainsTex,
    gFileSelGohtsRemainsTex,
    gFileSelGyorgsRemainsTex,
    gFileSelTwinmoldsRemainsTex,
};

// sFileSelDayENGTextures
TexturePtr D_80815820_cj0[] = {
    gFileSelFirstDayENGTex,
    gFileSelFirstDayENGTex,
    gFileSelSecondDayENGTex,
    gFileSelFinalDayENGTex,
};

// sFileSelHeartPieceTextures
TexturePtr D_80815830_cj0[] = {
    gFileSel0QuarterHeartENGTex,
    gFileSel1QuarterHeartENGTex,
    gFileSel2QuarterHeartENGTex,
    gFileSel3QuarterHeartENGTex,
};

// sHeartTextures
static TexturePtr D_80815840_cj0[][5] = {
    {
        gHeartEmptyTex,
        gHeartQuarterTex,
        gHeartHalfTex,
        gHeartThreeQuarterTex,
        gHeartFullTex,
    },
    {
        gDefenseHeartEmptyTex,
        gDefenseHeartQuarterTex,
        gDefenseHeartHalfTex,
        gDefenseHeartThreeQuarterTex,
        gDefenseHeartFullTex,
    },
};

// sHealthToQuarterHeartCount
u8 D_80815868_cj0[] = { 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3 };

// sFileSelRupeePrimColors
s16 D_80815878_cj0[3][3] = {
    { 200, 255, 100 }, // Default Wallet
    { 170, 170, 255 }, // Adult Wallet
    { 255, 105, 105 }, // Giant Wallet
};

// sFileSelRupeeEnvColors
s16 D_8081588C_cj0[3][3] = {
    { 0, 80, 0 },   // Default Wallet
    { 10, 10, 80 }, // Adult Wallet
    { 40, 10, 0 },  // Giant Wallet
};

// sHeartPrimColors
s16 D_808158A0_cj0[2][3] = {
    { 255, 70, 50 },
    { 200, 0, 0 },
};

// sHeartEnvColors
s16 D_808158AC_cj0[2][3] = {
    { 50, 40, 60 },
    { 255, 255, 255 },
};

// FileSelect_DrawFileInfo
void FileSelect_DrawFileInfo_JP(GameState* thisx, s16 fileIndex) {
    FileSelectState* this = (FileSelectState*)thisx;
    Font* font = &this->font;
    s16 var_t3;
    s16 var_s0;
    s32 var_a2;
    s16 var_s1;
    s16 var_t1;
    s16 sp23A;
    s16 sp230[5];
    u16 sp228[3];
    u8 var_t2;

    OPEN_DISPS(this->state.gfxCtx);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                      PRIMITIVE, 0);

    sp23A = fileIndex;

    if (this->nameAlpha[fileIndex] != 0) {
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 0x20], 32, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->nameAlpha[fileIndex]);

        for (var_s1 = 0, var_s0 = 0; var_s1 < 0x20; var_s1 += 4, var_s0++) {
            FileSelect_DrawTexQuadI4_JP(
                this->state.gfxCtx, font->fontBuf + this->fileNames[fileIndex][var_s0] * FONT_CHAR_TEX_SIZE, var_s1);
        }

        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex]], 32, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->nameAlpha[fileIndex]);

        for (var_s1 = 0, var_s0 = 0; var_s1 < 0x20; var_s1 += 4, var_s0++) {
            FileSelect_DrawTexQuadI4_JP(
                this->state.gfxCtx, font->fontBuf + this->fileNames[fileIndex][var_s0] * FONT_CHAR_TEX_SIZE, var_s1);
        }
    }

    if (fileIndex == this->selectedFileIndex || fileIndex == this->copyDestFileIndex) {

        if (this->isOwlSave[fileIndex + 2]) {
            sp23A = fileIndex + 2;
        }
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, D_80815878_cj0[this->walletUpgrades[sp23A]][0],
                        D_80815878_cj0[this->walletUpgrades[sp23A]][1], D_80815878_cj0[this->walletUpgrades[sp23A]][2],
                        this->fileInfoAlpha[fileIndex]);
        gDPSetEnvColor(POLY_OPA_DISP++, D_8081588C_cj0[this->walletUpgrades[sp23A]][0],
                       D_8081588C_cj0[this->walletUpgrades[sp23A]][1], D_8081588C_cj0[this->walletUpgrades[sp23A]][2],
                       255);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 200], 4, 0);
        gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelRupeeTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                          PRIMITIVE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 76], 12, 0);

        FileSelect_SplitNumber_JP(this->rupees[sp23A], &sp228[0], &sp228[1], &sp228[2]);

        for (var_s1 = 0, var_s0 = D_80815684_cj0[this->walletUpgrades[sp23A]]; var_s0 < 3; var_s1 += 4, var_s0++) {
            FileSelect_DrawTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + sp228[var_s0] * FONT_CHAR_TEX_SIZE, var_s1);
        }

        if (this->rupees[sp23A] == gUpgradeCapacities[4][this->walletUpgrades[sp23A]]) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 120, 255, 0, this->fileInfoAlpha[fileIndex]);
        } else if (this->rupees[sp23A] != 0) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        } else {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 100, 100, 100, this->fileInfoAlpha[fileIndex]);
        }

        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 64], 12, 0);

        for (var_s1 = 0, var_s0 = D_80815684_cj0[this->walletUpgrades[sp23A]]; var_s0 < 3; var_s1 += 4, var_s0++) {
            FileSelect_DrawTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + sp228[var_s0] * FONT_CHAR_TEX_SIZE, var_s1);
        }

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 0, 0, this->fileInfoAlpha[fileIndex]);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 204], 4, 0);
        POLY_OPA_DISP =
            FileSelect_DrawTexQuadIA8_JP(POLY_OPA_DISP, D_80815830_cj0[this->heartPieceCount[sp23A]], 24, 16, 0);
        if (this->defenseHearts[sp23A] == 0) {
            var_a2 = 0;
        } else {
            var_a2 = 1;
        }
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, D_808158A0_cj0[var_a2][0], D_808158A0_cj0[var_a2][1],
                        D_808158A0_cj0[var_a2][2], this->fileInfoAlpha[fileIndex]);
        gDPSetEnvColor(POLY_OPA_DISP++, D_808158AC_cj0[var_a2][0], D_808158AC_cj0[var_a2][1], D_808158AC_cj0[var_a2][2],
                       255);
        var_t1 = this->health[sp23A];
        var_s0 = this->healthCapacity[sp23A] / 16;
        if ((this->isOwlSave[sp23A] == 0) && (var_t1 < 0x31)) {
            var_t1 = 0x30;
        }

        var_t2 = 4;
        for (var_s1 = 0, var_t3 = 0; var_t3 < var_s0; var_s1 += 4, var_t3++) {

            if (var_t1 < 0x10) {
                if (var_t1 != 0) {
                    var_t2 = D_80815868_cj0[var_t1];
                    var_t1 = 0;
                } else {
                    var_t2 = 0;
                }
            } else {
                var_t1 -= 0x10;
            }

            gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 104 + var_s1], 4, 0);

            POLY_OPA_DISP = FileSelect_DrawTexQuadIA8_JP(POLY_OPA_DISP, D_80815840_cj0[var_a2][var_t2], 16, 16, 0);
        }

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);

        for (var_s1 = 0, var_t3 = 0; var_t3 < 4; var_t3++, var_s1 += 4) {
            if (this->questItems[sp23A] & gBitFlags[var_t3]) {
                gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 184 + var_s1], 4, 0);
                gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815810_cj0[var_t3], G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
                gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
            }
        }
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                          PRIMITIVE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 208], 8, 0);
        gDPLoadTextureBlock_4b(POLY_OPA_DISP++, gFileSelMASKSENGTex, G_IM_FMT_I, 64, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                               G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 96], 8, 0);
        FileSelect_SplitNumber_JP(this->maskCount[sp23A], &sp228[0], &sp228[1], &sp228[2]);

        for (var_s1 = 0, var_s0 = 1; var_s0 < 3; var_s1 += 4, var_s0++) {
            FileSelect_DrawTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + sp228[var_s0] * FONT_CHAR_TEX_SIZE, var_s1);
        }

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 88], 8, 0);

        for (var_s1 = 0, var_s0 = 1; var_s0 < 3; var_s1 += 4, var_s0++) {
            FileSelect_DrawTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + sp228[var_s0] * FONT_CHAR_TEX_SIZE, var_s1);
        }
    }

    if (this->isOwlSave[fileIndex + 2] != 0) {

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->nameAlpha[fileIndex]);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 216], 4, 0);
        gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelOwlSaveIconTex, G_IM_FMT_RGBA, G_IM_SIZ_32b, 24, 12, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 220], 8, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
        gDPLoadTextureBlock_4b(POLY_OPA_DISP++, D_80815820_cj0[this->day[sp23A]], G_IM_FMT_I, 48, 24, 0,
                               G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                               G_TX_NOLOD, G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

        sp230[0] = 0;
        sp230[1] = TIME_TO_MINUTES_F(this->time[sp23A]) / 60.0f;

        while (sp230[1] >= 10) {
            sp230[0]++;
            sp230[1] -= 10;
        }

        sp230[3] = 0;
        sp230[4] = (s32)TIME_TO_MINUTES_F(this->time[sp23A]) % 60;

        while (sp230[4] >= 10) {
            sp230[3]++;
            sp230[4] -= 10;
        }

        sp230[2] = 0xE3;

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                          PRIMITIVE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->fileInfoAlpha[fileIndex]);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 248], 20, 0);

        for (var_s0 = 0, var_s1 = 0; var_s0 < 5; var_s1 += 4, var_s0++) {
            FileSelect_DrawTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + sp230[var_s0] * FONT_CHAR_TEX_SIZE, var_s1);
        }

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 228], 20, 0);
        for (var_s0 = 0, var_s1 = 0; var_s0 < 5; var_s1 += 4, var_s0++) {
            FileSelect_DrawTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + sp230[var_s0] * FONT_CHAR_TEX_SIZE, var_s1);
        }

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->fileInfoAlpha[fileIndex]);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);

        for (var_s1 = 0, var_t3 = 0; var_t3 < 4; var_t3++, var_s1 += 4) {

            if (this->questItems[fileIndex] & gBitFlags[var_t3]) {
                gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[D_80815808_cj0[fileIndex] + 184 + var_s1], 4, 0);
                gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815810_cj0[var_t3], G_IM_FMT_RGBA, G_IM_SIZ_32b, 32, 32, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
                gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
            }
        }
    }
    CLOSE_DISPS(this->state.gfxCtx);
}

// sWindowContentColors
s16 D_80815678_cj0[] = { 100, 150, 255 };

// sFileInfoBoxTextures
TexturePtr D_808158B8_cj0[] = {
    gFileSelFileInfoBox0Tex, gFileSelFileInfoBox1Tex,      gFileSelFileInfoBox2Tex,      gFileSelFileInfoBox3Tex,
    gFileSelFileInfoBox4Tex, gFileSelFileExtraInfoBox0Tex, gFileSelFileExtraInfoBox1Tex,
};

// jp equivalent sTitleLabels
TexturePtr D_808158D4_cj0[][9] = {
    {
        gFileSelPleaseSelectAFileENGTex,
        gFileSelOpenThisFileENGTex,
        gFileSelCopyWhichFileENGTex,
        gFileSelCopyToWhichFileENGTex,
        gFileSelAreYouSureCopyENGTex,
        gFileSelFileCopiedENGTex,
        gFileSelEraseWhichFileENGTex,
        gFileSelAreYouSureEraseENGTex,
        gFileSelFileErasedENGTex,
    },
};

// jp equivalent sWarningLabels
TexturePtr D_808158F8_cj0[][5] = {
    {
        gFileSelNoFileToCopyENGTex,
        gFileSelNoFileToEraseENGTex,
        gFileSelNoEmptyFileENGTex,
        gFileSelFileEmptyENGTex,
        gFileSelFileInUseENGTex,
    },
};

// jp equivalent sFileButtonTextures
TexturePtr D_8081590C_cj0[][3] = {
    {
        gFileSelFile1ButtonENGTex,
        gFileSelFile2ButtonENGTex,
        gFileSelFile3ButtonENGTex,
    },
};

// jp equivalent sActionButtonTextures
TexturePtr D_80815918_cj0[][4] = { {
    gFileSelCopyButtonENGTex,
    gFileSelEraseButtonENGTex,
    gFileSelYesButtonENGTex,
    gFileSelQuitButtonENGTex,
} };

// jp array for jp equivalent to gFileSelOptionsButtonENGTex
TexturePtr D_80815928_cj0[] = {
    gFileSelOptionsButtonENGTex,
};

// FileSelect_DrawWindowContents
void FileSelect_DrawWindowContents_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    s16 var_s0;
    s16 var_s6;
    s16 var_t2;
    s16 var_t3;

    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[0]);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
    gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[0], 4, 0);
    gDPLoadTextureBlock(POLY_OPA_DISP++,
                        D_808158D4_cj0[/*gSaveContext.options.language*/ LANGUAGE_JPN][this->titleLabel], G_IM_FMT_IA,
                        G_IM_SIZ_8b, 128, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[1]);

    gDPLoadTextureBlock(POLY_OPA_DISP++,
                        D_808158D4_cj0[/*gSaveContext.options.language*/ LANGUAGE_JPN][this->nextTitleLabel],
                        G_IM_FMT_IA, G_IM_SIZ_8b, 128, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

    var_s6 = 4;

    gDPPipeSync(POLY_OPA_DISP++);

    for (var_s0 = 0; var_s0 < 3; var_s0++, var_s6 += 0x1C) {
        if (var_s0 < 2) {
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                            this->fileInfoAlpha[var_s0]);
            gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[var_s6], 28, 0);
            for (var_t3 = 0, var_t2 = 0; var_t2 < 7; var_t2++, var_t3 += 4) {
                if (var_t2 < 5 || (this->isOwlSave[var_s0 + 2] != 0 && var_t2 >= 5)) {
                    gDPLoadTextureBlock(POLY_OPA_DISP++, D_808158B8_cj0[var_t2], G_IM_FMT_IA, G_IM_SIZ_16b,
                                        D_80815668_cj0[var_t2], 56, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
                    gSP1Quadrangle(POLY_OPA_DISP++, var_t3, var_t3 + 2, var_t3 + 3, var_t3 + 1, 0);
                }
            }
        }
    }

    gDPPipeSync(POLY_OPA_DISP++);
    for (var_t2 = 0; var_t2 < 3; var_t2++, var_s6 += 0x10) {
        if (var_t2 < 2) {
            gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[var_s6], 16, 0);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, D_80815678_cj0[0], D_80815678_cj0[1], D_80815678_cj0[2],
                            this->fileButtonAlpha[var_t2]);
            gDPLoadTextureBlock(POLY_OPA_DISP++, D_8081590C_cj0[/*gSaveContext.options.language*/ LANGUAGE_JPN][var_t2],
                                G_IM_FMT_IA, G_IM_SIZ_16b, 64, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, D_80815678_cj0[0], D_80815678_cj0[1], D_80815678_cj0[2],
                            this->nameBoxAlpha[var_t2]);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelFileNameBoxTex, G_IM_FMT_IA, G_IM_SIZ_16b, 108, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);

            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, D_80815678_cj0[0], D_80815678_cj0[1], D_80815678_cj0[2],
                            this->connectorAlpha[var_t2]);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelConnectorTex, G_IM_FMT_IA, G_IM_SIZ_8b, 24, 16, 0,
                                G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 8, 10, 11, 9, 0);

            if (this->isOwlSave[var_t2 + 2] != 0) {

                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, D_80815678_cj0[0], D_80815678_cj0[1], D_80815678_cj0[2],
                                this->nameBoxAlpha[var_t2]);
                gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelBlankButtonTex, G_IM_FMT_IA, G_IM_SIZ_16b, 52, 16, 0,
                                    G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                    G_TX_NOLOD, G_TX_NOLOD);
                gSP1Quadrangle(POLY_OPA_DISP++, 12, 14, 15, 13, 0);
            }
        }
    }

    for (var_s0 = 0; var_s0 < 2; var_s0++) {
        FileSelect_DrawFileInfo_JP(&this->state, var_s0);
    }
    gDPPipeSync(POLY_OPA_DISP++);

    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

    gSPVertex(POLY_OPA_DISP++, &this->windowContentVtx[940], 20, 0);

    for (var_t3 = 0, var_t2 = 0; var_t2 < 2; var_t2++, var_t3 += 4) {
        gDPPipeSync(POLY_OPA_DISP++);

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->actionButtonAlpha[var_t2]);
        gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815918_cj0[/*gSaveContext.options.language*/ LANGUAGE_JPN][var_t2],
                            G_IM_FMT_IA, G_IM_SIZ_16b, 64, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, var_t3, var_t3 + 2, var_t3 + 3, var_t3 + 1, 0);
    }

    gDPPipeSync(POLY_OPA_DISP++);

    for (var_t3 = 0, var_t2 = 0; var_t2 < 2; var_t2++, var_t3 += 4) {
        var_s6 = this->confirmButtonTexIndices[var_t2];

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->confirmButtonAlpha[var_t2]);

        gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815918_cj0[/*gSaveContext.options.language*/ LANGUAGE_JPN][var_s6],
                            G_IM_FMT_IA, G_IM_SIZ_16b, 64, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, var_t3, var_t3 + 2, var_t3 + 3, var_t3 + 1, 0);
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                    this->optionButtonAlpha);

    if (/*gSaveContext.options.language == LANGUAGE_JPN*/ true) {
        gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelOptionsButtonENGTex, G_IM_FMT_IA, G_IM_SIZ_16b, 64, 16, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
    }
    gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815928_cj0[/*gSaveContext.options.language*/ LANGUAGE_JPN], G_IM_FMT_IA,
                        G_IM_SIZ_16b, 64, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 8, 10, 11, 9, 0);

    if (((this->menuMode == 0) &&
         ((this->configMode == 2) || (this->configMode == 4) || (this->configMode == 7) || (this->configMode == 0xC) ||
          (this->configMode == 0x16) || (this->configMode == 0x19))) ||
        ((this->menuMode == 1) && (this->selectMode == 3))) {
        gDPPipeSync(POLY_OPA_DISP++);

        gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                          PRIMITIVE, 0);

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->highlightColor[0], this->highlightColor[1],
                        this->highlightColor[2], this->highlightColor[3]);

        gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelBigButtonHighlightTex, G_IM_FMT_I, G_IM_SIZ_8b, 72, 24, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);

        gSP1Quadrangle(POLY_OPA_DISP++, 12, 14, 15, 13, 0);
    }
    if (this->warningLabel >= 0) {
        gDPPipeSync(POLY_OPA_DISP++);

        gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->emptyFileTextAlpha);

        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

        gDPLoadTextureBlock(POLY_OPA_DISP++,
                            D_808158F8_cj0[/*gSaveContext.options.language*/ LANGUAGE_JPN][this->warningLabel],
                            G_IM_FMT_IA, G_IM_SIZ_8b, 128, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSP1Quadrangle(POLY_OPA_DISP++, 16, 18, 19, 17, 0);
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIDECALA, G_CC_MODULATEIDECALA);
    CLOSE_DISPS(this->state.gfxCtx);
}

// FileSelect_ConfigModeDraw
void FileSelect_ConfigModeDraw_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    OPEN_DISPS(this->state.gfxCtx);
    gDPPipeSync(POLY_OPA_DISP++);

    Gfx_SetupDL42_Opa(this->state.gfxCtx);
    FileSelect_SetView_JP(this, 0.0f, 0.0f, 64.0f);
    FileSelect_SetWindowVtx_JP(&this->state);
    FileSelect_SetWindowContentVtx_JP(&this->state);

    FrameInterpolation_RecordOpenChild(this, this->configMode);

    if ((this->configMode != 0x24) && (this->configMode != 0x23)) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->windowAlpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
        Matrix_Translate(0.0f, 0.0f, -93.6f, 0);
        Matrix_Scale(0.78f, 0.78f, 0.78f, 1);
        if (this->windowRot != 0) {
            Matrix_RotateXFApply(this->windowRot / 100.0f);
        }

        gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);
        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);
        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);
        gDPPipeSync(POLY_OPA_DISP++);
        FileSelect_DrawWindowContents_JP(&this->state);
    }
    if ((this->configMode >= 0x22) && (this->configMode < 0x27)) {
        gDPPipeSync(POLY_OPA_DISP++);

        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->windowAlpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

        Matrix_Translate(0.0f, 0.0f, -93.6f, 0);
        Matrix_Scale(0.78f, 0.78f, 0.78f, 1);
        Matrix_RotateXFApply((this->windowRot - 314.0f) / 100.0f);
        gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);
        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);
        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);
        gDPPipeSync(POLY_OPA_DISP++);
        FileSelect_DrawNameEntry_JP(&this->state);
    }
    if ((this->configMode >= 0x27) && (this->configMode < 0x2C)) {

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                        this->windowAlpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
        Matrix_Translate(0.0f, 0.0f, -93.6f, 0);
        Matrix_Scale(0.78f, 0.78f, 0.78f, 1);
        Matrix_RotateXFApply((this->windowRot - 314.0f) / 100.0f);
        gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);
        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);
        gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
        gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);
        gDPPipeSync(POLY_OPA_DISP++);

        FileSelect_DrawOptions_JP(&this->state);
    }

    gDPPipeSync(POLY_OPA_DISP++);
    FileSelect_SetView_JP(this, 0.0f, 0.0f, 64.0f);

    FrameInterpolation_RecordCloseChild();

    CLOSE_DISPS(this->state.gfxCtx);
}

// FileSelect_FadeMainToSelect
void FileSelect_FadeMainToSelect_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;
    s16 i;

    for (i = 0; i < 3; i++) {
        if (i != this->buttonIndex) {
            this->fileButtonAlpha[i] -= 50;
            this->actionButtonAlpha[FS_BTN_ACTION_COPY] = this->actionButtonAlpha[FS_BTN_ACTION_ERASE] =
                this->optionButtonAlpha = this->fileButtonAlpha[i];

            if (!gSaveContext.flashSaveAvailable) {
                if (NO_FLASH_SLOT_OCCUPIED(sramCtx, i)) {
                    this->nameAlpha[i] = this->nameBoxAlpha[i] = this->fileButtonAlpha[i];
                    this->connectorAlpha[i] -= 255 / 4;
                }
            } else {
                if (SLOT_OCCUPIED(this, i)) {
                    this->nameAlpha[i] = this->nameBoxAlpha[i] = this->fileButtonAlpha[i];
                    this->connectorAlpha[i] -= 255 / 4;
                }
            }
        }
    }

    this->titleAlpha[FS_TITLE_CUR] -= 255 / 4;
    this->titleAlpha[FS_TITLE_NEXT] += 255 / 4;
    this->actionTimer--;

    if (this->actionTimer == 0) {
        this->actionTimer = 4;
        this->selectMode++; // SM_MOVE_FILE_TO_TOP
        this->confirmButtonIndex = FS_BTN_CONFIRM_YES;
    }
}

// sFileYOffsets
s16 D_8081592C_cj0[] = { 0, 16, 32 };

// FileSelect_MoveSelectedFileToTop
void FileSelect_MoveSelectedFileToTop_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    s32 yStep;

    yStep = ABS_ALT(this->buttonYOffsets[this->buttonIndex] - D_8081592C_cj0[this->buttonIndex]) / this->actionTimer;
    this->buttonYOffsets[this->buttonIndex] += yStep;
    this->actionTimer--;

    if ((this->actionTimer == 0) || (this->buttonYOffsets[this->buttonIndex] == D_8081592C_cj0[this->buttonIndex])) {
        this->buttonYOffsets[FS_BTN_SELECT_YES] = this->buttonYOffsets[FS_BTN_SELECT_QUIT] = -24;
        this->actionTimer = 4;
        this->selectMode++; // SM_FADE_IN_FILE_INFO
    }
}

// FileSelect_FadeInFileInfo
void FileSelect_FadeInFileInfo_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->fileInfoAlpha[this->buttonIndex] += 50;
    this->nameBoxAlpha[this->buttonIndex] -= 100;

    if (this->nameBoxAlpha[this->buttonIndex] <= 0) {
        this->nameBoxAlpha[this->buttonIndex] = 0;
    }

    this->actionTimer--;

    if (this->actionTimer == 0) {
        this->fileInfoAlpha[this->buttonIndex] = 200;
        this->actionTimer = 4;
        this->selectMode++; // SM_CONFIRM_FILE
    }

    this->confirmButtonAlpha[FS_BTN_CONFIRM_YES] = this->confirmButtonAlpha[FS_BTN_CONFIRM_QUIT] =
        this->fileInfoAlpha[this->buttonIndex];
}

// FileSelect_ConfirmFile
void FileSelect_ConfirmFile_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Input* input = CONTROLLER1(&this->state);

    if (CHECK_BTN_ALL(input->press.button, BTN_START) || (CHECK_BTN_ALL(input->press.button, BTN_A))) {
        if (this->confirmButtonIndex == FS_BTN_CONFIRM_YES) {
            Rumble_Request(300.0f, 180, 20, 100);
            Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
            this->selectMode = SM_FADE_OUT;
            Audio_MuteAllSeqExceptSystemAndOcarina(15);
        } else { // FS_BTN_CONFIRM_QUIT
            Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
            this->selectMode++; // SM_FADE_OUT_FILE_INFO
        }
    } else if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
        Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
        this->selectMode++; // SM_FADE_OUT_FILE_INFO
    } else if (ABS_ALT(this->stickAdjY) >= 30) {
        Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
        this->confirmButtonIndex ^= 1;
    }
}

// FileSelect_FadeOutFileInfo
void FileSelect_FadeOutFileInfo_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->fileInfoAlpha[this->buttonIndex] -= 200 / 4;
    this->nameBoxAlpha[this->buttonIndex] += 200 / 4;
    this->actionTimer--;

    if (this->actionTimer == 0) {
        this->buttonYOffsets[FS_BTN_SELECT_YES] = this->buttonYOffsets[FS_BTN_SELECT_QUIT] = 0;
        this->nameBoxAlpha[this->buttonIndex] = 200;
        this->fileInfoAlpha[this->buttonIndex] = 0;
        this->nextTitleLabel = FS_TITLE_SELECT_FILE;
        this->actionTimer = 4;
        this->selectMode++;
    }

    this->confirmButtonAlpha[FS_BTN_CONFIRM_YES] = this->confirmButtonAlpha[FS_BTN_CONFIRM_QUIT] =
        this->fileInfoAlpha[this->buttonIndex];
}

// FileSelect_MoveSelectedFileToSlot
void FileSelect_MoveSelectedFileToSlot_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;
    s32 yStep;
    s16 i;

    yStep = ABS_ALT(this->buttonYOffsets[this->buttonIndex]) / this->actionTimer;
    this->buttonYOffsets[this->buttonIndex] -= yStep;

    if (this->buttonYOffsets[this->buttonIndex] <= 0) {
        this->buttonYOffsets[this->buttonIndex] = 0;
    }

    for (i = 0; i < 3; i++) {
        if (i != this->buttonIndex) {
            this->fileButtonAlpha[i] += 200 / 4;

            if (this->fileButtonAlpha[i] >= 200) {
                this->fileButtonAlpha[i] = 200;
            }

            this->actionButtonAlpha[FS_BTN_ACTION_COPY] = this->actionButtonAlpha[FS_BTN_ACTION_ERASE] =
                this->optionButtonAlpha = this->fileButtonAlpha[i];

            if (!gSaveContext.flashSaveAvailable) {
                if (NO_FLASH_SLOT_OCCUPIED(sramCtx, i)) {
                    this->nameBoxAlpha[i] = this->nameAlpha[i] = this->fileButtonAlpha[i];
                    this->connectorAlpha[i] += 255 / 4;
                }
            } else {
                if (SLOT_OCCUPIED(this, i)) {
                    this->nameBoxAlpha[i] = this->nameAlpha[i] = this->fileButtonAlpha[i];
                    this->connectorAlpha[i] += 255 / 4;
                }
            }
        }
    }

    this->titleAlpha[FS_TITLE_CUR] -= 255 / 4;
    this->titleAlpha[FS_TITLE_NEXT] += 255 / 4;
    this->actionTimer--;

    if (this->actionTimer == 0) {
        this->titleAlpha[FS_TITLE_CUR] = 255;
        this->titleAlpha[FS_TITLE_NEXT] = 0;
        this->titleLabel = this->nextTitleLabel;
        this->actionTimer = 4;
        this->menuMode = 0;
        this->configMode = CM_MAIN_MENU;
        this->nextConfigMode = CM_MAIN_MENU;
        this->selectMode = SM_FADE_MAIN_TO_SELECT;
    }
}

// FileSelect_FadeOut
void FileSelect_FadeOut_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->screenFillAlpha += 40;

    if (this->screenFillAlpha >= 255) {
        this->screenFillAlpha = 255;
        this->selectMode++; // SM_LOAD_GAME
    }
}

// FileSelect_LoadGame
void FileSelect_LoadGame_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    u16 i;

    gSaveContext.fileNum = this->buttonIndex;
    Sram_OpenSave(this, &this->sramCtx);

    gSaveContext.gameMode = GAMEMODE_NORMAL;

    STOP_GAMESTATE(&this->state);
    SET_NEXT_GAMESTATE(&this->state, Play_Init, sizeof(PlayState));

    gSaveContext.respawnFlag = 0;
    gSaveContext.respawn[RESPAWN_MODE_DOWN].entrance = ENTR_LOAD_OPENING;
    gSaveContext.seqId = (u8)NA_BGM_DISABLED;
    gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
    gSaveContext.showTitleCard = true;
    gSaveContext.dogParams = 0;

    for (i = 0; i < TIMER_ID_MAX; i++) {
        gSaveContext.timerStates[i] = TIMER_STATE_OFF;
    }

    gSaveContext.prevHudVisibility = HUD_VISIBILITY_ALL;
    gSaveContext.nayrusLoveTimer = 0;
    gSaveContext.healthAccumulator = 0;
    gSaveContext.magicFlag = 0;
    gSaveContext.forcedSeqId = 0;
    gSaveContext.skyboxTime = CLOCK_TIME(0, 0);
    gSaveContext.nextTransitionType = TRANS_NEXT_TYPE_DEFAULT;
    gSaveContext.cutsceneTrigger = 0;
    gSaveContext.chamberCutsceneNum = 0;
    gSaveContext.nextDayTime = NEXT_TIME_NONE;
    gSaveContext.retainWeatherMode = false;

    gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;

    gSaveContext.hudVisibilityForceButtonAlphasByStatus = false;
    gSaveContext.nextHudVisibility = HUD_VISIBILITY_IDLE;
    gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
    gSaveContext.hudVisibilityTimer = 0;

    gSaveContext.save.saveInfo.playerData.tatlTimer = 0;
}

// sSelectModeUpdateFuncs
void (*D_80815934_cj0[])(GameState*) = {
    FileSelect_FadeMainToSelect_JP, FileSelect_MoveSelectedFileToTop_JP,
    FileSelect_FadeInFileInfo_JP,   FileSelect_ConfirmFile_JP,
    FileSelect_FadeOutFileInfo_JP,  FileSelect_MoveSelectedFileToSlot_JP,
    FileSelect_FadeOut_JP,          FileSelect_LoadGame_JP,
};

// FileSelect_SelectModeUpdate
void FileSelect_SelectModeUpdate_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    D_80815934_cj0[this->selectMode](&this->state);
}

// FileSelect_SelectModeDraw
void FileSelect_SelectModeDraw_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);

    Gfx_SetupDL42_Opa(this->state.gfxCtx);
    FileSelect_SetView_JP(this, 0.0f, 0.0f, 64.0f);
    FileSelect_SetWindowVtx_JP(&this->state);
    FileSelect_SetWindowContentVtx_JP(&this->state);

    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                    this->windowAlpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

    Matrix_Translate(0.0f, 0.0f, -93.6f, MTXMODE_NEW);
    Matrix_Scale(0.78f, 0.78f, 0.78f, MTXMODE_APPLY);
    Matrix_RotateXFApply(this->windowRot / 100.0f);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[0], 32, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow1DL);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[32], 32, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow2DL);

    gSPVertex(POLY_OPA_DISP++, &this->windowVtx[64], 16, 0);
    gSPDisplayList(POLY_OPA_DISP++, gFileSelWindow3DL);

    FileSelect_DrawWindowContents_JP(&this->state);
    gDPPipeSync(POLY_OPA_DISP++);
    FileSelect_SetView_JP(this, 0.0f, 0.0f, 64.0f);

    CLOSE_DISPS(this->state.gfxCtx);
}

// sScreenFillSetupDL
static Gfx D_80815640_cj0[] = {
    gsDPPipeSync(),
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                          G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH),
    gsDPSetOtherMode(G_AD_DISABLE | G_CD_MAGICSQ | G_CK_NONE | G_TC_FILT | G_TF_BILERP | G_TT_NONE | G_TL_TILE |
                         G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_1PRIMITIVE,
                     G_AC_NONE | G_ZS_PIXEL | G_RM_CLD_SURF | G_RM_CLD_SURF2),
    gsDPSetCombineMode(G_CC_PRIMITIVE, G_CC_PRIMITIVE),
    gsSPEndDisplayList(),
};

// sFileSelectSkyboxRotation
s16 D_80815680_cj0 = 0;

// FileSelect_UpdateAndDrawSkybox
void FileSelect_UpdateAndDrawSkybox_JP(FileSelectState* this) {
    s32 pad;
    f32 eyeX;
    f32 eyeY;
    f32 eyeZ;

    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);

    eyeX = 1000.0f * Math_CosS(D_80815680_cj0) - 1000.0f * Math_SinS(D_80815680_cj0);
    eyeY = -700.0f;
    eyeZ = 1000.0f * Math_SinS(D_80815680_cj0) + 1000.0f * Math_CosS(D_80815680_cj0);

    FileSelect_SetView_JP(this, eyeX, eyeY, eyeZ);
    Skybox_Draw(&this->skyboxCtx, this->state.gfxCtx, SKYBOX_NORMAL_SKY, this->envCtx.skyboxBlend, eyeX, eyeY, eyeZ);

    gDPSetTextureLUT(POLY_OPA_DISP++, G_TT_NONE);

    D_80815680_cj0 += -0xA;

    CLOSE_DISPS(this->state.gfxCtx);
}

// gFileSelectDrawFuncs
void (*D_80815954_cj0[])(GameState*) = {
    FileSelect_ConfigModeDraw_JP,
    FileSelect_SelectModeDraw_JP,
};

// gFileSelectUpdateFuncs
void (*D_8081595C_cj0[])(GameState*) = {
    FileSelect_ConfigModeUpdate_JP,
    FileSelect_SelectModeUpdate_JP,
};

// D_808147B4
TexturePtr D_80815964_cj0[] = {
    gFileSelPleaseWaitENGTex,
    gFileSelDecideCancelENGTex,
    gFileSelDecideSaveENGTex,
};

// D_808147C0
s16 D_80815970_cj0[] = { 48, 144, 152 };

// D_808147C8
s16 D_80815978_cj0[] = { 138, 90, 86 };

// FileSelect_Main
void FileSelect_Main_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Input* input = CONTROLLER1(&this->state);
    s32 texIndex;
    s32 pad;

    func_8012CF0C(this->state.gfxCtx, 0, 1, 0, 0, 0);

    OPEN_DISPS(this->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x01, this->staticSegment);
    gSPSegment(POLY_OPA_DISP++, 0x02, this->parameterSegment);

    this->stickAdjX = input->rel.stick_x;
    this->stickAdjY = input->rel.stick_y;

    if (this->stickAdjX < -30) {
        if (this->stickXDir == -1) {
            this->inputTimerX--;
            if (this->inputTimerX < 0) {
                this->inputTimerX = 2;
            } else {
                this->stickAdjX = 0;
            }
        } else {
            this->inputTimerX = 10;
            this->stickXDir = -1;
        }
    } else if (this->stickAdjX > 30) {
        if (this->stickXDir == 1) {
            this->inputTimerX--;
            if (this->inputTimerX < 0) {
                this->inputTimerX = 2;
            } else {
                this->stickAdjX = 0;
            }
        } else {
            this->inputTimerX = 10;
            this->stickXDir = 1;
        }
    } else {
        this->stickXDir = 0;
    }

    if (this->stickAdjY < -30) {
        if (this->stickYDir == -1) {
            this->inputTimerY--;
            if (this->inputTimerY < 0) {
                this->inputTimerY = 2;
            } else {
                this->stickAdjY = 0;
            }
        } else {
            this->inputTimerY = 10;
            this->stickYDir = -1;
        }
    } else if (this->stickAdjY > 30) {
        if (this->stickYDir == 1) {
            this->inputTimerY--;
            if (this->inputTimerY < 0) {
                this->inputTimerY = 2;
            } else {
                this->stickAdjY = 0;
            }
        } else {
            this->inputTimerY = 10;
            this->stickYDir = 1;
        }
    } else {
        this->stickYDir = 0;
    }

    this->emptyFileTextAlpha = 0;

    FileSelect_PulsateCursor_JP(&this->state);
    D_8081595C_cj0[this->menuMode](&this->state);
    FileSelect_UpdateAndDrawSkybox_JP(this);

    FrameInterpolation_StartRecord();
    D_80815954_cj0[this->menuMode](&this->state);
    FrameInterpolation_StopRecord();

    Gfx_SetupDL39_Opa(this->state.gfxCtx);

    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 100, 255, 255, this->controlsAlpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

    if (this->sramCtx.status > 0) {
        texIndex = 0;
    } else if ((this->configMode >= CM_MAIN_TO_OPTIONS) && (this->configMode <= CM_OPTIONS_TO_MAIN)) {
        texIndex = 2;
    } else {
        texIndex = 1;
    }

    gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815964_cj0[texIndex], G_IM_FMT_IA, G_IM_SIZ_8b, D_80815970_cj0[texIndex],
                        16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMASK, G_TX_NOLOD);

    gSPTextureRectangle(POLY_OPA_DISP++, D_80815978_cj0[texIndex] << 2, 204 << 2,
                        (D_80815978_cj0[texIndex] + D_80815970_cj0[texIndex]) << 2, (204 + 16) << 2, G_TX_RENDERTILE, 0,
                        0, 1 << 10, 1 << 10);

    gDPPipeSync(POLY_OPA_DISP++);
    gSPDisplayList(POLY_OPA_DISP++, D_80815640_cj0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->screenFillAlpha);
    gSPDisplayList(POLY_OPA_DISP++, D_0E000000_TO_SEGMENTED(fillRect));

    CLOSE_DISPS(this->state.gfxCtx);
}

// FileSelect_InitContext
void FileSelect_InitContext_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    EnvironmentContext* envCtx = &this->envCtx;

    Sram_Alloc(&this->state, &this->sramCtx);
    func_801457CC(&this->state, &this->sramCtx);

    this->menuMode = FS_MENU_MODE_INIT;

    this->buttonIndex = this->selectMode = this->selectedFileIndex = this->copyDestFileIndex =
        this->confirmButtonIndex = 0;

    this->confirmButtonTexIndices[0] = 2;
    this->confirmButtonTexIndices[1] = 3;
    this->titleLabel = FS_TITLE_SELECT_FILE;
    this->nextTitleLabel = FS_TITLE_OPEN_FILE;

    this->screenFillAlpha = 255;
    this->highlightPulseDir = 1;
    this->unk_244F4 = 0xC;
    this->highlightColor[0] = 155;
    this->highlightColor[1] = 255;
    this->highlightColor[2] = 255;
    this->highlightColor[3] = 70;
    this->configMode = CM_FADE_IN_START;
    this->windowRot = 0.0f;

    this->stickXDir = this->inputTimerX = 0;
    this->stickYDir = this->inputTimerY = 0;

    this->kbdX = this->kbdY = this->charIndex = 0;

    this->kbdButton = FS_KBD_BTN_NONE;

    this->windowColor[0] = 100;
    this->windowColor[1] = 150;
    this->windowColor[2] = 255;

    this->windowAlpha = this->titleAlpha[FS_TITLE_CUR] = this->titleAlpha[FS_TITLE_NEXT] = this->fileButtonAlpha[0] =
        this->fileButtonAlpha[1] = this->fileButtonAlpha[2] = this->nameBoxAlpha[0] = this->nameBoxAlpha[1] =
            this->nameBoxAlpha[2] = this->nameAlpha[0] = this->nameAlpha[1] = this->nameAlpha[2] =
                this->connectorAlpha[0] = this->connectorAlpha[1] = this->connectorAlpha[2] = this->fileInfoAlpha[0] =
                    this->fileInfoAlpha[1] = this->fileInfoAlpha[2] = this->actionButtonAlpha[FS_BTN_ACTION_COPY] =
                        this->actionButtonAlpha[FS_BTN_ACTION_ERASE] = this->confirmButtonAlpha[FS_BTN_CONFIRM_YES] =
                            this->confirmButtonAlpha[FS_BTN_CONFIRM_QUIT] = this->optionButtonAlpha =
                                this->nameEntryBoxAlpha = this->controlsAlpha = this->emptyFileTextAlpha = 0;

    this->windowPosX = 6;
    this->actionTimer = 4;
    this->warningLabel = FS_WARNING_NONE;

    this->warningButtonIndex = this->buttonYOffsets[0] = this->buttonYOffsets[1] = this->buttonYOffsets[2] =
        this->buttonYOffsets[3] = this->buttonYOffsets[4] = this->buttonYOffsets[5] = this->fileNamesY[0] =
            this->fileNamesY[1] = this->fileNamesY[2] = 0;

    this->unk_2451E[0] = 0;
    this->unk_2451E[1] = 3;
    this->unk_2451E[2] = 6;
    this->unk_2451E[3] = 8;
    this->unk_2451E[4] = 10;
    this->highlightTimer = 20;

    ShrinkWindow_Letterbox_SetSizeTarget(0);

    gSaveContext.skyboxTime = 0;
    gSaveContext.save.time = CLOCK_TIME(0, 0);

    Skybox_Init(&this->state, &this->skyboxCtx, 1);
    R_TIME_SPEED = 10;

    envCtx->changeSkyboxState = CHANGE_SKYBOX_INACTIVE;
    envCtx->changeSkyboxTimer = 0;
    envCtx->changeLightEnabled = false;
    envCtx->changeLightTimer = 0;
    envCtx->skyboxDmaState = 0;
    envCtx->skybox1Index = 99;
    envCtx->skybox2Index = 99;
    envCtx->lightConfig = 0;
    envCtx->changeLightNextConfig = 0;
    envCtx->lightSetting = 0;
    envCtx->skyboxConfig = 2;
    envCtx->skyboxDisabled = 0;
    envCtx->skyboxBlend = 0;
    envCtx->glareAlpha = 0.0f;
    envCtx->lensFlareAlphaScale = 0.0f;

    gSaveContext.buttonStatus[EQUIP_SLOT_B] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_LEFT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_DOWN] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_C_RIGHT] = BTN_ENABLED;
    gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;
}

// FileSelect_Destroy
void FileSelect_JP_Destroy(GameState* this) {
    ShrinkWindow_Destroy();
}

// FileSelect_Init
void FileSelect_JP_Init(GameState* thisx) {
    s32 pad;
    FileSelectState* this = (FileSelectState*)thisx;
    size_t size;

    // #Region [2S2H] JP Support
    if (ResourceMgr_GetGameDefaultLanguage(0) == LANGUAGE_JPN) {
        gSaveContext.options.language = LANGUAGE_JPN;
    } else {
        // todo: error
        return;
    }
    // #Region [2S2H] End

    GameState_SetFramerateDivisor(&this->state, 1);
    Matrix_Init(&this->state);
    ShrinkWindow_Init();
    View_Init(&this->view, this->state.gfxCtx);
    this->state.main = FileSelect_Main_JP;
    this->state.destroy = FileSelect_JP_Destroy;
    FileSelect_InitContext_JP(&this->state);
    Font_LoadOrderedFont_JP(&this->font);

    size = SEGMENT_ROM_SIZE(title_static);
    this->staticSegment = THA_AllocTailAlign16(&this->state.tha, size);
    DmaMgr_SendRequest0(this->staticSegment, SEGMENT_ROM_START(title_static), size);

    size = SEGMENT_ROM_SIZE(parameter_static);
    this->parameterSegment = THA_AllocTailAlign16(&this->state.tha, size);
    DmaMgr_SendRequest0(this->parameterSegment, SEGMENT_ROM_START(parameter_static), size);

    Audio_SetSpec(0xA);
    // Setting ioData to 1 and writing it to ioPort 7 will skip the harp intro
    Audio_PlaySequenceWithSeqPlayerIO(SEQ_PLAYER_BGM_MAIN, NA_BGM_FILE_SELECT, 0, 7, 1);
}
