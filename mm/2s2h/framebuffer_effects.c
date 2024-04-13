#include "framebuffer_effects.h"
#include "global.h"

int gfx_create_framebuffer(uint32_t width, uint32_t height, uint32_t native_width, uint32_t native_height,
                           uint8_t resize);

s32 gPauseFrameBuffer = -1;
s32 gBlurFrameBuffer = -1;
// A framebuffer that should only be used for drawing in the same frame that it is copied too
// i.e. the VisMono and VisFbuf effects
s32 gReusableFrameBuffer = -1;

// Picto box buffer is unscaled at 320x240
s32 gPictoBoxFrameBuffer = -1;

void FB_CreateFramebuffers(void) {
    if (gPauseFrameBuffer == -1) {
        gPauseFrameBuffer = gfx_create_framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, true);
    }

    if (gBlurFrameBuffer == -1) {
        gBlurFrameBuffer = gfx_create_framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, true);
    }

    if (gReusableFrameBuffer == -1) {
        gReusableFrameBuffer = gfx_create_framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, true);
    }

    if (gPictoBoxFrameBuffer == -1) {
        gPictoBoxFrameBuffer = gfx_create_framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, false);
    }
}

/**
 * Copies the current texture data from the source frame buffer to the destination frame buffer
 * Setting oncePerFrame ensures that the copy will only happen once every game frame. This
 * is important for effects that could be affected by increased frame interpolation (like motion blur).
 * A pointer to a boolean is passed to the render for the render to set once the copy has been performed.
 * This function uses opcodes from f3dex2 but may be called when s2dex is loaded, such as during shrink window. Make sure f3dex2 is loaded before this function is called.
 */
void FB_CopyToFramebuffer(Gfx** gfxp, s32 fb_src, s32 fb_dest, u8 oncePerFrame, u8* hasCopied) {
    Gfx* gfx = *gfxp;

    gSPMatrix(gfx++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    gDPSetOtherMode(gfx++,
                    G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | G_RM_OPA_SURF | G_RM_OPA_SURF2);

    gSPClearGeometryMode(gfx++, G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR);
    gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);

    gDPSetBlendColor(gfx++, 255, 255, 255, 8);
    gDPSetPrimDepth(gfx++, 0xFFFF, 0xFFFF);

    gDPSetEnvColor(gfx++, 255, 255, 255, 255);
    gDPSetCombineLERP(gfx++, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0,
                      ENVIRONMENT);

    gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    gDPCopyFB(gfx++, fb_dest, fb_src, oncePerFrame, hasCopied);

    *gfxp = gfx;
}

/**
 * Draws the texture data from the specified frame buffer as a full screen image
 */
void FB_DrawFromFramebuffer(Gfx** gfxp, s32 fb, u8 alpha) {
    Gfx* gfx = *gfxp;

    gSPMatrix(gfx++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    gDPSetEnvColor(gfx++, 255, 255, 255, alpha);

    gDPSetOtherMode(gfx++,
                    G_AD_NOISE | G_CD_NOISE | G_CK_NONE | G_TC_FILT | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | G_RM_CLD_SURF | G_RM_CLD_SURF2);

    gSPClearGeometryMode(gfx++, G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR);
    gSPSetGeometryMode(gfx++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);

    gDPSetCombineLERP(gfx++, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0,
                      ENVIRONMENT);

    gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    gDPSetTextureImageFB(gfx++, 0, 0, 0, fb);
    gDPImageRectangle(gfx++, OTRGetRectDimensionFromLeftEdge(0) << 2, 0 << 2, 0, 0,
                      OTRGetRectDimensionFromRightEdge(SCREEN_WIDTH) << 2, SCREEN_HEIGHT << 2,
                      OTRGetGameRenderWidth(), OTRGetGameRenderHeight(), G_TX_RENDERTILE,
                      OTRGetGameRenderWidth(), OTRGetGameRenderHeight());

    *gfxp = gfx;
}

/**
 * Similar to FB_DrawFromFramebuffer, but scales the image relative to the center of the screen.
 * This function uses opcodes from f3dex2 but may be called when s2dex is loaded, such as during shrink window. Make sure f3dex2 is loaded before this function is called.
 */
void FB_DrawFromFramebufferScaled(Gfx** gfxp, s32 fb, u8 alpha, float scaleX, float scaleY) {
    Gfx* gfx = *gfxp;

    gDPSetEnvColor(gfx++, 255, 255, 255, alpha);

    gDPSetOtherMode(gfx++,
                    G_AD_NOISE | G_CD_NOISE | G_CK_NONE | G_TC_FILT | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | G_RM_CLD_SURF | G_RM_CLD_SURF2);

    gDPSetCombineLERP(gfx++, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0,
                      ENVIRONMENT);

    gDPSetScissor(gfx++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    gDPSetTextureImageFB(gfx++, 0, 0, 0, fb);

    float x0 = gScreenWidth * 0.5f * scaleX;
    float y0 = gScreenHeight * 0.5f * scaleY;

    gDPImageRectangle(gfx++, OTRGetRectDimensionFromLeftEdge(x0) << 2,
                      (int)(y0) << 2, 0, 0,
                      OTRGetRectDimensionFromRightEdge((float)(gScreenWidth - x0)) << 2,
                      (int)((float)(gScreenHeight - y0)) << 2,
                      OTRGetGameRenderWidth(), OTRGetGameRenderHeight(), G_TX_RENDERTILE,
                      OTRGetGameRenderWidth(), OTRGetGameRenderHeight());

    *gfxp = gfx;
}
