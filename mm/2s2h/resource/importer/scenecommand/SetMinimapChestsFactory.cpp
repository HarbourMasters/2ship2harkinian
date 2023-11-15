#include "2s2h/resource/importer/scenecommand/SetMinimapChestsFactory.h"
#include "2s2h/resource/type/scenecommand/SetMinimapChests.h"
#include "spdlog/spdlog.h"

namespace LUS {

std::shared_ptr<IResource> SetMinimapChestsFactory::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                                      std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetMinimapChests>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<SetMinimapChestsFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load SetMinimapChestsFactory with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void SetMinimapChestsFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                                     std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetMinimapChests> chests = std::static_pointer_cast<SetMinimapChests>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, chests);

    ReadCommandId(chests, reader);

    size_t size = reader->ReadUInt32();

    chests->chests.reserve(size);

    for (size_t i = 0; i < size; i++) {
        MinimapChestData d;
        d.unk_0 = reader->ReadUInt16();
        d.unk_2 = reader->ReadUInt16();
        d.unk_4 = reader->ReadUInt16();
        d.unk_6 = reader->ReadUInt16();
        d.unk_8 = reader->ReadUInt16();
        chests->chests.emplace_back(d);
    }
}

} // namespace LUS