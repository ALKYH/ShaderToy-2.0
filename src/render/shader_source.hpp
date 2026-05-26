#pragma once

#include <filesystem>
#include <string>

namespace shader_toy
{
class ShaderSource
{
public:
    [[nodiscard]] static std::string load_file(const std::filesystem::path& path);
};
} // namespace shader_toy
