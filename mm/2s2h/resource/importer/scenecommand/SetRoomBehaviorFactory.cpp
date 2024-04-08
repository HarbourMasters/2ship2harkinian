#include "2s2h/resource/importer/scenecommand/SetRoomBehaviorFactory.h"
#include "2s2h/resource/type/scenecommand/SetRoomBehavior.h"
#include "spdlog/spdlog.h"

namespace SOH {
std::shared_ptr<LUS::IResource> SetRoomBehaviorMMFactory::ReadResource(std::shared_ptr<LUS::ResourceInitData> initData,
                                                                  std::shared_ptr<LUS::BinaryReader> reader) {
    auto setRoomBehavior = std::make_shared<SetRoomBehaviorMM>(initData);

    ReadCommandId(setRoomBehavior, reader);

    setRoomBehavior->roomBehavior.gameplayFlags = reader->ReadInt8();
    setRoomBehavior->roomBehavior.currRoomUnk2 = reader->ReadInt8();
    setRoomBehavior->roomBehavior.currRoomUnk5 = reader->ReadInt8();
    setRoomBehavior->roomBehavior.msgCtxUnk = reader->ReadInt8();
    setRoomBehavior->roomBehavior.enablePointLights = reader->ReadInt8();
    setRoomBehavior->roomBehavior.kankyoContextUnkE2 = reader->ReadInt8();

    return setRoomBehavior;
}
} // namespace SOH
