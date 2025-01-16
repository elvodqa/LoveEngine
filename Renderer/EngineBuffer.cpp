//
// Created by YAHAY on 20/12/2024.
//

#include "EngineBuffer.h"

#include "Renderer.h"
#include "ResourceManager.h"

EngineBuffer *EngineBuffer::init(uint32_t size, VkBufferUsageFlags usage, const VmaAllocationCreateInfo &allocinfo) {
    auto buffer = new EngineBuffer();
    buffer->buffer_create_info={
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &renderer::g_QueueFamily,
    };
    buffer->alloc_create_info = allocinfo;
    buffer->usage=usage;
    vmaCreateBuffer(renderer::vma_allocator,&buffer->buffer_create_info,&allocinfo,&buffer->buffer,&buffer->allocation,&buffer->allocation_info);
    return buffer;
}


void EngineBuffer::grow(VkCommandBuffer cb, uint32_t new_cap) {
    EngineBuffer::grow_exact(cb,std::max(new_cap,((uint32_t)1)<<std::bit_width(capacity)));
}

void EngineBuffer::grow_exact(VkCommandBuffer cb, uint32_t new_cap) {
    auto old_buf = buffer;
    deferffl([buffer=buffer,allocation=allocation]{vmaDestroyBuffer(renderer::vma_allocator,buffer,allocation);});
    buffer_create_info.size = new_cap;
    mark_and_barrier_many(cb,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_ACCESS_TRANSFER_READ_BIT,*this);

    vmaCreateBuffer(renderer::vma_allocator,&buffer_create_info,&alloc_create_info,&buffer,&allocation,&allocation_info);

    VkBufferCopy copy={
        .srcOffset = 0,
        .size = size,
        .dstOffset = 0,
    };
    vkCmdCopyBuffer(cb,old_buf,buffer,1,&copy);
    last_used_access=VK_ACCESS_TRANSFER_WRITE_BIT;
    last_used_stage=VK_PIPELINE_STAGE_TRANSFER_BIT;
    capacity=new_cap;
}
