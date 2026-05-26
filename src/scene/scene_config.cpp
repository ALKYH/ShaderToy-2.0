#include "scene/scene_config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace shader_toy
{
namespace
{
std::string trim(std::string value)
{
    value.erase(
        value.begin(),
        std::find_if(value.begin(), value.end(), [](unsigned char ch) { return !std::isspace(ch); })
    );
    value.erase(
        std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
        value.end()
    );
    return value;
}

glm::vec3 parse_vec3(const std::string& text)
{
    std::istringstream stream(text);
    std::string token;
    glm::vec3 result(0.0F);

    std::getline(stream, token, ',');
    result.x = std::stof(trim(token));
    std::getline(stream, token, ',');
    result.y = std::stof(trim(token));
    std::getline(stream, token, ',');
    result.z = std::stof(trim(token));
    return result;
}

bool parse_bool(const std::string& text)
{
    const std::string lowered = trim(text);
    return lowered == "true" || lowered == "1" || lowered == "yes";
}
} // namespace

SceneConfig SceneConfigLoader::load(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        throw std::runtime_error("Failed to open scene config: " + path.string());
    }

    SceneConfig config;
    config.source_path = path;
    config.source_directory = path.parent_path();
    config.models.push_back(ModelConfig{});
    config.models.front().name = "main";
    config.models.front().obj_path = config.source_directory / "../models/room/test_triangle.obj";
    config.obstacle_bounds.push_back(
        Aabb{glm::vec3(-0.5F, -0.2F, -0.5F), glm::vec3(0.5F, 1.0F, 0.5F)}
    );
    config.point_lights.push_back(PointLightConfig{});
    config.point_lights.front().name = "main";
    config.particles.push_back(ParticleConfig{});
    config.particles.front().name = "fire";

    auto get_or_create_model = [&](const std::string& name) -> ModelConfig& {
        for (ModelConfig& model : config.models)
        {
            if (model.name == name)
            {
                return model;
            }
        }

        config.models.push_back(ModelConfig{});
        config.models.back().name = name;
        return config.models.back();
    };

    auto get_or_create_obstacle = [&](std::size_t index) -> Aabb& {
        while (config.obstacle_bounds.size() <= index)
        {
            config.obstacle_bounds.push_back(Aabb{glm::vec3(0.0F), glm::vec3(0.0F)});
        }
        return config.obstacle_bounds[index];
    };

    auto get_or_create_point_light = [&](const std::string& name) -> PointLightConfig& {
        for (PointLightConfig& point_light : config.point_lights)
        {
            if (point_light.name == name)
            {
                return point_light;
            }
        }

        config.point_lights.push_back(PointLightConfig{});
        config.point_lights.back().name = name;
        return config.point_lights.back();
    };

    auto get_or_create_particles = [&](std::size_t index) -> ParticleConfig& {
        while (config.particles.size() <= index)
        {
            config.particles.push_back(ParticleConfig{});
        }
        return config.particles[index];
    };

    std::string current_section;
    std::string line;
    while (std::getline(input, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        if (line.front() == '[' && line.back() == ']')
        {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }

        const std::size_t separator = line.find('=');
        if (separator == std::string::npos)
        {
            continue;
        }

        const std::string key = trim(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));

        if (current_section.rfind("model.", 0) == 0)
        {
            const std::string model_name = current_section.substr(std::string("model.").size());
            ModelConfig& model = get_or_create_model(model_name);
            if (key == "obj_path") model.obj_path = config.source_directory / value;
            else if (key == "position") model.position = parse_vec3(value);
            else if (key == "rotation_axis") model.rotation_axis = parse_vec3(value);
            else if (key == "rotation_speed") model.rotation_speed = std::stof(value);
        }
        else if (current_section == "ground")
        {
            if (key == "y") config.ground.y = std::stof(value);
            else if (key == "extent") config.ground.extent = std::stof(value);
        }
        else if (current_section == "collision.room")
        {
            if (key == "min") config.room_bounds.min = parse_vec3(value);
            else if (key == "max") config.room_bounds.max = parse_vec3(value);
        }
        else if (current_section.rfind("collision.obstacle.", 0) == 0)
        {
            static std::unordered_map<std::string, std::size_t> obstacle_indices;
            const std::string obstacle_name = current_section.substr(std::string("collision.obstacle.").size());
            if (!obstacle_indices.contains(obstacle_name))
            {
                obstacle_indices[obstacle_name] = obstacle_indices.size();
            }
            Aabb& obstacle = get_or_create_obstacle(obstacle_indices[obstacle_name]);
            if (key == "min") obstacle.min = parse_vec3(value);
            else if (key == "max") obstacle.max = parse_vec3(value);
        }
        else if (current_section == "lighting.directional")
        {
            if (key == "direction") config.directional_light.direction = parse_vec3(value);
            else if (key == "color") config.directional_light.color = parse_vec3(value);
        }
        else if (current_section.rfind("lighting.point.", 0) == 0)
        {
            const std::string light_name = current_section.substr(std::string("lighting.point.").size());
            PointLightConfig& point_light = get_or_create_point_light(light_name);
            if (key == "position") point_light.position = parse_vec3(value);
            else if (key == "color") point_light.color = parse_vec3(value);
            else if (key == "linear") point_light.linear = std::stof(value);
            else if (key == "quadratic") point_light.quadratic = std::stof(value);
        }
        else if (current_section == "lighting.spot")
        {
            if (key == "color") config.spot_light.color = parse_vec3(value);
            else if (key == "inner_cutoff") config.spot_light.inner_cutoff = std::stof(value);
            else if (key == "outer_cutoff") config.spot_light.outer_cutoff = std::stof(value);
        }
        else if (current_section == "shadow")
        {
            if (key == "enabled") config.shadow.enabled = parse_bool(value);
            else if (key == "bias") config.shadow.bias = std::stof(value);
        }
        else if (current_section.rfind("particles.", 0) == 0)
        {
            static std::unordered_map<std::string, std::size_t> particle_indices;
            const std::string particle_name = current_section.substr(std::string("particles.").size());
            if (!particle_indices.contains(particle_name))
            {
                particle_indices[particle_name] = particle_indices.size();
            }
            ParticleConfig& particle = get_or_create_particles(particle_indices[particle_name]);
            particle.name = particle_name;
            if (key == "count") particle.count = std::stoi(value);
            else if (key == "origin") particle.origin = parse_vec3(value);
            else if (key == "spread") particle.spread = parse_vec3(value);
            else if (key == "base_velocity") particle.base_velocity = parse_vec3(value);
            else if (key == "lifetime") particle.lifetime = std::stof(value);
            else if (key == "point_size") particle.point_size = std::stof(value);
            else if (key == "color") particle.color = parse_vec3(value);
        }
    }

    return config;
}
} // namespace shader_toy
