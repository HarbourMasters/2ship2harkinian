#pragma once

#include "2s2h/resource/importer/scenecommand/SceneCommandFactory.h"

namespace LUS {
class SetAnimatedMaterialListFactory : public SceneCommandFactory {
  public:
    std::shared_ptr<IResource> ReadResource(std::shared_ptr<ResourceInitData> initData,
                                            std::shared_ptr<BinaryReader> reader) override;
};

class SetAnimatedMaterialListFactoryV0 : public SceneCommandVersionFactory {
  public:
    void ParseFileBinary(std::shared_ptr<BinaryReader> reader, std::shared_ptr<IResource> resource) override;
};

}
