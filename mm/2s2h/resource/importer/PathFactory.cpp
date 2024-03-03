#include "2s2h/resource/importer/PathFactory.h"
#include "2s2h/resource/type/Path.h"
#include "spdlog/spdlog.h"

namespace SOH {
std::shared_ptr<LUS::IResource> ResourceFactoryBinaryPathMMV0::ReadResource(std::shared_ptr<LUS::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }

    auto path = std::make_shared<PathMM>(file->InitData);
    auto reader = std::get<std::shared_ptr<LUS::BinaryReader>>(file->Reader);

    path->numPaths = reader->ReadUInt32();
    path->paths.reserve(path->numPaths);
    for (uint32_t k = 0; k < path->numPaths; k++) {
        std::vector<Vec3s> points;
        uint32_t pointCount = reader->ReadUInt32();

        uint8_t additionalPathIndex = reader->ReadUByte();
        int16_t customValue = reader->ReadInt16();

        for (uint32_t i = 0; i < pointCount; i++) {
            Vec3s point;
            point.x = reader->ReadInt16();
            point.y = reader->ReadInt16();
            point.z = reader->ReadInt16();

            points.push_back(point);
        }

        PathDataMM pathDataEntry;
        pathDataEntry.count = pointCount;
        pathDataEntry.additionalPathIndex = additionalPathIndex;
        pathDataEntry.customValue = customValue;

        path->paths.push_back(points);
        pathDataEntry.points = path->paths.back().data();

        path->pathData.push_back(pathDataEntry);
    }

    return path;
}
} // namespace SOH
