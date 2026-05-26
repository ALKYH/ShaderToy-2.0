#include "scene/collision.hpp"

#include <algorithm>

namespace shader_toy
{
glm::vec3 Collision::clamp_to_bounds(
    const glm::vec3& position,
    const Aabb& bounds,
    float radius
)
{
    glm::vec3 result = position;
    result.x = std::clamp(result.x, bounds.min.x + radius, bounds.max.x - radius);
    result.y = std::clamp(result.y, bounds.min.y + radius, bounds.max.y - radius);
    result.z = std::clamp(result.z, bounds.min.z + radius, bounds.max.z - radius);
    return result;
}

glm::vec3 Collision::resolve_obstacle(
    const glm::vec3& previous_position,
    const glm::vec3& candidate_position,
    const Aabb& obstacle,
    float radius
)
{
    const glm::vec3 expanded_min = obstacle.min - glm::vec3(radius);
    const glm::vec3 expanded_max = obstacle.max + glm::vec3(radius);

    const bool inside =
        candidate_position.x >= expanded_min.x && candidate_position.x <= expanded_max.x &&
        candidate_position.y >= expanded_min.y && candidate_position.y <= expanded_max.y &&
        candidate_position.z >= expanded_min.z && candidate_position.z <= expanded_max.z;

    if (!inside)
    {
        return candidate_position;
    }

    return previous_position;
}
} // namespace shader_toy
