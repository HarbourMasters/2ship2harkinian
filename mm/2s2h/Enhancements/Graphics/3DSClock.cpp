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

static TexturePtr sDigitTextures[] = {
    (TexturePtr)gFinalHoursClockDigit0Tex, (TexturePtr)gFinalHoursClockDigit1Tex, (TexturePtr)gFinalHoursClockDigit2Tex,
    (TexturePtr)gFinalHoursClockDigit3Tex, (TexturePtr)gFinalHoursClockDigit4Tex, (TexturePtr)gFinalHoursClockDigit5Tex,
    (TexturePtr)gFinalHoursClockDigit6Tex, (TexturePtr)gFinalHoursClockDigit7Tex, (TexturePtr)gFinalHoursClockDigit8Tex,
    (TexturePtr)gFinalHoursClockDigit9Tex, (TexturePtr)gFinalHoursClockColonTex,
};

static TexturePtr sFinalHoursDigitTextures[] = {
    (TexturePtr)gThreeDayClock3DSFinalHoursDigit0Tex, (TexturePtr)gThreeDayClock3DSFinalHoursDigit1Tex,
    (TexturePtr)gThreeDayClock3DSFinalHoursDigit2Tex, (TexturePtr)gThreeDayClock3DSFinalHoursDigit3Tex,
    (TexturePtr)gThreeDayClock3DSFinalHoursDigit4Tex, (TexturePtr)gThreeDayClock3DSFinalHoursDigit5Tex,
    (TexturePtr)gThreeDayClock3DSFinalHoursDigit6Tex, (TexturePtr)gThreeDayClock3DSFinalHoursDigit7Tex,
    (TexturePtr)gThreeDayClock3DSFinalHoursDigit8Tex, (TexturePtr)gThreeDayClock3DSFinalHoursDigit9Tex,
    (TexturePtr)gThreeDayClock3DSFinalHoursColonTex,
};

s16 finalHoursClockSlots[8];

void Register3DSClock() {

    static s16 sThreeDayClockAlpha = 255;

    static s32 sPreviousTimeCheck = -1;

    static s32 sFinalHoursIntro = 0;

    REGISTER_VB_SHOULD(GI_VB_PREVENT_CLOCK_DISPLAY, {
        if (CVarGetInteger("gEnhancements.Graphics.ClockType", CLOCK_TYPE_ORIGINAL) == CLOCK_TYPE_3DS) {
            *should = true;

            HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
            if (HudEditor_IsActiveElementHidden()) {
                return;
            }

            if ((R_TIME_SPEED != 0) &&
                ((gPlayState->msgCtx.msgMode == MSGMODE_NONE) ||
                 ((gPlayState->actorCtx.flags & ACTORCTX_FLAG_1) && !Play_InCsMode(gPlayState)) ||
                 (gPlayState->msgCtx.msgMode == MSGMODE_NONE) ||
                 ((gPlayState->msgCtx.currentTextId >= 0x100) && (gPlayState->msgCtx.currentTextId <= 0x200)) ||
                 (gSaveContext.gameMode == GAMEMODE_END_CREDITS)) &&
                !FrameAdvance_IsEnabled(&gPlayState->state) && !Environment_IsTimeStopped() &&
                (gSaveContext.save.day <= 3)) {

                sThreeDayClockAlpha = gPlayState->interfaceCtx.bAlpha;

                if ((gPlayState->pauseCtx.state == PAUSE_STATE_OFF) &&
                    (gPlayState->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {

                    OPEN_DISPS(gPlayState->state.gfxCtx);
                    s16 posX = 160;
                    s16 posY = 210;

                    // Draw background of 3DS clock
                    // TODO: Replace this with matrix/vertex handling to avoid gaps when scaled
                    OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSEdgeTex, 72,
                                                                 12, posX - 24 - 72, posY, 72, 12, 1 << 10, 1 << 10,
                                                                 255, 255, 255, sThreeDayClockAlpha);

                    HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                    OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadow(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSMiddleTex,
                                                                 48, 12, posX - 24, posY, 48, 12, 1 << 10, 1 << 10, 255,
                                                                 255, 255, sThreeDayClockAlpha);

                    HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                    OVERLAY_DISP = Gfx_DrawTexRectIA8_DropShadowOffset(
                        OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSEdgeTex, 72, 12, posX + 24, posY, 72, 12, 1 << 10,
                        1 << 10, 255, 255, 255, sThreeDayClockAlpha, 3, 72 << 5);

                    // Coloured day markers
                    u16 fillalpha = 64;
                    if (gSaveContext.save.day <= 1) {
                        fillalpha = 255;
                    }
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 128, 255, (fillalpha * sThreeDayClockAlpha) / 255);

                    HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                    OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSFillTex, 48, 12,
                                                      posX - 24 - 48, posY, 48, 12, 1 << 10, 1 << 10);

                    fillalpha = 64;
                    if (gSaveContext.save.day == 2) {
                        fillalpha = 255;
                    }
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 192, 0, (fillalpha * sThreeDayClockAlpha) / 255);

                    HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                    OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSFillTex, 48, 12,
                                                      posX - 24, posY, 48, 12, 1 << 10, 1 << 10);

                    fillalpha = 64;
                    if (gSaveContext.save.day >= 3) {
                        fillalpha = 255;
                    }
                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 64, 0, (fillalpha * sThreeDayClockAlpha) / 255);

                    HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                    OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSFillTex, 48, 12,
                                                      posX + 24, posY, 48, 12, 1 << 10, 1 << 10);

                    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 0, 0, sThreeDayClockAlpha);

                    // Compute percentage time through the 3 day cycle and draw the current time arrow
                    s32 currentTime = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) - 360;
                    if (currentTime < 0) {
                        currentTime += 1440;
                    }

                    s32 timeUntilCrash = (s32)TIME_TO_SECONDS_F(TIME_UNTIL_MOON_CRASH);

                    // This is a safety measure to delay skipping the arrow marker back a day on the frame before the
                    // day changes by just checking if time went backwards
                    if (sPreviousTimeCheck != -1 && sPreviousTimeCheck > 259200 - timeUntilCrash) {
                        s32 trueTimeUntilCrash = timeUntilCrash;
                        timeUntilCrash = 259200 - sPreviousTimeCheck;
                        sPreviousTimeCheck = 259200 - trueTimeUntilCrash;
                    } else {
                        sPreviousTimeCheck = 259200 - timeUntilCrash;
                    }

                    s32 timeoffset = std::max(std::min(3 * 48 - (timeUntilCrash * 3 * 48) / 259200, 3 * 48), 0);

                    u16 counterX = posX - 24 - 48 + timeoffset;
                    u16 counterY = posY - 4;

                    HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                    OVERLAY_DISP = Gfx_DrawTexRectIA8(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSArrowTex, 8, 8,
                                                      counterX - 4, posY + 4, 8, 8, 1 << 10, 1 << 10);

                    u16 curMinutes = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) % 60;
                    u16 curHours = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) / 60;

                    // Final hours timer
                    if ((CURRENT_DAY >= 4) ||
                        ((CURRENT_DAY == 3) && (((void)0, gSaveContext.save.time) >= (CLOCK_TIME(0, 0) + 5)) &&
                         (((void)0, gSaveContext.save.time) < CLOCK_TIME(6, 0)))) {

                        s32 timeInSeconds = timeUntilCrash % 60;
                        s32 timeInMinutes = (timeUntilCrash / 60) % 60;
                        s32 timeInHours = (timeUntilCrash / 60) / 60;

                        finalHoursClockSlots[0] = std::min(timeInHours / 10, 9);
                        finalHoursClockSlots[1] = timeInHours % 10;
                        finalHoursClockSlots[2] = 10;
                        finalHoursClockSlots[3] = timeInMinutes / 10;
                        finalHoursClockSlots[4] = timeInMinutes % 10;
                        finalHoursClockSlots[5] = 10;

                        finalHoursClockSlots[6] = timeInSeconds / 10;
                        finalHoursClockSlots[7] = timeInSeconds % 10;

                        u16 finalTimerSpacing = 8;

                        u16 finalTimerPos = posX - finalTimerSpacing * 4 - finalTimerSpacing / 2;
                        s16 i;

                        s32 percentToCrash = std::min(std::max((timeUntilCrash * 255) / 21600, 0), 255);

                        u16 finalHoursR1 = 255;
                        u16 finalHoursR2 = 255;
                        u16 finalHoursG1 = 0;
                        u16 finalHoursG2 = 255;
                        u16 finalHoursB1 = 0;
                        u16 finalHoursB2 = 0;

                        u16 finalHoursR =
                            (((255 - percentToCrash) * finalHoursR1) + (percentToCrash * finalHoursR2)) / 255;
                        u16 finalHoursG =
                            (((255 - percentToCrash) * finalHoursG1) + (percentToCrash * finalHoursG2)) / 255;
                        u16 finalHoursB =
                            (((255 - percentToCrash) * finalHoursB1) + (percentToCrash * finalHoursB2)) / 255;

                        s32 finalHoursOffset = 10;
                        s32 finalHoursModifier = 2;
                        if (sFinalHoursIntro < finalHoursOffset * finalHoursModifier) {
                            sFinalHoursIntro++;
                        }

                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, finalHoursR, finalHoursG, finalHoursB,
                                        (sThreeDayClockAlpha * sFinalHoursIntro) /
                                            (finalHoursOffset * finalHoursModifier));

                        for (i = 0; i < 8; i++) {

                            HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                            OVERLAY_DISP = Gfx_DrawTexRectIA8(
                                OVERLAY_DISP, sFinalHoursDigitTextures[finalHoursClockSlots[i]], 16, 16, finalTimerPos,
                                posY - 14 - finalHoursOffset + (sFinalHoursIntro / finalHoursModifier), 16, 16, 1 << 10,
                                1 << 10);
                            finalTimerPos += finalTimerSpacing;
                        }
                        HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                        OVERLAY_DISP = Gfx_DrawTexRectIA8(
                            OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSFinalHoursMoonTex, 16, 16, posX - 8,
                            posY - 28 - finalHoursOffset + (sFinalHoursIntro / finalHoursModifier), 16, 16, 1 << 10,
                            1 << 10);
                    } else {

                        sFinalHoursIntro = 0;

                        // Draw the current time in a small box
                        // TODO: Add scale slider to allow this box to be scaled independently of the main UI element
                        if (gSaveContext.save.timeSpeedOffset == -2) {
                            u16 pulseTime = ((s32)TIME_TO_SECONDS_F(gSaveContext.save.time) % 120);
                            u16 pulse;
                            if (pulseTime < 60) {
                                pulse = (pulseTime * 255) / 60;
                            } else {
                                pulse = 255 - ((pulseTime - 60) * 255) / 60;
                            }

                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, pulse, pulse, 0, sThreeDayClockAlpha / 2);
                        } else {
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 0, 0, 0, sThreeDayClockAlpha / 2);
                        }
                        HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                        OVERLAY_DISP =
                            Gfx_DrawTexRectIA8(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSTimeBackdropTex, 48, 16,
                                               counterX - 24, counterY - 4, 48, 16, 1 << 10, 1 << 10);

                        counterX -= 8;

                        TexturePtr daynightmarker;
                        if (curHours < 6 || curHours >= 18) {
                            daynightmarker = (TexturePtr)gThreeDayClockMoonHourTex;
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 200, 0, sThreeDayClockAlpha);
                        } else {
                            daynightmarker = (TexturePtr)gThreeDayClockSunHourTex;
                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 200, 64, 0, sThreeDayClockAlpha);
                        }
                        HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                        OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, daynightmarker, 24, 24, counterX - 11,
                                                         counterY - 2, 12, 12, 1 << 11, 1 << 11);
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

                        gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, sThreeDayClockAlpha);

                        // Inverted time
                        if (gSaveContext.save.timeSpeedOffset == -2) {
                            HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                            OVERLAY_DISP =
                                Gfx_DrawTexRectIA8(OVERLAY_DISP, (TexturePtr)gThreeDayClock3DSSlowTimeTex, 16, 16,
                                                   counterX + 8 + 20, counterY - 4, 16, 16, 1 << 10, 1 << 10);

                            gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 64, 192, 255, sThreeDayClockAlpha);
                        }

                        // Digital time
                        u16 timerSpacing = 6;

                        if (curTensHours > 0) {
                            timerSpacing = 4;

                            HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                            OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sDigitTextures[curTensHours], 8, 8, counterX,
                                                             counterY, 8, 8, 1 << 10, 1 << 10);
                            counterX += timerSpacing;
                        }

                        HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                        OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sDigitTextures[curHours], 8, 8, counterX,
                                                         counterY, 8, 8, 1 << 10, 1 << 10);
                        counterX += timerSpacing;

                        HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                        OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sDigitTextures[10], 8, 8, counterX, counterY, 8,
                                                         8, 1 << 10, 1 << 10);
                        counterX += timerSpacing;

                        HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                        OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sDigitTextures[curTensMinutes], 8, 8, counterX,
                                                         counterY, 8, 8, 1 << 10, 1 << 10);
                        counterX += timerSpacing;

                        HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_CLOCK);
                        OVERLAY_DISP = Gfx_DrawTexRectI8(OVERLAY_DISP, sDigitTextures[curMinutes], 8, 8, counterX,
                                                         counterY, 8, 8, 1 << 10, 1 << 10);
                    }
                    gDPPipeSync(OVERLAY_DISP++);

                    CLOSE_DISPS(gPlayState->state.gfxCtx);
                } else {
                    sPreviousTimeCheck = 259200 - (s32)TIME_TO_SECONDS_F(TIME_UNTIL_MOON_CRASH);
                }
            } else {
                sPreviousTimeCheck = 259200 - (s32)TIME_TO_SECONDS_F(TIME_UNTIL_MOON_CRASH);
            }

            hudEditorActiveElement = HUD_EDITOR_ELEMENT_NONE;
        }
    });
}
