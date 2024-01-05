#pragma once

#include "2s2h/resource/importer/scenecommand/SceneCommandFactory.h"

namespace LUS {
class SetRoomBehaviorFactory : public SceneCommandFactory {
  public:
    std::shared_ptr<IResource>
    ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) override;
};

class SetRoomBehaviorFactoryV0 : public SceneCommandVersionFactory {
  public:
    void ParseFileBinary(std::shared_ptr<BinaryReader> reader, std::shared_ptr<IResource> resource) override;
};

class SetRoomBehaviorMMFactory : public SceneCommandFactory {
  public:
    std::shared_ptr<IResource> ReadResource(std::shared_ptr<ResourceInitData> initData,
                                            std::shared_ptr<BinaryReader> reader) override;
};

class SetRoomBehaviorMMFactoryV0 : public SceneCommandVersionFactory {
  public:
    void ParseFileBinary(std::shared_ptr<BinaryReader> reader, std::shared_ptr<IResource> resource) override;
};

}; // namespace LUS
