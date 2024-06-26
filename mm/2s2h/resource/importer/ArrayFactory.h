#pragma once

#include "Resource.h"
#include "ResourceFactoryBinary.h"

namespace SOH {
class ResourceFactoryBinaryArrayV0 : public Ship::ResourceFactoryBinary {
  public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file) override;
};
} // namespace SOH