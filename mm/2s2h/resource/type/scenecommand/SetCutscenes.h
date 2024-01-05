#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include "Resource.h"
#include "2s2h/resource/type/scenecommand/SceneCommand.h"
#include "2s2h/resource/type/Cutscene.h"
// #include <libultraship/libultra.h>

namespace LUS {
class SetCutscenes : public SceneCommand<uint32_t> {
  public:
    using SceneCommand::SceneCommand;

    uint32_t* GetPointer();
    size_t GetPointerSize();

    std::string fileName;
    std::shared_ptr<Cutscene> cutscene;
};

class CutsceneScriptEntry {
  public:
    void* data;
    uint16_t exit;
    uint8_t entrance;
    uint8_t flag;
};


class SetCutscenesMM : public SceneCommand<CutsceneScriptEntry> {
  public:
    using SceneCommand::SceneCommand;

    CutsceneScriptEntry* GetPointer();
    size_t GetPointerSize();

    std::vector<CutsceneScriptEntry> entries;
};

}; // namespace LUS
