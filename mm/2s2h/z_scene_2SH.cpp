﻿#include "BenPort.h"
extern "C" {
#include "z64.h"
#include "vt.h"
#include "global.h"
}
#include <libultraship/libultraship.h>
#include <Blob.h>
#include <memory>
#include <cassert>
#include <Utils/StringHelper.h>
#include <DisplayList.h>
#include "2s2h/resource/type/Scene.h"
#include "2s2h/resource/type/CollisionHeader.h"
#include "2s2h/resource/type/Cutscene.h"
#include "2s2h/resource/type/Path.h"
#include "2s2h/resource/type/Text.h"
#include "2s2h/resource/type/scenecommand/SetCameraSettings.h"
#include "2s2h/resource/type/scenecommand/SetCutscenes.h"
#include "2s2h/resource/type/scenecommand/SetStartPositionList.h"
#include "2s2h/resource/type/scenecommand/SetActorList.h"
#include "2s2h/resource/type/scenecommand/SetCollisionHeader.h"
#include "2s2h/resource/type/scenecommand/SetRoomList.h"
#include "2s2h/resource/type/scenecommand/SetEntranceList.h"
#include "2s2h/resource/type/scenecommand/SetSpecialObjects.h"
#include "2s2h/resource/type/scenecommand/SetRoomBehavior.h"
#include "2s2h/resource/type/scenecommand/SetMesh.h"
#include "2s2h/resource/type/scenecommand/SetObjectList.h"
#include "2s2h/resource/type/scenecommand/SetLightList.h"
#include "2s2h/resource/type/scenecommand/SetPathways.h"
#include "2s2h/resource/type/scenecommand/SetTransitionActorList.h"
#include "2s2h/resource/type/scenecommand/SetSkyboxSettings.h"
#include "2s2h/resource/type/scenecommand/SetSkyboxModifier.h"
#include "2s2h/resource/type/scenecommand/SetTimeSettings.h"
#include "2s2h/resource/type/scenecommand/SetWindSettings.h"
#include "2s2h/resource/type/scenecommand/SetSoundSettings.h"
#include "2s2h/resource/type/scenecommand/SetEchoSettings.h"
#include "2s2h/resource/type/scenecommand/SetAlternateHeaders.h"
#include "2s2h/resource/type/scenecommand/SetActorCutsceneList.h"
#include "2s2h/resource/type/scenecommand/SetAnimatedMaterialList.h"
#include "2s2h/resource/type/scenecommand/SetMinimapList.h"
#include "2s2h/resource/type/scenecommand/SetMinimapChests.h"
#include "2s2h/resource/type/scenecommand/SetCsCamera.h"

s32 OTRScene_ExecuteCommands(PlayState* play, LUS::Scene* scene);


void Scene_CommandSpawnList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetStartPositionList* list = (LUS::SetStartPositionList*)cmd;
    ActorEntry* entries = (ActorEntry*)(list->GetRawPointer());
    s32 loadedCount;
    s16 playerObjectId;
    void* objectPtr;

    play->linkActorEntry = &entries[play->setupEntranceList[play->curSpawn].spawn];

    if ((PLAYER_GET_INITMODE(play->linkActorEntry) == PLAYER_INITMODE_TELESCOPE) ||
        ((gSaveContext.respawnFlag == 2) &&
         (gSaveContext.respawn[RESPAWN_MODE_RETURN].playerParams == PLAYER_PARAMS(0xFF, PLAYER_INITMODE_TELESCOPE)))) {
        // Skull Kid Object
        Object_SpawnPersistent(&play->objectCtx, OBJECT_STK);
        return;
    }

    loadedCount = Object_SpawnPersistent(&play->objectCtx, OBJECT_LINK_CHILD);
    objectPtr = play->objectCtx.slots[play->objectCtx.numEntries].segment;
    play->objectCtx.numEntries = loadedCount;
    play->objectCtx.numPersistentEntries = loadedCount;
    playerObjectId = gPlayerFormObjectIds[GET_PLAYER_FORM];
    gActorOverlayTable[0].initInfo->objectId = playerObjectId;
    Object_SpawnPersistent(&play->objectCtx, playerObjectId);

    play->objectCtx.slots[play->objectCtx.numEntries].segment = objectPtr;
}

void Scene_CommandActorList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetActorList* list = (LUS::SetActorList*)cmd;

    play->numSetupActors = list->numActors;
    play->actorCtx.halfDaysBit = 0;
    play->setupActorList = (ActorEntry*)list->GetRawPointer();
}

void Scene_CommandActorCutsceneCamList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetCsCamera* cams = (LUS::SetCsCamera*)cmd;

    play->actorCsCamList = (ActorCsCamInfo*)cams->GetPointer();
}

void Scene_CommandCollisionHeader(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetCollisionHeader* colHeader = (LUS::SetCollisionHeader*)cmd;
    BgCheck_Allocate(&play->colCtx, play, (CollisionHeader*)colHeader->GetRawPointer());
}

void Scene_CommandRoomList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetRoomList* list = (LUS::SetRoomList*)cmd;
    play->numRooms = list->numRooms;
    play->roomList = (RomFile*)list->GetPointer();
}

void Scene_CommandWindSettings(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetWindSettings* settings = (LUS::SetWindSettings*)cmd;

    play->envCtx.windDirection.x = settings->settings.windWest;
    play->envCtx.windDirection.y = settings->settings.windVertical;
    play->envCtx.windDirection.z = settings->settings.windSouth;
    play->envCtx.windSpeed = settings->settings.windSpeed;
}

void Scene_CommandEntranceList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetEntranceList* list = (LUS::SetEntranceList*)cmd;

    play->setupEntranceList = (EntranceEntry*)list->GetRawPointer();
}

void Scene_CommandSpecialFiles(PlayState* play, LUS::ISceneCommand* cmd) {
    printf("Un-implemented command %02X\n", cmd->cmdId);
    // Unused according to z_scene.c
}

void Scene_CommandRoomBehavior(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetRoomBehaviorMM* behavior = (LUS::SetRoomBehaviorMM*)cmd;

    play->roomCtx.curRoom.behaviorType1 = behavior->roomBehavior.gameplayFlags;
    play->roomCtx.curRoom.behaviorType2 = behavior->roomBehavior.currRoomUnk2;
    play->roomCtx.curRoom.lensMode = behavior->roomBehavior.currRoomUnk5;
    play->msgCtx.unk12044 = behavior->roomBehavior.msgCtxUnk;
    play->roomCtx.curRoom.enablePosLights = behavior->roomBehavior.enablePointLights;
    play->envCtx.stormState = behavior->roomBehavior.kankyoContextUnkE2;
    //play->roomCtx.curRoom.behaviorType1 = behavior->roomBehavior.gameplayFlags;
    //play->roomCtx.curRoom.behaviorType2 = behavior->roomBehavior.gameplayFlags2 & 0xFF;
    //play->roomCtx.curRoom.lensMode = (behavior->roomBehavior.gameplayFlags2 >> 8) & 1;
    //play->msgCtx.unk12044 = (behavior->roomBehavior.gameplayFlags2 >> 0xA) & 1;
    //play->roomCtx.curRoom.enablePosLights = (behavior->roomBehavior.gameplayFlags2 >> 0xB) & 1;
    //play->envCtx.stormState = (behavior->roomBehavior.gameplayFlags2 >> 0xC) & 1;
}

void Scene_Command09(PlayState* play, LUS::ISceneCommand* cmd) {
    // Empty in z_scene.c
}

void Scene_CommandMesh(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetMesh* mesh = (LUS::SetMesh*)cmd;

    play->roomCtx.curRoom.roomShape = (RoomShape*)mesh->GetRawPointer();
}

void Scene_CommandObjectList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetObjectList* objList = (LUS::SetObjectList*)cmd;

    s16* entry = (s16*)objList->GetRawPointer();

    for (unsigned int i = 0; i < objList->objects.size(); i++) {
        bool alreadyIncluded = true;

        for (unsigned int j = 0; j < play->objectCtx.numEntries; j++) {
            if (play->objectCtx.slots[j].id == objList->objects[i]) {
                alreadyIncluded = true;
                break;
            }
        }

        if (!alreadyIncluded) {
            play->objectCtx.slots[play->objectCtx.numEntries++].id = objList->objects[i];
            Actor_KillAllWithMissingObject(play, &play->actorCtx);
        }
    }
}

void Scene_CommandLightList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetLightList* lightList = (LUS::SetLightList*)cmd;
    
    for (unsigned int i = 0; i < lightList->numLights; i++) {
        LightContext_InsertLight(play, &play->lightCtx, (LightInfo*)&lightList->lightList[i]);
    }
}

void Scene_CommandPathList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetPathwaysMM* paths = (LUS::SetPathwaysMM*)cmd;

    play->setupPathList = (Path*)paths->GetPointer()[0];
}

void Scene_CommandTransiActorList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetTransitionActorList* list = (LUS::SetTransitionActorList*)cmd;

    play->transitionActors.count = list->numTransitionActors;
    play->transitionActors.list = (TransitionActorEntry*)list->GetRawPointer();
}

void Scene_CommandEnvLightSettings(PlayState* play, LUS::ISceneCommand* cmd) {
    play->envCtx.lightSettingsList = (EnvLightSettings*)cmd->GetRawPointer();
}

void Scene_CommandTimeSettings(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetTimeSettings* settings = (LUS::SetTimeSettings*)cmd;
    
    if ((settings->settings.hour != 0xFF) && (settings->settings.minute != 0xFF)) {
        gSaveContext.skyboxTime = gSaveContext.save.time = CLOCK_TIME_ALT2_F(settings->settings.hour, settings->settings.minute);
    }

    if (settings->settings.timeIncrement != 0xFF) {
        play->envCtx.sceneTimeSpeed = settings->settings.timeIncrement;
    } else {
        play->envCtx.sceneTimeSpeed = 0;
    }

    // Increase time speed during first cycle
    if ((gSaveContext.save.saveInfo.inventory.items[SLOT_OCARINA] == ITEM_NONE) && (play->envCtx.sceneTimeSpeed != 0)) {
        play->envCtx.sceneTimeSpeed = 5;
    }

    if (gSaveContext.sunsSongState == SUNSSONG_INACTIVE) {
        R_TIME_SPEED = play->envCtx.sceneTimeSpeed;
    }

    play->envCtx.sunPos.x = -(Math_SinS(((void)0, gSaveContext.save.time) - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f;
    play->envCtx.sunPos.y = (Math_CosS(((void)0, gSaveContext.save.time) - CLOCK_TIME(12, 0)) * 120.0f) * 25.0f;
    play->envCtx.sunPos.z = (Math_CosS(((void)0, gSaveContext.save.time) - CLOCK_TIME(12, 0)) * 20.0f) * 25.0f;

    if ((play->envCtx.sceneTimeSpeed == 0) && (gSaveContext.save.cutsceneIndex < 0xFFF0)) {
        gSaveContext.skyboxTime = gSaveContext.save.time;

        if ((gSaveContext.skyboxTime >= CLOCK_TIME(4, 0)) && (gSaveContext.skyboxTime < CLOCK_TIME(6, 30))) {
            gSaveContext.skyboxTime = CLOCK_TIME(5, 0);
        } else if ((gSaveContext.skyboxTime >= CLOCK_TIME(6, 30)) && (gSaveContext.skyboxTime < CLOCK_TIME(8, 0))) {
            gSaveContext.skyboxTime = CLOCK_TIME(8, 0);
        } else if ((gSaveContext.skyboxTime >= CLOCK_TIME(16, 0)) && (gSaveContext.skyboxTime < CLOCK_TIME(17, 0))) {
            gSaveContext.skyboxTime = CLOCK_TIME(17, 0);
        } else if ((gSaveContext.skyboxTime >= CLOCK_TIME(18, 0)) && (gSaveContext.skyboxTime < CLOCK_TIME(19, 0))) {
            gSaveContext.skyboxTime = CLOCK_TIME(19, 0);
        }
    }
}

void Scene_CommandSkyboxSettings(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetSkyboxSettings* settings = (LUS::SetSkyboxSettings*)cmd;
    
    play->skyboxId = settings->settings.skyboxId & 3;
    // BENTODO z_scene.c reads from skyboxSettings.skyboxConfig not weather
    // Settings uses names from OOT
    play->envCtx.skyboxConfig = play->envCtx.changeSkyboxNextConfig = settings->settings.weather;
    play->envCtx.lightMode = settings->settings.indoors;
    //Scene_LoadAreaTextures(play, settings->settings.)
}

void Scene_CommandSkyboxDisables(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetSkyboxModifier* mod = (LUS::SetSkyboxModifier*)cmd;

    play->envCtx.skyboxDisabled = mod->modifier.skyboxDisabled;
    play->envCtx.sunDisabled = mod->modifier.sunMoonDisabled;
}

void Scene_CommandExitList(PlayState* play, LUS::ISceneCommand* cmd) {
    play->setupExitList = (u16*)cmd->GetRawPointer();
}

void Scene_CommandSoundSettings(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetSoundSettings* settings = (LUS::SetSoundSettings*)cmd;

    play->sequenceCtx.seqId = settings->settings.seqId;
    play->sequenceCtx.ambienceId = settings->settings.natureAmbienceId;

    if(gSaveContext.seqId == (u8)NA_BGM_DISABLED || AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN) == NA_BGM_FINAL_HOURS) {
        Audio_SetSpec(settings->settings.reverb); // BENTODO Verify if this should be reverb
    }
 }

void Scene_CommandEchoSetting(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetEchoSettings* echo = (LUS::SetEchoSettings*)cmd;
    play->roomCtx.curRoom.echo = echo->settings.echo;
}

void Scene_CommandCutsceneScriptList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetCutscenesMM* cs = (LUS::SetCutscenesMM*)cmd;
    play->csCtx.scriptListCount = cs->entries.size();
    // BENTODO do this the right way with get pointer
    play->csCtx.scriptList = (CutsceneScriptEntry*)cs->entries.data();

}

void Scene_CommandAltHeaderList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetAlternateHeaders* headers = (LUS::SetAlternateHeaders*)cmd;

    if (gSaveContext.sceneLayer != 0) {
        LUS::Scene* desiredHeader =
            std::static_pointer_cast<LUS::Scene>(headers->headers[gSaveContext.sceneLayer - 1]).get();

        if (desiredHeader != nullptr) {
            OTRScene_ExecuteCommands(play, desiredHeader);
            // z_scene does (cmd + 1)->base.code = 0x14;
        }
    }
}

void Scene_CommandSetRegionVisitedFlag(PlayState* play, LUS::ISceneCommand* cmd) {
        s16 j = 0;
    s16 i = 0;

    while (true) {
        if (gSceneIdsPerRegion[i][j] == 0xFFFF) {
            i++;
            j = 0;

            if (i == REGION_MAX) {
                break;
            }
        }

        if (play->sceneId == gSceneIdsPerRegion[i][j]) {
            break;
        }

        j++;
    }

    if (i < REGION_MAX) {
        gSaveContext.save.saveInfo.regionsVisited =
            (gBitFlags[i] | gSaveContext.save.saveInfo.regionsVisited) | gSaveContext.save.saveInfo.regionsVisited;
    }
}

void Scene_CommandAnimatedMaterials(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetAnimatedMaterialList* list = (LUS::SetAnimatedMaterialList*)cmd;
    play->sceneMaterialAnims = (AnimatedMaterial*)list->mat;
}

void Scene_CommandCutsceneList(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetActorCutsceneList* list = (LUS::SetActorCutsceneList*)cmd;

    CutsceneManager_Init(play, (ActorCutscene*)list->GetPointer(), list->numEntries);
}

void Scene_CommandMiniMap(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetMinimapList* list = (LUS::SetMinimapList*)cmd;

    MapDisp_Init(play);

    MapDisp_InitMapData(play, list->GetPointer());
}

void Scene_Command1D(PlayState* play, LUS::ISceneCommand* cmd) {

}

void Scene_CommandMiniMapCompassInfo(PlayState* play, LUS::ISceneCommand* cmd) {
    LUS::SetMinimapChests* chests = (LUS::SetMinimapChests*)cmd;

    MapDisp_InitChestData(play, chests->chests.size(), chests->GetPointer());
}

void (*sSceneCmdHandlersOTR[SCENE_CMD_MAX])(PlayState*, LUS::ISceneCommand*) = {
    Scene_CommandSpawnList,            // SCENE_CMD_ID_SPAWN_LIST
    Scene_CommandActorList,            // SCENE_CMD_ID_ACTOR_LIST
    Scene_CommandActorCutsceneCamList, // SCENE_CMD_ID_ACTOR_CUTSCENE_CAM_LIST
    Scene_CommandCollisionHeader,      // SCENE_CMD_ID_COL_HEADER
    Scene_CommandRoomList,             // SCENE_CMD_ID_ROOM_LIST
    Scene_CommandWindSettings,         // SCENE_CMD_ID_WIND_SETTINGS
    Scene_CommandEntranceList,         // SCENE_CMD_ID_ENTRANCE_LIST
    Scene_CommandSpecialFiles,         // SCENE_CMD_ID_SPECIAL_FILES
    Scene_CommandRoomBehavior,         // SCENE_CMD_ID_ROOM_BEHAVIOR
    Scene_Command09,                   // SCENE_CMD_ID_UNK_09
    Scene_CommandMesh,                 // SCENE_CMD_ID_ROOM_SHAPE
    Scene_CommandObjectList,           // SCENE_CMD_ID_OBJECT_LIST
    Scene_CommandLightList,            // SCENE_CMD_ID_LIGHT_LIST
    Scene_CommandPathList,             // SCENE_CMD_ID_PATH_LIST
    Scene_CommandTransiActorList,      // SCENE_CMD_ID_TRANSI_ACTOR_LIST
    Scene_CommandEnvLightSettings,     // SCENE_CMD_ID_ENV_LIGHT_SETTINGS
    Scene_CommandTimeSettings,         // SCENE_CMD_ID_TIME_SETTINGS
    Scene_CommandSkyboxSettings,       // SCENE_CMD_ID_SKYBOX_SETTINGS
    Scene_CommandSkyboxDisables,       // SCENE_CMD_ID_SKYBOX_DISABLES
    Scene_CommandExitList,             // SCENE_CMD_ID_EXIT_LIST
    NULL,                              // SCENE_CMD_ID_END
    Scene_CommandSoundSettings,        // SCENE_CMD_ID_SOUND_SETTINGS
    Scene_CommandEchoSetting,          // SCENE_CMD_ID_ECHO_SETTINGS
    Scene_CommandCutsceneScriptList,   // SCENE_CMD_ID_CUTSCENE_SCRIPT_LIST
    Scene_CommandAltHeaderList,        // SCENE_CMD_ID_ALTERNATE_HEADER_LIST
    Scene_CommandSetRegionVisitedFlag, // SCENE_CMD_ID_SET_REGION_VISITED
    Scene_CommandAnimatedMaterials,    // SCENE_CMD_ID_ANIMATED_MATERIAL_LIST
    Scene_CommandCutsceneList,         // SCENE_CMD_ID_ACTOR_CUTSCENE_LIST
    Scene_CommandMiniMap,              // SCENE_CMD_ID_MINIMAP_INFO
    Scene_Command1D,                   // SCENE_CMD_ID_UNUSED_1D
    Scene_CommandMiniMapCompassInfo,   // SCENE_CMD_ID_MINIMAP_COMPASS_ICON_INFO
};

s32 OTRScene_ExecuteCommands(PlayState* play, LUS::Scene* scene) {
    LUS::SceneCommandID cmdCode;

    for (int i = 0; i < scene->commands.size(); i++) {
        auto sceneCmd = scene->commands[i];

        if (sceneCmd == nullptr) // UH OH
            continue;

        cmdCode = sceneCmd->cmdId;
        // osSyncPrintf("*** Scene_Word = { code=%d, data1=%02x, data2=%04x } ***\n", cmdCode, sceneCmd->base.data1, sceneCmd->base.data2);
        //SPDLOG_TRACE("CMD {:X}", cmdCode);
        if ((int)cmdCode == SCENE_CMD_ID_END) {
            break;
        } else if (cmdCode == LUS::SceneCommandID::SetCutscenesMM)
            cmdCode = LUS::SceneCommandID::SetCutscenes;

        //if ((int)cmdCode <= 0x19) 
            sSceneCmdHandlersOTR[(int)cmdCode](play, sceneCmd.get());


        // sceneCmd++;
    }
    return 0;
}


std::shared_ptr<LUS::IResource> GetResourceByNameHandlingMQ(const char* path);

extern "C" s32 OTRfunc_8009728C(PlayState* play, RoomContext* roomCtx, s32 roomNum) {
    
    u32 size;

    if (roomCtx->status == 0) {
        roomCtx->prevRoom = roomCtx->curRoom;
        roomCtx->curRoom.num = roomNum;
        roomCtx->curRoom.segment = NULL;
        roomCtx->status = 1;

        //assert(roomNum < play->numRooms);

        if (roomNum >= play->numRooms)
            return 0; // UH OH

        size = play->roomList[roomNum].vromEnd - play->roomList[roomNum].vromStart;
        //roomCtx->activeRoomVram =
        //    (void*)((uintptr_t)roomCtx->roomMemPages[roomCtx->activeMemPage] - ((size + 8) * roomCtx->activeMemPage + 7));

        // DmaMgr_SendRequest2(&roomCtx->dmaRequest, roomCtx->unk_34, play->roomList[roomNum].vromStart, size, 0,
        //&roomCtx->loadQueue, NULL, __FILE__, __LINE__);
        printf("File Name %s\n", play->roomList[roomNum].fileName);
        auto roomData =
            std::static_pointer_cast<LUS::Scene>(GetResourceByNameHandlingMQ(play->roomList[roomNum].fileName));
        roomCtx->status = 1;
        roomCtx->activeRoomVram = roomData.get();

        roomCtx->activeMemPage ^= 1;

        //SPDLOG_INFO("Room Init - curRoom.num: {0:#x}", roomCtx->curRoom.num);

        return 1;
    }

    return 0;
}