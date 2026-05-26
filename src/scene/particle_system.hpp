#pragma once

#include <cstddef>
#include <vector>

#include <glm/glm.hpp>

namespace shader_toy
{
struct Particle
{
    glm::vec3 position{0.0F};
    glm::vec3 velocity{0.0F};
    float lifetime = 0.0F;
};

struct ParticleEmitterConfig
{
    glm::vec3 origin{0.0F};
    glm::vec3 spread{0.16F, 0.0F, 0.2F};
    glm::vec3 base_velocity{0.0F, 0.6F, 0.0F};
    float lifetime = 1.5F;
};

class ParticleSystem
{
public:
    explicit ParticleSystem(std::size_t max_particles);
    ParticleSystem(std::size_t max_particles, const ParticleEmitterConfig& config);

    void update(float delta_time);

    [[nodiscard]] const std::vector<Particle>& particles() const noexcept;
    [[nodiscard]] std::vector<float> packed_positions() const;

private:
    void respawn(Particle& particle, std::size_t index);

    std::vector<Particle> particles_;
    ParticleEmitterConfig config_{};
    float time_accumulator_ = 0.0F;
};
} // namespace shader_toy
