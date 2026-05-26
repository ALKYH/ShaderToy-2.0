cmake_minimum_required(VERSION 3.24)

project(
    ShaderToy2
    VERSION 0.1.0
    DESCRIPTION "OpenGL indoor scene project with GLFW, GLAD, and ImGui"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(CTest)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9b
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)

FetchContent_MakeAvailable(glfw glad imgui glm stb)

add_subdirectory("${glad_SOURCE_DIR}/cmake" "${glad_BINARY_DIR}")
glad_add_library(glad REPRODUCIBLE API gl:core=3.3)

set(IMGUI_DIR "${imgui_SOURCE_DIR}")

add_library(
    imgui_backend
    STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(
    imgui_backend
    PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui_backend PUBLIC glfw glad)

add_library(
    shader_toy_core
    STATIC
    src/scene/collision.cpp
    src/scene/camera.cpp
    src/scene/particle_system.cpp
    src/scene/rigid_animator.cpp
    src/scene/scene_config.cpp
    src/render/ground_mesh.cpp
    src/render/shader_source.cpp
    src/render/image_loader.cpp
    src/render/obj_loader.cpp
    src/render/triangle_mesh.cpp
    src/render/textured_quad_mesh.cpp
)

target_include_directories(
    shader_toy_core
    PUBLIC
    src
    "${glm_SOURCE_DIR}"
    "${stb_SOURCE_DIR}"
)
target_link_libraries(shader_toy_core PUBLIC glm)

add_executable(
    shader_toy_app
    src/main.cpp
)

target_include_directories(shader_toy_app PRIVATE src)

target_link_libraries(
    shader_toy_app
    PRIVATE
    glfw
    glad
    imgui_backend
    shader_toy_core
)

if(MINGW)
    target_link_libraries(shader_toy_app PRIVATE opengl32 imm32)
elseif(WIN32)
    target_link_libraries(shader_toy_app PRIVATE opengl32)
endif()

target_compile_definitions(
    shader_toy_app
    PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLAD
    SHADER_TOY_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

add_executable(
    camera_tests
    tests/camera_tests.cpp
)

target_include_directories(camera_tests PRIVATE src "${glm_SOURCE_DIR}")
target_link_libraries(camera_tests PRIVATE shader_toy_core glm)
target_compile_definitions(
    camera_tests
    PRIVATE
    SHADER_TOY_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME camera_tests COMMAND camera_tests)

add_executable(
    resource_tests
    tests/resource_tests.cpp
)

target_include_directories(
    resource_tests
    PRIVATE
    src
    "${glm_SOURCE_DIR}"
    "${stb_SOURCE_DIR}"
)
target_link_libraries(resource_tests PRIVATE shader_toy_core glm)
target_compile_definitions(
    resource_tests
    PRIVATE
    SHADER_TOY_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME resource_tests COMMAND resource_tests)

add_executable(
    render_data_tests
    tests/render_data_tests.cpp
)

target_include_directories(
    render_data_tests
    PRIVATE
    src
    "${glm_SOURCE_DIR}"
    "${stb_SOURCE_DIR}"
)
target_link_libraries(render_data_tests PRIVATE shader_toy_core glm)
target_compile_definitions(
    render_data_tests
    PRIVATE
    SHADER_TOY_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME render_data_tests COMMAND render_data_tests)

add_executable(
    obj_loader_tests
    tests/obj_loader_tests.cpp
)

target_include_directories(
    obj_loader_tests
    PRIVATE
    src
    "${glm_SOURCE_DIR}"
    "${stb_SOURCE_DIR}"
)
target_link_libraries(obj_loader_tests PRIVATE shader_toy_core glm)
target_compile_definitions(
    obj_loader_tests
    PRIVATE
    SHADER_TOY_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME obj_loader_tests COMMAND obj_loader_tests)

add_executable(
    scene_logic_tests
    tests/scene_logic_tests.cpp
)

target_include_directories(
    scene_logic_tests
    PRIVATE
    src
    "${glm_SOURCE_DIR}"
    "${stb_SOURCE_DIR}"
)
target_link_libraries(scene_logic_tests PRIVATE shader_toy_core glm)
target_compile_definitions(
    scene_logic_tests
    PRIVATE
    SHADER_TOY_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME scene_logic_tests COMMAND scene_logic_tests)

add_executable(
    scene_config_tests
    tests/scene_config_tests.cpp
)

target_include_directories(
    scene_config_tests
    PRIVATE
    src
    "${glm_SOURCE_DIR}"
    "${stb_SOURCE_DIR}"
)
target_link_libraries(scene_config_tests PRIVATE shader_toy_core glm)
target_compile_definitions(
    scene_config_tests
    PRIVATE
    SHADER_TOY_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

add_test(NAME scene_config_tests COMMAND scene_config_tests)
