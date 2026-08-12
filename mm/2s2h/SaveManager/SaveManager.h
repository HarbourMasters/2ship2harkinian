
#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#ifdef __cplusplus
#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>
std::string SaveManager_GetFileName(int fileNum, bool isBackup = false);
bool SaveManager_HandleFileDropped(char* filePath);
bool BinarySaveConverter_HandleFileDropped(char* filePath);
int SaveManager_GetOpenFileSlot();
void SaveManager_WriteSaveFile(const std::filesystem::path& fileName, nlohmann::json j);
// Remove a save file from the saves folder (no-op when it isn't there). Used by the fleet combo to
// keep MM's derived files paired with OoT's: OoT erases a file, MM's half goes with it.
void SaveManager_DeleteSaveFile(const std::filesystem::path& fileName);
// Read a save file's raw json WITHOUT loading it into the game. 0 = ok, -1 = no such file,
// -2 = unreadable/not json. Used by the fleet combo to check a slot's seed before booting it.
int SaveManager_ReadSaveFile(const std::filesystem::path& fileName, nlohmann::json& j);
#else
void SaveManager_SysFlashrom_WriteData(u8* addr, u32 pageNum, u32 pageCount);
s32 SaveManager_SysFlashrom_ReadData(void* addr, u32 pageNum, u32 pageCount);
#endif

#endif // SAVE_MANAGER_H
