#ifndef MESHMANAGER_H
#define MESHMANAGER_H
#include <vector>

#include "EngineBuffer.h"

namespace renderer::mesh_manager {
    struct Mesh {
        uint32_t vertex_offset;
        uint32_t vertex_count;
        uint32_t index_offset;
        uint32_t index_count;
        uint32_t UV_offset;
        uint32_t tangent_offset;
        uint32_t normal_offset;
    };
    inline std::vector<Mesh> meshes;
    inline EngineBuffer *static_vb, *static_ib, *static_uvb,*static_tanb, *static_normalb;
    void init();

    int load_static_mesh(VkCommandBuffer cb, float *vertices, uint32_t vtx_count, uint32_t *indices, uint32_t idx_count, float *UVs, float *
                         normals, float *tangents);

}
namespace renderer {

}
#endif //MESHMANAGER_H
