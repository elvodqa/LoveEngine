//
// Created by YAHAY on 20/12/2024.
//

#ifndef ENGINEBUFFER_H
#define ENGINEBUFFER_H
#include <volk.h>
#include <vk_mem_alloc.h>

struct EngineBuffer {
    public:
    VkBuffer buffer;
    VmaAllocation allocation;


    //4 GiB den büyük allocluyosak zaten geçmiş olsun

    uint32_t size,capacity;
    VmaAllocationInfo allocation_info;
    VkBufferUsageFlags usage;
    VmaAllocationCreateInfo alloc_create_info;
    VkBufferCreateInfo buffer_create_info;

    VkPipelineStageFlags last_used_stage;
    VkAccessFlags last_used_access;
    uint32_t dirty_offset,dirty_size;
    operator VkBuffer() const {return buffer;}


    static EngineBuffer *init(uint32_t size, VkBufferUsageFlags usage, const VmaAllocationCreateInfo &allocinfo);
    void grow(VkCommandBuffer cb, uint32_t new_cap);
    void grow_exact(VkCommandBuffer cb, uint32_t new_cap);
};



#endif //ENGINEBUFFER_H
