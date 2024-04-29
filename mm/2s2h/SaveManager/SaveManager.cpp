#include "SaveManager.h"

#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>


#include "macros.h"
#include "BenJsonConversions.hpp"

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
#define IS_VALID_FILE(save)                                      \
    ((GET_NEWF(save, 0) == 'Z') && (GET_NEWF(save, 1) == 'E') && \
     (GET_NEWF(save, 2) == 'L') && (GET_NEWF(save, 3) == 'D') && \
     (GET_NEWF(save, 4) == 'A') && (GET_NEWF(save, 5) == '3'))

const std::filesystem::path savesFolderPath(Ship::Context::GetPathRelativeToAppDirectory("Save"));

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
            if (isBackup) fileName += ".bak";

            if (IS_VALID_FILE(save)) {
                SaveManager_WriteSaveFile(fileName, save);
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
            if (isBackup) fileName += ".bak";

            if (IS_VALID_FILE(saveContext.save)) {
                SaveManager_WriteSaveFile(fileName, saveContext);
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
            if (isBackup) fileName += ".bak";

            nlohmann::json j;
            int result = SaveManager_ReadSaveFile(fileName, j);
            if (result != 0) return result;

            Save save = j;

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
            if (isBackup) fileName += ".bak";

            nlohmann::json j;
            int result = SaveManager_ReadSaveFile(fileName, j);
            if (result != 0) return result;

            SaveContext saveContext = j;

            memcpy(saveBuffer, &saveContext, offsetof(SaveContext, fileNum));
            return result;
        }
        case FLASH_SLOT_FILE_SRAM_HEADER:
        case FLASH_SLOT_FILE_SRAM_HEADER_BACKUP: {
            std::string fileName = "global.sav";

            nlohmann::json j;
            int result = SaveManager_ReadSaveFile(fileName, j);
            if (result != 0) return result;

            SaveOptions saveOptions = j;

            memcpy(saveBuffer, &saveOptions, sizeof(SaveOptions));
            return 0;
        }
    }
}