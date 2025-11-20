
#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H

#include <string>
#include <nlohmann/json.hpp>

extern nlohmann::json vanillaEnhancedPresetJ;
extern nlohmann::json defaultsPresetJ;

bool PresetManager_HandleFileDropped(const std::string& filePath);
void PresetManager_Draw();
void PresetManager_ApplyPreset(nlohmann::json j);

#endif // PRESET_MANAGER_H
