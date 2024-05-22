#pragma once

#include "Resource.h"
#include "ResourceFactoryBinary.h"
#include "ResourceFactoryXML.h"

namespace SOH {
class ResourceFactoryBinaryTextMMV0 : public Ship::ResourceFactoryBinary {
  public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file) override;
};

class ResourceFactoryXMLTextMMV0 : public Ship::ResourceFactoryXML {
  public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file) override;
};

#if 0
class TextMMFactory : public Ship::ResourceFactoryBinary
{
  public:
    std::shared_ptr<IResource>
    ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) override;
    std::shared_ptr<IResource>
    ReadResourceXML(std::shared_ptr<ResourceInitData> initData, tinyxml2::XMLElement *reader) override;
};
#endif
}; // namespace SOH
