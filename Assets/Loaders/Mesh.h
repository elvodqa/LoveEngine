#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <vector>

#include "Assets/Asset.h"

struct MemMesh{
    std::vector<uint32_t> indices;
    std::vector<uint16_t> mesh_i_offsets;
    std::vector<uint16_t> mesh_v_offsets;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> tangents;
    template <typename T>
    std::vector<T> load_via(VkCommandBuffer cb, T(*callback)(VkCommandBuffer cb, float *vertices, uint32_t vtx_count,
    uint32_t *indices, uint32_t idx_count, float *UVs,float* normals, float* tangents)) {
        std::vector<T> results;
        for (auto i = 0; i < mesh_i_offsets.size(); i++) {
            auto verts = (float*)(vertices.data() + mesh_v_offsets[i]);
            auto vcount = i == mesh_v_offsets.size() - 1 ? vertices.size()-mesh_v_offsets[i] : mesh_v_offsets[i + 1] - mesh_v_offsets[i];
            auto ids = indices.data() + mesh_i_offsets[i];
            auto id_count = i == mesh_i_offsets.size()-1 ? indices.size()-mesh_i_offsets[i] : mesh_i_offsets[i + 1] - mesh_i_offsets[i];
            results.push_back(callback(
                cb,
                verts,vcount,
                ids,id_count,
                (float*)(uvs.data()+mesh_v_offsets[i]),
                (float*)(normals.data()+mesh_v_offsets[i]),
                (float*)(tangents.data()+mesh_v_offsets[i]))
                );
        }
        return results;
    }

    static MemMesh *loadMeshfromAsset(love::Asset *asset);
};
