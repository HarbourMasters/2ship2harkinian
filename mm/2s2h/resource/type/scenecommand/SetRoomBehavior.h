#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "Resource.h"
#include "SceneCommand.h"
#include <libultraship/libultra/types.h>

namespace LUS {
typedef struct {
  int8_t gameplayFlags;
  int32_t gameplayFlags2;
} RoomBehavior;

typedef struct {
    int8_t gameplayFlags;
    int8_t currRoomUnk2;
    int8_t currRoomUnk5;
    int8_t msgCtxUnk;
    int8_t enablePointLights;
    int8_t kankyoContextUnkE2;
} RoomBehaviorMM;

class SetRoomBehavior : public SceneCommand<RoomBehavior> {
  public:
    using SceneCommand::SceneCommand;

    RoomBehavior* GetPointer();
    size_t GetPointerSize();

    RoomBehavior roomBehavior;
};


class SetRoomBehaviorMM : public SceneCommand<RoomBehaviorMM> {
  public:
    using SceneCommand::SceneCommand;

    RoomBehaviorMM* GetPointer();
    size_t GetPointerSize();

    RoomBehaviorMM roomBehavior;
};

}; // namespace LUS
