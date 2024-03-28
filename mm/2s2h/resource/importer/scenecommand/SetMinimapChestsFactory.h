#pragma once

#include "2s2h/resource/importer/scenecommand/SceneCommandFactory.h"

namespace SOH {
class SetMinimapChestsFactory : public SceneCommandFactoryBinaryV0 {
  public:
    std::shared_ptr<LUS::IResource> ReadResource(std::shared_ptr<LUS::ResourceInitData> initData,
                                            std::shared_ptr<LUS::BinaryReader> reader) override;
};
}