#include "2s2h/resource/importer/scenecommand/SetCsCameraFactory.h"
#include "2s2h/resource/type/scenecommand/SetCsCamera.h"
#include "spdlog/spdlog.h"

namespace LUS {
std::shared_ptr<IResource>
SetCsCameraFactory::ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetCsCamera>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
    case 0:
	    factory = std::make_shared<SetCsCameraFactoryV0>();
	    break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load SetCsCamera with version {}", resource->GetInitData()->ResourceVersion);
	return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void LUS::SetCsCameraFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                        std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetCsCamera> setCsCamera = std::static_pointer_cast<SetCsCamera>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, setCsCamera);

    ReadCommandId(setCsCamera, reader);
	
    size_t camSize = reader->ReadUInt32(); 

    for (size_t i = 0; i < camSize; i++) {
        ActorCsCamInfoData data;
        data.setting = reader->ReadUInt16();
        data.count = reader->ReadUInt16();
        if (data.count == 0) {
            data.actorCsCamFuncData = nullptr;
            continue;
        }
        data.actorCsCamFuncData = new z64Vec3s[data.count];
        for (size_t j = 0; j < data.count; j++) {
            data.actorCsCamFuncData->x = reader->ReadInt16();
            data.actorCsCamFuncData->y = reader->ReadInt16();
            data.actorCsCamFuncData->z = reader->ReadInt16();
        }
        setCsCamera->csCamera.emplace_back(data);
    }
    
}

} // namespace LUS
