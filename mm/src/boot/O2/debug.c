#include "global.h"
#include "fault.h"

#if 0
void _dbg_hungup(const char* file, int lineNum) {
    #if 0
    osGetThreadId(NULL);
    #endif
    Fault_AddHungupAndCrash(file, lineNum);
}
#endif

void Reset(void) {
    Fault_AddHungupAndCrash("Reset", 0);
}
