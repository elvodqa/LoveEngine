//
// Created by YAHAY on 20/12/2024.
//

#include "MeshManager.h"

#include "ResourceManager.h"

void renderer::mesh_manager::init() {
    VmaAllocationCreateInfo alloc={
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };
    static_vb = EngineBuffer::init(10000,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,alloc);
    static_tanb = EngineBuffer::init(10000,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,alloc);
    static_uvb = EngineBuffer::init(10000,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,alloc);
    static_normalb = EngineBuffer::init(10000,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,alloc);
    static_ib = EngineBuffer::init(10000,VK_BUFFER_USAGE_INDEX_BUFFER_BIT,alloc);
}
int renderer::mesh_manager::load_static_mesh(VkCommandBuffer cb, float *vertices, uint32_t vtx_count,
    uint32_t *indices, uint32_t idx_count, float *UVs, float* normals, float* tangents) {
    if (!UVs || !indices || !vertices || !tangents ||!normals) panic("missing mesh channel");
    uint32_t idx_bytes = idx_count*sizeof(uint32_t);
    if (static_ib->size+idx_bytes>=static_ib->capacity) {
        static_ib->grow(cb, static_ib->size+idx_bytes);
    }
    uint32_t vtx_bytes = vtx_count*(3*sizeof(float));
    if (static_vb->size+vtx_bytes>=static_vb->capacity) {
        static_vb->grow(cb, static_vb->size+vtx_bytes);
    }
    uint32_t uv_bytes = vtx_count*(2*sizeof(float));
    if (static_uvb->size+uv_bytes>=static_uvb->capacity) {
        static_uvb->grow(cb, static_uvb->size+uv_bytes);
    }
    uint32_t tan_bytes = vtx_count*(3*sizeof(float));
    if (static_tanb->size+tan_bytes>=static_tanb->capacity) {
        static_tanb->grow(cb, static_tanb->size+tan_bytes);
    }
    uint32_t normal_bytes = vtx_count*(3*sizeof(float));
    if (static_normalb->size+normal_bytes>=static_normalb->capacity) {
        static_normalb->grow(cb, static_normalb->size+normal_bytes);
    }
    Mesh mesh{
        .vertex_offset = static_vb->size,
        .index_offset = static_uvb->size,
        .vertex_count = vtx_count,
        .index_count = idx_count,
        .UV_offset = static_uvb->size,
        .tangent_offset = static_tanb->size,
        .normal_offset = static_normalb->size,
    };
    meshes.push_back(mesh);
    uint32_t swap_needed = 0;
    mark_and_barrier_many(cb,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_ACCESS_TRANSFER_WRITE_BIT,*static_ib,*static_vb,*static_uvb, *static_tanb, *static_normalb);
    if (vtx_bytes<60000) {//handle UV with vertex
        vkCmdUpdateBuffer(cb,*static_vb,static_vb->size,vtx_bytes,vertices);
        vkCmdUpdateBuffer(cb,*static_uvb,static_uvb->size,uv_bytes,UVs);
        vkCmdUpdateBuffer(cb,*static_normalb,static_normalb->size,normal_bytes,normals);
        vkCmdUpdateBuffer(cb,*static_tanb,static_tanb->size,tan_bytes,tangents);
    }
    else {
        swap_needed += vtx_bytes+uv_bytes+tan_bytes+normal_bytes;
    }
    if (idx_bytes<60000) {
        vkCmdUpdateBuffer(cb,*static_ib,static_ib->size,idx_bytes,indices);
    }
    else {
        swap_needed += idx_bytes;
    }

    if (swap_needed>0) {
        VkBufferCreateInfo bufcreateinfo={
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = swap_needed,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &renderer::g_QueueFamily
            };

        VmaAllocationCreateInfo alloccreateinfo={
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT|VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            .requiredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            };
        VkBuffer staging_buf;
        VmaAllocation alloc;
        VmaAllocationInfo allocinfo;
        vmaCreateBuffer(renderer::vma_allocator,&bufcreateinfo,&alloccreateinfo,&staging_buf,&alloc,&allocinfo);
        deferffl([staging_buf, alloc]{vmaDestroyBuffer(renderer::vma_allocator,staging_buf,alloc);});
        uint32_t offset=0;
        auto p_mapped_data = (char*)allocinfo.pMappedData;
        if (vtx_bytes>=60000) {
            memcpy(p_mapped_data+offset,vertices,vtx_bytes);
            VkBufferCopy region={
                .srcOffset = offset,
                .dstOffset = static_vb->size,
                .size = vtx_bytes};
            offset+=vtx_bytes;

            vkCmdCopyBuffer(cb,staging_buf,*static_vb,1,&region);
        }
        if (idx_bytes>=60000) {
            memcpy(p_mapped_data+offset,indices,idx_bytes);
            VkBufferCopy region={
                .srcOffset = offset,
                .dstOffset = static_ib->size,
                .size = idx_bytes};
            offset+=idx_bytes;
            vkCmdCopyBuffer(cb,staging_buf,*static_ib,1,&region);
        }
        if (vtx_bytes>=60000) {
            memcpy(p_mapped_data+offset,UVs,uv_bytes);
            VkBufferCopy region={
                .srcOffset = offset,
                .dstOffset = static_uvb->size,
                .size = uv_bytes};
            offset+=uv_bytes;

            vkCmdCopyBuffer(cb,staging_buf,*static_uvb,1,&region);
        }
        if (vtx_bytes>=60000) {
            memcpy(p_mapped_data+offset,tangents,normal_bytes);
            VkBufferCopy region={
                .srcOffset = offset,
                .dstOffset = static_normalb->size,
                .size = normal_bytes};
            offset+=normal_bytes;

            vkCmdCopyBuffer(cb,staging_buf,*static_normalb,1,&region);
        }
        if (vtx_bytes>=60000) {
            memcpy(p_mapped_data+offset,tangents,tan_bytes);
            VkBufferCopy region={
                .srcOffset = offset,
                .dstOffset = static_tanb->size,
                .size = tan_bytes};
            offset+=tan_bytes;

            vkCmdCopyBuffer(cb,staging_buf,*static_tanb,1,&region);
        }
        if (offset!=swap_needed) panic("offset!=swap_needed");
        vmaFlushAllocation(renderer::vma_allocator,alloc,0,swap_needed);
        set_dirty_range(*static_vb, static_vb->size, vtx_bytes);
        static_vb->size+=vtx_bytes;
        set_dirty_range(*static_ib, static_ib->size, idx_bytes);
        static_ib->size+=idx_bytes;
        set_dirty_range(*static_uvb, static_uvb->size, uv_bytes);
        static_uvb->size+=uv_bytes;
        set_dirty_range(*static_normalb, static_normalb->size, normal_bytes);
        static_normalb->size+=normal_bytes;
        set_dirty_range(*static_tanb, static_tanb->size, tan_bytes);
        static_tanb->size+=tan_bytes;
    }
    return meshes.size()-1;
}
