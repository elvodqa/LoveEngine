#include "ResourceManager.h"

void default_cleanup(){}
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
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = renderer::g_QueueFamily,
    };
    for (auto & commandPool : commandPools)
        vkCreateCommandPool(renderer::device, &pool_info, renderer::g_vk_Allocator, &commandPool);
}
