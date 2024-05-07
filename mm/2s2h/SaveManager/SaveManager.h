
#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <libultraship/libultraship.h>

#ifndef __cplusplus
void SaveManager_SysFlashrom_WriteData(u8* addr, u32 pageNum, u32 pageCount);
s32 SaveManager_SysFlashrom_ReadData(void* addr, u32 pageNum, u32 pageCount);
#endif

#endif // SAVE_MANAGER_H
