#include "render/image_loader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace shader_toy
{
namespace
{
ImageData load_ascii_ppm(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        throw std::runtime_error("Failed to open image file: " + path.string());
    }

    std::string magic;
    input >> magic;
    if (magic != "P3")
    {
        throw std::runtime_error("Unsupported ASCII PPM header in: " + path.string());
    }

    ImageData image;
    int max_value = 0;
    input >> image.width >> image.height >> max_value;

    if (image.width <= 0 || image.height <= 0 || max_value <= 0)
    {
        throw std::runtime_error("Invalid PPM metadata in: " + path.string());
    }

    image.channels = 3;
    image.pixels.reserve(
        static_cast<std::size_t>(image.width)
        * static_cast<std::size_t>(image.height)
        * static_cast<std::size_t>(image.channels)
    );

    for (int i = 0; i < image.width * image.height * image.channels; ++i)
    {
        int value = 0;
        input >> value;
        if (!input.good())
        {
            throw std::runtime_error("Unexpected end of PPM pixel data in: " + path.string());
        }

        image.pixels.push_back(static_cast<std::uint8_t>(value));
    }

    return image;
}
} // namespace

ImageData ImageLoader::load(const std::filesystem::path& path)
{
    if (path.extension() == ".ppm")
    {
        return load_ascii_ppm(path);
    }

    stbi_set_flip_vertically_on_load(0);

    ImageData image;
    unsigned char* raw_pixels = stbi_load(
        path.string().c_str(),
        &image.width,
        &image.height,
        &image.channels,
        0
    );

    if (raw_pixels == nullptr)
    {
        throw std::runtime_error("Failed to load image file: " + path.string());
    }

    const std::size_t pixel_count = static_cast<std::size_t>(image.width)
        * static_cast<std::size_t>(image.height)
        * static_cast<std::size_t>(image.channels);
    image.pixels.assign(raw_pixels, raw_pixels + pixel_count);

    stbi_image_free(raw_pixels);
    return image;
}
} // namespace shader_toy
