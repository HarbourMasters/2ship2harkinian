#pragma once

#include <cstdint>
#include <vector>
#include "Resource.h"
#include <libultraship/libultra/types.h>
#include "z64math.h"

namespace LUS {

typedef struct {
    /* 0x00 */ u8 count; // number of points in the path
    /* 0x04 */ Vec3s* points; // Segment Address to the array of points
} PathData; // size = 0x8

typedef struct {
    /* 0x0 */ u8 count; // Number of points in the path
    /* 0x1 */ u8 additionalPathIndex;
    /* 0x2 */ s16 customValue; // Path specific to help distinguish different paths
    /* 0x4 */ Vec3s* points;   // Segment Address to the array of points
} PathDataMM; // size = 0x8

class Path : public Resource<PathData> {
public:
  using Resource::Resource;

    Path() : Resource(std::shared_ptr<ResourceInitData>()) {}

    PathData* GetPointer();
    size_t GetPointerSize();

    uint32_t numPaths;
    std::vector<PathData> pathData;
    std::vector<std::vector<Vec3s>> paths;
};

class PathMM : public Resource<PathDataMM> {
  public:
    using Resource::Resource;

    PathMM() : Resource(std::shared_ptr<ResourceInitData>()) {
    }

    PathDataMM* GetPointer();
    size_t GetPointerSize();

    uint32_t numPaths;
    std::vector<PathDataMM> pathData;
    std::vector<std::vector<Vec3s>> paths;
};

}; // namespace LUS
