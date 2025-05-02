//
// Created by YAHAY on 17/12/2024.
//

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H
#include <functional>
#include <vector>

#include "renderer_constants.h"
#include <volk.h>
#include <vk_mem_alloc.h>
#include <concepts>
#include <iostream>
#include "EngineBuffer.h"
#include "Renderer.h"
#include "../debug_panic.h"
static std::vector<std::function<void()>> cleanups[MAX_INFLIGHT_FRAMES];
static VkCommandPool command_pools[MAX_INFLIGHT_FRAMES];
static int currentFrame = 0;
static void default_cleanup();
static EngineBuffer bbb;
/**
 * defer for frame lifetime
 * @param fn cleanup function to run after all proccessing with current frame finishes
 */
void deferffl(std::function<void()> &&fn);
void advance_frame_and_execute_cleanups();
VkCommandBuffer make_cb_for_frame();
void init_frame_resource_manager();
static constexpr bool is_write(VkAccessFlags access) {
    constexpr VkAccessFlags write_mask =
        VK_ACCESS_MEMORY_WRITE_BIT |
        VK_ACCESS_HOST_WRITE_BIT |
        VK_ACCESS_SHADER_WRITE_BIT|
        VK_ACCESS_TRANSFER_WRITE_BIT|
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT|
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    return access&write_mask;
}
void set_dirty_range(EngineBuffer& buf,uint32_t offset, uint32_t size);
template <typename ...Buffers>
void mark_and_barrier_many(VkCommandBuffer cb,VkPipelineStageFlags dst_stage,VkAccessFlags dst_access, Buffers &...buffers) requires ((std::same_as<Buffers, EngineBuffer> && ...)) {
    VkPipelineStageFlags src_stage_mask=0;
    VkBufferMemoryBarrier barrier[sizeof...(Buffers)];
    int i=0;

    ([&src_stage_mask,&i,&barrier,dst_stage,dst_access](EngineBuffer buffers) {
        auto dirty_offset = buffers.dirty_offset;
        auto dirty_size = buffers.dirty_size;
        if (dirty_size==0)dirty_size=buffers.size;
        if (is_write(dst_access)) {
            if (is_write(buffers.last_used_access)) { // w->w
                barrier[i++]=VkBufferMemoryBarrier{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .srcAccessMask = buffers.last_used_access,
                    .dstAccessMask = dst_stage,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = buffers,
                    .offset = dirty_offset,
                    .size = dirty_size,
                };
            }
            else {// exec dependency only r->w
                src_stage_mask |= buffers.last_used_stage;
            }
        }else {
            if (is_write(buffers.last_used_access)) {//w-r
                barrier[i++]=VkBufferMemoryBarrier{
                                   .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                   .srcAccessMask = buffers.last_used_access,
                                   .dstAccessMask = dst_stage,
                                   .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                   .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                   .buffer = buffers,
                                   .offset = dirty_offset,
                                   .size = dirty_size,
                               };
            }
            else {//read-read nothing
                ;
            }
        }
        buffers.last_used_access = dst_access;
        buffers.last_used_stage = dst_stage;
        dirty_offset=0;
        dirty_size=buffers.size;
    }(buffers), ...);
    src_stage_mask=src_stage_mask==0?VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT:src_stage_mask;
    vkCmdPipelineBarrier(cb,src_stage_mask,dst_stage,0,0,nullptr,i,barrier,0, nullptr);
}

int get_frame_no();

// inline void mark_and_barrier_many(VkCommandBuffer cb,VkPipelineStageFlags dst_stage,VkAccessFlags dst_access,std::initializer_list<EngineBuffer&> items) {
//     VkPipelineStageFlags src_mask=0;
//     for (auto item : items) {
//
//         if (is_write(dst_access)) {
//             if (is_write(item.last_used_access)) {
//
//             }
//             else {// exec dependency only
//                 // src_mask |= item;
//             }
//         }
//
//     }
//     // vkCmdPipelineBarrier(cb,src_mask,dst_access,0,);
// }


#endif //RESOURCEMANAGER_H
