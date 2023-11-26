#include "SetRoomBehavior.h"

namespace LUS {
RoomBehavior* SetRoomBehavior::GetPointer() {
    return &roomBehavior;
}

size_t SetRoomBehavior::GetPointerSize() {
	return sizeof(RoomBehavior);
}
RoomBehaviorMM* SetRoomBehaviorMM::GetPointer() {
    return &roomBehavior;
}
size_t SetRoomBehaviorMM::GetPointerSize() {
    return sizeof(RoomBehaviorMM);
}
} // namespace LUS
