#include "z64.h"
#include "regs.h"
#include "functions.h"
#include "macros.h"

// Variables are put before most headers as a hacky way to bypass bss reordering
OSViMode sNotebookViMode; // placeholder name
void* gFramebuffers[2];
OSViMode* gActiveViMode;
u16* gZBufferPtr;
void* gWorkBuffer;
u64* gGfxSPTaskOutputBufferPtr;
void* gGfxSPTaskOutputBufferEnd;

void* sCfbLoRes1;
void* sCfbLoRes0;
u16 (*gZBufferLoRes)[SCREEN_WIDTH * SCREEN_HEIGHT];
u16 (*gWorkBufferLoRes)[SCREEN_WIDTH * SCREEN_HEIGHT];
u64 (*gGfxSPTaskOutputBufferLoRes)[0x3000];
void* gGfxSPTaskOutputBufferEndLoRes;

void* sCfbHiRes1;
void* sCfbHiRes0;
u16 (*gZBufferHiRes)[HIRES_BUFFER_WIDTH * HIRES_BUFFER_HEIGHT];
u16 (*gWorkBufferHiRes)[HIRES_BUFFER_WIDTH * HIRES_BUFFER_HEIGHT];
u64 (*gGfxSPTaskOutputBufferHiRes)[0x3000];
void* gGfxSPTaskOutputBufferEndHiRes;

s16 gCfbWidth;
s16 gCfbHeight;
s16 gCfbLeftAdjust;
s16 gCfbUpperAdjust;

u8 gSysCfbHiResEnabled;

#include "variables.h"
#include "sys_cfb.h"
#include "libc/stdbool.h"
#include "buffers.h"
#include "system_malloc.h"
#include "z64vimode.h"

extern u16 gFramebufferHiRes0[HIRES_BUFFER_WIDTH][HIRES_BUFFER_HEIGHT];
extern u16 gFramebufferHiRes1[HIRES_BUFFER_WIDTH][HIRES_BUFFER_HEIGHT];

void SysCfb_SetLoResMode(void) {
    gFramebuffers[1] = sCfbLoRes1;
    gFramebuffers[0] = sCfbLoRes0;
    gZBufferPtr = *gZBufferLoRes;
    gWorkBuffer = gWorkBufferLoRes;
    gGfxSPTaskOutputBufferPtr = *gGfxSPTaskOutputBufferLoRes;
    gGfxSPTaskOutputBufferEnd = gGfxSPTaskOutputBufferEndLoRes;
    gCfbWidth = SCREEN_WIDTH;
    gCfbHeight = SCREEN_HEIGHT;
    gCfbLeftAdjust = 0;
    gCfbUpperAdjust = 0;
    gSysCfbHiResEnabled = false;
    gScreenWidth = gCfbWidth;
    gScreenHeight = gCfbHeight;
    gActiveViMode = &osViModeNtscLan1;
}

void SysCfb_SetHiResMode(void) {
    gFramebuffers[1] = sCfbHiRes1;
    gFramebuffers[0] = sCfbHiRes0;
    gZBufferPtr = *gZBufferHiRes;
    gWorkBuffer = gWorkBufferHiRes;
    gGfxSPTaskOutputBufferPtr = *gGfxSPTaskOutputBufferHiRes;
    gGfxSPTaskOutputBufferEnd = gGfxSPTaskOutputBufferEndHiRes;
    if (1) {}
    gCfbWidth = HIRES_BUFFER_WIDTH;
    gCfbHeight = HIRES_BUFFER_HEIGHT;
    gCfbLeftAdjust = 30;
    gCfbUpperAdjust = 10;
    gScreenWidth = gCfbWidth;
    gScreenHeight = gCfbHeight;
    if ((gCfbWidth == SCREEN_WIDTH_HIRES) && (gCfbHeight == SCREEN_HEIGHT_HIRES)) {
        gActiveViMode = &osViModeNtscHpf1;
    } else {
        s32 rightAdjust;
        s32 lowerAdjust;

    //! FAKE:
    l1:
        rightAdjust = gCfbWidth - 610;
        lowerAdjust = gCfbHeight - 470;
        ViMode_Configure(&sNotebookViMode, -1, osTvType, 0, 1, 0, 1, gCfbWidth, gCfbHeight, 30, rightAdjust, 10,
                         lowerAdjust);
        gActiveViMode = &sNotebookViMode;
    }
    gSysCfbHiResEnabled = true;
}

void SysCfb_Init(void) {
    sCfbLoRes1 = gFramebuffer1;
    sCfbLoRes0 = gFramebuffer0;
    sCfbHiRes1 = gFramebufferHiRes1;
    sCfbHiRes0 = gFramebufferHiRes0;
    SysCfb_SetLoResMode();
}

// Unused
void SysCfb_Reset(void) {
    gFramebuffers[0] = NULL;
    gFramebuffers[1] = NULL;
}

void* SysCfb_GetFramebuffer(s32 index) {
    if (index < 2) {
        return gFramebuffers[index];
    }
    return NULL;
}

void* SysCfb_GetZBuffer(void) {
    return gZBufferPtr;
}

void* SysCfb_GetWorkBuffer(void) {
    return gWorkBuffer;
}

u16 SysCfb_GetZBufferPixel(s32 x, s32 y) {
    u16* zBuff = SysCfb_GetZBuffer();
    u16 val;

    if (zBuff != NULL) {
        val = (&zBuff[x])[y * gCfbWidth];
    } else {
        val = 0;
    }
    return val;
}

s32 SysCfb_GetZBufferInt(s32 x, s32 y) {
    return Environment_ZBufValToFixedPoint(SysCfb_GetZBufferPixel(x, y) << 2) >> 3;
}
