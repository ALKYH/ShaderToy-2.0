#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "render/ground_mesh.hpp"
#include "render/image_loader.hpp"
#include "render/obj_loader.hpp"
#include "render/shader_source.hpp"
#include "scene/collision.hpp"
#include "scene/particle_system.hpp"
#include "scene/rigid_animator.hpp"
#include "scene/camera.hpp"
#include "scene/scene_config.hpp"

namespace
{
constexpr int kWindowWidth = 1600;
constexpr int kWindowHeight = 900;
constexpr int kShadowMapSize = 1024;
constexpr const char* kWindowTitle = "ShaderToy 2.0";
constexpr const char* kGlslVersion = "#version 330";

shader_toy::Camera g_camera;
shader_toy::SceneConfig g_scene_config;
bool g_first_mouse = true;
bool g_mouse_captured = true;
float g_last_x = static_cast<float>(kWindowWidth) * 0.5F;
float g_last_y = static_cast<float>(kWindowHeight) * 0.5F;
std::string g_shader_status = "Not loaded";
std::string g_texture_status = "Not loaded";
std::string g_render_status = "Renderer not initialized";

struct LightingState
{
    float dir_light_direction[3] = {-0.2F, -1.0F, -0.3F};
    float dir_light_color[3] = {1.0F, 0.95F, 0.85F};

    float spot_light_color[3] = {1.0F, 0.9F, 0.8F};
    float spot_inner_cutoff = 12.5F;
    float spot_outer_cutoff = 20.0F;

    bool shadows_enabled = true;
    float shadow_bias = 0.005F;
};

LightingState g_lighting{};
shader_toy::ParticleSystem g_particles(24);
std::vector<shader_toy::ParticleSystem> g_particle_emitters;

struct MeshDraw
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei vertex_count = 0;
};

struct TextureResource
{
    GLuint id = 0;
};

struct MainRenderer
{
    GLuint lit_program = 0;
    GLuint depth_program = 0;
    GLuint particle_program = 0;
    GLuint shadow_fbo = 0;
    GLuint shadow_map = 0;
    MeshDraw ground_mesh;
    TextureResource ground_texture;
    std::vector<MeshDraw> model_meshes;
    std::vector<TextureResource> model_textures;
    std::vector<MeshDraw> particle_meshes;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;

    if (!g_mouse_captured)
    {
        return;
    }

    const float x = static_cast<float>(xpos);
    const float y = static_cast<float>(ypos);

    if (g_first_mouse)
    {
        g_last_x = x;
        g_last_y = y;
        g_first_mouse = false;
    }

    const float xoffset = x - g_last_x;
    const float yoffset = g_last_y - y;

    g_last_x = x;
    g_last_y = y;

    g_camera.process_mouse_movement(xoffset, yoffset);
}

void process_input(GLFWwindow* window, float delta_time)
{
    const glm::vec3 previous_position = g_camera.get_position();
    static bool tab_was_pressed = false;

    const bool tab_pressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
    if (tab_pressed && !tab_was_pressed)
    {
        g_mouse_captured = !g_mouse_captured;
        glfwSetInputMode(
            window,
            GLFW_CURSOR,
            g_mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
        );
        g_first_mouse = true;
    }
    tab_was_pressed = tab_pressed;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    g_camera.set_speed_multiplier(
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 2.5F : 1.0F
    );

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_camera.process_keyboard(shader_toy::CameraMovement::Forward, delta_time);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_camera.process_keyboard(shader_toy::CameraMovement::Backward, delta_time);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_camera.process_keyboard(shader_toy::CameraMovement::Left, delta_time);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_camera.process_keyboard(shader_toy::CameraMovement::Right, delta_time);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        g_camera.process_keyboard(shader_toy::CameraMovement::Up, delta_time);
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        g_camera.process_keyboard(shader_toy::CameraMovement::Down, delta_time);
    }

    glm::vec3 resolved_position = shader_toy::Collision::clamp_to_bounds(
        g_camera.get_position(),
        g_scene_config.room_bounds,
        0.2F
    );
    for (const shader_toy::Aabb& obstacle : g_scene_config.obstacle_bounds)
    {
        resolved_position = shader_toy::Collision::resolve_obstacle(
            previous_position,
            resolved_position,
            obstacle,
            0.2F
        );
    }
    g_camera.set_position(resolved_position);
}

GLFWwindow* create_window()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        kWindowWidth,
        kWindowHeight,
        kWindowTitle,
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        throw std::runtime_error("Failed to create GLFW window.");
    }

    return window;
}

void initialize_imgui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        throw std::runtime_error("Failed to initialize ImGui GLFW backend.");
    }

    if (!ImGui_ImplOpenGL3_Init(kGlslVersion))
    {
        throw std::runtime_error("Failed to initialize ImGui OpenGL backend.");
    }
}

void shutdown_imgui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

GLuint compile_shader(GLenum shader_type, const std::string& source)
{
    const GLuint shader = glCreateShader(shader_type);
    const char* source_ptr = source.c_str();
    glShaderSource(shader, 1, &source_ptr, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE)
    {
        return shader;
    }

    GLint log_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    std::string log(static_cast<std::size_t>(log_length), '\0');
    glGetShaderInfoLog(shader, log_length, nullptr, log.data());
    glDeleteShader(shader);
    throw std::runtime_error("Shader compilation failed: " + log);
}

GLuint create_program(
    const std::filesystem::path& vertex_path,
    const std::filesystem::path& fragment_path
)
{
    const std::string vertex_source = shader_toy::ShaderSource::load_file(vertex_path);
    const std::string fragment_source = shader_toy::ShaderSource::load_file(fragment_path);

    const GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    const GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    GLint link_success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);
    if (link_success == GL_TRUE)
    {
        return program;
    }

    GLint log_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    std::string log(static_cast<std::size_t>(log_length), '\0');
    glGetProgramInfoLog(program, log_length, nullptr, log.data());
    glDeleteProgram(program);
    throw std::runtime_error("Shader link failed: " + log);
}

MeshDraw create_mesh(const std::vector<float>& vertices, std::size_t stride)
{
    MeshDraw mesh;
    mesh.vertex_count = static_cast<GLsizei>(vertices.size() / stride);

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        reinterpret_cast<void*>(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        8 * sizeof(float),
        reinterpret_cast<void*>(5 * sizeof(float))
    );
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return mesh;
}

MeshDraw create_particle_mesh(std::size_t particle_count)
{
    MeshDraw mesh;
    mesh.vertex_count = static_cast<GLsizei>(particle_count);
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    return mesh;
}

TextureResource create_texture_from_image(const shader_toy::ImageData& image)
{
    TextureResource texture;
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = GL_RGB;
    if (image.channels == 4)
    {
        format = GL_RGBA;
    }

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        static_cast<GLint>(format),
        image.width,
        image.height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        image.pixels.data()
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

GLuint create_shadow_map(GLuint& shadow_fbo)
{
    GLuint depth_texture = 0;
    glGenFramebuffers(1, &shadow_fbo);
    glGenTextures(1, &depth_texture);

    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        kShadowMapSize,
        kShadowMapSize,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float border_color[] = {1.0F, 1.0F, 1.0F, 1.0F};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Shadow framebuffer is incomplete.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return depth_texture;
}

MainRenderer create_renderer()
{
    const std::filesystem::path project_root = SHADER_TOY_PROJECT_ROOT;

    MainRenderer renderer;
    renderer.lit_program = create_program(
        project_root / "assets/shaders/lit_shadowed_model.vert",
        project_root / "assets/shaders/lit_shadowed_model.frag"
    );
    renderer.depth_program = create_program(
        project_root / "assets/shaders/shadow_depth.vert",
        project_root / "assets/shaders/shadow_depth.frag"
    );
    renderer.particle_program = create_program(
        project_root / "assets/shaders/particle.vert",
        project_root / "assets/shaders/particle.frag"
    );

    renderer.ground_mesh = create_mesh(
        shader_toy::GroundMesh::vertices(),
        shader_toy::GroundMesh::stride()
    );
    renderer.model_meshes.reserve(g_scene_config.models.size());
    renderer.model_textures.reserve(g_scene_config.models.size());
    for (const shader_toy::ModelConfig& model_config : g_scene_config.models)
    {
        const shader_toy::ObjMeshData obj = shader_toy::ObjLoader::load(model_config.obj_path);
        const std::filesystem::path obj_texture_path =
            obj.material.diffuse_map_path.empty()
                ? (project_root / "tests/fixtures/test_rgba.ppm")
                : obj.material.diffuse_map_path;
        const shader_toy::ImageData image = shader_toy::ImageLoader::load(obj_texture_path);

        renderer.model_meshes.push_back(create_mesh(obj.vertices, obj.stride));
        renderer.model_textures.push_back(create_texture_from_image(image));
    }
    const shader_toy::ImageData ground_image = shader_toy::ImageLoader::load(
        project_root / "tests/fixtures/test_rgba.ppm"
    );
    renderer.ground_texture = create_texture_from_image(ground_image);
    renderer.shadow_map = create_shadow_map(renderer.shadow_fbo);
    renderer.particle_meshes.reserve(g_scene_config.particles.size());
    for (const shader_toy::ParticleConfig& particle_config : g_scene_config.particles)
    {
        renderer.particle_meshes.push_back(
            create_particle_mesh(static_cast<std::size_t>(particle_config.count))
        );
    }

    glUseProgram(renderer.lit_program);
    glUniform1i(glGetUniformLocation(renderer.lit_program, "uTexture"), 0);
    glUniform1i(glGetUniformLocation(renderer.lit_program, "uShadowMap"), 1);
    glUseProgram(0);

    g_render_status = "Shadowed lighting renderer ready";
    return renderer;
}

void destroy_mesh(MeshDraw& mesh)
{
    if (mesh.vbo != 0)
    {
        glDeleteBuffers(1, &mesh.vbo);
        mesh.vbo = 0;
    }
    if (mesh.vao != 0)
    {
        glDeleteVertexArrays(1, &mesh.vao);
        mesh.vao = 0;
    }
}

void destroy_texture(TextureResource& texture)
{
    if (texture.id != 0)
    {
        glDeleteTextures(1, &texture.id);
        texture.id = 0;
    }
}

void destroy_renderer(MainRenderer& renderer)
{
    destroy_mesh(renderer.ground_mesh);
    destroy_texture(renderer.ground_texture);
    for (MeshDraw& mesh : renderer.model_meshes)
    {
        destroy_mesh(mesh);
    }
    for (TextureResource& texture : renderer.model_textures)
    {
        destroy_texture(texture);
    }
    for (MeshDraw& mesh : renderer.particle_meshes)
    {
        destroy_mesh(mesh);
    }

    if (renderer.shadow_map != 0)
    {
        glDeleteTextures(1, &renderer.shadow_map);
        renderer.shadow_map = 0;
    }
    if (renderer.shadow_fbo != 0)
    {
        glDeleteFramebuffers(1, &renderer.shadow_fbo);
        renderer.shadow_fbo = 0;
    }
    if (renderer.lit_program != 0)
    {
        glDeleteProgram(renderer.lit_program);
        renderer.lit_program = 0;
    }
    if (renderer.depth_program != 0)
    {
        glDeleteProgram(renderer.depth_program);
        renderer.depth_program = 0;
    }
    if (renderer.particle_program != 0)
    {
        glDeleteProgram(renderer.particle_program);
        renderer.particle_program = 0;
    }
}

glm::mat4 compute_light_space_matrix()
{
    const glm::vec3 light_dir = glm::normalize(glm::vec3(
        g_lighting.dir_light_direction[0],
        g_lighting.dir_light_direction[1],
        g_lighting.dir_light_direction[2]
    ));
    const glm::vec3 light_pos = -light_dir * 4.0F;
    const glm::mat4 light_projection = glm::ortho(-4.0F, 4.0F, -4.0F, 4.0F, 0.1F, 10.0F);
    const glm::mat4 light_view = glm::lookAt(light_pos, glm::vec3(0.0F), glm::vec3(0.0F, 1.0F, 0.0F));
    return light_projection * light_view;
}

void upload_lighting_uniforms(const MainRenderer& renderer, int width, int height)
{
    glUseProgram(renderer.lit_program);

    const glm::mat4 view = g_camera.get_view_matrix();
    const glm::mat4 projection = glm::perspective(
        glm::radians(45.0F),
        static_cast<float>(width) / static_cast<float>(height),
        0.1F,
        100.0F
    );
    const glm::mat4 light_space = compute_light_space_matrix();

    glUniformMatrix4fv(
        glGetUniformLocation(renderer.lit_program, "uView"),
        1,
        GL_FALSE,
        &view[0][0]
    );
    glUniformMatrix4fv(
        glGetUniformLocation(renderer.lit_program, "uProjection"),
        1,
        GL_FALSE,
        &projection[0][0]
    );
    glUniformMatrix4fv(
        glGetUniformLocation(renderer.lit_program, "uLightSpaceMatrix"),
        1,
        GL_FALSE,
        &light_space[0][0]
    );

    const glm::vec3& camera_position = g_camera.get_position();
    const glm::vec3& camera_front = g_camera.get_front();
    glUniform3f(
        glGetUniformLocation(renderer.lit_program, "uViewPos"),
        camera_position.x,
        camera_position.y,
        camera_position.z
    );

    glUniform3f(
        glGetUniformLocation(renderer.lit_program, "uDirLightDirection"),
        g_lighting.dir_light_direction[0],
        g_lighting.dir_light_direction[1],
        g_lighting.dir_light_direction[2]
    );
    glUniform3f(
        glGetUniformLocation(renderer.lit_program, "uDirLightColor"),
        g_lighting.dir_light_color[0],
        g_lighting.dir_light_color[1],
        g_lighting.dir_light_color[2]
    );
    constexpr int kMaxPointLights = 4;
    const int point_light_count = static_cast<int>(
        std::min<std::size_t>(g_scene_config.point_lights.size(), static_cast<std::size_t>(kMaxPointLights))
    );
    glUniform1i(glGetUniformLocation(renderer.lit_program, "uPointLightCount"), point_light_count);
    for (int i = 0; i < point_light_count; ++i)
    {
        const shader_toy::PointLightConfig& point_light = g_scene_config.point_lights[static_cast<std::size_t>(i)];
        const std::string index = std::to_string(i);
        glUniform3f(
            glGetUniformLocation(renderer.lit_program, ("uPointLightPosition[" + index + "]").c_str()),
            point_light.position.x,
            point_light.position.y,
            point_light.position.z
        );
        glUniform3f(
            glGetUniformLocation(renderer.lit_program, ("uPointLightColor[" + index + "]").c_str()),
            point_light.color.x,
            point_light.color.y,
            point_light.color.z
        );
        glUniform1f(
            glGetUniformLocation(renderer.lit_program, ("uPointLightLinear[" + index + "]").c_str()),
            point_light.linear
        );
        glUniform1f(
            glGetUniformLocation(renderer.lit_program, ("uPointLightQuadratic[" + index + "]").c_str()),
            point_light.quadratic
        );
    }
    glUniform3f(
        glGetUniformLocation(renderer.lit_program, "uSpotLightPosition"),
        camera_position.x,
        camera_position.y,
        camera_position.z
    );
    glUniform3f(
        glGetUniformLocation(renderer.lit_program, "uSpotLightDirection"),
        camera_front.x,
        camera_front.y,
        camera_front.z
    );
    glUniform3f(
        glGetUniformLocation(renderer.lit_program, "uSpotLightColor"),
        g_lighting.spot_light_color[0],
        g_lighting.spot_light_color[1],
        g_lighting.spot_light_color[2]
    );
    glUniform1f(
        glGetUniformLocation(renderer.lit_program, "uSpotLightInnerCutoff"),
        std::cos(glm::radians(g_lighting.spot_inner_cutoff))
    );
    glUniform1f(
        glGetUniformLocation(renderer.lit_program, "uSpotLightOuterCutoff"),
        std::cos(glm::radians(g_lighting.spot_outer_cutoff))
    );
    glUniform1i(
        glGetUniformLocation(renderer.lit_program, "uShadowsEnabled"),
        g_lighting.shadows_enabled ? 1 : 0
    );
    glUniform1f(
        glGetUniformLocation(renderer.lit_program, "uShadowBias"),
        g_lighting.shadow_bias
    );

    glUseProgram(0);
}

void draw_mesh_with_program(GLuint program, const MeshDraw& mesh, const glm::mat4& model)
{
    glUniformMatrix4fv(
        glGetUniformLocation(program, "uModel"),
        1,
        GL_FALSE,
        &model[0][0]
    );
    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);
    glBindVertexArray(0);
}

void render_shadow_pass(const MainRenderer& renderer)
{
    const glm::mat4 light_space = compute_light_space_matrix();

    glViewport(0, 0, kShadowMapSize, kShadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer.shadow_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(renderer.depth_program);
    glUniformMatrix4fv(
        glGetUniformLocation(renderer.depth_program, "uLightSpaceMatrix"),
        1,
        GL_FALSE,
        &light_space[0][0]
    );

    const float time_seconds = static_cast<float>(glfwGetTime());
    for (std::size_t i = 0; i < g_scene_config.models.size(); ++i)
    {
        const shader_toy::ModelConfig& model = g_scene_config.models[i];
        const glm::mat4 animated_model = shader_toy::RigidAnimator::rotating_model(
            model.position,
            model.rotation_axis,
            model.rotation_speed,
            time_seconds
        );
        draw_mesh_with_program(renderer.depth_program, renderer.model_meshes[i], animated_model);
    }
    draw_mesh_with_program(renderer.depth_program, renderer.ground_mesh, glm::mat4(1.0F));

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_lit_scene(const MainRenderer& renderer)
{
    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float time_seconds = static_cast<float>(glfwGetTime());

    glUseProgram(renderer.lit_program);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderer.shadow_map);

    for (std::size_t i = 0; i < g_scene_config.models.size(); ++i)
    {
        const shader_toy::ModelConfig& model = g_scene_config.models[i];
        const glm::mat4 animated_model = shader_toy::RigidAnimator::rotating_model(
            model.position,
            model.rotation_axis,
            model.rotation_speed,
            time_seconds
        );
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer.model_textures[i].id);
        draw_mesh_with_program(renderer.lit_program, renderer.model_meshes[i], animated_model);
    }

    glBindTexture(GL_TEXTURE_2D, renderer.ground_texture.id);
    draw_mesh_with_program(renderer.lit_program, renderer.ground_mesh, glm::mat4(1.0F));

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    const glm::mat4 particle_view = g_camera.get_view_matrix();
    const glm::mat4 particle_projection = glm::perspective(
        glm::radians(45.0F),
        static_cast<float>(kWindowWidth) / static_cast<float>(kWindowHeight),
        0.1F,
        100.0F
    );
    glUseProgram(renderer.particle_program);
    glUniformMatrix4fv(
        glGetUniformLocation(renderer.particle_program, "uView"),
        1,
        GL_FALSE,
        &particle_view[0][0]
    );
    glUniformMatrix4fv(
        glGetUniformLocation(renderer.particle_program, "uProjection"),
        1,
        GL_FALSE,
        &particle_projection[0][0]
    );
    glUniform3f(
        glGetUniformLocation(renderer.particle_program, "uParticleColor"),
        1.0F,
        1.0F,
        1.0F
    );
    for (std::size_t i = 0; i < g_scene_config.particles.size(); ++i)
    {
        const shader_toy::ParticleConfig& emitter = g_scene_config.particles[i];
        std::vector<float> particle_positions = g_particle_emitters[i].packed_positions();
        glBindVertexArray(renderer.particle_meshes[i].vao);
        glBindBuffer(GL_ARRAY_BUFFER, renderer.particle_meshes[i].vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(particle_positions.size() * sizeof(float)),
            particle_positions.data(),
            GL_DYNAMIC_DRAW
        );
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glUniform3f(
            glGetUniformLocation(renderer.particle_program, "uParticleColor"),
            emitter.color.x,
            emitter.color.y,
            emitter.color.z
        );
        glPointSize(emitter.point_size);
        glDrawArrays(GL_POINTS, 0, renderer.particle_meshes[i].vertex_count);
    }
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void preload_resource_status()
{
    const std::filesystem::path project_root = SHADER_TOY_PROJECT_ROOT;

    try
    {
        const std::string vertex_source = shader_toy::ShaderSource::load_file(
            project_root / "assets/shaders/lit_shadowed_model.vert"
        );
        g_shader_status = "Loaded lit shader (" + std::to_string(vertex_source.size()) + " chars)";
    }
    catch (const std::exception& exception)
    {
        g_shader_status = exception.what();
    }

    try
    {
        const shader_toy::ObjMeshData obj = shader_toy::ObjLoader::load(g_scene_config.models.front().obj_path);
        const std::filesystem::path obj_texture_path =
            obj.material.diffuse_map_path.empty()
                ? (project_root / "tests/fixtures/test_rgba.ppm")
                : obj.material.diffuse_map_path;
        const shader_toy::ImageData image = shader_toy::ImageLoader::load(obj_texture_path);
        g_texture_status = "Loaded primary material (" + std::to_string(image.width) + "x"
            + std::to_string(image.height) + ", channels=" + std::to_string(image.channels)
            + ") from " + obj_texture_path.filename().string();
    }
    catch (const std::exception& exception)
    {
        g_texture_status = exception.what();
    }
}
} // namespace

int main()
{
    if (glfwInit() != GLFW_TRUE)
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        const std::filesystem::path project_root = SHADER_TOY_PROJECT_ROOT;
        g_scene_config = shader_toy::SceneConfigLoader::load(
            project_root / "assets/config/default_scene.cfg"
        );
        g_lighting.dir_light_direction[0] = g_scene_config.directional_light.direction.x;
        g_lighting.dir_light_direction[1] = g_scene_config.directional_light.direction.y;
        g_lighting.dir_light_direction[2] = g_scene_config.directional_light.direction.z;
        g_lighting.dir_light_color[0] = g_scene_config.directional_light.color.x;
        g_lighting.dir_light_color[1] = g_scene_config.directional_light.color.y;
        g_lighting.dir_light_color[2] = g_scene_config.directional_light.color.z;
        g_lighting.spot_light_color[0] = g_scene_config.spot_light.color.x;
        g_lighting.spot_light_color[1] = g_scene_config.spot_light.color.y;
        g_lighting.spot_light_color[2] = g_scene_config.spot_light.color.z;
        g_lighting.spot_inner_cutoff = g_scene_config.spot_light.inner_cutoff;
        g_lighting.spot_outer_cutoff = g_scene_config.spot_light.outer_cutoff;
        g_lighting.shadows_enabled = g_scene_config.shadow.enabled;
        g_lighting.shadow_bias = g_scene_config.shadow.bias;
        g_particle_emitters.clear();
        g_particle_emitters.reserve(g_scene_config.particles.size());
        for (const shader_toy::ParticleConfig& particle_config : g_scene_config.particles)
        {
            shader_toy::ParticleEmitterConfig emitter;
            emitter.origin = particle_config.origin;
            emitter.spread = particle_config.spread;
            emitter.base_velocity = particle_config.base_velocity;
            emitter.lifetime = particle_config.lifetime;
            g_particle_emitters.emplace_back(
                static_cast<std::size_t>(particle_config.count),
                emitter
            );
        }

        GLFWwindow* window = create_window();
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSwapInterval(1);

        const int glad_status = gladLoadGL(glfwGetProcAddress);
        if (glad_status == 0)
        {
            throw std::runtime_error("Failed to initialize GLAD.");
        }

        glViewport(0, 0, kWindowWidth, kWindowHeight);
        glEnable(GL_DEPTH_TEST);

        initialize_imgui(window);
        preload_resource_status();
        MainRenderer renderer = create_renderer();

        float clear_color[4] = {0.08F, 0.09F, 0.12F, 1.0F};
        float last_frame = 0.0F;

        while (!glfwWindowShouldClose(window))
        {
            const float current_frame = static_cast<float>(glfwGetTime());
            const float delta_time = current_frame - last_frame;
            last_frame = current_frame;

            process_input(window, delta_time);
            for (shader_toy::ParticleSystem& emitter : g_particle_emitters)
            {
                emitter.update(delta_time);
            }
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Renderer Bootstrap");
            ImGui::Text("Runtime Control Panel");
            ImGui::TextWrapped("Tab toggles mouse capture. Unlock mouse to operate the left UI.");
            ImGui::Text("Mouse Capture: %s", g_mouse_captured ? "Locked" : "Unlocked");
            ImGui::ColorEdit3("Clear Color", clear_color);
            if (ImGui::BeginTabBar("MainTabs"))
            {
                if (ImGui::BeginTabItem("Scene"))
                {
                    ImGui::TextWrapped("Config: %s", g_scene_config.source_path.filename().string().c_str());
                    ImGui::TextWrapped("Shader Source: %s", g_shader_status.c_str());
                    ImGui::TextWrapped("Texture Decode: %s", g_texture_status.c_str());
                    ImGui::TextWrapped("Render Status: %s", g_render_status.c_str());
                    ImGui::Text("Rigid animation: rotating model");
                    ImGui::Text("Collision: room bounds + center obstacle");
                    ImGui::Text("Configured Models: %d", static_cast<int>(g_scene_config.models.size()));
                    ImGui::Text("Configured Obstacles: %d", static_cast<int>(g_scene_config.obstacle_bounds.size()));
                    ImGui::Text("Configured Point Lights: %d", static_cast<int>(g_scene_config.point_lights.size()));
                    ImGui::Text("Particle Emitters: %d", static_cast<int>(g_scene_config.particles.size()));
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Lighting"))
                {
                    ImGui::Text("Directional Light");
                    ImGui::SliderFloat3("Dir Direction", g_lighting.dir_light_direction, -1.0F, 1.0F);
                    ImGui::ColorEdit3("Dir Color", g_lighting.dir_light_color);
                    ImGui::Separator();
                    ImGui::Text("Point Lights: %d", static_cast<int>(g_scene_config.point_lights.size()));
                    for (std::size_t i = 0; i < g_scene_config.point_lights.size(); ++i)
                    {
                        shader_toy::PointLightConfig& point_light = g_scene_config.point_lights[i];
                        ImGui::Separator();
                        ImGui::Text("Point: %s", point_light.name.c_str());
                        ImGui::SliderFloat3(
                            ("Position##point" + std::to_string(i)).c_str(),
                            &point_light.position.x,
                            -3.0F,
                            3.0F
                        );
                        ImGui::ColorEdit3(
                            ("Color##point" + std::to_string(i)).c_str(),
                            &point_light.color.x
                        );
                        ImGui::SliderFloat(
                            ("Linear##point" + std::to_string(i)).c_str(),
                            &point_light.linear,
                            0.01F,
                            0.5F
                        );
                        ImGui::SliderFloat(
                            ("Quadratic##point" + std::to_string(i)).c_str(),
                            &point_light.quadratic,
                            0.01F,
                            0.5F
                        );
                    }
                    ImGui::Separator();
                    ImGui::Text("Spot Light");
                    ImGui::ColorEdit3("Spot Color", g_lighting.spot_light_color);
                    ImGui::SliderFloat("Spot Inner", &g_lighting.spot_inner_cutoff, 1.0F, 30.0F);
                    ImGui::SliderFloat("Spot Outer", &g_lighting.spot_outer_cutoff, 1.0F, 45.0F);
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Effects"))
                {
                    ImGui::Checkbox("Shadows Enabled", &g_lighting.shadows_enabled);
                    ImGui::SliderFloat("Shadow Bias", &g_lighting.shadow_bias, 0.0005F, 0.02F);
                    ImGui::Separator();
                    int total_particles = 0;
                    for (const shader_toy::ParticleConfig& emitter : g_scene_config.particles)
                    {
                        total_particles += emitter.count;
                    }
                    ImGui::Text("Total Particles: %d", total_particles);
                    for (std::size_t i = 0; i < g_scene_config.particles.size(); ++i)
                    {
                        const shader_toy::ParticleConfig& emitter = g_scene_config.particles[i];
                        ImGui::Separator();
                        ImGui::Text("Emitter: %s", emitter.name.c_str());
                        ImGui::Text("Count / Size: %d / %.2f", emitter.count, emitter.point_size);
                        ImGui::Text("Color: %.2f %.2f %.2f", emitter.color.x, emitter.color.y, emitter.color.z);
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Debug"))
                {
                    const glm::vec3& position = g_camera.get_position();
                    const glm::vec3& front = g_camera.get_front();
                    ImGui::Text("Camera Position: %.2f %.2f %.2f", position.x, position.y, position.z);
                    ImGui::Text("Camera Front: %.2f %.2f %.2f", front.x, front.y, front.z);
                    ImGui::Text("Yaw / Pitch: %.2f / %.2f", g_camera.get_yaw(), g_camera.get_pitch());
                    ImGui::Separator();
                    ImGui::Text("Controls");
                    ImGui::BulletText("WASD: horizontal move");
                    ImGui::BulletText("Q / E: vertical move");
                    ImGui::BulletText("Left Shift: accelerate");
                    ImGui::BulletText("Tab: toggle mouse capture");
                    ImGui::BulletText("ESC: close");
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
            ImGui::End();

            ImGui::Render();

            glClearColor(
                clear_color[0],
                clear_color[1],
                clear_color[2],
                clear_color[3]
            );

            upload_lighting_uniforms(renderer, kWindowWidth, kWindowHeight);
            render_shadow_pass(renderer);
            render_lit_scene(renderer);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }

        destroy_renderer(renderer);
        shutdown_imgui();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
