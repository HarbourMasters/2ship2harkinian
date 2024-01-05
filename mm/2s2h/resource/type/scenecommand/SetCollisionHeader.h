#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include "Resource.h"
#include "2s2h/resource/type/scenecommand/SceneCommand.h"
#include "2s2h/resource/type/CollisionHeader.h"
// #include <libultraship/libultra/types.h>

namespace LUS {
class SetCollisionHeader : public SceneCommand<CollisionHeaderData> {
  public:
    using SceneCommand::SceneCommand;

    CollisionHeaderData* GetPointer();
    size_t GetPointerSize();

    std::string fileName;

    std::shared_ptr<CollisionHeader> collisionHeader;
};
}; // namespace LUS
