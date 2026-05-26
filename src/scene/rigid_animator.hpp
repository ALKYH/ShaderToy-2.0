#pragma once

#include <glm/glm.hpp>

namespace shader_toy
{
class RigidAnimator
{
public:
    [[nodiscard]] static glm::mat4 rotating_model(
        const glm::vec3& position,
        const glm::vec3& axis,
        float degrees_per_second,
        float elapsed_seconds
    );
};
} // namespace shader_toy
