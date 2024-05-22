#include "SaveManager.h"

#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "macros.h"
#include "BenJsonConversions.hpp"

extern "C" {
#include "src/overlays/gamestates/ovl_file_choose/z_file_select.h"
extern FileSelectState* gFileSelectState;
}

// This entire thing is temporary until we have a more robust save system that
// supports backwards compatability, migrations, threaded saving, save sections, etc.
typedef enum FlashSlotFile {
    /* -1 */ FLASH_SLOT_FILE_UNAVAILABLE = -1,
    /*  0 */ FLASH_SLOT_FILE_1_NEW_CYCLE,
    /*  1 */ FLASH_SLOT_FILE_1_NEW_CYCLE_BACKUP,
    /*  2 */ FLASH_SLOT_FILE_2_NEW_CYCLE,
    /*  3 */ FLASH_SLOT_FILE_2_NEW_CYCLE_BACKUP,
    /*  4 */ FLASH_SLOT_FILE_1_OWL_SAVE,
    /*  5 */ FLASH_SLOT_FILE_1_OWL_SAVE_BACKUP,
    /*  6 */ FLASH_SLOT_FILE_2_OWL_SAVE,
    /*  7 */ FLASH_SLOT_FILE_2_OWL_SAVE_BACKUP,
    /*  8 */ FLASH_SLOT_FILE_SRAM_HEADER,
    /*  9 */ FLASH_SLOT_FILE_SRAM_HEADER_BACKUP,
} FlashSlotFile;

#define GET_NEWF(save, index) (save.saveInfo.playerData.newf[index])
#define IS_VALID_FILE(save)                                                                    \
    ((GET_NEWF(save, 0) == 'Z') && (GET_NEWF(save, 1) == 'E') && (GET_NEWF(save, 2) == 'L') && \
     (GET_NEWF(save, 3) == 'D') && (GET_NEWF(save, 4) == 'A') && (GET_NEWF(save, 5) == '3'))

const std::filesystem::path savesFolderPath(Ship::Context::GetPathRelativeToAppDirectory("Save"));

// Migrations
// The idea here is that we can read in any version of the save as generic JSON, then apply migrations
// to the JSON to ensure it's in the correct shape for the current to_json/from_json helpers to convert
// it to the current struct that the game uses.
//
// To add a new migration:
// - Increment CURRENT_SAVE_VERSION
// - Create the migration file in the Migrations folder with the name `{CURRENT_SAVE_VERSION}.cpp`
// - Add the migration function definition below and add it to the `migrations` map with the key being the previous
// version
const uint32_t CURRENT_SAVE_VERSION = 3;

void SaveManager_Migration_1(nlohmann::json& j);
void SaveManager_Migration_2(nlohmann::json& j);
void SaveManager_Migration_3(nlohmann::json& j);

const std::unordered_map<uint32_t, std::function<void(nlohmann::json&)>> migrations = {
    { 0, SaveManager_Migration_1 },
    { 1, SaveManager_Migration_2 },
    { 2, SaveManager_Migration_3 },
};

void SaveManager_MigrateSave(nlohmann::json& j) {
    int version = j.value("version", 0);

    // BENTODO: what should we do if the version number is greater?
    while (version < CURRENT_SAVE_VERSION) {
        if (migrations.contains(version)) {
            auto migration = migrations.at(version);
            migration(j);
        }
        version = j["version"] = version + 1;
    }
}

void SaveManager_WriteSaveFile(std::filesystem::path fileName, nlohmann::json j) {
    const std::filesystem::path filePath = savesFolderPath / fileName;

    if (!std::filesystem::exists(savesFolderPath)) {
        std::filesystem::create_directory(savesFolderPath);
    }

    std::ofstream o(filePath);
    o << std::setw(4) << j << std::endl;
    o.close();
}

void SaveManager_DeleteSaveFile(std::filesystem::path fileName) {
    const std::filesystem::path filePath = savesFolderPath / fileName;

    if (std::filesystem::exists(filePath)) {
        std::filesystem::remove(filePath);
    }
}

int SaveManager_ReadSaveFile(std::filesystem::path fileName, nlohmann::json& j) {
    const std::filesystem::path filePath = savesFolderPath / fileName;

    if (!std::filesystem::exists(filePath)) {
        return -1;
    }

    std::ifstream i(filePath);
    i >> j;
    i.close();
    return 0;
}

int SaveManager_GetOpenFileSlot() {
    std::string fileName = "save_0.sav";
    if (!std::filesystem::exists(savesFolderPath / fileName)) {
        return 0;
    }

    fileName = "save_2.sav";
    if (!std::filesystem::exists(savesFolderPath / fileName)) {
        return 2;
    }

    return -1;
}

bool SaveManager_HandleFileDropped(std::string filePath) {
    try {
        std::ifstream fileStream(filePath);

        if (!fileStream.is_open()) {
            return false;
        }

        // Check if first byte is "{"
        if (fileStream.peek() != '{') {
            return false;
        }

        nlohmann::json j;
        try {
            fileStream >> j;
        } catch (nlohmann::json::parse_error& e) {
            SPDLOG_ERROR("Failed to parse JSON: {}", e.what());
            return false;
        }

        if (!j.contains("type") || j["type"] != "2S2H_SAVE") {
            SPDLOG_ERROR("Invalid save file type");
            return false;
        }

        int saveSlot = SaveManager_GetOpenFileSlot();
        if (saveSlot == -1) {
            SPDLOG_ERROR("No save slot available");
            auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
            gui->GetGameOverlay()->TextDrawNotification(30.0f, true, "No save slot available");
            return true;
        }

        std::string fileName = "save_" + std::to_string(saveSlot) + ".sav";

        SaveManager_WriteSaveFile(fileName, j);

        if (gFileSelectState != NULL) {
            func_801457CC(&gFileSelectState->state, &gFileSelectState->sramCtx);
            if (gFileSelectState->menuMode == FS_MENU_MODE_CONFIG && gFileSelectState->configMode == CM_MAIN_MENU) {
                gFileSelectState->configMode = CM_FADE_IN_START;
            }
        }

        return true;
    } catch (std::exception& e) {
        SPDLOG_ERROR("Failed to load file: {}", e.what());
        auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
        gui->GetGameOverlay()->TextDrawNotification(30.0f, true, "Failed to load file");
        return false;
    } catch (...) {
        SPDLOG_ERROR("Failed to load file");
        auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
        gui->GetGameOverlay()->TextDrawNotification(30.0f, true, "Failed to load file");
        return false;
    }
}

extern "C" void SaveManager_SysFlashrom_WriteData(u8* saveBuffer, u32 pageNum, u32 pageCount) {
    FlashSlotFile flashSlotFile = FLASH_SLOT_FILE_UNAVAILABLE;
    bool isBackup = false;
    for (u32 i = 0; i < ARRAY_COUNT(gFlashSaveStartPages) - 1; i++) {
        // Verify that the requested pages align with expected values
        if (pageNum == (u32)gFlashSaveStartPages[i] &&
            (pageCount == (u32)gFlashSaveNumPages[i] || pageCount == (u32)gFlashSpecialSaveNumPages[i])) {
            flashSlotFile = static_cast<FlashSlotFile>(i);
            break;
        }
    }

    switch (flashSlotFile) {
        case FLASH_SLOT_FILE_UNAVAILABLE:
            return;
        case FLASH_SLOT_FILE_1_NEW_CYCLE_BACKUP:
        case FLASH_SLOT_FILE_2_NEW_CYCLE_BACKUP:
            isBackup = true;
            // fallthrough
        case FLASH_SLOT_FILE_1_NEW_CYCLE:
        case FLASH_SLOT_FILE_2_NEW_CYCLE: {
            Save save;
            memcpy(&save, saveBuffer, sizeof(Save));

            std::string fileName = "save_" + std::to_string(flashSlotFile) + ".sav";
            if (isBackup)
                fileName += ".bak";

            if (IS_VALID_FILE(save)) {
                nlohmann::json j;

                j["save"] = save;
                j["version"] = CURRENT_SAVE_VERSION;
                j["type"] = "2S2H_SAVE";

                SaveManager_WriteSaveFile(fileName, j);
            } else {
                SaveManager_DeleteSaveFile(fileName);
            }
            break;
        }
        case FLASH_SLOT_FILE_1_OWL_SAVE_BACKUP:
        case FLASH_SLOT_FILE_2_OWL_SAVE_BACKUP:
            isBackup = true;
            // fallthrough
        case FLASH_SLOT_FILE_1_OWL_SAVE:
        case FLASH_SLOT_FILE_2_OWL_SAVE: {
            SaveContext saveContext;
            memcpy(&saveContext, saveBuffer, offsetof(SaveContext, fileNum));

            std::string fileName = "save_" + std::to_string(flashSlotFile) + ".sav";
            if (isBackup)
                fileName += ".bak";

            if (IS_VALID_FILE(saveContext.save)) {
                nlohmann::json j = saveContext;

                j["version"] = CURRENT_SAVE_VERSION;
                j["type"] = "2S2H_SAVE";

                SaveManager_WriteSaveFile(fileName, j);
            } else {
                SaveManager_DeleteSaveFile(fileName);
            }
            break;
        }
        case FLASH_SLOT_FILE_SRAM_HEADER_BACKUP:
        case FLASH_SLOT_FILE_SRAM_HEADER: {
            SaveOptions saveOptions;
            memcpy(&saveOptions, saveBuffer, sizeof(SaveOptions));

            std::string fileName = "global.sav";
            SaveManager_WriteSaveFile(fileName, saveOptions);
            break;
        }
    }
}

extern "C" s32 SaveManager_SysFlashrom_ReadData(void* saveBuffer, u32 pageNum, u32 pageCount) {
    FlashSlotFile flashSlotFile = FLASH_SLOT_FILE_UNAVAILABLE;
    bool isBackup = false;
    for (u32 i = 0; i < ARRAY_COUNT(gFlashSaveStartPages) - 1; i++) {
        // Verify that the requested pages align with expected values
        if (pageNum == (u32)gFlashSaveStartPages[i] &&
            (pageCount == (u32)gFlashSaveNumPages[i] || pageCount == (u32)gFlashSpecialSaveNumPages[i])) {
            flashSlotFile = static_cast<FlashSlotFile>(i);
            break;
        }
    }

    switch (flashSlotFile) {
        case FLASH_SLOT_FILE_UNAVAILABLE:
            return -1;
        case FLASH_SLOT_FILE_1_NEW_CYCLE_BACKUP:
        case FLASH_SLOT_FILE_2_NEW_CYCLE_BACKUP:
            isBackup = true;
            // fallthrough
        case FLASH_SLOT_FILE_1_NEW_CYCLE:
        case FLASH_SLOT_FILE_2_NEW_CYCLE: {
            std::string fileName = "save_" + std::to_string(flashSlotFile) + ".sav";
            if (isBackup)
                fileName += ".bak";

            nlohmann::json j;
            int result = SaveManager_ReadSaveFile(fileName, j);
            if (result != 0)
                return result;

            SaveManager_MigrateSave(j);

            Save save = j["save"];

            memcpy(saveBuffer, &save, sizeof(Save));
            return result;
        }
        case FLASH_SLOT_FILE_1_OWL_SAVE_BACKUP:
        case FLASH_SLOT_FILE_2_OWL_SAVE_BACKUP:
            isBackup = true;
            // fallthrough
        case FLASH_SLOT_FILE_1_OWL_SAVE:
        case FLASH_SLOT_FILE_2_OWL_SAVE: {
            std::string fileName = "save_" + std::to_string(flashSlotFile) + ".sav";
            if (isBackup)
                fileName += ".bak";

            nlohmann::json j;
            int result = SaveManager_ReadSaveFile(fileName, j);
            if (result != 0)
                return result;

            SaveManager_MigrateSave(j);

            SaveContext saveContext = j;

            memcpy(saveBuffer, &saveContext, offsetof(SaveContext, fileNum));
            return result;
        }
        case FLASH_SLOT_FILE_SRAM_HEADER:
        case FLASH_SLOT_FILE_SRAM_HEADER_BACKUP: {
            std::string fileName = "global.sav";

            nlohmann::json j;
            int result = SaveManager_ReadSaveFile(fileName, j);
            if (result != 0)
                return result;

            SaveOptions saveOptions = j;

            memcpy(saveBuffer, &saveOptions, sizeof(SaveOptions));
            return 0;
        }
    }
}
