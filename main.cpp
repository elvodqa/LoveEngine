

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"

#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include "entt/entt.hpp"



struct State {
    SDL_Window* window;
    bool running;

    VkInstance instance;
};


static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    SDL_Log("[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}


int main() {
#if not NDEBUG // cmake -DNDEBUG or define somewhere else idc
    SDL_SetLogPriorities(SDL_LogPriority::SDL_LOG_PRIORITY_DEBUG);
#endif
    int v = SDL_GetVersion();
    SDL_Log("SDL: %d.%d.%d", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v));
    auto platform = SDL_GetPlatform();
    SDL_Log("Platform: %s", platform);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Error initializing SDL3: %s", SDL_GetError());
        return -1;
    }


    if (!SDL_Vulkan_LoadLibrary(nullptr)) { // /usr/local/lib/libvulkan.dylib
        SDL_Log("Error loading SDL3 vulkan: %s", SDL_GetError());
        return -1;
    }

    State state = {0};

    state.window = SDL_CreateWindow("LoveVK", 1280, 720, SDL_WINDOW_VULKAN);
    if (state.window == nullptr) {
        SDL_Log("Error creating window: %s", SDL_GetError());
        return -1;
    }

    const std::vector<const char*> validationLayers = {
            ///has bug
            //"VK_LAYER_LUNARG_standard_validation"
    };

    Uint32 extensionCount;
    auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "aaa";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = validationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    instanceCreateInfo.enabledExtensionCount = extensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = extensions;

    for (int i = 0; i < extensionCount; i++) {
        SDL_Log("Loaded extension: %s", extensions[i]);
    }

    auto instanceResult = vkCreateInstance(&instanceCreateInfo, nullptr, &state.instance);
    if (instanceResult != VK_SUCCESS) {
        check_vk_result(instanceResult);
        return -1;
    }

    state.running = true;
    while (state.running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    state.running = false;
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    if (e.window.windowID == SDL_GetWindowID(state.window)) {
                        state.running = false;
                    }
                    break;
            }
        }

        // blah


    }

    SDL_DestroyWindow(state.window);
    SDL_Quit();
    return 0;
}
