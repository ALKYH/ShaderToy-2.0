#pragma once

#include <cstddef>
#include <vector>

namespace shader_toy
{
class TriangleMesh
{
public:
    [[nodiscard]] static const std::vector<float>& vertices();
    [[nodiscard]] static constexpr std::size_t stride() noexcept
    {
        return 3;
    }
};
} // namespace shader_toy
