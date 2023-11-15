#include "BenPort.h"
#include <libultraship/libultraship.h>
#include "2s2h/resource/type/Scene.h"
#include <Utils/StringHelper.h>
#include <Vertex.h>
extern "C" {
#include "global.h"
#include "vt.h"
#include <z64scene.h>
}
LUS::IResource* OTRPlay_LoadFile(PlayState* play, const char* fileName) {
    auto res = LUS::Context::GetInstance()->GetResourceManager()->LoadResource(fileName);
    return res.get();
}

s32 OTRScene_ExecuteCommands(PlayState* play, LUS::Scene* scene);

extern "C" void OTRPlay_InitScene(PlayState* play, s32 spawn) {
    play->curSpawn = spawn;
    play->linkActorEntry = NULL;
    play->actorCsCamList = NULL;
    play->setupEntranceList = NULL;
    play->setupExitList = NULL;
    play->naviQuestHints = NULL;
    play->setupPathList = NULL;
    play->sceneMaterialAnims = NULL;
    play->roomCtx.unk74 = NULL;
    play->numSetupActors = 0;
    Object_InitContext(&play->state, &play->objectCtx);
    LightContext_Init(play, &play->lightCtx);
    Scene_ResetTransiActorList(&play->state, &play->transitionActors);
    Room_Init(play, &play->roomCtx);
    gSaveContext.worldMapArea = 0;
    OTRScene_ExecuteCommands(play, (LUS::Scene*)play->sceneSegment);
    Play_InitEnvironment(play, play->skyboxId);
}

extern "C" void OTRPlay_SpawnScene(PlayState* play, s32 sceneId, s32 spawn) {
    s32 pad;
    SceneTableEntry* scene = &gSceneTable[sceneId];

    scene->unk_D = 0;
    play->loadedScene = scene;
    play->sceneId = sceneId;
    play->sceneConfig = scene->drawConfig;
    std::string scenePath = StringHelper::Sprintf("scenes/nonmq/%s/%s", scene->segment.fileName, scene->segment.fileName);
    play->sceneSegment = OTRPlay_LoadFile(play, scenePath.c_str());
    scene->unk_D = 0;
    gSegments[2] = (uintptr_t)VIRTUAL_TO_PHYSICAL(play->sceneSegment);
    OTRPlay_InitScene(play, spawn);
    Room_AllocateAndLoad(play, &play->roomCtx);
}

extern "C" s32 OTRfunc_800973FC(PlayState* play, RoomContext* roomCtx) {
    if (roomCtx->status == 1) {
        // if (!osRecvMesg(&roomCtx->loadQueue, NULL, OS_MESG_NOBLOCK)) {
        if (1) {
            roomCtx->status = 0;
            roomCtx->curRoom.segment = roomCtx->activeRoomVram;
            gSegments[3] = (uintptr_t)VIRTUAL_TO_PHYSICAL(roomCtx->activeRoomVram);

            OTRScene_ExecuteCommands(play, (LUS::Scene*)roomCtx->curRoom.segment);
            func_80123140(play, GET_PLAYER(play));
            Actor_SpawnTransitionActors(play, &play->actorCtx);
            if (((play->sceneId != SCENE_IKANA) || (roomCtx->curRoom.num != 1)) && (play->sceneId != SCENE_IKNINSIDE)) {
                play->envCtx.lightSettingOverride = LIGHT_SETTING_OVERRIDE_NONE;
                play->envCtx.lightBlendOverride = LIGHT_BLEND_OVERRIDE_NONE;
            }
            func_800FEAB0();
            if (Environment_GetStormState(play) == STORM_STATE_OFF) {
                Environment_StopStormNatureAmbience(play);
            }
            return 1;
        }

        return 0;
    }

    return 1;
}