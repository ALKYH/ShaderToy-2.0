#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "scene/scene_config.hpp"

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
    const std::filesystem::path project_root = SHADER_TOY_PROJECT_ROOT;
    const shader_toy::SceneConfig config = shader_toy::SceneConfigLoader::load(
        project_root / "tests/fixtures/test_scene.cfg"
    );

    require(config.models.size() >= 2, "Config should support multiple model entries.");
    const shader_toy::ModelConfig* main_model = nullptr;
    for (const shader_toy::ModelConfig& model : config.models)
    {
        if (model.name == "main")
        {
            main_model = &model;
            break;
        }
    }
    require(main_model != nullptr, "Config should preserve named main model instance.");
    require(main_model->obj_path.filename() == "test_triangle.obj", "Config should resolve model obj_path.");
    require(main_model->rotation_speed == 45.0F, "Config should read named main model rotation speed.");
    require(config.room_bounds.max.x == 2.8F, "Config should read room collision bounds.");
    require(config.obstacle_bounds.size() >= 2, "Config should support multiple collision obstacles.");
    require(config.point_lights.size() >= 2, "Config should support multiple point lights.");
    require(config.point_lights[0].position.z == 1.5F, "Config should read first point light position.");
    require(config.point_lights[1].name == "fill", "Config should preserve named fill point light.");
    require(config.shadow.enabled, "Config should read shadow enabled flag.");
    require(config.particles.size() >= 2, "Config should support multiple particle emitters.");
    const shader_toy::ParticleConfig* fire_particles = nullptr;
    const shader_toy::ParticleConfig* smoke_particles = nullptr;
    for (const shader_toy::ParticleConfig& particle : config.particles)
    {
        if (particle.name == "fire") fire_particles = &particle;
        if (particle.name == "smoke") smoke_particles = &particle;
    }
    require(fire_particles != nullptr, "Config should preserve fire particle emitter.");
    require(smoke_particles != nullptr, "Config should preserve smoke particle emitter.");
    require(fire_particles->count == 24, "Config should read primary particle emitter count.");
    require(smoke_particles->point_size == 4.0F, "Config should read secondary particle emitter point size.");

    std::cout << "scene_config_tests passed" << std::endl;
    return 0;
}
