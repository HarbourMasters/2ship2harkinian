#include "2s2h/resource/importer/scenecommand/SetAnimatedMaterialListFactory.h"
#include "2s2h/resource/type/scenecommand/SetAnimatedMaterialList.h"
#include "2s2h/resource/type/TextureAnimation.h"
#include <libultraship/libultraship.h>

namespace SOH {

std::shared_ptr<LUS::IResource> SetAnimatedMaterialListFactory::ReadResource(std::shared_ptr<LUS::ResourceInitData> initData,
                                                                             std::shared_ptr<LUS::BinaryReader> reader) {
    auto setAnimatedMat = std::make_shared<SetAnimatedMaterialList>(initData);
    
    ReadCommandId(setAnimatedMat, reader);

    std::string str = reader->ReadString();
    const auto data = std::static_pointer_cast<TextureAnimation>(
        LUS::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(str.c_str()));

    AnimatedMaterial* res = data->GetPointer();
    setAnimatedMat->mat = res;
    
    return setAnimatedMat;
}
} // namespace LUS