#include "2s2h/resource/importer/scenecommand/SetAnimatedMaterialListFactory.h"
#include "2s2h/resource/type/scenecommand/SetAnimatedMaterialList.h"
#include "2s2h/resource/type/TextureAnimation.h"
#include "spdlog/spdlog.h"
#include <libultraship/libultraship.h>

namespace LUS {

std::shared_ptr<IResource> LUS::SetAnimatedMaterialListFactory::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                                             std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<SetAnimatedMaterialList>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<SetAnimatedMaterialListFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load SetAnimatedMaterialList with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void LUS::SetAnimatedMaterialListFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                                            std::shared_ptr<IResource> resource) {
    std::shared_ptr<SetAnimatedMaterialList> setAnimatedMat =
        std::static_pointer_cast<SetAnimatedMaterialList>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, setAnimatedMat);

    ReadCommandId(setAnimatedMat, reader);
    AnimatedMaterialData* res = (AnimatedMaterialData*)ResourceGetDataByName(reader->ReadString().c_str());
    setAnimatedMat->mat = res;
}

} // namespace LUS