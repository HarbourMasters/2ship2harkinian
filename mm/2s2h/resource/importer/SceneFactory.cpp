#include "spdlog/spdlog.h"
#include "2s2h/resource/importer/SceneFactory.h"
#include "2s2h/resource/type/Scene.h"
#include "2s2h/resource/type/scenecommand/SceneCommand.h"
#include "2s2h/resource/importer/scenecommand/SetLightingSettingsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetWindSettingsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetExitListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetTimeSettingsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetSkyboxModifierFactory.h"
#include "2s2h/resource/importer/scenecommand/SetEchoSettingsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetSoundSettingsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetSkyboxSettingsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetRoomBehaviorFactory.h"
#include "2s2h/resource/importer/scenecommand/SetCsCameraFactory.h"
#include "2s2h/resource/importer/scenecommand/SetCameraSettingsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetRoomListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetCollisionHeaderFactory.h"
#include "2s2h/resource/importer/scenecommand/SetEntranceListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetSpecialObjectsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetObjectListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetStartPositionListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetActorListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetTransitionActorListFactory.h"
#include "2s2h/resource/importer/scenecommand/EndMarkerFactory.h"
#include "2s2h/resource/importer/scenecommand/SetAlternateHeadersFactory.h"
#include "2s2h/resource/importer/scenecommand/SetPathwaysFactory.h"
#include "2s2h/resource/importer/scenecommand/SetCutscenesFactory.h"
#include "2s2h/resource/importer/scenecommand/SetLightListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetMeshFactory.h"
#include "2s2h/resource/importer/scenecommand/SetAnimatedMaterialListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetMinimapListFactory.h"
#include "2s2h/resource/importer/scenecommand/SetMinimapChestsFactory.h"
#include "2s2h/resource/importer/scenecommand/SetActorCutsceneListFactory.h"

namespace LUS {

std::shared_ptr<IResource>
SceneFactory::ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) {
    if (SceneFactory::sceneCommandFactories.empty()) {
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetLightingSettings] = std::make_shared<SetLightingSettingsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetWind] = std::make_shared<SetWindSettingsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetExitList] = std::make_shared<SetExitListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetTimeSettings] = std::make_shared<SetTimeSettingsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetSkyboxModifier] = std::make_shared<SetSkyboxModifierFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetEchoSettings] = std::make_shared<SetEchoSettingsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetSoundSettings] = std::make_shared<SetSoundSettingsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetSkyboxSettings] = std::make_shared<SetSkyboxSettingsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetRoomBehavior] = std::make_shared<SetRoomBehaviorMMFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetCsCamera] = std::make_shared<SetCsCameraFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetCameraSettings] = std::make_shared<SetCameraSettingsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetRoomList] = std::make_shared<SetRoomListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetCollisionHeader] = std::make_shared<SetCollisionHeaderFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetEntranceList] = std::make_shared<SetEntranceListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetSpecialObjects] = std::make_shared<SetSpecialObjectsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetObjectList] = std::make_shared<SetObjectListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetStartPositionList] = std::make_shared<SetStartPositionListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetActorList] = std::make_shared<SetActorListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetTransitionActorList] = std::make_shared<SetTransitionActorListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::EndMarker] = std::make_shared<EndMarkerFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetAlternateHeaders] = std::make_shared<SetAlternateHeadersFactory>();
        // TODO should we use a different custom scene command like cutscenes?
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetPathways] = std::make_shared<SetPathwaysMMFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetCutscenes] = std::make_shared<SetCutscenesFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetLightList] = std::make_shared<SetLightListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetMesh] = std::make_shared<SetMeshFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetCutscenesMM] = std::make_shared<SetCutsceneFactoryMM>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetAnimatedMaterialList] =
            std::make_shared<SetAnimatedMaterialListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetMinimapList] =
            std::make_shared<SetMinimapListFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetMinimapChests] =
            std::make_shared<SetMinimapChestsFactory>();
        SceneFactory::sceneCommandFactories[LUS::SceneCommandID::SetActorCutsceneList] =
            std::make_shared<SetActorCutsceneListFactory>();
    }

    auto resource = std::make_shared<Scene>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
    case 0:
	    factory = std::make_shared<SceneFactoryV0>();
	    break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load Scene with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void SceneFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                        std::shared_ptr<IResource> resource)
{
    std::shared_ptr<Scene> scene = std::static_pointer_cast<Scene>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, scene);

    ParseSceneCommands(scene, reader);
}

void SceneFactoryV0::ParseSceneCommands(std::shared_ptr<Scene> scene, std::shared_ptr<BinaryReader> reader) {
    uint32_t commandCount = reader->ReadUInt32();
    scene->commands.reserve(commandCount);

    for (uint32_t i = 0; i < commandCount; i++) {
        scene->commands.push_back(ParseSceneCommand(scene, reader, i));
    }
}

std::shared_ptr<ISceneCommand> SceneFactoryV0::ParseSceneCommand(std::shared_ptr<Scene> scene,
                                                                std::shared_ptr<BinaryReader> reader, uint32_t index) {
    SceneCommandID cmdID = (SceneCommandID)reader->ReadInt32();

    reader->Seek(-sizeof(int32_t), SeekOffsetType::Current);

    std::shared_ptr<ISceneCommand> result = nullptr;
    std::shared_ptr<SceneCommandFactory> commandFactory = SceneFactory::sceneCommandFactories[cmdID];

    if (commandFactory != nullptr) {
        auto initData = std::make_shared<ResourceInitData>();
        initData->Id = scene->GetInitData()->Id;
        initData->Type = ResourceType::SOH_SceneCommand;
        initData->Path = scene->GetInitData()->Path + "/SceneCommand" + std::to_string(index);
        initData->ResourceVersion = scene->GetInitData()->ResourceVersion;
        result = std::static_pointer_cast<ISceneCommand>(commandFactory->ReadResource(initData, reader));
        // Cache the resource?
    }

    if (result == nullptr) {
        SPDLOG_ERROR("Failed to load scene command of type {} in scene {}", (uint32_t)cmdID, scene->GetInitData()->Path);
    }

    return result;
}

} // namespace LUS
