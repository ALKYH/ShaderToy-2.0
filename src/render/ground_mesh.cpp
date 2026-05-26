#include "render/ground_mesh.hpp"

namespace shader_toy
{
const std::vector<float>& GroundMesh::vertices()
{
    static const std::vector<float> kVertices = {
        // x, y, z, u, v, nx, ny, nz
        -3.0F, -0.8F, -3.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
         3.0F, -0.8F, -3.0F, 3.0F, 0.0F, 0.0F, 1.0F, 0.0F,
         3.0F, -0.8F,  3.0F, 3.0F, 3.0F, 0.0F, 1.0F, 0.0F,

        -3.0F, -0.8F, -3.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
         3.0F, -0.8F,  3.0F, 3.0F, 3.0F, 0.0F, 1.0F, 0.0F,
        -3.0F, -0.8F,  3.0F, 0.0F, 3.0F, 0.0F, 1.0F, 0.0F,
    };

    return kVertices;
}
} // namespace shader_toy
