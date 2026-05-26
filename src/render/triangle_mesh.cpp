#include "render/triangle_mesh.hpp"

namespace shader_toy
{
const std::vector<float>& TriangleMesh::vertices()
{
    static const std::vector<float> kVertices = {
        -0.5F, -0.5F, 0.0F,
         0.5F, -0.5F, 0.0F,
         0.0F,  0.5F, 0.0F,
    };

    return kVertices;
}
} // namespace shader_toy
