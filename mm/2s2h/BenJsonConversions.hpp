#ifndef BenJsonConversions_hpp
#define BenJsonConversions_hpp

#include "z64.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void to_json(json& j, const ItemEquips& itemEquips) {
    j = json{
        { "buttonItems", itemEquips.buttonItems },
        { "cButtonSlots", itemEquips.cButtonSlots },
        { "equipment", itemEquips.equipment },
    };
}

void from_json(const json& j, ItemEquips& itemEquips) {
    j.at("equipment").get_to(itemEquips.equipment);
    // buttonItems and cButtonSlots are arrays of arrays, so we need to manually parse them
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            itemEquips.buttonItems[x][y] = j.at("buttonItems")[x][y].get<u8>();
            itemEquips.cButtonSlots[x][y] = j.at("cButtonSlots")[x][y].get<u8>();
        }
    }
}

void to_json(json& j, const Inventory& inventory) {
    j = json{
        { "items", inventory.items },
        { "ammo", inventory.ammo },
        { "upgrades", inventory.upgrades },
        { "questItems", inventory.questItems },
        { "dungeonItems", inventory.dungeonItems },
        { "dungeonKeys", inventory.dungeonKeys },
        { "defenseHearts", inventory.defenseHearts },
        { "strayFairies", inventory.strayFairies },
        { "dekuPlaygroundPlayerName", inventory.dekuPlaygroundPlayerName },
    };
}

void from_json(const json& j, Inventory& inventory) {
    j.at("items").get_to(inventory.items);
    j.at("ammo").get_to(inventory.ammo);
    j.at("upgrades").get_to(inventory.upgrades);
    j.at("questItems").get_to(inventory.questItems);
    j.at("dungeonItems").get_to(inventory.dungeonItems);
    j.at("dungeonKeys").get_to(inventory.dungeonKeys);
    j.at("defenseHearts").get_to(inventory.defenseHearts);
    j.at("strayFairies").get_to(inventory.strayFairies);
    // dekuPlaygroundPlayerName is an array of char arrays, so we need to manually parse it
    for (int i = 0; i < 3; i++) {
        std::string name = j.at("dekuPlaygroundPlayerName")[i].get<std::string>();
        for (int j = 0; j < 8; j++) {
            inventory.dekuPlaygroundPlayerName[i][j] = name[j];
        }
    }
}

void to_json(json& j, const PermanentSceneFlags& permanentSceneFlags) {
    j = json{
        { "chest", permanentSceneFlags.chest },
        { "switch0", permanentSceneFlags.switch0 },
        { "switch1", permanentSceneFlags.switch1 },
        { "clearedRoom", permanentSceneFlags.clearedRoom },
        { "collectible", permanentSceneFlags.collectible },
        { "unk_14", permanentSceneFlags.unk_14 },
        { "rooms", permanentSceneFlags.rooms },
    };
}

void from_json(const json& j, PermanentSceneFlags& permanentSceneFlags) {
    j.at("chest").get_to(permanentSceneFlags.chest);
    j.at("switch0").get_to(permanentSceneFlags.switch0);
    j.at("switch1").get_to(permanentSceneFlags.switch1);
    j.at("clearedRoom").get_to(permanentSceneFlags.clearedRoom);
    j.at("collectible").get_to(permanentSceneFlags.collectible);
    j.at("unk_14").get_to(permanentSceneFlags.unk_14);
    j.at("rooms").get_to(permanentSceneFlags.rooms);
}

void to_json(json& j, const SavePlayerData& savePlayerData) {
    j = json{
        { "newf", savePlayerData.newf },
        { "threeDayResetCount", savePlayerData.threeDayResetCount },
        { "playerName", savePlayerData.playerName },
        { "healthCapacity", savePlayerData.healthCapacity },
        { "health", savePlayerData.health },
        { "magicLevel", savePlayerData.magicLevel },
        { "magic", savePlayerData.magic },
        { "rupees", savePlayerData.rupees },
        { "swordHealth", savePlayerData.swordHealth },
        { "tatlTimer", savePlayerData.tatlTimer },
        { "isMagicAcquired", savePlayerData.isMagicAcquired },
        { "isDoubleMagicAcquired", savePlayerData.isDoubleMagicAcquired },
        { "doubleDefense", savePlayerData.doubleDefense },
        { "unk_1F", savePlayerData.unk_1F },
        { "unk_20", savePlayerData.unk_20 },
        { "owlActivationFlags", savePlayerData.owlActivationFlags },
        { "unk_24", savePlayerData.unk_24 },
        { "savedSceneId", savePlayerData.savedSceneId },
    };
}

void from_json(const json& j, SavePlayerData& savePlayerData) {
    // newf is an array of chars, so we need to manually parse it
    std::string newf = j.at("newf").get<std::string>();
    for (int i = 0; i < 6; i++) {
        savePlayerData.newf[i] = newf[i];
    }
    j.at("threeDayResetCount").get_to(savePlayerData.threeDayResetCount);
    std::string playerName = j.at("playerName").get<std::string>();
    for (int i = 0; i < 8; i++) {
        savePlayerData.playerName[i] = playerName[i];
    }
    j.at("healthCapacity").get_to(savePlayerData.healthCapacity);
    j.at("health").get_to(savePlayerData.health);
    j.at("magicLevel").get_to(savePlayerData.magicLevel);
    j.at("magic").get_to(savePlayerData.magic);
    j.at("rupees").get_to(savePlayerData.rupees);
    j.at("swordHealth").get_to(savePlayerData.swordHealth);
    j.at("tatlTimer").get_to(savePlayerData.tatlTimer);
    j.at("isMagicAcquired").get_to(savePlayerData.isMagicAcquired);
    j.at("isDoubleMagicAcquired").get_to(savePlayerData.isDoubleMagicAcquired);
    j.at("doubleDefense").get_to(savePlayerData.doubleDefense);
    j.at("unk_1F").get_to(savePlayerData.unk_1F);
    j.at("unk_20").get_to(savePlayerData.unk_20);
    j.at("owlActivationFlags").get_to(savePlayerData.owlActivationFlags);
    j.at("unk_24").get_to(savePlayerData.unk_24);
    j.at("savedSceneId").get_to(savePlayerData.savedSceneId);
}

void to_json(json& j, const Vec3s& vec) {
    j = json{
        { "x", vec.x },
        { "y", vec.y },
        { "z", vec.z },
    };
}

void from_json(const json& j, Vec3s& vec) {
    j.at("x").get_to(vec.x);
    j.at("y").get_to(vec.y);
    j.at("z").get_to(vec.z);
}

void to_json(json& j, const HorseData& horseData) {
    j = json{
        { "sceneId", horseData.sceneId },
        { "pos", horseData.pos },
        { "yaw", horseData.yaw },
    };
}

void from_json(const json& j, HorseData& horseData) {
    j.at("sceneId").get_to(horseData.sceneId);
    j.at("pos").get_to(horseData.pos);
    j.at("yaw").get_to(horseData.yaw);
}

void to_json(json& j, const SaveInfo& saveInfo) {
    j = json{
        { "playerData", saveInfo.playerData },
        { "equips", saveInfo.equips },
        { "inventory", saveInfo.inventory },
        { "permanentSceneFlags", saveInfo.permanentSceneFlags },
        { "unk_DF4", saveInfo.unk_DF4 },
        { "dekuPlaygroundHighScores", saveInfo.dekuPlaygroundHighScores },
        { "pictoFlags0", saveInfo.pictoFlags0 },
        { "pictoFlags1", saveInfo.pictoFlags1 },
        { "unk_E5C", saveInfo.unk_E5C },
        { "unk_E60", saveInfo.unk_E60 },
        { "unk_E64", saveInfo.unk_E64 },
        { "scenesVisible", saveInfo.scenesVisible },
        { "skullTokenCount", saveInfo.skullTokenCount },
        { "unk_EA0", saveInfo.unk_EA0 },
        { "unk_EA4", saveInfo.unk_EA4 },
        { "unk_EA8", saveInfo.unk_EA8 },
        { "stolenItems", saveInfo.stolenItems },
        { "unk_EB4", saveInfo.unk_EB4 },
        { "highScores", saveInfo.highScores },
        { "weekEventReg", saveInfo.weekEventReg },
        { "regionsVisited", saveInfo.regionsVisited },
        { "worldMapCloudVisibility", saveInfo.worldMapCloudVisibility },
        { "unk_F40", saveInfo.unk_F40 },
        { "scarecrowSpawnSongSet", saveInfo.scarecrowSpawnSongSet },
        { "scarecrowSpawnSong", saveInfo.scarecrowSpawnSong },
        { "bombersCaughtNum", saveInfo.bombersCaughtNum },
        { "bombersCaughtOrder", saveInfo.bombersCaughtOrder },
        { "lotteryCodes", saveInfo.lotteryCodes },
        { "spiderHouseMaskOrder", saveInfo.spiderHouseMaskOrder },
        { "bomberCode", saveInfo.bomberCode },
        { "horseData", saveInfo.horseData },
        { "checksum", saveInfo.checksum },
    };
}

void from_json(const json& j, SaveInfo& saveInfo) {
    j.at("playerData").get_to(saveInfo.playerData);
    j.at("equips").get_to(saveInfo.equips);
    j.at("inventory").get_to(saveInfo.inventory);
    j.at("permanentSceneFlags").get_to(saveInfo.permanentSceneFlags);
    j.at("unk_DF4").get_to(saveInfo.unk_DF4);
    j.at("dekuPlaygroundHighScores").get_to(saveInfo.dekuPlaygroundHighScores);
    j.at("pictoFlags0").get_to(saveInfo.pictoFlags0);
    j.at("pictoFlags1").get_to(saveInfo.pictoFlags1);
    j.at("unk_E5C").get_to(saveInfo.unk_E5C);
    j.at("unk_E60").get_to(saveInfo.unk_E60);
    j.at("unk_E64").get_to(saveInfo.unk_E64);
    j.at("scenesVisible").get_to(saveInfo.scenesVisible);
    j.at("skullTokenCount").get_to(saveInfo.skullTokenCount);
    j.at("unk_EA0").get_to(saveInfo.unk_EA0);
    j.at("unk_EA4").get_to(saveInfo.unk_EA4);
    j.at("unk_EA8").get_to(saveInfo.unk_EA8);
    j.at("stolenItems").get_to(saveInfo.stolenItems);
    j.at("unk_EB4").get_to(saveInfo.unk_EB4);
    j.at("highScores").get_to(saveInfo.highScores);
    j.at("weekEventReg").get_to(saveInfo.weekEventReg);
    j.at("regionsVisited").get_to(saveInfo.regionsVisited);
    j.at("worldMapCloudVisibility").get_to(saveInfo.worldMapCloudVisibility);
    j.at("unk_F40").get_to(saveInfo.unk_F40);
    j.at("scarecrowSpawnSongSet").get_to(saveInfo.scarecrowSpawnSongSet);
    j.at("scarecrowSpawnSong").get_to(saveInfo.scarecrowSpawnSong);
    j.at("bombersCaughtNum").get_to(saveInfo.bombersCaughtNum);
    j.at("bombersCaughtOrder").get_to(saveInfo.bombersCaughtOrder);
    // lotteryCodes is an array of arrays, so we need to manually parse it
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            saveInfo.lotteryCodes[x][y] = j.at("lotteryCodes")[x][y].get<s8>();
        }
    }
    j.at("spiderHouseMaskOrder").get_to(saveInfo.spiderHouseMaskOrder);
    j.at("bomberCode").get_to(saveInfo.bomberCode);
    j.at("horseData").get_to(saveInfo.horseData);
    j.at("checksum").get_to(saveInfo.checksum);
}

void to_json(json& j, const Save& save) {
    j = json{
        { "entrance", save.entrance },
        { "equippedMask", save.equippedMask },
        { "isFirstCycle", save.isFirstCycle },
        { "unk_06", save.unk_06 },
        { "linkAge", save.linkAge },
        { "cutsceneIndex", save.cutsceneIndex },
        { "time", save.time },
        { "owlSaveLocation", save.owlSaveLocation },
        { "isNight", save.isNight },
        { "timeSpeedOffset", save.timeSpeedOffset },
        { "day", save.day },
        { "eventDayCount", save.eventDayCount },
        { "playerForm", save.playerForm },
        { "snowheadCleared", save.snowheadCleared },
        { "hasTatl", save.hasTatl },
        { "isOwlSave", save.isOwlSave },
        { "saveInfo", save.saveInfo },
    };
}

void from_json(const json& j, Save& save) {
    j.at("entrance").get_to(save.entrance);
    j.at("equippedMask").get_to(save.equippedMask);
    j.at("isFirstCycle").get_to(save.isFirstCycle);
    j.at("unk_06").get_to(save.unk_06);
    j.at("linkAge").get_to(save.linkAge);
    j.at("cutsceneIndex").get_to(save.cutsceneIndex);
    j.at("time").get_to(save.time);
    j.at("owlSaveLocation").get_to(save.owlSaveLocation);
    j.at("isNight").get_to(save.isNight);
    j.at("timeSpeedOffset").get_to(save.timeSpeedOffset);
    j.at("day").get_to(save.day);
    j.at("eventDayCount").get_to(save.eventDayCount);
    j.at("playerForm").get_to(save.playerForm);
    j.at("snowheadCleared").get_to(save.snowheadCleared);
    j.at("hasTatl").get_to(save.hasTatl);
    j.at("isOwlSave").get_to(save.isOwlSave);
    j.at("saveInfo").get_to(save.saveInfo);
}

void to_json(json& j, const SaveContext& saveContext) {
    j = json{
        { "save", saveContext.save },
        { "eventInf", saveContext.eventInf },
        { "unk_1014", saveContext.unk_1014 },
        { "bButtonStatus", saveContext.bButtonStatus },
        { "jinxTimer", saveContext.jinxTimer },
        { "rupeeAccumulator", saveContext.rupeeAccumulator },
        { "bottleTimerStates", saveContext.bottleTimerStates },
        { "bottleTimerStartOsTimes", saveContext.bottleTimerStartOsTimes },
        { "bottleTimerTimeLimits", saveContext.bottleTimerTimeLimits },
        { "bottleTimerCurTimes", saveContext.bottleTimerCurTimes },
        { "bottleTimerPausedOsTimes", saveContext.bottleTimerPausedOsTimes },
        { "pictoPhotoI5", saveContext.pictoPhotoI5 },
    };
}

void from_json(const json& j, SaveContext& saveContext) {
    j.at("save").get_to(saveContext.save);
    j.at("eventInf").get_to(saveContext.eventInf);
    j.at("unk_1014").get_to(saveContext.unk_1014);
    j.at("bButtonStatus").get_to(saveContext.bButtonStatus);
    j.at("jinxTimer").get_to(saveContext.jinxTimer);
    j.at("rupeeAccumulator").get_to(saveContext.rupeeAccumulator);
    j.at("bottleTimerStates").get_to(saveContext.bottleTimerStates);
    j.at("bottleTimerStartOsTimes").get_to(saveContext.bottleTimerStartOsTimes);
    j.at("bottleTimerTimeLimits").get_to(saveContext.bottleTimerTimeLimits);
    j.at("bottleTimerCurTimes").get_to(saveContext.bottleTimerCurTimes);
    j.at("bottleTimerPausedOsTimes").get_to(saveContext.bottleTimerPausedOsTimes);
    j.at("pictoPhotoI5").get_to(saveContext.pictoPhotoI5);
}

void to_json(json& j, const SaveOptions& saveOptions) {
    j = json{
        { "optionId", saveOptions.optionId },
        { "language", saveOptions.language },
        { "audioSetting", saveOptions.audioSetting },
        { "languageSetting", saveOptions.languageSetting },
        { "zTargetSetting", saveOptions.zTargetSetting },
    };
}

void from_json(const json& j, SaveOptions& saveOptions) {
    j.at("optionId").get_to(saveOptions.optionId);
    j.at("language").get_to(saveOptions.language);
    j.at("audioSetting").get_to(saveOptions.audioSetting);
    j.at("languageSetting").get_to(saveOptions.languageSetting);
    j.at("zTargetSetting").get_to(saveOptions.zTargetSetting);
}

#endif // BenJsonConversions_hpp
