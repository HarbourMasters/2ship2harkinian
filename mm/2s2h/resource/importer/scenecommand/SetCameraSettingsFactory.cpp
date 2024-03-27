#include "2s2h/resource/importer/scenecommand/SetCameraSettingsFactory.h"
#include "2s2h/resource/type/scenecommand/SetCameraSettings.h"
#include "spdlog/spdlog.h"

namespace SOH {
std::shared_ptr<LUS::IResource> SetCameraSettingsFactory::ReadResource(std::shared_ptr<LUS::ResourceInitData> initData,
                                                                 std::shared_ptr<LUS::BinaryReader> reader) {
    auto setCameraSettings = std::make_shared<SetCameraSettings>(initData);

    ReadCommandId(setCameraSettings, reader);

    return setCameraSettings;
}
} // namespace SOH
