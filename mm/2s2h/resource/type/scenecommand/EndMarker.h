#pragma once

#include <cstdint>
#include "Resource.h"
#include "SceneCommand.h"
#include <libultraship/libultra/types.h>

namespace SOH {
typedef struct {

} Marker;

class EndMarker : public SceneCommand<Marker> {
  public:
    using SceneCommand::SceneCommand;

    Marker* GetPointer();
    size_t GetPointerSize();

    Marker endMarker;
};
}; // namespace SOH
