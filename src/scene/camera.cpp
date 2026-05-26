#include "scene/camera.hpp"

#include <algorithm>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace shader_toy
{
Camera::Camera()
    : position_(0.0F, 0.0F, 3.0F),
      front_(0.0F, 0.0F, -1.0F),
      up_(0.0F, 1.0F, 0.0F),
      right_(1.0F, 0.0F, 0.0F),
      world_up_(0.0F, 1.0F, 0.0F),
      yaw_(-90.0F),
      pitch_(0.0F),
      movement_speed_(2.5F),
      speed_multiplier_(1.0F),
      mouse_sensitivity_(0.1F)
{
    update_vectors();
}

void Camera::process_keyboard(CameraMovement direction, float delta_time)
{
    const float velocity = movement_speed_ * speed_multiplier_ * delta_time;

    switch (direction)
    {
    case CameraMovement::Forward:
        position_ += front_ * velocity;
        break;
    case CameraMovement::Backward:
        position_ -= front_ * velocity;
        break;
    case CameraMovement::Left:
        position_ -= right_ * velocity;
        break;
    case CameraMovement::Right:
        position_ += right_ * velocity;
        break;
    case CameraMovement::Up:
        position_ += world_up_ * velocity;
        break;
    case CameraMovement::Down:
        position_ -= world_up_ * velocity;
        break;
    }
}

void Camera::process_mouse_movement(float xoffset, float yoffset)
{
    yaw_ += xoffset * mouse_sensitivity_;
    pitch_ += yoffset * mouse_sensitivity_;
    pitch_ = std::clamp(pitch_, -89.0F, 89.0F);

    update_vectors();
}

const glm::vec3& Camera::get_position() const noexcept
{
    return position_;
}

const glm::vec3& Camera::get_front() const noexcept
{
    return front_;
}

const glm::vec3& Camera::get_up() const noexcept
{
    return up_;
}

float Camera::get_yaw() const noexcept
{
    return yaw_;
}

float Camera::get_pitch() const noexcept
{
    return pitch_;
}

glm::mat4 Camera::get_view_matrix() const
{
    return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::set_position(const glm::vec3& position) noexcept
{
    position_ = position;
}

void Camera::set_speed_multiplier(float multiplier) noexcept
{
    speed_multiplier_ = multiplier;
}

void Camera::update_vectors()
{
    glm::vec3 front;
    front.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front.y = std::sin(glm::radians(pitch_));
    front.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));

    front_ = glm::normalize(front);
    right_ = glm::normalize(glm::cross(front_, world_up_));
    up_ = glm::normalize(glm::cross(right_, front_));
}
} // namespace shader_toy
