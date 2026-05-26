#pragma once

#include <glm/glm.hpp>

namespace shader_toy
{
struct Aabb
{
    glm::vec3 min;
    glm::vec3 max;
};

class Collision
{
public:
    [[nodiscard]] static glm::vec3 clamp_to_bounds(
        const glm::vec3& position,
        const Aabb& bounds,
        float radius
    );

    [[nodiscard]] static glm::vec3 resolve_obstacle(
        const glm::vec3& previous_position,
        const glm::vec3& candidate_position,
        const Aabb& obstacle,
        float radius
    );
};
} // namespace shader_toy
