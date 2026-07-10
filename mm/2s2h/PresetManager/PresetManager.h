
#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H

#include <string>
extern nlohmann::json voyage3PresetJ;

bool PresetManager_HandleFileDropped(const std::string& filePath);
void PresetManager_ApplyPreset(nlohmann::json j);
void PresetManager_Draw();

#endif // PRESET_MANAGER_H
