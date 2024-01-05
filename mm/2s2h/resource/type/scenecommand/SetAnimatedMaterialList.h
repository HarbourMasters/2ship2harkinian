#pragma once

#include "Resource.h"
#include "SceneCommand.h"
#include <libultraship/libultra/types.h>

namespace LUS {
typedef struct {
    /* 0x0 */ int8_t segment;
    /* 0x2 */ int16_t type;
    /* 0x4 */ void* params;
} AnimatedMaterialData; // size = 0x8

class SetAnimatedMaterialList : public SceneCommand<AnimatedMaterialData> {
  public:
    using SceneCommand::SceneCommand;

    AnimatedMaterialData* GetPointer();
    size_t GetPointerSize();

    AnimatedMaterialData* mat;
};
}; // namespace LUS
