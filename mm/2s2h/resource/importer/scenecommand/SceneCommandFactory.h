#pragma once

#include <memory>
#include "Resource.h"
#include "ResourceFactory.h"
#include "2s2h/resource/type/scenecommand/SceneCommand.h"

namespace LUS {
class SceneCommandFactory : public ResourceFactory {};

class SceneCommandVersionFactory : public ResourceVersionFactory {
protected:
    void ReadCommandId(std::shared_ptr<ISceneCommand> command, std::shared_ptr<BinaryReader> reader);
};
}; // namespace LUS
