#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "scene/collision.hpp"

namespace shader_toy
{
struct ModelConfig
{
    std::string name;
    std::filesystem::path obj_path;
    glm::vec3 position{0.0F};
    glm::vec3 rotation_axis{0.0F, 1.0F, 0.0F};
    float rotation_speed = 60.0F;
};

struct GroundConfig
{
    float y = -0.8F;
    float extent = 3.0F;
};

struct DirectionalLightConfig
{
    glm::vec3 direction{-0.2F, -1.0F, -0.3F};
    glm::vec3 color{1.0F, 0.95F, 0.85F};
};

struct PointLightConfig
{
    std::string name;
    glm::vec3 position{1.2F, 0.8F, 1.5F};
    glm::vec3 color{0.3F, 0.7F, 1.0F};
    float linear = 0.14F;
    float quadratic = 0.07F;
};

struct SpotLightConfig
{
    glm::vec3 color{1.0F, 0.9F, 0.8F};
    float inner_cutoff = 12.5F;
    float outer_cutoff = 20.0F;
};

struct ShadowConfig
{
    bool enabled = true;
    float bias = 0.005F;
};

struct ParticleConfig
{
    std::string name;
    int count = 24;
    glm::vec3 origin{0.0F, -0.55F, 0.0F};
    glm::vec3 spread{0.16F, 0.0F, 0.2F};
    glm::vec3 base_velocity{0.0F, 0.6F, 0.0F};
    float lifetime = 1.5F;
    float point_size = 6.0F;
    glm::vec3 color{1.0F, 0.55F, 0.15F};
};

struct SceneConfig
{
    std::filesystem::path source_path;
    std::filesystem::path source_directory;
    std::vector<ModelConfig> models;
    GroundConfig ground;
    Aabb room_bounds{glm::vec3(-2.8F, -0.2F, -2.8F), glm::vec3(2.8F, 1.8F, 2.8F)};
    std::vector<Aabb> obstacle_bounds;
    DirectionalLightConfig directional_light;
    std::vector<PointLightConfig> point_lights;
    SpotLightConfig spot_light;
    ShadowConfig shadow;
    std::vector<ParticleConfig> particles;
};

class SceneConfigLoader
{
public:
    [[nodiscard]] static SceneConfig load(const std::filesystem::path& path);
};
} // namespace shader_toy
