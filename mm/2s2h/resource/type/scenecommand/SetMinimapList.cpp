#include "SetMinimapList.h"

namespace LUS {

MinimapListData* LUS::SetMinimapList::GetPointer() {
    return &list;
}

size_t LUS::SetMinimapList::GetPointerSize() {
    return sizeof(list);
}

} // namespace LUS