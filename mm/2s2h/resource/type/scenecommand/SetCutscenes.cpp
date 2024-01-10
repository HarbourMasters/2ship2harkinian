#include "SetCutscenes.h"

namespace LUS {
uint32_t* SetCutscenes::GetPointer() {
    if (cutscene == nullptr) {
        return nullptr;
    }
    return cutscene->GetPointer();
}

size_t SetCutscenes::GetPointerSize() {
    if (cutscene == nullptr) {
        return 0;
    }
	return cutscene->GetPointerSize();
}

CutsceneScriptEntry* SetCutscenesMM::GetPointer() {
    if (entries.size() == 0) {
        return nullptr;
    }
    return entries.data();
}

size_t SetCutscenesMM::GetPointerSize() {
    if (entries.size() == 0) {
        return 0;
    }
    return entries.size() * sizeof(CutsceneScriptEntry);
}

} // namespace LUS
