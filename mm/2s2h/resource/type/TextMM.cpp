#include "TextMM.h"

namespace LUS {
MessageEntryMM* TextMM::GetPointer() {
    return messages.data();
}

size_t TextMM::GetPointerSize() {
	return messages.size() * sizeof(MessageEntryMM);
}
} // namespace LUS
