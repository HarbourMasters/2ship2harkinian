/*
 * File: z_file_nameset_JP.c
 * Overlay: ovl_file_choose
 * Description: Entering name on a new file, selecting options from the options menu
 */

#include "z_file_select.h"
#include "z64rumble.h"
#include "misc/title_static/title_static.h"
#include "overlays/ovl_file_choose/ovl_file_choose.h"

// FileSelect_DrawTexQuadI4
void FileSelect_DrawTexQuadI4_JP(GraphicsContext* gfxCtx, TexturePtr texture, s16 point) {
    OPEN_DISPS(gfxCtx);

    gDPLoadTextureBlock_4b(POLY_OPA_DISP++, texture, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_CLAMP,
                           G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, point, point + 2, point + 3, point + 1, 0);

    CLOSE_DISPS(gfxCtx);
}

// FileSelect_DrawMultiTexQuadI4
void FileSelect_DrawMultiTexQuadI4_JP(GraphicsContext* gfxCtx, TexturePtr texture1, TexturePtr texture2, s16 point) {
    OPEN_DISPS(gfxCtx);

    gDPLoadTextureBlock_4b(POLY_OPA_DISP++, texture1, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gDPLoadMultiBlock_4b(POLY_OPA_DISP++, texture2, 0x0080, 1, G_IM_FMT_I, 16, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                         G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, point, point + 2, point + 3, point + 1, 0);

    CLOSE_DISPS(gfxCtx);
}

// D_80814404
s16 D_808154F0_cj0[] = { -94, -96, -48, 0, 32, 64 };

// D_80814410
s16 D_808154FC_cj0[] = { 56, 44, 44, 28, 28, 44 };

// D_8081441C
s16 D_80815508_cj0[] = { 72, -48, -48, -48, -48, -48 };

// FileSelect_SetKeyboardVtx
void FileSelect_SetKeyboardVtx_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    s16 phi_s1;
    s16 phi_s2;
    s16 phi_t0;
    s16 phi_t1;
    s16 phi_t3;

    this->keyboardVtx = GRAPH_ALLOC(this->state.gfxCtx, sizeof(Vtx) * 4 * 5 * 13);
    phi_s1 = 0x26;

    for (phi_s2 = 0, phi_t3 = 0; phi_s2 < 5; phi_s2++) {
        for (phi_t0 = -0x60, phi_t1 = 0; phi_t1 < 13; phi_t1++, phi_t3 += 4) {
            this->keyboardVtx[phi_t3 + 2].v.ob[0] = phi_t0;

            this->keyboardVtx[phi_t3].v.ob[0] = this->keyboardVtx[phi_t3 + 2].v.ob[0];

            this->keyboardVtx[phi_t3 + 1].v.ob[0] = this->keyboardVtx[phi_t3 + 3].v.ob[0] = phi_t0 + 0xC;

            this->keyboardVtx[phi_t3 + 1].v.ob[1] = phi_s1;

            this->keyboardVtx[phi_t3].v.ob[1] = this->keyboardVtx[phi_t3 + 1].v.ob[1];

            this->keyboardVtx[phi_t3 + 2].v.ob[1] = this->keyboardVtx[phi_t3 + 3].v.ob[1] = phi_s1 - 0xC;

            this->keyboardVtx[phi_t3].v.ob[2] = this->keyboardVtx[phi_t3 + 1].v.ob[2] =
                this->keyboardVtx[phi_t3 + 2].v.ob[2] = this->keyboardVtx[phi_t3 + 3].v.ob[2] = 0;

            this->keyboardVtx[phi_t3].v.flag = this->keyboardVtx[phi_t3 + 1].v.flag =
                this->keyboardVtx[phi_t3 + 2].v.flag = this->keyboardVtx[phi_t3 + 3].v.flag = 0;

            this->keyboardVtx[phi_t3].v.tc[0] = this->keyboardVtx[phi_t3].v.tc[1] =
                this->keyboardVtx[phi_t3 + 1].v.tc[1] = this->keyboardVtx[phi_t3 + 2].v.tc[0] = 0;

            this->keyboardVtx[phi_t3 + 1].v.tc[0] = this->keyboardVtx[phi_t3 + 2].v.tc[1] =
                this->keyboardVtx[phi_t3 + 3].v.tc[0] = this->keyboardVtx[phi_t3 + 3].v.tc[1] = 16 << 5;

            this->keyboardVtx[phi_t3].v.cn[0] = this->keyboardVtx[phi_t3 + 1].v.cn[0] =
                this->keyboardVtx[phi_t3 + 2].v.cn[0] = this->keyboardVtx[phi_t3 + 3].v.cn[0] =
                    this->keyboardVtx[phi_t3].v.cn[1] = this->keyboardVtx[phi_t3 + 1].v.cn[1] =
                        this->keyboardVtx[phi_t3 + 2].v.cn[1] = this->keyboardVtx[phi_t3 + 3].v.cn[1] =
                            this->keyboardVtx[phi_t3].v.cn[2] = this->keyboardVtx[phi_t3 + 1].v.cn[2] =
                                this->keyboardVtx[phi_t3 + 2].v.cn[2] = this->keyboardVtx[phi_t3 + 3].v.cn[2] =
                                    this->keyboardVtx[phi_t3].v.cn[3] = this->keyboardVtx[phi_t3 + 1].v.cn[3] =
                                        this->keyboardVtx[phi_t3 + 2].v.cn[3] = this->keyboardVtx[phi_t3 + 3].v.cn[3] =
                                            255;
            phi_t0 += 0x10;
        }
        phi_s1 -= 0x10;
    }

    this->keyboard2Vtx = GRAPH_ALLOC(this->state.gfxCtx, sizeof(Vtx) * 24);

    for (phi_t1 = 0, phi_t3 = 0; phi_t3 < 6; phi_t3++, phi_t1 += 4) {
        this->keyboard2Vtx[phi_t1].v.ob[0] = this->keyboard2Vtx[phi_t1 + 2].v.ob[0] = D_808154F0_cj0[phi_t3] + 1;

        this->keyboard2Vtx[phi_t1 + 1].v.ob[0] = this->keyboard2Vtx[phi_t1 + 3].v.ob[0] =
            this->keyboard2Vtx[phi_t1].v.ob[0] + D_808154FC_cj0[phi_t3] - 2;

        this->keyboard2Vtx[phi_t1].v.ob[1] = this->keyboard2Vtx[phi_t1 + 1].v.ob[1] = D_80815508_cj0[phi_t3] - 1;

        this->keyboard2Vtx[phi_t1 + 2].v.ob[1] = this->keyboard2Vtx[phi_t1 + 3].v.ob[1] =
            this->keyboard2Vtx[phi_t1].v.ob[1] - 0xE;

        this->keyboard2Vtx[phi_t1].v.ob[2] = this->keyboard2Vtx[phi_t1 + 1].v.ob[2] =
            this->keyboard2Vtx[phi_t1 + 2].v.ob[2] = this->keyboard2Vtx[phi_t1 + 3].v.ob[2] = 0;

        this->keyboard2Vtx[phi_t1].v.flag = this->keyboard2Vtx[phi_t1 + 1].v.flag =
            this->keyboard2Vtx[phi_t1 + 2].v.flag = this->keyboard2Vtx[phi_t1 + 3].v.flag = 0;

        this->keyboard2Vtx[phi_t1].v.tc[0] = this->keyboard2Vtx[phi_t1].v.tc[1] =
            this->keyboard2Vtx[phi_t1 + 1].v.tc[1] = this->keyboard2Vtx[phi_t1 + 2].v.tc[0] = 0;

        this->keyboard2Vtx[phi_t1 + 1].v.tc[0] = this->keyboard2Vtx[phi_t1 + 3].v.tc[0] = D_808154FC_cj0[phi_t3] << 5;

        this->keyboard2Vtx[phi_t1 + 2].v.tc[1] = this->keyboard2Vtx[phi_t1 + 3].v.tc[1] = 16 << 5;

        this->keyboard2Vtx[phi_t1].v.cn[0] = this->keyboard2Vtx[phi_t1 + 1].v.cn[0] =
            this->keyboard2Vtx[phi_t1 + 2].v.cn[0] = this->keyboard2Vtx[phi_t1 + 3].v.cn[0] =
                this->keyboard2Vtx[phi_t1].v.cn[1] = this->keyboard2Vtx[phi_t1 + 1].v.cn[1] =
                    this->keyboard2Vtx[phi_t1 + 2].v.cn[1] = this->keyboard2Vtx[phi_t1 + 3].v.cn[1] =
                        this->keyboard2Vtx[phi_t1].v.cn[2] = this->keyboard2Vtx[phi_t1 + 1].v.cn[2] =
                            this->keyboard2Vtx[phi_t1 + 2].v.cn[2] = this->keyboard2Vtx[phi_t1 + 3].v.cn[2] =
                                this->keyboard2Vtx[phi_t1].v.cn[3] = this->keyboard2Vtx[phi_t1 + 1].v.cn[3] =
                                    this->keyboard2Vtx[phi_t1 + 2].v.cn[3] = this->keyboard2Vtx[phi_t1 + 3].v.cn[3] =
                                        255;
    }
}

// jp exclusive
TexturePtr D_80815514_cj0[] = {
    gFileSelNameENGTex,
};

// jp version of sBackspaceEndTextures
TexturePtr D_80815518_cj0[] = {
    gFileSelHiraganaCharButtonTex, gFileSelKatakanaCharButtonTex, gFileSelLatinCharButtonTex,
    gFileSelBackspaceButtonTex,    gFileSelENDButtonENGTex,
};

// jp version of sBackspaceEndWidths
u16 D_8081552C_cj0[] = { 44, 44, 28, 28, 44 };

// jp version of D_80814434
s16 D_80815538_cj0[] = { -30, -16, -6, 4, 14, 24, 34, 44, 54, -16, -16 };

// D_8081444C
s16 D_80815550_cj0[] = { 72, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69 };

// FileSelect_SetNameEntryVtx
void FileSelect_SetNameEntryVtx_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Font* font = &this->font;
    s16 var_s0;
    s16 var_t2;
    s16 sp132;

    OPEN_DISPS(this->state.gfxCtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[FS_TITLE_CUR]);

    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

    gSPVertex(POLY_OPA_DISP++, this->keyboard2Vtx, 24, 0);
    // @bug: D_80815514_cj0 only has size 1 so this requires the language to be japanese, otherwise it is UB
    gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815514_cj0[/*gSaveContext.options.language*/ LANGUAGE_JPN], G_IM_FMT_IA,
                        G_IM_SIZ_8b, 56, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    gDPPipeSync(POLY_OPA_DISP++);

    for (var_t2 = 0, var_s0 = 4; var_t2 < 5; var_t2++, var_s0 += 4) {
        if (/*gSaveContext.options.language == LANGUAGE_JPN*/ true) {

            if (this->kbdButton == var_t2) {
                this->keyboard2Vtx[var_s0].v.ob[0] = this->keyboard2Vtx[var_s0 + 2].v.ob[0] =
                    D_808154F0_cj0[var_t2 + 1];
                this->keyboard2Vtx[var_s0 + 1].v.ob[0] = this->keyboard2Vtx[var_s0 + 3].v.ob[0] =
                    this->keyboard2Vtx[var_s0].v.ob[0] + D_808154FC_cj0[var_t2 + 1];
                this->keyboard2Vtx[var_s0].v.ob[1] = this->keyboard2Vtx[var_s0 + 1].v.ob[1] =
                    D_80815508_cj0[var_t2 + 1];
                this->keyboard2Vtx[var_s0 + 2].v.ob[1] = this->keyboard2Vtx[var_s0 + 3].v.ob[1] =
                    this->keyboard2Vtx[var_s0].v.ob[1] - 0x10;
            }
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                            255);
            gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

            gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815518_cj0[var_t2], G_IM_FMT_IA, G_IM_SIZ_16b,
                                D_8081552C_cj0[var_t2], 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            gSP1Quadrangle(POLY_OPA_DISP++, var_s0, var_s0 + 2, var_s0 + 3, var_s0 + 1, 0);
        } else if (var_t2 >= 3) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                            255);
            gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

            gDPLoadTextureBlock(POLY_OPA_DISP++, D_80815518_cj0[var_t2], G_IM_FMT_IA, G_IM_SIZ_16b,
                                D_8081552C_cj0[var_t2], 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

            gSP1Quadrangle(POLY_OPA_DISP++, var_s0, var_s0 + 2, var_s0 + 3, var_s0 + 1, 0);
        }
    }

    this->nameEntryVtx = GRAPH_ALLOC(this->state.gfxCtx, 44 * sizeof(Vtx));

    for (var_s0 = 0, var_t2 = 0; var_t2 < 44; var_t2 += 4, var_s0++) {
        this->nameEntryVtx[var_t2].v.ob[0] = this->nameEntryVtx[var_t2 + 2].v.ob[0] =
            D_80815538_cj0[var_s0] + this->nameEntryBoxPosX;

        this->nameEntryVtx[var_t2 + 1].v.ob[0] = this->nameEntryVtx[var_t2 + 3].v.ob[0] =
            this->nameEntryVtx[var_t2].v.ob[0] + 0xA;

        this->nameEntryVtx[var_t2].v.ob[1] = this->nameEntryVtx[var_t2 + 1].v.ob[1] = D_80815550_cj0[var_s0];

        this->nameEntryVtx[var_t2 + 2].v.ob[1] = this->nameEntryVtx[var_t2 + 3].v.ob[1] =
            this->nameEntryVtx[var_t2].v.ob[1] - 0xA;

        this->nameEntryVtx[var_t2].v.ob[2] = this->nameEntryVtx[var_t2 + 1].v.ob[2] =
            this->nameEntryVtx[var_t2 + 2].v.ob[2] = this->nameEntryVtx[var_t2 + 3].v.ob[2] = 0;

        this->nameEntryVtx[var_t2].v.flag = this->nameEntryVtx[var_t2 + 1].v.flag =
            this->nameEntryVtx[var_t2 + 2].v.flag = this->nameEntryVtx[var_t2 + 3].v.flag = 0;

        this->nameEntryVtx[var_t2].v.tc[0] = this->nameEntryVtx[var_t2].v.tc[1] =
            this->nameEntryVtx[var_t2 + 1].v.tc[1] = this->nameEntryVtx[var_t2 + 2].v.tc[0] = 0;

        this->nameEntryVtx[var_t2 + 1].v.tc[0] = this->nameEntryVtx[var_t2 + 2].v.tc[1] =
            this->nameEntryVtx[var_t2 + 3].v.tc[0] = this->nameEntryVtx[var_t2 + 3].v.tc[1] = 0x200;

        this->nameEntryVtx[var_t2].v.cn[0] = this->nameEntryVtx[var_t2 + 1].v.cn[0] =
            this->nameEntryVtx[var_t2 + 2].v.cn[0] = this->nameEntryVtx[var_t2 + 3].v.cn[0] =
                this->nameEntryVtx[var_t2].v.cn[1] = this->nameEntryVtx[var_t2 + 1].v.cn[1] =
                    this->nameEntryVtx[var_t2 + 2].v.cn[1] = this->nameEntryVtx[var_t2 + 3].v.cn[1] =
                        this->nameEntryVtx[var_t2].v.cn[2] = this->nameEntryVtx[var_t2 + 1].v.cn[2] =
                            this->nameEntryVtx[var_t2 + 2].v.cn[2] = this->nameEntryVtx[var_t2 + 3].v.cn[2] =
                                this->nameEntryVtx[var_t2].v.cn[3] = this->nameEntryVtx[var_t2 + 1].v.cn[3] =
                                    this->nameEntryVtx[var_t2 + 2].v.cn[3] = this->nameEntryVtx[var_t2 + 3].v.cn[3] =
                                        0xFF;
    }

    this->nameEntryVtx[1].v.ob[0] = this->nameEntryVtx[3].v.ob[0] = this->nameEntryVtx[0].v.ob[0] + 0x6C;
    this->nameEntryVtx[2].v.ob[1] = this->nameEntryVtx[3].v.ob[1] = this->nameEntryVtx[0].v.ob[1] - 0x10;
    this->nameEntryVtx[1].v.tc[0] = this->nameEntryVtx[3].v.tc[0] = 108 << 5;

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->windowColor[0], this->windowColor[1], this->windowColor[2],
                    this->nameEntryBoxAlpha);
    gSPVertex(POLY_OPA_DISP++, this->nameEntryVtx, 4, 0);
    gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelFileNameBoxTex, G_IM_FMT_IA, G_IM_SIZ_16b, 108, 16, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                      PRIMITIVE, 0);
    gSPVertex(POLY_OPA_DISP++, this->nameEntryVtx + 4, 32, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->nameEntryBoxAlpha);

    for (sp132 = 0, var_s0 = 0; var_s0 < 0x20; var_s0 += 4, sp132++) {
        FileSelect_DrawTexQuadI4_JP(
            this->state.gfxCtx, font->fontBuf + this->fileNames[this->buttonIndex][sp132] * FONT_CHAR_TEX_SIZE, var_s0);
    }

    this->nameEntryVtx[37].v.tc[0] = this->nameEntryVtx[38].v.tc[1] = this->nameEntryVtx[39].v.tc[0] =
        this->nameEntryVtx[39].v.tc[1] = this->nameEntryVtx[41].v.tc[0] = this->nameEntryVtx[42].v.tc[1] =
            this->nameEntryVtx[43].v.tc[0] = this->nameEntryVtx[43].v.tc[1] = 24 << 5;

    if ((this->kbdButton == FS_KBD_BTN_HIRA) || (this->kbdButton == FS_KBD_BTN_KATA) ||
        (this->kbdButton == FS_KBD_BTN_END)) {
        this->nameEntryVtx[41].v.tc[0] = this->nameEntryVtx[43].v.tc[0] = 56 << 5;
    } else if ((this->kbdButton == FS_KBD_BTN_ENG) || (this->kbdButton == FS_KBD_BTN_BACKSPACE)) {
        this->nameEntryVtx[41].v.tc[0] = this->nameEntryVtx[43].v.tc[0] = 40 << 5;
    }

    CLOSE_DISPS(this->state.gfxCtx);
}

// FileSelect_DrawKeyboard
void FileSelect_DrawKeyboard_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Font* font = &this->font;
    s16 i;
    s16 vtx;
    s16 var_s1;

    vtx = 0;
    i = 0;

    OPEN_DISPS(this->state.gfxCtx);
    Gfx_SetupDL42_Opa(this->state.gfxCtx);

    gDPSetCycleType(POLY_OPA_DISP++, G_CYC_2CYCLE);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_PASS, G_RM_XLU_SURF2);
    gDPSetCombineLERP(POLY_OPA_DISP++, 0, 0, 0, PRIMITIVE, TEXEL1, TEXEL0, PRIM_LOD_FRAC, TEXEL0, 0, 0, 0, COMBINED, 0,
                      0, 0, COMBINED);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, this->charBgAlpha, 255, 255, 255, 255);

    if ((this->charPage == 0) || (this->charPage == 3) || (this->charPage == 5)) {
        if (this->charPage != 5) {
            for (; vtx < 0x100; vtx += 0x20) {
                gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[vtx], 32, 0);

                for (var_s1 = 0; var_s1 < 0x20; i++, var_s1 += 4) {
                    FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx,
                                                     font->fontBuf + D_808153D0_cj0[i] * FONT_CHAR_TEX_SIZE,
                                                     font->fontBuf + D_80815414_cj0[i] * FONT_CHAR_TEX_SIZE, var_s1);
                }
            }
            gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[0x100], 4, 0);
            FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + D_808153D0_cj0[i] * FONT_CHAR_TEX_SIZE,
                                             font->fontBuf + D_80815414_cj0[i] * FONT_CHAR_TEX_SIZE, 0);
        } else {
            for (; vtx < 0x100; vtx += 0x20) {
                gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[vtx], 32, 0);

                for (var_s1 = 0; var_s1 < 0x20; i++, var_s1 += 4) {
                    FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx,
                                                     font->fontBuf + D_808153D0_cj0[i] * FONT_CHAR_TEX_SIZE,
                                                     font->fontBuf + D_80815458_cj0[i] * FONT_CHAR_TEX_SIZE, var_s1);
                }
            }
            gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[0x100], 4, 0);
            FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + D_808153D0_cj0[i] * FONT_CHAR_TEX_SIZE,
                                             font->fontBuf + D_80815458_cj0[i] * FONT_CHAR_TEX_SIZE, 0);
        }
    } else if ((this->charPage == 1) || (this->charPage == 4) || (this->charPage == 7)) {
        if (this->charPage != 7) {
            for (; vtx < 0x100; vtx += 0x20) {
                gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[vtx], 32, 0);

                for (var_s1 = 0; var_s1 < 0x20; i++, var_s1 += 4) {
                    FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx,
                                                     font->fontBuf + D_80815414_cj0[i] * FONT_CHAR_TEX_SIZE,
                                                     font->fontBuf + D_808153D0_cj0[i] * FONT_CHAR_TEX_SIZE, var_s1);
                }
            }
            gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[0x100], 4, 0);
            FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + D_80815414_cj0[i] * FONT_CHAR_TEX_SIZE,
                                             font->fontBuf + D_808153D0_cj0[i] * FONT_CHAR_TEX_SIZE, 0);
        } else {
            for (; vtx < 0x100; vtx += 0x20) {
                gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[vtx], 32, 0);

                for (var_s1 = 0; var_s1 < 0x20; i++, var_s1 += 4) {
                    FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx,
                                                     font->fontBuf + D_80815414_cj0[i] * FONT_CHAR_TEX_SIZE,
                                                     font->fontBuf + D_80815458_cj0[i] * FONT_CHAR_TEX_SIZE, var_s1);
                }
            }
            gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[0x100], 4, 0);
            FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + D_80815414_cj0[i] * FONT_CHAR_TEX_SIZE,
                                             font->fontBuf + D_80815458_cj0[i] * FONT_CHAR_TEX_SIZE, 0);
        }
    } else if (this->charPage != 8) {
        for (; vtx < 0x100; vtx += 0x20) {
            gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[vtx], 32, 0);

            for (var_s1 = 0; var_s1 < 0x20; i++, var_s1 += 4) {
                FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx,
                                                 font->fontBuf + D_80815458_cj0[i] * FONT_CHAR_TEX_SIZE,
                                                 font->fontBuf + D_808153D0_cj0[i] * FONT_CHAR_TEX_SIZE, var_s1);
            }
        }
        gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[0x100], 4, 0);
        FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + D_80815458_cj0[i] * FONT_CHAR_TEX_SIZE,
                                         font->fontBuf + D_808153D0_cj0[i] * FONT_CHAR_TEX_SIZE, 0);
    } else {
        for (; vtx < 0x100; vtx += 0x20) {
            gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[vtx], 32, 0);

            for (var_s1 = 0; var_s1 < 0x20; i++, var_s1 += 4) {
                FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx,
                                                 font->fontBuf + D_80815458_cj0[i] * FONT_CHAR_TEX_SIZE,
                                                 font->fontBuf + D_80815414_cj0[i] * FONT_CHAR_TEX_SIZE, var_s1);
            }
        }
        gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[0x100], 4, 0);
        FileSelect_DrawMultiTexQuadI4_JP(this->state.gfxCtx, font->fontBuf + D_80815458_cj0[i] * FONT_CHAR_TEX_SIZE,
                                         font->fontBuf + D_80815414_cj0[i] * FONT_CHAR_TEX_SIZE, 0);
    }
    CLOSE_DISPS(this->state.gfxCtx);
}

// jp exclusives
u8 D_80815568_cj0[] = { 0x0F, 0x23, 0x41, 0x50, 0x55, 0x5F, 0x73, 0x91, 0xA0, 0xA5, 0x5C, 0xAA };

u8 D_80815574_cj0[] = { 0x1D, 0x27, 0x4F, 0x54, 0x59, 0x6D, 0x77, 0x9F, 0xA4, 0xA9, 0x5C, 0xAA };

u8 D_80815580_cj0[] = { 0x32, 0x2D, 0xCE, 0x05, 0xCE, 0x32, 0x2D, 0xCE, 0x05, 0xCE, 0x4E, 0xB2 };

u8 D_8081558C_cj0[] = { 0x0F, 0x23, 0x55, 0x5F, 0x73, 0xA5, 0x5C, 0xAA };

u8 D_80815594_cj0[] = { 0x1D, 0x27, 0x59, 0x6D, 0x77, 0xA9, 0x5C, 0xAA };

u8 D_8081559C_cj0[] = { 0x32, 0x2D, 0xFB, 0x32, 0x2D, 0xFB, 0x4E, 0xB2 };

u8 D_808155A4_cj0[] = { 0x23, 0x50, 0x73, 0xA0 };

u8 D_808155A8_cj0[] = { 0x27, 0x54, 0x77, 0xA4 };

u8 D_808155AC_cj0[] = { 0x32, 0x05, 0x32, 0x05 };

// Function does not exist on z_file_nameset_NES

s32 func_808097B0_cj0(GameState* thisx, u8 arg1, s16 arg2) {
    FileSelectState* this = (FileSelectState*)thisx;
    s16 var_v1;
    s32 temp_a1;

    if (arg1 == 0xDF) {
        for (var_v1 = 0; var_v1 < 12; var_v1++) {
            if (this->fileNames[this->buttonIndex][arg2] >= D_80815568_cj0[var_v1] &&
                D_80815574_cj0[var_v1] >= this->fileNames[this->buttonIndex][arg2]) {
                this->fileNames[this->buttonIndex][arg2] =
                    D_80815580_cj0[var_v1] + this->fileNames[this->buttonIndex][arg2];
                return true;
            }
        }
    } else if (arg1 == 0xE7) {
        for (var_v1 = 0; var_v1 < 8; var_v1++) {
            if (this->fileNames[this->buttonIndex][arg2] >= D_8081558C_cj0[var_v1] &&
                D_80815594_cj0[var_v1] >= this->fileNames[this->buttonIndex][arg2]) {
                this->fileNames[this->buttonIndex][arg2] =
                    D_8081559C_cj0[var_v1] + this->fileNames[this->buttonIndex][arg2];
                return true;
            }
        }
    } else if (arg1 == 0xE8) {
        for (var_v1 = 0; var_v1 < 4; var_v1++) {
            if ((this->fileNames[this->buttonIndex][arg2] >= D_808155A4_cj0[var_v1]) &&
                (D_808155A8_cj0[var_v1] >= this->fileNames[this->buttonIndex][arg2])) {
                this->fileNames[this->buttonIndex][arg2] =
                    D_808155AC_cj0[var_v1] + this->fileNames[this->buttonIndex][arg2];
                return true;
            }
        }
    }
    return false;
}

// Function does not exist on z_file_nameset_NES

s32 func_808099AC_cj0(GameState* thisx, u8 arg1) {
    FileSelectState* this = (FileSelectState*)thisx;
    s32 pad;

    if (!func_808097B0_cj0(&this->state, arg1, this->newFileNameCharCount)) {
        if (this->newFileNameCharCount != 0) {
            if (!func_808097B0_cj0(&this->state, arg1, this->newFileNameCharCount - 1)) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

// FileSelect_DrawNameEntry
void FileSelect_DrawNameEntry_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;
    Font* font = &this->font;
    Input* input = CONTROLLER1(&this->state);
    s16 i;
    s16 tmp;
    u16 time;
    s16 validName;

    OPEN_DISPS(this->state.gfxCtx);

    FileSelect_SetKeyboardVtx_JP(&this->state);
    FileSelect_SetNameEntryVtx_JP(&this->state);
    FileSelect_PulsateCursor_JP(&this->state);

    tmp = (this->newFileNameCharCount * 4) + 4;

    this->nameEntryVtx[0x24].v.ob[0] = this->nameEntryVtx[0x26].v.ob[0] = this->nameEntryVtx[tmp].v.ob[0] - 6;

    this->nameEntryVtx[0x25].v.ob[0] = this->nameEntryVtx[0x27].v.ob[0] = this->nameEntryVtx[0x24].v.ob[0] + 24;

    this->nameEntryVtx[0x24].v.ob[1] = this->nameEntryVtx[0x25].v.ob[1] = this->nameEntryVtx[tmp].v.ob[1] + 7;

    this->nameEntryVtx[0x26].v.ob[1] = this->nameEntryVtx[0x27].v.ob[1] = this->nameEntryVtx[0x24].v.ob[1] - 24;

    if ((this->kbdButton == 0) || (this->kbdButton == 1) || (this->kbdButton == 4)) {
        this->nameEntryVtx[40].v.ob[0] = this->nameEntryVtx[42].v.ob[0] =
            this->keyboard2Vtx[(this->kbdX + 1) * 4].v.ob[0] - 4;

        this->nameEntryVtx[41].v.ob[0] = this->nameEntryVtx[43].v.ob[0] = this->nameEntryVtx[40].v.ob[0] + 52;

        this->nameEntryVtx[40].v.ob[1] = this->nameEntryVtx[41].v.ob[1] =
            this->keyboard2Vtx[(this->kbdX + 1) * 4].v.ob[1] + 4;
    } else if ((this->kbdButton == 2) || (this->kbdButton == 3)) {
        this->nameEntryVtx[40].v.ob[0] = this->nameEntryVtx[42].v.ob[0] =
            this->keyboard2Vtx[(this->kbdX + 1) * 4].v.ob[0] - 4;

        this->nameEntryVtx[41].v.ob[0] = this->nameEntryVtx[43].v.ob[0] = this->nameEntryVtx[40].v.ob[0] + 40;

        this->nameEntryVtx[40].v.ob[1] = this->nameEntryVtx[41].v.ob[1] =
            this->keyboard2Vtx[(this->kbdX + 1) * 4].v.ob[1] + 4;
    } else {
        this->nameEntryVtx[40].v.ob[0] = this->nameEntryVtx[42].v.ob[0] =
            this->keyboardVtx[this->charIndex * 4].v.ob[0] - 6;

        this->nameEntryVtx[41].v.ob[0] = this->nameEntryVtx[43].v.ob[0] = this->nameEntryVtx[40].v.ob[0] + 0x18;

        this->nameEntryVtx[40].v.ob[1] = this->nameEntryVtx[41].v.ob[1] =
            this->keyboardVtx[this->charIndex * 4].v.ob[1] + 6;
    }

    this->nameEntryVtx[42].v.ob[1] = this->nameEntryVtx[43].v.ob[1] = this->nameEntryVtx[40].v.ob[1] - 24;

    gSPVertex(POLY_OPA_DISP++, &this->nameEntryVtx[36], 8, 0);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, 1, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0, TEXEL0, 0,
                      PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, this->highlightColor[0], this->highlightColor[1], this->highlightColor[2],
                    this->highlightColor[3]);
    gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelCharHighlightTex, G_IM_FMT_I, G_IM_SIZ_8b, 24, 24, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);

    if ((this->kbdButton == 0) || (this->kbdButton == 1) || (this->kbdButton == 4)) {
        gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelMediumButtonHighlightTex, G_IM_FMT_I, G_IM_SIZ_8b, 56, 24, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
    } else if ((this->kbdButton == 2) || (this->kbdButton == 3)) {
        gDPLoadTextureBlock(POLY_OPA_DISP++, gFileSelSmallButtonHighlightTex, G_IM_FMT_I, G_IM_SIZ_8b, 40, 24, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
    }

    gSP1Quadrangle(POLY_OPA_DISP++, 4, 6, 7, 5, 0);

    FileSelect_DrawKeyboard_JP(&this->state);

    gDPPipeSync(POLY_OPA_DISP++);

    Gfx_SetupDL42_Opa(this->state.gfxCtx);

    gDPSetCombineLERP(POLY_OPA_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0, 0,
                      PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, 255);

    if (this->configMode == 0x24) {
        if (CHECK_BTN_ALL(input->press.button, BTN_START)) {
            Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
            this->kbdY = 5;
            this->kbdX = 4;
        } else if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
            if ((this->newFileNameCharCount == 7) && (this->fileNames[this->buttonIndex][7] != 0xDF)) {
                for (i = this->newFileNameCharCount; i < 7; i++) {
                    this->fileNames[this->buttonIndex][i] = this->fileNames[this->buttonIndex][i + 1];
                }
                this->fileNames[this->buttonIndex][i] = 0xDF;
                Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
            } else {
                this->newFileNameCharCount--;
                if (this->newFileNameCharCount < 0) {
                    this->newFileNameCharCount = 0;
                    this->configMode = 0x26;
                    Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
                } else {
                    for (i = this->newFileNameCharCount; i < 7; i++) {
                        this->fileNames[this->buttonIndex][i] = this->fileNames[this->buttonIndex][i + 1];
                    }
                    this->fileNames[this->buttonIndex][i] = 0xDF;
                    Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
                }
            }
        } else {
            if (this->charPage < 3) {
                if (this->kbdY != 5) {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 0, 255);

                    this->keyboardVtx[(this->charIndex * 4)].v.ob[0] =
                        this->keyboardVtx[(this->charIndex * 4) + 2].v.ob[0] =
                            this->keyboardVtx[(this->charIndex * 4)].v.ob[0] - 2;

                    this->keyboardVtx[(this->charIndex * 4) + 1].v.ob[0] =
                        this->keyboardVtx[(this->charIndex * 4) + 3].v.ob[0] =
                            this->keyboardVtx[(this->charIndex * 4)].v.ob[0] + 16;

                    this->keyboardVtx[(this->charIndex * 4)].v.ob[1] =
                        this->keyboardVtx[(this->charIndex * 4) + 1].v.ob[1] =
                            this->keyboardVtx[(this->charIndex * 4)].v.ob[1] + 2;

                    this->keyboardVtx[(this->charIndex * 4) + 2].v.ob[1] =
                        this->keyboardVtx[(this->charIndex * 4) + 3].v.ob[1] =
                            this->keyboardVtx[(this->charIndex * 4)].v.ob[1] - 16;

                    gSPVertex(POLY_OPA_DISP++, &this->keyboardVtx[(this->charIndex * 4)], 4, 0);

                    if (this->charPage == 0) {
                        FileSelect_DrawTexQuadI4_JP(
                            this->state.gfxCtx, font->fontBuf + D_808153D0_cj0[this->charIndex] * FONT_CHAR_TEX_SIZE,
                            0);
                        if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
                            if ((D_808153D0_cj0[this->charIndex] == 0xE7) ||
                                (D_808153D0_cj0[this->charIndex] == 0xE8)) {
                                if (func_808099AC_cj0(&this->state, D_808153D0_cj0[this->charIndex]) == 0) {
                                    Audio_PlaySfx(NA_SE_SY_FSEL_ERROR);
                                } else {
                                    Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_S);
                                }
                            } else {
                                Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_S);
                                this->fileNames[this->buttonIndex][this->newFileNameCharCount] =
                                    D_808153D0_cj0[this->charIndex];
                                this->newFileNameCharCount++;
                                if (this->newFileNameCharCount >= 8) {
                                    this->newFileNameCharCount = 7;
                                }
                            }
                        }
                    } else if (this->charPage == 1) {
                        FileSelect_DrawTexQuadI4_JP(
                            this->state.gfxCtx, font->fontBuf + D_80815414_cj0[this->charIndex] * FONT_CHAR_TEX_SIZE,
                            0);
                        if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
                            if ((D_80815414_cj0[this->charIndex] == 0xE7) ||
                                (D_80815414_cj0[this->charIndex] == 0xE8)) {
                                if (func_808099AC_cj0(&this->state, D_80815414_cj0[this->charIndex]) == 0) {
                                    Audio_PlaySfx(NA_SE_SY_FSEL_ERROR);
                                } else {
                                    Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_S);
                                }
                            } else {
                                Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_S);
                                this->fileNames[this->buttonIndex][this->newFileNameCharCount] =
                                    D_80815414_cj0[this->charIndex];
                                this->newFileNameCharCount++;
                                if (this->newFileNameCharCount >= 8) {
                                    this->newFileNameCharCount = 7;
                                }
                            }
                        }
                    } else {
                        FileSelect_DrawTexQuadI4_JP(
                            this->state.gfxCtx, font->fontBuf + D_80815458_cj0[this->charIndex] * FONT_CHAR_TEX_SIZE,
                            0);
                        if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
                            Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_S);
                            this->fileNames[this->buttonIndex][this->newFileNameCharCount] =
                                D_80815458_cj0[this->charIndex];
                            this->newFileNameCharCount++;
                            if (this->newFileNameCharCount >= 8) {
                                this->newFileNameCharCount = 7;
                            }
                        }
                    }
                } else if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
                    if (this->charPage != this->kbdButton) {
                        if (this->kbdButton == 0) {
                            if (this->charPage == 1) {
                                this->charPage = 4;
                            } else {
                                this->charPage = 6;
                            }
                            Audio_PlaySfx(NA_SE_SY_WIN_OPEN);
                        } else if (this->kbdButton == 1) {
                            if (this->charPage == 0) {
                                this->charPage = 3;
                            } else {
                                this->charPage = 8;
                            }
                            Audio_PlaySfx(NA_SE_SY_WIN_OPEN);
                        } else if (this->kbdButton == 2) {
                            if (this->charPage == 0) {
                                this->charPage = 5;
                            } else {
                                this->charPage = 7;
                            }
                            Audio_PlaySfx(NA_SE_SY_WIN_OPEN);
                        } else if (this->kbdButton == 3) {
                            if (this->newFileNameCharCount == 7 && this->fileNames[this->buttonIndex][7] != 0xDF) {
                                for (i = this->newFileNameCharCount; i < 7; i++) {
                                    this->fileNames[this->buttonIndex][i] = this->fileNames[this->buttonIndex][i + 1];
                                }
                                this->fileNames[this->buttonIndex][i] = 0xDF;
                                Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
                            } else {
                                this->newFileNameCharCount--;
                                if (this->newFileNameCharCount < 0) {
                                    this->newFileNameCharCount = 0;
                                }
                                for (i = this->newFileNameCharCount; i < 7; i++) {
                                    this->fileNames[this->buttonIndex][i] = this->fileNames[this->buttonIndex][i + 1];
                                }
                                this->fileNames[this->buttonIndex][i] = 0xDF;
                                Audio_PlaySfx(NA_SE_SY_FSEL_CLOSE);
                            }
                        } else if (this->kbdButton == 4) {
                            validName = false;
                            for (i = 0; i < 8; i++) {
                                if (this->fileNames[this->buttonIndex][i] != 0xDF) {
                                    validName = true;
                                    break;
                                }
                            }

                            if (validName) {
                                Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
                                gSaveContext.fileNum = this->buttonIndex;
                                time = gSaveContext.save.time;
                                Sram_InitSave(this, sramCtx);
                                gSaveContext.save.time = time;
                                if (gSaveContext.flashSaveAvailable == 0) {
                                    this->configMode = CM_NAME_ENTRY_TO_MAIN;
                                } else {
                                    Sram_SetFlashPagesDefault(sramCtx, gFlashSaveStartPages[this->buttonIndex * 2],
                                                              gFlashSpecialSaveNumPages[this->buttonIndex * 2]);
                                    Sram_StartWriteToFlashDefault(sramCtx);
                                    this->configMode = CM_NAME_ENTRY_WAIT_FOR_FLASH_SAVE;
                                }
                                this->nameBoxAlpha[this->buttonIndex] = this->nameAlpha[this->buttonIndex] = 200;
                                this->connectorAlpha[this->buttonIndex] = 255;
                                Rumble_Request(300.0f, 180, 20, 100);
                            } else {
                                Audio_PlaySfx(NA_SE_SY_FSEL_ERROR);
                            }
                        }
                    }
                }
                if (CHECK_BTN_ALL(input->press.button, BTN_CRIGHT)) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                    this->newFileNameCharCount++;
                    if (this->newFileNameCharCount >= 8) {
                        this->newFileNameCharCount = 7;
                    }
                } else if (CHECK_BTN_ALL(input->press.button, BTN_CLEFT)) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                    this->newFileNameCharCount--;
                    if (this->newFileNameCharCount < 0) {
                        this->newFileNameCharCount = 0;
                    }
                } else if (CHECK_BTN_ALL(input->press.button, BTN_Z)) {
                    if (func_808099AC_cj0(&this->state, 0xDF) != 0) {
                        Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_S);
                    }
                }
            }
        }
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIDECALA, G_CC_MODULATEIDECALA);

    CLOSE_DISPS(this->state.gfxCtx);
}

// FileSelect_StartNameEntry
void FileSelect_StartNameEntry_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->nameEntryBoxAlpha += 25;

    if (this->nameEntryBoxAlpha >= 255) {
        this->nameEntryBoxAlpha = 255;
    }

    this->nameEntryBoxPosX -= 30;

    if (this->nameEntryBoxPosX <= 0) {
        this->nameEntryBoxPosX = 0;
        this->nameEntryBoxAlpha = 255;
        this->kbdX = 0;
        this->kbdY = 0;
        this->kbdButton = FS_KBD_BTN_NONE;
        this->configMode = CM_NAME_ENTRY;
    }
}

s16 D_8081549C_cj0[] = { 0, 1, 2, 1, 0, 2, 0, 2, 1, 0 }; // jp exclusive

// FileSelect_UpdateKeyboardCursor
void FileSelect_UpdateKeyboardCursor_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    Input* input = &this->state.input[0];
    s16 prevKbdX;

    if (this->charPage < 3) {
        if (CHECK_BTN_ALL(input->press.button, BTN_R)) {
            if (/*gSaveContext.options.language == LANGUAGE_JPN*/ true) {
                if (this->charPage == 0) {
                    this->charPage = 3;
                    Audio_PlaySfx(NA_SE_SY_WIN_OPEN);
                } else if (this->charPage == 1) {
                    this->charPage = 7;
                    Audio_PlaySfx(NA_SE_SY_WIN_OPEN);
                } else if (this->charPage == 2) {
                    this->charPage = 6;
                    Audio_PlaySfx(NA_SE_SY_WIN_OPEN);
                }
            }
        } else {
            this->kbdButton = 99;
            if (this->kbdY != 5) {
                if (this->stickAdjX < -30) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                    this->charIndex--;
                    this->kbdX--;
                    if (this->kbdX < 0) {
                        this->kbdX = 12;
                        this->charIndex = this->kbdY * 13 + this->kbdX;
                    }
                } else if (this->stickAdjX > 30) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                    this->charIndex++;
                    this->kbdX++;
                    if (this->kbdX >= 13) {
                        this->kbdX = 0;
                        this->charIndex = this->kbdY * 13 + this->kbdX;
                    }
                }
            } else if (/*gSaveContext.options.language == LANGUAGE_JPN*/ true) {
                if (this->stickAdjX < -30) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                    this->kbdX--;
                    if (this->kbdX < 0) {
                        this->kbdX = 4;
                    }
                } else if (this->stickAdjX > 30) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                    this->kbdX++;
                    if (this->kbdX >= 5) {
                        this->kbdX = 0;
                    }
                }
            } else {
                if (this->stickAdjX < -30) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                    this->kbdX--;
                    if (this->kbdX < 3) {
                        this->kbdX = 4;
                    }
                } else if (this->stickAdjX > 30) {
                    Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                    this->kbdX++;
                    if (this->kbdX >= 5) {
                        this->kbdX = 3;
                    }
                }
            }

            if (this->stickAdjY > 30) {
                Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                this->kbdY--;
                if (this->kbdY < 0) {
                    if (/*gSaveContext.options.language == LANGUAGE_JPN*/ true) {
                        this->kbdY = 5;
                        this->charIndex += 52;
                        prevKbdX = this->kbdX;
                        if (this->kbdX < 3) {
                            this->kbdX = 0;
                        } else if (this->kbdX < 6) {
                            this->kbdX = 1;
                        } else if (this->kbdX < 8) {
                            this->kbdX = 2;
                        } else if (this->kbdX < 10) {
                            this->kbdX = 3;
                        } else if (this->kbdX < 13) {
                            this->kbdX = 4;
                        }

                        this->unk_2451E[this->kbdX] = prevKbdX;
                    } else if (this->kbdX < 8) {
                        this->kbdY = 4;
                        this->charIndex = this->kbdX + 52;
                    } else {
                        this->kbdY = 5;
                        this->charIndex += 52;
                        prevKbdX = this->kbdX;
                        if (this->kbdX < 10) {
                            this->kbdX = 3;
                        } else if (this->kbdX < 13) {
                            this->kbdX = 4;
                        }

                        this->unk_2451E[this->kbdX] = prevKbdX;
                    }
                } else {
                    this->charIndex -= 13;
                    if (this->kbdY == 4) {
                        this->charIndex = 52;
                        this->kbdX = this->unk_2451E[this->kbdX];
                        this->charIndex += this->kbdX;
                    }
                }
            } else if (this->stickAdjY < -30) {
                Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
                this->kbdY++;
                if (this->kbdY >= 6) {
                    this->kbdY = 0;
                    this->kbdX = this->unk_2451E[this->kbdX];
                    this->charIndex = this->kbdX;
                } else {
                    this->charIndex += 13;
                    if (this->kbdY == 5) {
                        if (/*gSaveContext.options.language != LANGUAGE_JPN*/ false) {
                            if (this->kbdX < 8) {
                                this->kbdY = 0;
                                this->charIndex = this->kbdX;
                            } else {
                                prevKbdX = this->kbdX;
                                if (this->kbdX < 3) {
                                    this->kbdX = 0;
                                } else if (this->kbdX < 6) {
                                    this->kbdX = 1;
                                } else if (this->kbdX < 8) {
                                    this->kbdX = 2;
                                } else if (this->kbdX < 10) {
                                    this->kbdX = 3;
                                } else if (this->kbdX < 13) {
                                    this->kbdX = 4;
                                }

                                this->unk_2451E[this->kbdX] = prevKbdX;
                            }
                        } else {
                            prevKbdX = this->kbdX;
                            if (this->kbdX < 3) {
                                this->kbdX = 0;
                            } else if (this->kbdX < 6) {
                                this->kbdX = 1;
                            } else if (this->kbdX < 8) {
                                this->kbdX = 2;
                            } else if (this->kbdX < 10) {
                                this->kbdX = 3;
                            } else if (this->kbdX < 13) {
                                this->kbdX = 4;
                            }
                            this->unk_2451E[this->kbdX] = prevKbdX;
                        }
                    }
                }
            }
            if (this->kbdY == 5) {
                this->kbdButton = this->kbdX;
            }
        }
    } else {
        this->charBgAlpha += 30;
        if (this->charBgAlpha >= 255) {
            this->charBgAlpha = 0;
            this->charPage = D_8081549C_cj0[this->charPage];
        }
    }
}

// FileSelect_NameEntryWaitForFlashSave
void FileSelect_NameEntryWaitForFlashSave_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;

    Sram_UpdateWriteToFlashDefault(sramCtx);

    if (sramCtx->status == 0) {
        this->configMode = CM_NAME_ENTRY_TO_MAIN;
    }
}

// FileSelect_StartOptions
void FileSelect_StartOptions_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;

    this->nameEntryBoxAlpha += 25;

    if (this->nameEntryBoxAlpha >= 255) {
        this->nameEntryBoxAlpha = 255;
    }

    this->nameEntryBoxPosX -= 30;

    if (this->nameEntryBoxPosX <= 0) {
        this->nameEntryBoxPosX = 0;
        this->nameEntryBoxAlpha = 255;
        this->configMode = CM_OPTIONS_MENU;
    }
}

// sSelectedSetting
u8 B_808161A0_cj0;

// FileSelect_UpdateOptionsMenu
void FileSelect_UpdateOptionsMenu_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;
    Input* input = CONTROLLER1(&this->state);
    s32 pad;

    if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
        Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
        Sram_WriteSaveOptionsToBuffer(sramCtx);

        if (!gSaveContext.flashSaveAvailable) {
            this->configMode = CM_OPTIONS_TO_MAIN;
        } else {
            Sram_SetFlashPagesDefault(sramCtx, gFlashSaveStartPages[8], gFlashSpecialSaveNumPages[8]);
            Sram_StartWriteToFlashDefault(sramCtx);
            this->configMode = CM_OPTIONS_WAIT_FOR_FLASH_SAVE;
        }
        Audio_SetFileSelectSettings(gSaveContext.options.audioSetting);
        return;
    }

    if (this->stickAdjX < -30) {
        Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);

        if (B_808161A0_cj0 == FS_SETTING_AUDIO) {
            gSaveContext.options.audioSetting--;

            // because audio setting is unsigned, can't check for < 0
            if (gSaveContext.options.audioSetting > 0xF0) {
                gSaveContext.options.audioSetting = SAVE_AUDIO_SURROUND;
            }
        } else {
            gSaveContext.options.zTargetSetting ^= 1;
        }
    } else if (this->stickAdjX > 30) {
        Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);

        if (B_808161A0_cj0 == FS_SETTING_AUDIO) {
            gSaveContext.options.audioSetting++;
            if (gSaveContext.options.audioSetting > SAVE_AUDIO_SURROUND) {
                gSaveContext.options.audioSetting = SAVE_AUDIO_STEREO;
            }
        } else {
            gSaveContext.options.zTargetSetting ^= 1;
        }
    }

    if ((this->stickAdjY < -30) || (this->stickAdjY > 30)) {
        Audio_PlaySfx(NA_SE_SY_FSEL_CURSOR);
        B_808161A0_cj0 ^= 1;
    } else if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
        Audio_PlaySfx(NA_SE_SY_FSEL_DECIDE_L);
        B_808161A0_cj0 ^= 1;
    }
}

// FileSelect_OptionsWaitForFlashSave
void FileSelect_OptionsWaitForFlashSave_JP(GameState* thisx) {
    FileSelectState* this = (FileSelectState*)thisx;
    SramContext* sramCtx = &this->sramCtx;

    Sram_UpdateWriteToFlashDefault(sramCtx);

    if (sramCtx->status == 0) {
        this->configMode = CM_OPTIONS_TO_MAIN;
    }
}

typedef struct {
    TexturePtr texture;
    u16 width;
    u16 height;
} OptionsMenuTextureInfo; // size = 0x8

// gOptionsMenuHeaders
OptionsMenuTextureInfo D_808155B0_cj0[] = {
    { gFileSelOptionsENGTex, 128, 16 },          { gFileSelSoundENGTex, 64, 16 },
    { gFileSelTargetingENGTex, 64, 16 },         { gFileSelCheckBrightnessENGTex, 96, 16 },
    { gFileSelDolbySurroundLogoENGTex, 48, 17 },
};

// gOptionsMenuSettings
OptionsMenuTextureInfo D_808155D8_cj0[] = {
    { gFileSelStereoENGTex, 48, 16 },   { gFileSelMonoENGTex, 48, 16 },   { gFileSelHeadsetENGTex, 48, 16 },
    { gFileSelSurroundENGTex, 48, 16 }, { gFileSelSwitchENGTex, 48, 16 }, { gFileSelHoldENGTex, 48, 16 },
};

// FileSelect_DrawOptionsImpl
void FileSelect_DrawOptionsImpl_JP(GameState* thisx) {
    static s16 D_80815608_cj0 = 255; // sCursorPrimRed
    static s16 D_8081560C_cj0 = 255; // sCursorPrimGreen
    static s16 D_80815610_cj0 = 255; // sCursorPrimBlue
    static s16 D_80815614_cj0 = 0;   // sCursorEnvRed
    static s16 D_80815618_cj0 = 0;   // sCursorEnvGreen
    static s16 D_8081561C_cj0 = 0;   // sCursorEnvBlue
    static s16 D_80815620_cj0 = 1;   // sCursorPulseDir
    static s16 D_80815624_cj0 = 20;  // sCursorFlashTimer
    static s16 D_80815628_cj0[2][3] = {
        // sCursorPrimColors
        { 255, 255, 255 },
        { 0, 255, 255 },
    };
    static s16 D_80815634_cj0[2][3] = {
        // sCursorEnvColors
        { 0, 0, 0 },
        { 0, 150, 150 },
    };
    FileSelectState* this = (FileSelectState*)thisx;

    s16 i;
    s16 vtx;
    s16 cursorRedStep;
    s16 cursorGreenStep;
    s16 cursorBlueStep;

    OPEN_DISPS(this->state.gfxCtx);

    cursorRedStep = ABS_ALT(D_80815608_cj0 - D_80815628_cj0[D_80815620_cj0][0]) / D_80815624_cj0;

    cursorGreenStep = ABS_ALT(D_8081560C_cj0 - D_80815628_cj0[D_80815620_cj0][1]) / D_80815624_cj0;

    cursorBlueStep = ABS_ALT(D_80815610_cj0 - D_80815628_cj0[D_80815620_cj0][2]) / D_80815624_cj0;

    if (D_80815608_cj0 >= D_80815628_cj0[D_80815620_cj0][0]) {
        D_80815608_cj0 = D_80815608_cj0 - cursorRedStep;
    } else {
        D_80815608_cj0 = D_80815608_cj0 + cursorRedStep;
    }

    if (D_8081560C_cj0 >= D_80815628_cj0[D_80815620_cj0][1]) {
        D_8081560C_cj0 = D_8081560C_cj0 - cursorGreenStep;
    } else {
        D_8081560C_cj0 = D_8081560C_cj0 + cursorGreenStep;
    }

    if (D_80815610_cj0 >= D_80815628_cj0[D_80815620_cj0][2]) {
        D_80815610_cj0 = D_80815610_cj0 - cursorBlueStep;
    } else {
        D_80815610_cj0 = D_80815610_cj0 + cursorBlueStep;
    }

    cursorRedStep = ABS_ALT(D_80815614_cj0 - D_80815634_cj0[D_80815620_cj0][0]) / D_80815624_cj0;

    cursorGreenStep = ABS_ALT(D_80815618_cj0 - D_80815634_cj0[D_80815620_cj0][1]) / D_80815624_cj0;

    cursorBlueStep = ABS_ALT(D_8081561C_cj0 - D_80815634_cj0[D_80815620_cj0][2]) / D_80815624_cj0;

    if (D_80815614_cj0 >= D_80815634_cj0[D_80815620_cj0][0]) {
        D_80815614_cj0 = D_80815614_cj0 - cursorRedStep;
    } else {
        D_80815614_cj0 = D_80815614_cj0 + cursorRedStep;
    }
    if (D_80815618_cj0 >= D_80815634_cj0[D_80815620_cj0][1]) {
        D_80815618_cj0 = D_80815618_cj0 - cursorGreenStep;
    } else {
        D_80815618_cj0 = D_80815618_cj0 + cursorGreenStep;
    }
    if (D_8081561C_cj0 >= D_80815634_cj0[D_80815620_cj0][2]) {
        D_8081561C_cj0 = D_8081561C_cj0 - cursorBlueStep;
    } else {
        D_8081561C_cj0 = D_8081561C_cj0 + cursorBlueStep;
    }

    if (--D_80815624_cj0 == 0) {
        D_80815608_cj0 = D_80815628_cj0[D_80815620_cj0][0];
        D_8081560C_cj0 = D_80815628_cj0[D_80815620_cj0][1];
        D_80815610_cj0 = D_80815628_cj0[D_80815620_cj0][2];
        D_80815614_cj0 = D_80815634_cj0[D_80815620_cj0][0];
        D_80815618_cj0 = D_80815634_cj0[D_80815620_cj0][1];
        D_8081561C_cj0 = D_80815634_cj0[D_80815620_cj0][2];

        D_80815624_cj0 = 20;

        if (++D_80815620_cj0 >= 2) {
            D_80815620_cj0 = 0;
        }
    }

    gSPVertex(POLY_OPA_DISP++, D_80813DF0, 24, 0);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[0]);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);

    for (i = 0, vtx = 0; i < 5; i++, vtx += 4) {
        if (i == 4) {
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetCombineLERP(POLY_OPA_DISP++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, TEXEL0,
                              0, PRIMITIVE, 0);
            if (gSaveContext.options.audioSetting == 3) {
                gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, 255);
            } else {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, this->titleAlpha[0]);
                gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
            }
        }

        gDPLoadTextureBlock(POLY_OPA_DISP++, D_808155B0_cj0[i].texture, G_IM_FMT_IA, G_IM_SIZ_8b,
                            D_808155B0_cj0[i].width, D_808155B0_cj0[i].height, 0, G_TX_NOMIRROR | G_TX_WRAP,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gSP1Quadrangle(POLY_OPA_DISP++, vtx, vtx + 2, vtx + 3, vtx + 1, 0);
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[0]);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    gSPVertex(POLY_OPA_DISP++, D_80813F30, 24, 0);

    for (i = 0, vtx = 0; i < 4; i++, vtx += 4) {
        gDPPipeSync(POLY_OPA_DISP++);
        if (gSaveContext.options.audioSetting == i) {
            if (B_808161A0_cj0 == 0) {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, D_80815608_cj0, D_8081560C_cj0, D_80815610_cj0,
                                this->titleAlpha[0]);
                gDPSetEnvColor(POLY_OPA_DISP++, D_80815614_cj0, D_80815618_cj0, D_8081561C_cj0, 255);
            } else {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[0]);
                gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
            }
        } else {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 120, 120, 120, this->titleAlpha[0]);
            gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
        }
        gDPLoadTextureBlock(POLY_OPA_DISP++, D_808155D8_cj0[i].texture, G_IM_FMT_IA, G_IM_SIZ_8b,
                            D_808155D8_cj0[i].width, D_808155B0_cj0[i].height, 0, G_TX_NOMIRROR | G_TX_WRAP,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, vtx, vtx + 2, vtx + 3, vtx + 1, 0);
    }
    for (; i < 6; i++, vtx += 4) {
        gDPPipeSync(POLY_OPA_DISP++);
        if ((gSaveContext.options.zTargetSetting + 4) == i) { // audio setting?
            if (B_808161A0_cj0 != 0) {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, D_80815608_cj0, D_8081560C_cj0, D_80815610_cj0,
                                this->titleAlpha[0]);
                gDPSetEnvColor(POLY_OPA_DISP++, D_80815614_cj0, D_80815618_cj0, D_8081561C_cj0, 255);
            } else {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, this->titleAlpha[0]);
                gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
            }
        } else {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 120, 120, 120, this->titleAlpha[0]);
            gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
        }
        gDPLoadTextureBlock(POLY_OPA_DISP++, D_808155D8_cj0[i].texture, G_IM_FMT_IA, G_IM_SIZ_8b,
                            D_808155D8_cj0[i].width, D_808155B0_cj0[i].height, 0, G_TX_NOMIRROR | G_TX_WRAP,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
        gSP1Quadrangle(POLY_OPA_DISP++, vtx, vtx + 2, vtx + 3, vtx + 1, 0);
    }
    vtx = 0;
    gDPPipeSync(POLY_OPA_DISP++);
    gSPVertex(POLY_OPA_DISP++, &D_80813F30[24], 8, 0);
    gDPLoadTextureBlock_4b(POLY_OPA_DISP++, gFileSelBrightnessCheckTex, G_IM_FMT_IA, 96, 16, 0,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                           G_TX_NOLOD);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 55, 55, 55, this->titleAlpha[0]);
    gDPSetEnvColor(POLY_OPA_DISP++, 40, 40, 40, 255);
    gSP1Quadrangle(POLY_OPA_DISP++, vtx, vtx + 2, vtx + 3, vtx + 1, 0);
    vtx += 4;
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 30, 30, 30, this->titleAlpha[0]);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    gSP1Quadrangle(POLY_OPA_DISP++, vtx, vtx + 2, vtx + 3, vtx + 1, 0);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 255, 255, this->titleAlpha[0]);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
    gDPLoadTextureBlock_4b(POLY_OPA_DISP++, gFileSelOptionsDividerTex, G_IM_FMT_IA, 256, 2, 0,
                           G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                           G_TX_NOLOD);

    Matrix_Push();
    Matrix_Translate(0.0f, 0.1f, 0.0f, MTXMODE_APPLY);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPVertex(POLY_OPA_DISP++, gOptionsDividerTopVtx, 4, 0);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    Matrix_Pop();
    Matrix_Push();
    Matrix_Translate(0.0f, 0.2f, 0.0f, MTXMODE_APPLY);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPVertex(POLY_OPA_DISP++, gOptionsDividerMiddleVtx, 4, 0);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    Matrix_Pop();
    Matrix_Push();
    Matrix_Translate(0.0f, 0.4f, 0.0f, MTXMODE_APPLY);
    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPVertex(POLY_OPA_DISP++, gOptionsDividerBottomVtx, 4, 0);
    gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
    Matrix_Pop();
    CLOSE_DISPS(this->state.gfxCtx);
}

// FileSelect_DrawOptions
void FileSelect_DrawOptions_JP(GameState* thisx) {
    FileSelect_DrawOptionsImpl_JP(thisx);
}
