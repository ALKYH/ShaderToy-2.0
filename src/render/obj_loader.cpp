#include "render/obj_loader.hpp"

#include <array>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace shader_toy
{
namespace
{
struct Vec2
{
    float x = 0.0F;
    float y = 0.0F;
};

struct Vec3
{
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
};
} // namespace

ObjMeshData ObjLoader::load(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        throw std::runtime_error("Failed to open OBJ file: " + path.string());
    }

    std::vector<Vec3> positions;
    std::vector<Vec2> texcoords;
    std::vector<Vec3> normals;
    ObjMeshData mesh;
    mesh.source_path = path;
    const std::filesystem::path obj_directory = path.parent_path();

    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream stream(line);
        std::string prefix;
        stream >> prefix;

        if (prefix == "v")
        {
            Vec3 position;
            stream >> position.x >> position.y >> position.z;
            positions.push_back(position);
        }
        else if (prefix == "vt")
        {
            Vec2 texcoord;
            stream >> texcoord.x >> texcoord.y;
            texcoords.push_back(texcoord);
        }
        else if (prefix == "vn")
        {
            Vec3 normal;
            stream >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (prefix == "mtllib")
        {
            std::string material_library_name;
            stream >> material_library_name;
            mesh.material_library_path = obj_directory / material_library_name;
        }
        else if (prefix == "usemtl")
        {
            stream >> mesh.material_name;
        }
        else if (prefix == "f")
        {
            std::array<std::string, 3> face_tokens{};
            stream >> face_tokens[0] >> face_tokens[1] >> face_tokens[2];

            for (const std::string& token : face_tokens)
            {
                std::istringstream token_stream(token);
                std::string position_index_string;
                std::string texcoord_index_string;
                std::string normal_index_string;

                std::getline(token_stream, position_index_string, '/');
                std::getline(token_stream, texcoord_index_string, '/');
                std::getline(token_stream, normal_index_string, '/');

                const int position_index = std::stoi(position_index_string) - 1;
                const int texcoord_index = std::stoi(texcoord_index_string) - 1;
                const int normal_index = std::stoi(normal_index_string) - 1;

                const Vec3& position = positions.at(static_cast<std::size_t>(position_index));
                const Vec2& texcoord = texcoords.at(static_cast<std::size_t>(texcoord_index));
                const Vec3& normal = normals.at(static_cast<std::size_t>(normal_index));

                mesh.vertices.push_back(position.x);
                mesh.vertices.push_back(position.y);
                mesh.vertices.push_back(position.z);
                mesh.vertices.push_back(texcoord.x);
                mesh.vertices.push_back(texcoord.y);
                mesh.vertices.push_back(normal.x);
                mesh.vertices.push_back(normal.y);
                mesh.vertices.push_back(normal.z);
            }
        }
    }

    if (!mesh.material_library_path.empty())
    {
        std::ifstream material_input(mesh.material_library_path);
        if (!material_input.is_open())
        {
            throw std::runtime_error(
                "Failed to open MTL file referenced by OBJ: " + mesh.material_library_path.string()
            );
        }

        std::string active_material_name;
        while (std::getline(material_input, line))
        {
            if (line.empty() || line[0] == '#')
            {
                continue;
            }

            std::istringstream material_stream(line);
            std::string prefix_token;
            material_stream >> prefix_token;

            if (prefix_token == "newmtl")
            {
                material_stream >> active_material_name;
                if (active_material_name == mesh.material_name)
                {
                    mesh.material.name = active_material_name;
                }
            }
            else if (
                prefix_token == "map_Kd"
                && !active_material_name.empty()
                && active_material_name == mesh.material_name
            )
            {
                std::string diffuse_path;
                material_stream >> diffuse_path;
                mesh.material.diffuse_map_path = mesh.material_library_path.parent_path() / diffuse_path;
            }
        }
    }

    return mesh;
}
} // namespace shader_toy
