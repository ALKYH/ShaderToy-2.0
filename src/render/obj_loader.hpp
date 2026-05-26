#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace shader_toy
{
struct MaterialData
{
    std::string name;
    std::filesystem::path diffuse_map_path;
};

struct ObjMeshData
{
    std::vector<float> vertices;
    std::size_t stride = 8;
    std::filesystem::path source_path;
    std::filesystem::path material_library_path;
    std::string material_name;
    MaterialData material;
};

class ObjLoader
{
public:
    [[nodiscard]] static ObjMeshData load(const std::filesystem::path& path);
};
} // namespace shader_toy
