#include "SetAnimatedMaterialList.h"

namespace LUS {

AnimatedMaterialData* LUS::SetAnimatedMaterialList::GetPointer() {
    return mat;
}

size_t LUS::SetAnimatedMaterialList::GetPointerSize() {
    return sizeof(AnimatedMaterialData);
}

} // namespace LUS