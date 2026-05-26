#pragma once

#include <cstddef>
#include <vector>

namespace shader_toy
{
class TexturedQuadMesh
{
public:
    [[nodiscard]] static const std::vector<float>& vertices();
    [[nodiscard]] static constexpr std::size_t stride() noexcept
    {
        return 5;
    }
    [[nodiscard]] static constexpr std::size_t vertex_count() noexcept
    {
        return 6;
    }
};
} // namespace shader_toy
