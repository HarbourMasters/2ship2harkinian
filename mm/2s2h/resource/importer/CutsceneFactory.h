#pragma once

#include "Resource.h"
#include "ResourceFactory.h"


namespace LUS {
class Cutscene;
class CutsceneFactory : public ResourceFactory
{
  public:
    std::shared_ptr<IResource>
    ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) override;
};

class CutsceneFactoryV0 : public ResourceVersionFactory
{
  public:
    void ParseFileBinary(std::shared_ptr<BinaryReader> reader, std::shared_ptr<IResource> resource) override;

  private:
    void ParseFileBinaryOoT(std::shared_ptr<BinaryReader> reader,std::shared_ptr<Cutscene> cutscene);
    void ParseFileBinaryMM(std::shared_ptr<BinaryReader> reader, std::shared_ptr<Cutscene> cutscene);
};
}; // namespace LUS
