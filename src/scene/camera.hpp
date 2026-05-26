#pragma once

#include <glm/glm.hpp>

namespace shader_toy
{
enum class CameraMovement
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera
{
public:
    Camera();

    void process_keyboard(CameraMovement direction, float delta_time);
    void process_mouse_movement(float xoffset, float yoffset);

    [[nodiscard]] const glm::vec3& get_position() const noexcept;
    [[nodiscard]] const glm::vec3& get_front() const noexcept;
    [[nodiscard]] const glm::vec3& get_up() const noexcept;
    [[nodiscard]] float get_yaw() const noexcept;
    [[nodiscard]] float get_pitch() const noexcept;
    [[nodiscard]] glm::mat4 get_view_matrix() const;
    void set_position(const glm::vec3& position) noexcept;
    void set_speed_multiplier(float multiplier) noexcept;

private:
    void update_vectors();

    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 world_up_;
    float yaw_;
    float pitch_;
    float movement_speed_;
    float speed_multiplier_;
    float mouse_sensitivity_;
};
} // namespace shader_toy
