#include <cmath>
#include <iostream>
#include <stdexcept>

#include <glm/glm.hpp>

#include "scene/collision.hpp"
#include "scene/particle_system.hpp"
#include "scene/rigid_animator.hpp"

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}
} // namespace

int main()
{
    {
        const shader_toy::Aabb bounds{
            glm::vec3(-2.0F, -1.0F, -2.0F),
            glm::vec3(2.0F, 2.0F, 2.0F)
        };
        const glm::vec3 clamped = shader_toy::Collision::clamp_to_bounds(
            glm::vec3(3.0F, 0.0F, -3.0F),
            bounds,
            0.2F
        );
        require(clamped.x <= 1.8F && clamped.z >= -1.8F, "Collision bounds should clamp camera position.");
    }

    {
        const shader_toy::Aabb obstacle{
            glm::vec3(-0.4F, -1.0F, -0.4F),
            glm::vec3(0.4F, 1.0F, 0.4F)
        };
        const glm::vec3 resolved = shader_toy::Collision::resolve_obstacle(
            glm::vec3(1.0F, 0.0F, 0.0F),
            glm::vec3(0.1F, 0.0F, 0.0F),
            obstacle,
            0.2F
        );
        require(resolved.x == 1.0F, "Collision obstacle resolution should restore previous position.");
    }

    {
        const glm::mat4 model = shader_toy::RigidAnimator::rotating_model(
            glm::vec3(1.0F, 0.0F, 0.0F),
            glm::vec3(0.0F, 1.0F, 0.0F),
            90.0F,
            1.0F
        );
        require(std::fabs(model[3][0] - 1.0F) < 0.001F, "Rigid animator should preserve translation.");
    }

    {
        shader_toy::ParticleEmitterConfig emitter;
        emitter.origin = glm::vec3(0.0F, -0.55F, 0.0F);
        emitter.spread = glm::vec3(0.16F, 0.0F, 0.2F);
        emitter.base_velocity = glm::vec3(0.0F, 0.6F, 0.0F);
        emitter.lifetime = 1.5F;
        shader_toy::ParticleSystem particles(8, emitter);
        particles.update(0.1F);
        const std::vector<float> packed = particles.packed_positions();
        require(packed.size() == 24, "Particle system should pack xyz positions for each particle.");
        require(packed[1] > -0.55F, "Particle system update should move particles upward.");
    }

    std::cout << "scene_logic_tests passed" << std::endl;
    return 0;
}
