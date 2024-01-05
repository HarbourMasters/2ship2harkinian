#include "2s2h/resource/importer/scenecommand/SetMinimapListFactory.h"
#include "2s2h/resource/type/scenecommand/SetMinimapList.h"
#include "spdlog/spdlog.h"

namespace LUS {
std::shared_ptr<IResource> SetMinimapListFactory::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                               std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetMinimapList>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<SetMinimapListFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load SetMinimapListFactory with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}
void SetMinimapListFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                              std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetMinimapList> mapList = std::static_pointer_cast<SetMinimapList>(resource);

    ResourceVersionFactory::ParseFileBinary(reader, mapList);

    ReadCommandId(mapList, reader);

    size_t size = reader->ReadUInt32();

    mapList->list.scale = reader->ReadUInt16();
    mapList->entries.reserve(size);

    for (size_t i = 0; i < size; i++) {
        MinimapEntryData data;
        data.mapId = reader->ReadUInt16();
        data.unk2 = reader->ReadUInt16();
        data.unk4 = reader->ReadUInt16();
        data.unk6 = reader->ReadUInt16();
        data.unk8 = reader->ReadUInt16();
        mapList->entries.emplace_back(data);
    }
    mapList->list.entry = mapList->entries.data();
}
} // namespace LUS
