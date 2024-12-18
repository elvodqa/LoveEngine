//
// Created by YAHAY on 16/12/2024.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL_video.h>
#include "volk.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "vk_mem_alloc.h"
namespace renderer {
    inline VkAllocationCallbacks*   g_vk_Allocator = nullptr;
    inline VmaAllocator             vma_allocator = VK_NULL_HANDLE;
    inline VkInstance               vk_Instance = VK_NULL_HANDLE;
    inline VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
    inline VkDevice                 device = VK_NULL_HANDLE;
    inline uint32_t                 g_QueueFamily = (uint32_t)-1;
    inline VkQueue                  g_Queue = VK_NULL_HANDLE;
    inline VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
    inline VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
    inline VkDescriptorPool         imgui_DescriptorPool = VK_NULL_HANDLE;

    inline uint32_t                 g_MinImageCount = 2;
    inline bool                     g_SwapChainRebuild = false;

    inline SDL_Window*              window = nullptr;

    void init();
    void CleanupVulkan();
    void CleanupVulkanWindow();
}

namespace renderer::imgui {
    inline ImGui_ImplVulkanH_Window imgui_MainWindowData;
    inline ImGui_ImplVulkanH_Window* wd;

}
#endif //RENDERER_H
