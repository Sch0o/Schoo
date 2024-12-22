#include<unordered_map>
#include<filesystem>
#include"external/tinygltf/tiny_gltf.h"

struct GeoSurface {
    uint32_t startIndex;
    uint32_t count;
};

struct MeshAsset{
    std::string name;
    std::vector<GeoSurface>surfaces;
};

