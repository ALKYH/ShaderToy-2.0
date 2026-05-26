#include "render/shader_source.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace shader_toy
{
std::string ShaderSource::load_file(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        throw std::runtime_error("Failed to open shader source file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}
} // namespace shader_toy
