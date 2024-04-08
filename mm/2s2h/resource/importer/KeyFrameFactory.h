#pragma once

#include "Resource.h"
#include "ResourceFactoryBinary.h"

namespace SOH {
class ResourceFactoryBinaryKeyFrameSkel : public LUS::ResourceFactoryBinary {
  public:
    std::shared_ptr<LUS::IResource> ReadResource(std::shared_ptr<LUS::File> file) override;
};

class ResourceFactoryBinaryKeyFrameAnim : public LUS::ResourceFactoryBinary {
  public:
    std::shared_ptr<LUS::IResource> ReadResource(std::shared_ptr<LUS::File> file) override;
};

};