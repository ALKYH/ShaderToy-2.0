#include "render/textured_quad_mesh.hpp"

namespace shader_toy
{
const std::vector<float>& TexturedQuadMesh::vertices()
{
    static const std::vector<float> kVertices = {
        // x, y, z, u, v
        -0.6F, -0.6F, 0.0F, 0.0F, 0.0F,
         0.6F, -0.6F, 0.0F, 1.0F, 0.0F,
         0.6F,  0.6F, 0.0F, 1.0F, 1.0F,

        -0.6F, -0.6F, 0.0F, 0.0F, 0.0F,
         0.6F,  0.6F, 0.0F, 1.0F, 1.0F,
        -0.6F,  0.6F, 0.0F, 0.0F, 1.0F,
    };

    return kVertices;
}
} // namespace shader_toy
