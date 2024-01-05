#include "SetMinimapChests.h"

namespace LUS {

MinimapChestData* SetMinimapChests::GetPointer() {
    return chests.data();
}

size_t SetMinimapChests::GetPointerSize() {
    return sizeof(MinimapChestData);
}

} // namespace LUS