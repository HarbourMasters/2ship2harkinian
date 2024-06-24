#include <libultraship/libultraship.h>
#include "2s2h/BenGui/HudEditor.h"
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"
#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "misc/title_static/title_static.h"
#include "2s2h_assets.h"

extern "C" {
#include <z64.h>
#include <macros.h>
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "interface/parameter_static/parameter_static.h"

extern PlayState* gPlayState;
Gfx* Gfx_DrawTexRectIA8(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft, s16 rectTop,
                        s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy);

Gfx* Gfx_DrawTexRectIA8_DropShadow(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, s16 rectLeft,
                                   s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy, s16 r, s16 g, s16 b,
                                   s16 a);

Gfx* Gfx_DrawTexRectIA8_DropShadowOffset(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight,
                                          s16 rectLeft, s16 rectTop, s16 rectWidth, s16 rectHeight, u16 dsdx, u16 dtdy,
                                          s16 r, s16 g, s16 b, s16 a, s32 masks, s32 rects);

}

static TexturePtr sFinalHoursDigitTextures[] = {
    (TexturePtr)gFinalHoursClockDigit0Tex,
    (TexturePtr)gFinalHoursClockDigit1Tex,
    (TexturePtr)gFinalHoursClockDigit2Tex,
    (TexturePtr)gFinalHoursClockDigit3Tex,
    (TexturePtr)gFinalHoursClockDigit4Tex,
    (TexturePtr)gFinalHoursClockDigit5Tex,
    (TexturePtr)gFinalHoursClockDigit6Tex,
    (TexturePtr)gFinalHoursClockDigit7Tex,
    (TexturePtr)gFinalHoursClockDigit8Tex,
    (TexturePtr)gFinalHoursClockDigit9Tex, 
    (TexturePtr)gFinalHoursClockColonTex,
};


void Register3DSClock() {
    REGISTER_VB_SHOULD(GI_VB_PREVENT_CLOCK_DISPLAY, {
        if (CVarGetInteger("gEnhancements.Graphics.ClockType", CLOCK_TYPE_ORIGINAL) == CLOCK_TYPE_3DS) {
            *should = true;

            HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_NONE);
            if (HudEditor_IsActiveElementHidden()) {
                return;
            }
            OPEN_DISPS(gPlayState->state.gfxCtx);

            
           if ((gPlayState->pauseCtx.state == PAUSE_STATE_OFF) &&
                (gPlayState->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {

            s16 posX = 160;
            s16 posY = 204;

            //Draw background of 3D clock
            OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSEdgeTex, 72, 12,
                                                          posX-24-72, posY, 72, 12, 1 << 10, 1 << 10, 255, 255, 255, 255);
            OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSMiddleTex, 48, 12,
                                                          posX-24, posY, 48, 12, 1 << 10, 1 << 10, 255, 255, 255, 255);
            OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadowOffset(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSEdgeTex, 72, 12, posX + 24, posY, 72, 12, 1 << 10, 1 << 10, 255,
                                                                255, 255, 255, 3, 72<<5);

            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 0, 0, 255);

            //Compute percentage time through the 3 day cycle and draw the current time arrow
            s32 currentTime = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) - 360;
            if (currentTime < 0) {
                currentTime += 1440;
            }

            s32 timeoffset = std::max(std::min(((currentTime + (gSaveContext.save.day - 1) * 1440) * (3 * 48)) / 4320, 3 * 48), 0);

            u16 counterX = posX - 24 - 48 - 4 + timeoffset;
            u16 counterY = posY - 4;

            OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSArrowTex, 8, 8,
                                              counterX,
                                              posY+4, 8, 8, 1 << 10, 1 << 10);


            //Draw the current time in a small box
            u16 curMinutes = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) % 60;
            u16 curHours = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) / 60;

           gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, 255);
           OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSTimeBackdropTex, 38, 12,
                                             counterX - 12,
                                             counterY - 3, 38, 12,
                                             1 << 10, 1 << 10);

            TexturePtr daynightmarker;
            if (curHours < 6 || curHours >= 18) {
                daynightmarker = (TexturePtr)gThreeDayClockMoonHourTex;
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 200, 0, 255);
            }else{
                daynightmarker = (TexturePtr)gThreeDayClockSunHourTex;
                gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 64, 0, 255);
            }
            OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, daynightmarker, 24, 24, counterX - 12,
                                             counterY - 3, 12,
                                             12, 1 << 11, 1 << 11);
            if (!CVarGetInteger("gEnhancements.Graphics.24HoursClock", 0)) {
                curHours %= 12;
                if (curHours == 0) {
                    curHours = 12;
                }
            }

            u16 curTensHours = curHours / 10;
            curHours %= 10;

            u16 curTensMinutes = curMinutes / 10;
            curMinutes %= 10;

            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, 255);

            u16 timerSpacing = 6;

            if (curTensHours > 0) {
                timerSpacing = 4;
                OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sFinalHoursDigitTextures[curTensHours], 8, 8, counterX,
                                                 counterY, 8, 8,
                                                 1 << 10, 1 << 10);
                counterX += timerSpacing;
            }

            OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sFinalHoursDigitTextures[curHours], 8, 8, counterX, counterY,
                                             8,
                                             8, 1 << 10, 1 << 10);
            counterX += timerSpacing;

            OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sFinalHoursDigitTextures[10], 8, 8, counterX, counterY, 8,
                                             8, 1 << 10, 1 << 10);
            counterX += timerSpacing;

            
            OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sFinalHoursDigitTextures[curTensMinutes], 8, 8, counterX,
                                             counterY, 8, 8,
                                             1 << 10, 1 << 10);
            counterX += timerSpacing;

            OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sFinalHoursDigitTextures[curMinutes], 8, 8, counterX,
                                             counterY, 8, 8, 1 << 10, 1 << 10);

            gDPPipeSync(OVERLAY_DISP++);
            /* if ((R_TIME_SPEED != 0) &&
                ((gPlayState->msgCtx.msgMode == MSGMODE_NONE) ||
                 ((gPlayState->actorCtx.flags & ACTORCTX_FLAG_1) && !Play_InCsMode(gPlayState)) ||
                 (gPlayState->msgCtx.msgMode == MSGMODE_NONE) ||
                 ((gPlayState->msgCtx.currentTextId >= 0x100) && (gPlayState->msgCtx.currentTextId <= 0x200)) ||
                 (gSaveContext.gameMode == GAMEMODE_END_CREDITS)) &&
                !FrameAdvance_IsEnabled(&gPlayState->state) && !Environment_IsTimeStopped() &&
                (gSaveContext.save.day <= 3)) {

                OPEN_DISPS(gPlayState->state.gfxCtx);

                if ((gPlayState->pauseCtx.state == PAUSE_STATE_OFF) &&
                    (gPlayState->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {
                    Gfx_SetupDL39_Overlay(gPlayState->state.gfxCtx);

                    u16 curMinutes = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) % 60;
                    u16 curHours = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) / 60;

                    u16 timeUntilMoonCrash = (s32)TIME_UNTIL_MOON_CRASH;
                    u16 timeInMinutes = (s32)TIME_TO_MINUTES_F(timeUntilMoonCrash) % 60;
                    u16 timeInHours = (s32)TIME_TO_MINUTES_F(timeUntilMoonCrash) / 60;

                    char formattedTime[10];
                    char formattedCrashTime[10];

                    //
                    // Section: Draw Clock's Border
                    //
                    gDPPipeSync(OVERLAY_DISP++);

                    OPEN_PRINTER(OVERLAY_DISP);

                    // Inverted time
                    if (gSaveContext.save.timeSpeedOffset == -2) {
                        GfxPrint_SetColor(&printer, 0, 204, 255, 255);
                    } else {
                        GfxPrint_SetColor(&printer, 255, 255, 255, 255);
                    }

                    // Scale starting position values by 8 for use with hud editor
                    s16 posX = 13 * 8;
                    s16 posY = 26 * 8;

                    HudEditor_ModifyRectPosValues(&posX, &posY);

                    // scale back down and clamp to avoid negative values
                    posX = std::max(posX / 8, 0);
                    posY = std::max(posY / 8, 0);

                    if (CVarGetInteger("gEnhancements.Graphics.24HoursClock", 0)) {
                        sprintf(formattedTime, "%02d:%02d", curHours, curMinutes);
                        GfxPrint_SetPos(&printer, posX + 1, posY);
                    } else { // Format hours and minutes for 12-hour AM/PM clock
                        char amPm[3] = "am";
                        if (curHours >= 12) {
                            strcpy(amPm, "pm");
                            if (curHours > 12) {
                                curHours -= 12;
                            }
                        }
                        if (curHours == 0) {
                            curHours = 12;
                        }
                        sprintf(formattedTime, "%02d:%02d%s", curHours, curMinutes, amPm);
                        GfxPrint_SetPos(&printer, posX, posY);
                    }

                    GfxPrint_Printf(&printer, "Day %d: %s", gSaveContext.save.day, formattedTime);
                    GfxPrint_SetPos(&printer, posX, posY + 1);

                    // Crash Countdown
                    if ((CURRENT_DAY >= 4) ||
                        ((CURRENT_DAY == 3) && (((void)0, gSaveContext.save.time) >= (CLOCK_TIME(0, 0) + 5)) &&
                         (((void)0, gSaveContext.save.time) < CLOCK_TIME(6, 0)))) {
                        GfxPrint_SetColor(&printer, 255, 0, 0, 255);
                        sprintf(formattedCrashTime, "%02d:%02d", timeInHours, timeInMinutes);
                        GfxPrint_Printf(&printer, "Crash in %s", formattedCrashTime);
                    }

                    CLOSE_PRINTER(printer, OVERLAY_DISP);
                }
                */

                CLOSE_DISPS(gPlayState->state.gfxCtx);
            }

            hudEditorActiveElement = HUD_EDITOR_ELEMENT_NONE;
        }
    });
}
