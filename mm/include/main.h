#ifndef MAIN_H
#define MAIN_H

#include "ultra64.h"
#include "scheduler.h"
#include "padmgr.h"

extern s32 gScreenWidth;
extern s32 gScreenHeight;
extern size_t gSystemHeapSize;

extern uintptr_t gSegments[NUM_SEGMENTS];
extern SchedContext gSchedContext;
extern OSThread gGraphThread;
extern PadMgr gPadMgr;

void Main(void* arg);

#endif
