#include <libultraship/libultraship.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "Enhancements/Enhancements.h"

extern "C" {
#include <z64.h>
#include <macros.h>
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

extern PlayState* gPlayState;
}

#define OPEN_PRINTER(printer, disp) \
    {                               \
        GfxPrint printer;           \
        GfxPrint_Init(&printer);    \
        GfxPrint_Open(&printer, disp);

#define CLOSE_PRINTER(printer, disp) \
    disp = GfxPrint_Close(&printer); \
    GfxPrint_Destroy(&printer);      \
    }  

void RegisterTextBasedClock() {
    REGISTER_VB_SHOULD(GI_VB_PREVENT_CLOCK_DISPLAY, {

        if (CVarGetInteger("gEnhancements.Graphics.ClockType", 0) == CLOCK_TYPE_HIDDEN) {
            *should = true;
            return;
        }

        if (CVarGetInteger("gEnhancements.Graphics.ClockType", 0) == CLOCK_TYPE_TEXT_BASED) {
            *should = true;
            if ((R_TIME_SPEED != 0) &&
                ((gPlayState->msgCtx.msgMode == MSGMODE_NONE) ||
                 ((gPlayState->actorCtx.flags & ACTORCTX_FLAG_1) && !Play_InCsMode(gPlayState)) ||
                 (gPlayState->msgCtx.msgMode == MSGMODE_NONE) ||
                 ((gPlayState->msgCtx.currentTextId >= 0x100) && (gPlayState->msgCtx.currentTextId <= 0x200)) ||
                 (gSaveContext.gameMode == GAMEMODE_END_CREDITS)) &&
                !FrameAdvance_IsEnabled(&gPlayState->state) && !Environment_IsTimeStopped() && (gSaveContext.save.day <= 3)) {
                OPEN_DISPS(gPlayState->state.gfxCtx);
                if ((gPlayState->pauseCtx.state == PAUSE_STATE_OFF) && (gPlayState->pauseCtx.debugEditor == DEBUG_EDITOR_NONE)) {
                    Gfx_SetupDL39_Overlay(gPlayState->state.gfxCtx);

                    u16 curMinutes = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) % 60;
                    u16 curHours = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) / 60;

                    u16 timeUntilMoonCrash = (s32)TIME_UNTIL_MOON_CRASH;
                    u16 timeInMinutes = (s32)TIME_TO_MINUTES_F(timeUntilMoonCrash) % 60;
                    u16 timeInHours = (s32)TIME_TO_MINUTES_F(timeUntilMoonCrash) / 60;

                    char formattedTime[10];
                    char formattedCrashTime[10];

                    OPEN_PRINTER(printer, OVERLAY_DISP);
                    GfxPrint_SetPos(&printer, 14, 26);
                    if (gSaveContext.save.timeSpeedOffset == -2) {
                        GfxPrint_SetColor(&printer, 0, 204, 255, 255);
                    } else {
                        GfxPrint_SetColor(&printer, 255, 255, 255, 255);
                    }

                    if (CVarGetInteger("gEnhancements.Graphics.24HoursClock", 0)) {
                        sprintf(formattedTime, "%02d:%02d", curHours, curMinutes);
                    } else { // Format hours and minutes for 12-hour AM/PM clock
                        char amPm[3] = "AM";
                        if (curHours >= 12) {
                            strcpy(amPm, "PM");
                            if (curHours > 12) {
                                curHours -= 12;
                            }
                        }
                        if (curHours == 0) {
                            curHours = 12;
                        }
                        sprintf(formattedTime, "%d:%02d %s", curHours, curMinutes, amPm);
                    }

                    GfxPrint_Printf(&printer, "Day %d: %s", gSaveContext.save.day, formattedTime);
                    if (CVarGetInteger("gEnhancements.Graphics.24HoursClock", 0)) {
                        GfxPrint_SetPos(&printer, 13, 27);
                    } else {
                        GfxPrint_SetPos(&printer, 14, 27);
                    }

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
                CLOSE_DISPS(gPlayState->state.gfxCtx);
            }
        }
    });
}