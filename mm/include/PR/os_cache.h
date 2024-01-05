#ifndef PR_OS_CACHE_H
#define PR_OS_CACHE_H

#include "ultratypes.h"
#include "libc/stddef.h"

void osInvalDCache(void* vaddr, size_t nbytes);
void osInvalICache(void* vaddr, size_t nbytes);
void osWritebackDCache(void* vaddr, s32 nbytes);
void osWritebackDCacheAll(void);

#endif
