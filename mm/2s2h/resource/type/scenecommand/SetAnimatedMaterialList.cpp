#include "SetAnimatedMaterialList.h"

namespace SOH {

AnimatedMaterialData* SetAnimatedMaterialList::GetPointer() {
    return mat;
}

size_t SetAnimatedMaterialList::GetPointerSize() {
    return sizeof(AnimatedMaterialData);
}

} // namespace SOH