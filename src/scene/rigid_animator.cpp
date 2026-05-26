#include "scene/rigid_animator.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace shader_toy
{
glm::mat4 RigidAnimator::rotating_model(
    const glm::vec3& position,
    const glm::vec3& axis,
    float degrees_per_second,
    float elapsed_seconds
)
{
    glm::mat4 model(1.0F);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(degrees_per_second * elapsed_seconds), axis);
    return model;
}
} // namespace shader_toy
