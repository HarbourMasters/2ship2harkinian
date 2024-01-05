#include "2s2h/resource/importer/PathFactory.h"
#include "2s2h/resource/type/Path.h"
#include "spdlog/spdlog.h"

namespace LUS {
std::shared_ptr<IResource>
PathFactory::ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<Path>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
    case 0:
	factory = std::make_shared<PathFactoryV0>();
	break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load Path with version {}", resource->GetInitData()->ResourceVersion);
	return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void LUS::PathFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                          std::shared_ptr<IResource> resource) {
    std::shared_ptr<Path> path = std::static_pointer_cast<Path>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, path);

    path->numPaths = reader->ReadUInt32();
    path->paths.reserve(path->numPaths);
    for (uint32_t k = 0; k < path->numPaths; k++) {
        std::vector<Vec3s> points;
        uint32_t pointCount = reader->ReadUInt32();
        points.reserve(pointCount);
        for (uint32_t i = 0; i < pointCount; i++) {
            Vec3s point;
            point.x = reader->ReadInt16();
            point.y = reader->ReadInt16();
            point.z = reader->ReadInt16();

            points.push_back(point);
        }

        PathData pathDataEntry;
        pathDataEntry.count = pointCount;
        
        path->paths.push_back(points);
        pathDataEntry.points = path->paths.back().data();

        path->pathData.push_back(pathDataEntry);
    }
}

std::shared_ptr<IResource> PathFactoryMM::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                     std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<Path>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<PathFactoryMMV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load Path with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void LUS::PathFactoryMMV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader, std::shared_ptr<IResource> resource) {
    std::shared_ptr<PathMM> path = std::static_pointer_cast<PathMM>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, path);

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
}


} // namespace LUS
