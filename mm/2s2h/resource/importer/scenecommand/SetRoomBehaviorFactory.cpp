#include "2s2h/resource/importer/scenecommand/SetRoomBehaviorFactory.h"
#include "2s2h/resource/type/scenecommand/SetRoomBehavior.h"
#include "spdlog/spdlog.h"

namespace LUS {
// OOT
#if 0
std::shared_ptr<IResource>
SetRoomBehaviorFactory::ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetRoomBehavior>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
    case 0:
	factory = std::make_shared<SetRoomBehaviorFactoryV0>();
	break;
    }

    if (factory == nullptr) {
	SPDLOG_ERROR("Failed to load SetRoomBehavior with version {}", resource->GetInitData()->ResourceVersion);
	return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void LUS::SetRoomBehaviorFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                        std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetRoomBehavior> setRoomBehavior = std::static_pointer_cast<SetRoomBehavior>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, setRoomBehavior);

    ReadCommandId(setRoomBehavior, reader);
	
    setRoomBehavior->roomBehavior.gameplayFlags = reader->ReadInt8();
    setRoomBehavior->roomBehavior.gameplayFlags2 = reader->ReadInt32();
}
#endif
// MM

std::shared_ptr<IResource> SetRoomBehaviorMMFactory::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                                  std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetRoomBehaviorMM>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<SetRoomBehaviorMMFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load SetRoomBehavior with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void SetRoomBehaviorMMFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                                 std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetRoomBehaviorMM> setRoomBehavior = std::static_pointer_cast<SetRoomBehaviorMM>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, setRoomBehavior);

    ReadCommandId(setRoomBehavior, reader);

    setRoomBehavior->roomBehavior.gameplayFlags = reader->ReadInt8();
    setRoomBehavior->roomBehavior.currRoomUnk2 = reader->ReadInt8();
    setRoomBehavior->roomBehavior.currRoomUnk5 = reader->ReadInt8();
    setRoomBehavior->roomBehavior.msgCtxUnk = reader->ReadInt8();
    setRoomBehavior->roomBehavior.enablePointLights = reader->ReadInt8();
    setRoomBehavior->roomBehavior.kankyoContextUnkE2 = reader->ReadInt8();
}

} // namespace LUS
