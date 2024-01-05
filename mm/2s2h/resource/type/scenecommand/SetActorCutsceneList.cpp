#include "SetActorCutsceneList.h"
#include "2s2h/resource/type/scenecommand/SetActorCutsceneList.h"

namespace LUS {
CutsceneEntry* SetActorCutsceneList::GetPointer() {
    return entries.data();
}

size_t SetActorCutsceneList::GetPointerSize() {
    return entries.size() * sizeof(CutsceneEntry);
}

} // namespace LUS