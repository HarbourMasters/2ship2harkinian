
#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H

#include <string>
#include <vector>

// Added to allow the preset button to function properly under the custom Races Menu.
extern nlohmann::json voyage3PresetJ;

bool PresetManager_HandleFileDropped(const std::string& filePath);
void PresetManager_ApplyPreset(nlohmann::json j);
void PresetManager_Draw();

std::vector<std::string> PresetManager_GetPresetNames();
bool PresetManager_ApplyPresetByName(const std::string& name);

#endif // PRESET_MANAGER_H
