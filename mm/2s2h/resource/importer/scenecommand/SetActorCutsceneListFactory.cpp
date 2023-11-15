#include "2s2h/resource/importer/scenecommand/SetActorCutsceneListFactory.h"
#include "2s2h/resource/type/scenecommand/SetActorCutsceneList.h"
#include "spdlog/spdlog.h"

namespace LUS {
std::shared_ptr<IResource> SetActorCutsceneListFactory::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                                     std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetActorCutsceneList>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<SetActorCutsceneListFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load SetActorList with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}


void SetActorCutsceneListFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                                  std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetActorCutsceneList> setActorCsList = std::static_pointer_cast<SetActorCutsceneList>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, setActorCsList);

    ReadCommandId(setActorCsList, reader);

    setActorCsList->numEntries = reader->ReadUInt32();
    setActorCsList->entries.reserve(setActorCsList->numEntries);

    for (uint32_t i = 0; i < setActorCsList->numEntries; i++) {
        CutsceneEntry e;
        e.priority = reader->ReadInt16();
        e.length = reader->ReadInt16();
        e.csCamId = reader->ReadInt16();
        e.scriptIndex = reader->ReadInt16();
        e.additionalCsId = reader->ReadInt16();
        e.endSfx = reader->ReadUByte();
        e.customValue = reader->ReadUByte();
        e.hudVisibility = reader->ReadInt16();
        e.endCam = reader->ReadUByte();
        e.letterboxSize = reader->ReadUByte();
        setActorCsList->entries.emplace_back(e);
    }

}

} // namespace LUS