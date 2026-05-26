#include <iostream>
#include <filesystem>
#include <stdexcept>

#include "render/image_loader.hpp"
#include "render/shader_source.hpp"

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
    using shader_toy::ImageData;
    using shader_toy::ImageLoader;
    using shader_toy::ShaderSource;

    const std::filesystem::path project_root = SHADER_TOY_PROJECT_ROOT;

    {
        const std::string source = ShaderSource::load_file(
            project_root / "assets/shaders/debug_triangle.vert"
        );
        require(
            source.find("gl_Position") != std::string::npos,
            "Vertex shader source should contain gl_Position assignment."
        );
    }

    {
        const ImageData image = ImageLoader::load(project_root / "tests/fixtures/test_rgba.ppm");
        require(image.width == 2, "Image width should be 2.");
        require(image.height == 1, "Image height should be 1.");
        require(image.channels >= 3, "Image should provide at least RGB channels.");
        require(image.pixels.size() >= 6, "Image pixel buffer should contain decoded bytes.");
        require(image.pixels[0] == 255, "First pixel red component should be 255.");
        require(image.pixels[1] == 0, "First pixel green component should be 0.");
        require(image.pixels[2] == 0, "First pixel blue component should be 0.");
    }

    std::cout << "resource_tests passed" << std::endl;
    return 0;
}
