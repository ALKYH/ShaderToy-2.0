#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "render/obj_loader.hpp"

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}
} // namespace

int main()
{
    const std::filesystem::path project_root = SHADER_TOY_PROJECT_ROOT;
    const shader_toy::ObjMeshData mesh = shader_toy::ObjLoader::load(
        project_root / "tests/fixtures/test_triangle.obj"
    );

    require(mesh.stride == 8, "OBJ vertex stride should be 8 floats: pos3 + uv2 + normal3.");
    require(mesh.vertices.size() == 24, "OBJ triangle should expand to 3 vertices * 8 floats.");
    require(mesh.vertices[0] == -0.5F, "First vertex x should match OBJ position.");
    require(mesh.vertices[3] == 0.0F, "First vertex u should match OBJ texcoord.");
    require(mesh.vertices[5] == 0.0F, "First vertex normal x should match OBJ normal.");
    require(mesh.vertices[7] == 1.0F, "First vertex normal z should match OBJ normal.");
    require(
        mesh.material_library_path.filename() == "test_triangle.mtl",
        "OBJ should expose referenced material library path."
    );
    require(
        mesh.material_name == "triangle_material",
        "OBJ should expose active material name."
    );
    require(
        mesh.material.diffuse_map_path.filename() == "test_rgba.ppm",
        "OBJ material should resolve diffuse texture path from MTL."
    );

    std::cout << "obj_loader_tests passed" << std::endl;
    return 0;
}
