#include <cmath>
#include <iostream>
#include <stdexcept>

#include <glm/geometric.hpp>

#include "scene/camera.hpp"

namespace
{
void require_close(float actual, float expected, float epsilon, const char* message)
{
    if (std::fabs(actual - expected) > epsilon)
    {
        throw std::runtime_error(message);
    }
}

void require_vec3_close(
    const glm::vec3& actual,
    const glm::vec3& expected,
    float epsilon,
    const char* message
)
{
    if (glm::length(actual - expected) > epsilon)
    {
        throw std::runtime_error(message);
    }
}
} // namespace

int main()
{
    using shader_toy::Camera;
    using shader_toy::CameraMovement;

    {
        Camera camera;
        camera.process_keyboard(CameraMovement::Forward, 1.0F);
        require_vec3_close(
            camera.get_position(),
            glm::vec3(0.0F, 0.0F, 0.5F),
            0.001F,
            "Forward movement should move camera along negative Z."
        );
    }

    {
        Camera camera;
        camera.process_mouse_movement(90.0F, 0.0F);
        require_close(
            camera.get_yaw(),
            -81.0F,
            0.001F,
            "Mouse X offset should update yaw using sensitivity."
        );
    }

    {
        Camera camera;
        camera.process_mouse_movement(0.0F, 1000.0F);
        require_close(
            camera.get_pitch(),
            89.0F,
            0.001F,
            "Pitch should be clamped to avoid camera flipping."
        );
    }

    std::cout << "camera_tests passed" << std::endl;
    return 0;
}
