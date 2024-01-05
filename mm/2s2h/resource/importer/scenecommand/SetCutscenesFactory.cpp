#include "2s2h/resource/importer/scenecommand/SetCutscenesFactory.h"
#include "2s2h/resource/type/scenecommand/SetCutscenes.h"
#include <libultraship/libultraship.h>
#include "spdlog/spdlog.h"

namespace LUS {
std::shared_ptr<IResource>
SetCutscenesFactory::ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetCutscenes>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
    case 0:
	    factory = std::make_shared<SetCutscenesFactoryV0>();
	    break;
    }

    if (factory == nullptr)
    {
        SPDLOG_ERROR("Failed to load SetCutscenes with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void LUS::SetCutscenesFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                        		  std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetCutscenes> setCutscenes = std::static_pointer_cast<SetCutscenes>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, setCutscenes);

    ReadCommandId(setCutscenes, reader);
    
    setCutscenes->fileName = reader->ReadString();
    setCutscenes->cutscene = std::static_pointer_cast<Cutscene>(LUS::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(setCutscenes->fileName.c_str()));
}

std::shared_ptr<IResource> SetCutsceneFactoryMM::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                             std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetCutscenesMM>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<SetCutscenesFactoryMMV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load SetCutscenes with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void LUS::SetCutscenesFactoryMMV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                                 std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetCutscenesMM> setCutscenes = std::static_pointer_cast<SetCutscenesMM>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, setCutscenes);

    ReadCommandId(setCutscenes, reader);

    size_t numCs = reader->ReadUByte();

    for (size_t i = 0; i < numCs; i++) {
        CutsceneScriptEntry entry;
        std::string path = reader->ReadString();
        entry.exit = reader->ReadUInt16();
        entry.entrance = reader->ReadUByte();
        entry.flag = reader->ReadUByte();
        entry.data = ResourceGetDataByName(path.c_str());
        setCutscenes->entries.emplace_back(entry);
    }
}

} // namespace LUS
