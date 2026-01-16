#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

#define CVAR_NAME "gEnhancements.Items.ColorPictograph"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void convertImage(u16* destI, u16* srcRgba16, s32 rgba16Width, s32 pixelLeft, s32 pixelTop, s32 pixelRight,
                  s32 pixelBottom) {

    for (int i = pixelTop; i <= pixelBottom; i++) {
        for (int j = pixelLeft; j <= pixelRight; j++) {
            u16 px = srcRgba16[i * rgba16Width + j];
            px |= 1;
            px = (px >> 8) | (px << 8);
            destI[(i - pixelTop) * PICTO_PHOTO_WIDTH + (j - pixelLeft)] = px;
        }
    }
}

void drawPicto(s16 sp2CC) {

    OPEN_DISPS(gPlayState->state.gfxCtx);

    // Get rid of Sepia tone from prior gDPSetPrimColor call
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_DECALRGBA, G_CC_DECALRGBA);

    // Calling twice because I couldn't cast as a u16*
    gSPInvalidateTexCache(OVERLAY_DISP++, (uintptr_t)(gSaveContext.shipSaveContext.pictoPhotoRGBA) + (0x500 * sp2CC));
    gSPInvalidateTexCache(OVERLAY_DISP++, (uintptr_t)(gSaveContext.shipSaveContext.pictoPhotoRGBA) + (0xA00 * sp2CC));
    gDPLoadTextureBlock(OVERLAY_DISP++, (u16*)(gSaveContext.shipSaveContext.pictoPhotoRGBA) + (0x500 * sp2CC),
                        G_IM_FMT_RGBA, G_IM_SIZ_16b, PICTO_PHOTO_WIDTH, 8, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void RegisterColorPictograph() {

    COND_VB_SHOULD(VB_PICTO_TAKE, true /*maybe cvar?*/, {
        PreRender* prerender = va_arg(args, PreRender*);
        // 320came from SCREEN_WIDTH
        convertImage(gSaveContext.shipSaveContext.pictoPhotoRGBA, prerender->fbufSave, 320, PICTO_PHOTO_TOPLEFT_X,
                     PICTO_PHOTO_TOPLEFT_Y, (PICTO_PHOTO_TOPLEFT_X + PICTO_PHOTO_WIDTH) - 1,
                     (PICTO_PHOTO_TOPLEFT_Y + PICTO_PHOTO_HEIGHT) - 1);
    });

    COND_VB_SHOULD(VB_PICTO_DISPLAY, CVAR, {
        s16 sp2CC = va_arg(args, s16);

        if (gSaveContext.shipSaveContext.pictoPhotoRGBA[0] != 0) {
            drawPicto(sp2CC);
            *should = false;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterColorPictograph, { CVAR_NAME });
