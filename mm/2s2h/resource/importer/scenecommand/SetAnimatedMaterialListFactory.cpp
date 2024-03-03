#include "2s2h/resource/importer/scenecommand/SetAnimatedMaterialListFactory.h"
#include "2s2h/resource/type/scenecommand/SetAnimatedMaterialList.h"
#include "2s2h/resource/type/TextureAnimation.h"
#include <libultraship/libultraship.h>

namespace SOH {

std::shared_ptr<LUS::IResource> SetAnimatedMaterialListFactory::ReadResource(std::shared_ptr<LUS::ResourceInitData> initData,
                                                                             std::shared_ptr<LUS::BinaryReader> reader) {
    auto setAnimatedMat = std::make_shared<SetAnimatedMaterialList>(initData);
    
    ReadCommandId(setAnimatedMat, reader);
    AnimatedMaterialData* res = (AnimatedMaterialData*)ResourceGetDataByName(reader->ReadString().c_str());
    setAnimatedMat->mat = res;
    
    return setAnimatedMat;
}
} // namespace LUS