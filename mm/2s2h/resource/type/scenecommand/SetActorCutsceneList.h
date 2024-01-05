#pragma once

#include "Resource.h"
#include "SceneCommand.h"

namespace LUS {
typedef struct CutsceneEntry {
    int16_t priority;
    int16_t length;
    int16_t csCamId;
    int16_t scriptIndex;
    int16_t additionalCsId;
    uint8_t endSfx;
    uint8_t customValue;
    int16_t hudVisibility;
    uint8_t endCam;
    uint8_t letterboxSize;
} CutsceneEntry;

class SetActorCutsceneList : public SceneCommand<CutsceneEntry> {
  public:
      using SceneCommand::SceneCommand;

      CutsceneEntry* GetPointer();
      size_t GetPointerSize();

      uint32_t numEntries;
      std::vector<CutsceneEntry> entries;
};

}