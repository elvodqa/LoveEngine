#include "ResourceManager.h"

static void default_cleanup(){}
void deferffl(std::function<void()> &&fn) {
    cleanups[currentFrame%MAX_INFLIGHT_FRAMES].push_back(std::move(fn));
}
void advance_frame_and_execute_cleanups() {
    currentFrame++;
    for (const auto& f : cleanups[currentFrame%MAX_INFLIGHT_FRAMES]) {
        f();
    }
    cleanups[currentFrame%MAX_INFLIGHT_FRAMES].clear();
    deferffl(default_cleanup);
}
VkCommandBuffer make_cb_for_frame() {
    VkCommandBuffer cb;
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPools[currentFrame%MAX_INFLIGHT_FRAMES],
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    vkAllocateCommandBuffers(renderer::device,&allocInfo,&cb);
    return cb;
}
void init_frame_resource_manager() {
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0,
    };
    vkCreateFence(renderer::device,&fenceInfo,renderer::g_vk_Allocator,&renderer::thread0_load_fence);

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = renderer::g_QueueFamily,
    };
    for (auto & commandPool : commandPools)
        vkCreateCommandPool(renderer::device, &pool_info, renderer::g_vk_Allocator, &commandPool);
}

void set_dirty_range(EngineBuffer& buf, uint32_t offset, uint32_t size) {
    buf.dirty_offset=offset;
    buf.dirty_size=size;
}

int get_frame_no() {
    return  currentFrame;
}
