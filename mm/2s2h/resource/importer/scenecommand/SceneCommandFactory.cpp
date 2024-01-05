#include "2s2h/resource/importer/scenecommand/SceneCommandFactory.h"
#include "2s2h/resource/type/scenecommand/SceneCommand.h"
#include "spdlog/spdlog.h"

namespace LUS {
void SceneCommandVersionFactory::ReadCommandId(std::shared_ptr<ISceneCommand> command, std::shared_ptr<BinaryReader> reader) {
    command->cmdId = (SceneCommandID)reader->ReadInt32();
}
}
    