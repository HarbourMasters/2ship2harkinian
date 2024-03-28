#include "2s2h/resource/importer/scenecommand/SetCutscenesFactory.h"
#include "2s2h/resource/type/scenecommand/SetCutscenes.h"
#include <libultraship/libultraship.h>
#include "spdlog/spdlog.h"

namespace SOH {
std::shared_ptr<LUS::IResource> SetCutsceneFactoryMM::ReadResource(std::shared_ptr<LUS::ResourceInitData> initData,
                                                             std::shared_ptr<LUS::BinaryReader> reader) {
    auto setCutscenes = std::make_shared<SetCutscenesMM>(initData);

    ReadCommandId(setCutscenes, reader);

    size_t numCs = reader->ReadUByte();
    setCutscenes->entries.reserve(numCs);

    for (size_t i = 0; i < numCs; i++) {
        CutsceneScriptEntry entry;
        std::string path = reader->ReadString();
        entry.exit = reader->ReadUInt16();
        entry.entrance = reader->ReadUByte();
        entry.flag = reader->ReadUByte();
        entry.data = std::static_pointer_cast<Cutscene>(
            LUS::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(path.c_str()))->GetPointer();
        setCutscenes->entries.emplace_back(entry);
    }

    return setCutscenes;
}
} // namespace SOH
