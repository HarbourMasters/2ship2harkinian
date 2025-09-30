#include "DebugConsole.h"

#include "public/bridge/consolevariablebridge.h"
#include "Window.h"
#include "ConsoleWindow.h"
#include "2s2h/BenPort.h"
#include "2s2h/resource/type/Scene.h"
#include "2s2h/resource/type/scenecommand/SceneCommand.h"
#include "2s2h/resource/type/scenecommand/SetActorList.h"
#include "2s2h/resource/type/scenecommand/SetRoomList.h"
#include <vector>
#include <string>
#include <utils/StringHelper.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <spdlog/spdlog.h>

extern "C" {
#include <z64.h>
#include "variables.h"
#include "functions.h"
#include "macros.h"

#include "overlays/gamestates/ovl_file_choose/z_file_select.h"
#include "overlays/gamestates/ovl_title/z_title.h"
}

extern std::unordered_map<s16, const char*> actorNames;
extern std::unordered_map<s16, const char*> sceneNames;

extern Ship::IResource* OTRPlay_LoadFile(PlayState* play, const char* fileName);

#define CMD_REGISTER Ship::Context::GetInstance()->GetConsole()->AddCommand
// TODO: Commands should be using the output passed in.
#define ERROR_MESSAGE                                                                 \
    std::reinterpret_pointer_cast<Ship::ConsoleWindow>(                               \
        Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Console")) \
        ->SendErrorMessage
#define INFO_MESSAGE                                                                  \
    std::reinterpret_pointer_cast<Ship::ConsoleWindow>(                               \
        Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Console")) \
        ->SendInfoMessage

static bool ActorSpawnHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                              std::string* output) {
    if ((args.size() != 9) && (args.size() != 3) && (args.size() != 6)) {
        ERROR_MESSAGE("Not enough arguments passed to actorspawn");
        return 1;
    }

    if (gPlayState == nullptr) {
        ERROR_MESSAGE("PlayState == nullptr");
        return 1;
    }

    Player* player = GET_PLAYER(gPlayState);
    PosRot spawnPoint;
    s16 actorId = 0;
    try {
        actorId = std::stoi(args[1]);
    } catch (std::invalid_argument const& ex) {
        ERROR_MESSAGE("Invalid actor ID");
        return 1;
    }

    const s16 params = std::stoi(args[2]);

    spawnPoint = player->actor.world;

    switch (args.size()) {
        case 9:
            if (args[6][0] != ',') {
                spawnPoint.rot.x = std::stoi(args[6]);
            }
            if (args[7][0] != ',') {
                spawnPoint.rot.y = std::stoi(args[7]);
            }
            if (args[8][0] != ',') {
                spawnPoint.rot.z = std::stoi(args[8]);
            }
        case 6:
            if (args[3][0] != ',') {
                spawnPoint.pos.x = std::stoi(args[3]);
            }
            if (args[4][0] != ',') {
                spawnPoint.pos.y = std::stoi(args[4]);
            }
            if (args[5][0] != ',') {
                spawnPoint.pos.z = std::stoi(args[5]);
            }
    }

    if (Actor_Spawn(&gPlayState->actorCtx, gPlayState, actorId, spawnPoint.pos.x, spawnPoint.pos.y, spawnPoint.pos.z,
                    spawnPoint.rot.x, spawnPoint.rot.y, spawnPoint.rot.z, params) == NULL) {
        ERROR_MESSAGE("Failed to spawn actor. Actor_Spawn returned NULL");
        return 1;
    }
    return 0;
}

static bool LoadSceneHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>&,
                             std::string* output) {
    gSaveContext.respawnFlag = 0;
    gSaveContext.seqId = NA_BGM_DISABLED;
    gSaveContext.gameMode = 0;

    return 0;
}

static bool SetPosHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string> args,
                          std::string* output) {
    if (gPlayState == nullptr) {
        ERROR_MESSAGE("PlayState == nullptr");
        return 1;
    }

    Player* player = GET_PLAYER(gPlayState);

    if (args.size() == 1) {
        INFO_MESSAGE("Player position is [ %.2f, %.2f, %.2f ]", player->actor.world.pos.x, player->actor.world.pos.y,
                     player->actor.world.pos.z);
        return 0;
    }
    if (args.size() < 4)
        return 1;

    player->actor.world.pos.x = std::stof(args[1]);
    player->actor.world.pos.y = std::stof(args[2]);
    player->actor.world.pos.z = std::stof(args[3]);

    INFO_MESSAGE("Set player position to [ %.2f, %.2f, %.2f ]", player->actor.world.pos.x, player->actor.world.pos.y,
                 player->actor.world.pos.z);
    return 0;
}

static bool ResetHandler(std::shared_ptr<Ship::Console> Console, std::vector<std::string> args, std::string* output) {
    if (gGameState == nullptr) {
        ERROR_MESSAGE("gGameState == nullptr");
        return 1;
    }

    STOP_GAMESTATE(gGameState);
    SET_NEXT_GAMESTATE(gGameState, ConsoleLogo_Init, sizeof(ConsoleLogoState));
    // GI-TODO
    // GameInteractor::Instance->ExecuteHooks<GameInteractor::OnExitGame>(gSaveContext.fileNum);
    return 0;
}

static bool BHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                     std::string* output) {
    if (args.size() < 2) {
        ERROR_MESSAGE("[2S2H] Unexpected arguments passed");
        return 1;
    }

    u8 itemId;
    try {
        itemId = std::stoi(args[1]);
    } catch (std::invalid_argument const& ex) {
        ERROR_MESSAGE("[2S2H] Item ID must be an integer.");
        return 1;
    }

    if (gPlayState == nullptr) {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }

    BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = itemId;
    Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
    return 0;
}

static bool GiveItemHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string> args,
                            std::string* output) {
    if (args.size() < 2) {
        ERROR_MESSAGE("[2S2H] Unexpected arguments passed");
        return 1;
    }

    GetItemId getItemId;
    try {
        getItemId = (GetItemId)std::stoi(args[1]);
    } catch (std::invalid_argument const& ex) {
        ERROR_MESSAGE("[2S2H] Item ID must be an integer.");
        return 1;
    }

    if (gPlayState == nullptr) {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }

    Player* player = GET_PLAYER(gPlayState);

    // Cheat by using Tatl to give the item
    if (player == nullptr || player->tatlActor == nullptr) {
        ERROR_MESSAGE("Unable to receive item");
        return 1;
    }

    if (!Actor_OfferGetItemFar(player->tatlActor, gPlayState, getItemId)) {
        ERROR_MESSAGE("Unable to receive item");
        return 1;
    }
    return 0;
}

static bool EntranceHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                            std::string* output) {
    if (args.size() < 2) {
        ERROR_MESSAGE("[2S2H] Unexpected arguments passed");
        return 1;
    }

    unsigned int entrance;

    try {
        entrance = std::stoi(args[1], nullptr, 16);
    } catch (std::invalid_argument const& ex) {
        ERROR_MESSAGE("[2S2H] Entrance value must be a Hex number.");
        return 1;
    }

    gPlayState->nextEntrance = entrance;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_INSTANT;
    gSaveContext.nextTransitionType = TRANS_TYPE_INSTANT;
    return 0;
}

static bool VoidHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                        std::string* output) {
    if (gPlayState != nullptr) {
        func_80169EFC(&gPlayState->state);
    } else {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }
    return 0;
}

static bool ReloadHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                          std::string* output) {
    if (gPlayState != nullptr) {
        gPlayState->nextEntrance = gSaveContext.save.entrance;
        gPlayState->transitionTrigger = TRANS_TRIGGER_START;
        gPlayState->transitionType = TRANS_TYPE_INSTANT;
        gSaveContext.nextTransitionType = TRANS_TYPE_INSTANT;
    } else {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }
    return 0;
}

static bool FileSelectHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                              std::string* output) {
    if (gPlayState != nullptr) {
        STOP_GAMESTATE(&gPlayState->state);
        SET_NEXT_GAMESTATE(&gPlayState->state, FileSelect_Init, sizeof(FileSelectState));
    } else {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }
    return 0;
}

static bool QuitHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                        std::string* output) {
    Ship::Context::GetInstance()->GetWindow()->Close();
    return 0;
}

void traverseScene(std::string sceneName, std::string roomName, SOH::Scene* scene, std::set<std::string>* sceneSet,
                   nlohmann::json* actorsJson, std::string* output) {
    if (scene == nullptr) {
        return;
    }
    for (auto& sceneCmd : scene->commands) {
        switch ((SceneCommandTypeId)sceneCmd->cmdId) {
            case SCENE_CMD_ID_ACTOR_LIST: {
                SOH::SetActorList* actorList = (SOH::SetActorList*)sceneCmd.get();
                std::string locationName = sceneName;
                if (roomName != "") {
                    int roomNum = roomName.find("_room");
                    locationName += ", " + roomName.substr(roomNum + 1, 7); // roomName;
                }
                for (auto& actorSpawn : actorList->actorList) {
                    actorSpawn.id &= 0x1FFF; // Mask out rotation flags
                    // SPDLOG_INFO("Actor spawn: {}", actorSpawn.id);
                    if (actorSpawn.id >= ACTOR_ID_MAX || actorSpawn.id < ACTOR_PLAYER)
                        continue;
                    // SPDLOG_INFO("Actor spawn: {}, params: {}", actorNames[actorSpawn.id], actorSpawn.params);
                    // *output += fmt::format("\t\t{} {}\n", actorNames[actorSpawn.id], actorSpawn.params);
                    // TODO: Take half day bit into account
                    sceneSet->emplace(actorNames[actorSpawn.id]); // TODO: No, put rooms. maybe
                    std::set<std::string> actorSceneSet;
                    if (!actorsJson->contains(actorNames[actorSpawn.id])) {
                        (*actorsJson)[actorNames[actorSpawn.id]] = actorSceneSet = {};
                    }
                    actorSceneSet = (*actorsJson)[actorNames[actorSpawn.id]];
                    actorSceneSet.emplace(locationName);
                    (*actorsJson)[actorNames[actorSpawn.id]] = actorSceneSet;
                }
            } break;
            case SCENE_CMD_ID_ROOM_LIST: {
                SOH::SetRoomList* roomList = (SOH::SetRoomList*)sceneCmd.get();
                for (auto& room : roomList->rooms) {
                    // SPDLOG_INFO("Room: {}", room.fileName);
                    // *output += fmt::format("\tRoom: {} \n", room.fileName);
                    SOH::Scene* sceneRoom = (SOH::Scene*)OTRPlay_LoadFile(gPlayState, room.fileName);
                    traverseScene(sceneName, room.fileName, sceneRoom, sceneSet, actorsJson, output);
                }
            } break;
            default:
                break;
        }
    }
}

static bool SceneDumpHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                             std::string* output) {
    nlohmann::json scenesJson = {};
    nlohmann::json actorsJson = {};
    for (int sceneId = SCENE_20SICHITAI2; sceneId < SCENE_MAX; sceneId++) {
        SceneTableEntry* sceneTableEntry = &gSceneTable[sceneId];
        if (sceneTableEntry->segment.fileName == nullptr) {
            continue;
        }
        std::string scenePath = StringHelper::Sprintf("scenes/nonmq/%s/%s", sceneTableEntry->segment.fileName,
                                                      sceneTableEntry->segment.fileName);
        SOH::Scene* scene =
            (SOH::Scene*)OTRPlay_LoadFile(gPlayState, scenePath.c_str()); // Takes PlayState arg, but does not use it
        // SPDLOG_INFO("Scene: {}, titleTextId: {}, commands: {}", scenePath, sceneTableEntry->titleTextId,
        // scene->commands.size()); *output += fmt::format("Scene: {}, titleTextId: {}, commands: {}", scenePath,
        // sceneTableEntry->titleTextId, scene->commands.size());
        std::set<std::string> sceneSet = {};
        traverseScene(sceneNames[sceneId], "", scene, &sceneSet, &actorsJson, output);
        scenesJson[sceneNames[sceneId]] = sceneSet;
    }
    /*
     * Structure:
     * {
     *  "scenes": {
     *              "sceneId": [setOfActors]
     *            },
     *  "actors": {
     *             "actorId": [setOfScenes]
     *            }
     * }
     */
    nlohmann::json result = { { "scenes", scenesJson }, { "actors", actorsJson } };

    try {
        std::ofstream o("sceneDump.json");
        o << std::setw(4) << result << std::endl;
        o.close();
    } catch (...) {
        *output = "Failed to write sceneDump file";
        SPDLOG_ERROR("Failed to write sceneDump file");
        return 1;
    }
    *output = "Dumped scene data to sceneDump.json";
    return 0;
}

void DebugConsole_Init(void) {
    // Console
    CMD_REGISTER("file_select", { FileSelectHandler, "Returns to the file select." });
    CMD_REGISTER("reset", { ResetHandler, "Resets the game." });
    CMD_REGISTER("quit", { QuitHandler, "Quits the game." });

    // Map & Location
    CMD_REGISTER("void", { VoidHandler, "Voids out of the current map." });
    CMD_REGISTER("reload", { ReloadHandler, "Reloads the current map." });
    CMD_REGISTER("entrance", { EntranceHandler,
                               "Sends player to the entered entrance (hex)",
                               { { "entrance", Ship::ArgumentType::NUMBER } } });

    // Gameplay
    CMD_REGISTER("give_item", { GiveItemHandler,
                                "Gives an item to the player as if it was given from an actor",
                                { { "giveItemID", Ship::ArgumentType::NUMBER } } });

    CMD_REGISTER("bItem", { BHandler, "Set an item to the B button.", { { "Item ID", Ship::ArgumentType::NUMBER } } });

    CMD_REGISTER("spawn", { ActorSpawnHandler,
                            "Spawn an actor.",
                            { { "actor id", Ship::ArgumentType::NUMBER },
                              { "data", Ship::ArgumentType::NUMBER },
                              { "x", Ship::ArgumentType::NUMBER, true },
                              { "y", Ship::ArgumentType::NUMBER, true },
                              { "z", Ship::ArgumentType::NUMBER, true },
                              { "rx", Ship::ArgumentType::NUMBER, true },
                              { "ry", Ship::ArgumentType::NUMBER, true },
                              { "rz", Ship::ArgumentType::NUMBER, true } } });

    CMD_REGISTER("pos", { SetPosHandler,
                          "Sets the position of the player.",
                          { { "x", Ship::ArgumentType::NUMBER, true },
                            { "y", Ship::ArgumentType::NUMBER, true },
                            { "z", Ship::ArgumentType::NUMBER, true } } });

    // Data
    CMD_REGISTER("dump_scene", { SceneDumpHandler, "Dumps scene data, TBD" });
}
