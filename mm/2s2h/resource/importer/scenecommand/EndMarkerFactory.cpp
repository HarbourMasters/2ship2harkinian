#include "2s2h/resource/importer/scenecommand/EndMarkerFactory.h"
#include "2s2h/resource/type/scenecommand/EndMarker.h"
#include "spdlog/spdlog.h"

namespace SOH {
std::shared_ptr<LUS::IResource>
EndMarkerFactory::ReadResource(std::shared_ptr<LUS::ResourceInitData> initData, std::shared_ptr<LUS::BinaryReader> reader) {
    auto endMarker = std::make_shared<EndMarker>(initData);

    ReadCommandId(endMarker, reader);

    return endMarker;
}
} // namespace SOH
