#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace shader_toy
{
struct ImageData
{
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<std::uint8_t> pixels;
};

class ImageLoader
{
public:
    [[nodiscard]] static ImageData load(const std::filesystem::path& path);
};
} // namespace shader_toy
