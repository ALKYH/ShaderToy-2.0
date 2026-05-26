#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "render/shader_source.hpp"
#include "render/ground_mesh.hpp"
#include "render/textured_quad_mesh.hpp"
#include "render/triangle_mesh.hpp"

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

    {
        const auto& vertices = shader_toy::TriangleMesh::vertices();
        require(vertices.size() == 9, "Triangle mesh should contain 3 vertices with xyz components.");
        require(shader_toy::TriangleMesh::stride() == 3, "Triangle mesh stride should be 3 floats.");
    }

    {
        const auto& vertices = shader_toy::TexturedQuadMesh::vertices();
        require(
            vertices.size() == shader_toy::TexturedQuadMesh::vertex_count() * shader_toy::TexturedQuadMesh::stride(),
            "Textured quad should contain 6 vertices with xyzuv components."
        );
        require(
            shader_toy::TexturedQuadMesh::stride() == 5,
            "Textured quad stride should be 5 floats."
        );
    }

    {
        const auto& vertices = shader_toy::GroundMesh::vertices();
        require(
            vertices.size() == shader_toy::GroundMesh::vertex_count() * shader_toy::GroundMesh::stride(),
            "Ground mesh should contain 6 vertices with xyzuvnormal components."
        );
    }

    {
        const std::string fragment_shader = shader_toy::ShaderSource::load_file(
            project_root / "assets/shaders/debug_triangle.frag"
        );
        require(
            fragment_shader.find("FragColor") != std::string::npos,
            "Fragment shader should define FragColor output."
        );
    }

    {
        const std::string textured_fragment_shader = shader_toy::ShaderSource::load_file(
            project_root / "assets/shaders/textured_quad.frag"
        );
        require(
            textured_fragment_shader.find("sampler2D") != std::string::npos,
            "Textured fragment shader should use sampler2D."
        );
    }

    {
        const std::string lit_fragment_shader = shader_toy::ShaderSource::load_file(
            project_root / "assets/shaders/lit_model.frag"
        );
        require(
            lit_fragment_shader.find("applyDirectionalLight") != std::string::npos,
            "Lit fragment shader should contain directional light calculation."
        );
        require(
            lit_fragment_shader.find("applyPointLight") != std::string::npos,
            "Lit fragment shader should contain point light calculation."
        );
        require(
            lit_fragment_shader.find("applySpotLight") != std::string::npos,
            "Lit fragment shader should contain spotlight calculation."
        );
    }

    {
        const std::string shadow_fragment_shader = shader_toy::ShaderSource::load_file(
            project_root / "assets/shaders/lit_shadowed_model.frag"
        );
        require(
            shadow_fragment_shader.find("uShadowMap") != std::string::npos,
            "Shadowed fragment shader should sample the shadow map."
        );
        require(
            shadow_fragment_shader.find("computeShadow") != std::string::npos,
            "Shadowed fragment shader should compute shadow visibility."
        );
    }

    std::cout << "render_data_tests passed" << std::endl;
    return 0;
}
