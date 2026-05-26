#include "scene/particle_system.hpp"

namespace shader_toy
{
ParticleSystem::ParticleSystem(std::size_t max_particles)
    : ParticleSystem(max_particles, ParticleEmitterConfig{})
{
}

ParticleSystem::ParticleSystem(std::size_t max_particles, const ParticleEmitterConfig& config)
    : particles_(max_particles),
      config_(config)
{
    for (std::size_t i = 0; i < particles_.size(); ++i)
    {
        respawn(particles_[i], i);
        particles_[i].lifetime -= static_cast<float>(i) * 0.05F;
    }
}

void ParticleSystem::update(float delta_time)
{
    time_accumulator_ += delta_time;

    for (std::size_t i = 0; i < particles_.size(); ++i)
    {
        Particle& particle = particles_[i];
        particle.lifetime -= delta_time;
        if (particle.lifetime <= 0.0F)
        {
            respawn(particle, i);
            continue;
        }

        particle.position += particle.velocity * delta_time;
    }
}

const std::vector<Particle>& ParticleSystem::particles() const noexcept
{
    return particles_;
}

std::vector<float> ParticleSystem::packed_positions() const
{
    std::vector<float> packed;
    packed.reserve(particles_.size() * 3);

    for (const Particle& particle : particles_)
    {
        packed.push_back(particle.position.x);
        packed.push_back(particle.position.y);
        packed.push_back(particle.position.z);
    }

    return packed;
}

void ParticleSystem::respawn(Particle& particle, std::size_t index)
{
    const float spread_x = static_cast<float>(index % 5) / 4.0F - 0.5F;
    const float spread_z = static_cast<float>(index % 3) / 2.0F - 0.5F;

    particle.position = config_.origin + glm::vec3(
        spread_x * config_.spread.x * 2.0F,
        config_.spread.y,
        spread_z * config_.spread.z * 2.0F
    );
    particle.velocity = config_.base_velocity + glm::vec3(
        0.0F,
        0.05F * static_cast<float>(index % 4),
        0.0F
    );
    particle.lifetime = config_.lifetime + 0.1F * static_cast<float>(index % 5);
}
} // namespace shader_toy
