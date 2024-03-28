#include "2s2h/resource/importer/scenecommand/SetLightListFactory.h"
#include "2s2h/resource/type/scenecommand/SetLightList.h"
#include "spdlog/spdlog.h"

namespace SOH {
std::shared_ptr<LUS::IResource>
SetLightListFactory::ReadResource(std::shared_ptr<LUS::ResourceInitData> initData, std::shared_ptr<LUS::BinaryReader> reader) {
    auto setLightList = std::make_shared<SetLightList>(initData);

    ReadCommandId(setLightList, reader);
    
    setLightList->numLights = reader->ReadUInt32();
    setLightList->lightList.reserve(setLightList->numLights);
    for (uint32_t i = 0; i < setLightList->numLights; i++) {
        LightInfo light;

        light.type = reader->ReadUByte();
		
	    light.params.point.x = reader->ReadInt16();
        light.params.point.y = reader->ReadInt16();
        light.params.point.z = reader->ReadInt16();

        light.params.point.color[0] = reader->ReadUByte(); // r
        light.params.point.color[1] = reader->ReadUByte(); // g
        light.params.point.color[2] = reader->ReadUByte(); // b

        light.params.point.drawGlow = reader->ReadUByte();
	    light.params.point.radius = reader->ReadInt16();

        setLightList->lightList.emplace_back(light);
    }

    return setLightList;
}
} // namespace SOH
